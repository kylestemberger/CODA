/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

// The CoreGraphicsMetalLayerRenderer requires macOS 10.14 and iOS 12.
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability", "-Wunguarded-availability-new")

namespace juce
{

//==============================================================================
class CoreGraphicsMetalLayerRenderer
{
public:
    //==============================================================================
    static auto create()
    {
        ObjCObjectHandle<id<MTLDevice>> device { MTLCreateSystemDefaultDevice() };
        return rawToUniquePtr (device != nullptr ? new CoreGraphicsMetalLayerRenderer (device)
                                                 : nullptr);
    }

    ~CoreGraphicsMetalLayerRenderer()
    {
        if (memoryBlitCommandBuffer != nullptr)
        {
            stopGpuCommandSubmission = true;
            [memoryBlitCommandBuffer.get() waitUntilCompleted];
        }
    }

    /*  Returns any regions that weren't redrawn, and which should be retried next frame. */
    template <typename Callback>
    [[nodiscard]] RectangleList<float> drawRectangleList (CAMetalLayer* layer,
                                                          float scaleFactor,
                                                          Callback&& drawRectWithContext,
                                                          RectangleList<float> dirtyRegions,
                                                          const bool renderSync)
    {
        layer.presentsWithTransaction = renderSync;

        if (memoryBlitCommandBuffer != nullptr)
        {
            switch ([memoryBlitCommandBuffer.get() status])
            {
                case MTLCommandBufferStatusNotEnqueued:
                case MTLCommandBufferStatusEnqueued:
                case MTLCommandBufferStatusCommitted:
                case MTLCommandBufferStatusScheduled:
                    // If we haven't finished blitting the CPU texture to the GPU then
                    // report that we have been unable to draw anything.
                    return dirtyRegions;
                case MTLCommandBufferStatusCompleted:
                case MTLCommandBufferStatusError:
                    break;
            }
        }

        layer.contentsScale = scaleFactor;

        const auto drawableSizeTransform = CGAffineTransformMakeScale (layer.contentsScale, layer.contentsScale);
        const auto transformedFrameSize = CGSizeApplyAffineTransform (layer.bounds.size, drawableSizeTransform);

        if (CGSizeEqualToSize (transformedFrameSize, CGSizeZero))
            return dirtyRegions;

        if (resources == nullptr || ! CGSizeEqualToSize (layer.drawableSize, transformedFrameSize))
        {
            layer.drawableSize = transformedFrameSize;
            resources = std::make_unique<Resources> (device.get(), layer);
            dirtyRegions.clear();
            dirtyRegions.add (convertToRectFloat (layer.bounds));
        }

        auto gpuTexture = resources->getGpuTexture();

        if (gpuTexture == nullptr)
        {
            jassertfalse;
            return dirtyRegions;
        }

        auto cgContext = resources->getCGContext();

        for (auto rect : dirtyRegions)
        {
            const auto cgRect = convertToCGRect (rect);

            CGContextSaveGState (cgContext);

            CGContextClipToRect (cgContext, cgRect);
            drawRectWithContext (cgContext, cgRect);

            CGContextRestoreGState (cgContext);
        }

        resources->signalBufferModifiedByCpu();

        auto sharedTexture = resources->getSharedTexture();

        auto encodeBlit = [] (id<MTLCommandBuffer> commandBuffer,
                              id<MTLTexture> source,
                              id<MTLTexture> destination)
        {
            auto blitCommandEncoder = [commandBuffer blitCommandEncoder];
            [blitCommandEncoder copyFromTexture: source
                                    sourceSlice: 0
                                    sourceLevel: 0
                                   sourceOrigin: MTLOrigin{}
                                     sourceSize: MTLSize { source.width, source.height, 1 }
                                      toTexture: destination
                               destinationSlice: 0
                               destinationLevel: 0
                              destinationOrigin: MTLOrigin{}];
            [blitCommandEncoder endEncoding];
        };

        if (renderSync)
        {
            @autoreleasepool
            {
                id<MTLCommandBuffer> commandBuffer = [commandQueue.get() commandBuffer];

                id<CAMetalDrawable> drawable = [layer nextDrawable];
                encodeBlit (commandBuffer, sharedTexture, drawable.texture);

                [commandBuffer commit];
                [commandBuffer waitUntilScheduled];
                [drawable present];
            }
        }
        else
        {
            // Command buffers are usually considered temporary, and are automatically released by
            // the operating system when the rendering pipeline is finished. However, we want to keep
            // this one alive so that we can wait for pipeline completion in the destructor.
            memoryBlitCommandBuffer.reset ([[commandQueue.get() commandBuffer] retain]);

            encodeBlit (memoryBlitCommandBuffer.get(), sharedTexture, gpuTexture);

            [memoryBlitCommandBuffer.get() addScheduledHandler: ^(id<MTLCommandBuffer>)
            {
                // We're on a Metal thread, so we can make a blocking nextDrawable call
                // without stalling the message thread.

                // Check if we can do an early exit.
                if (stopGpuCommandSubmission)
                    return;

                @autoreleasepool
                {
                    id<CAMetalDrawable> drawable = [layer nextDrawable];

                    id<MTLCommandBuffer> presentationCommandBuffer = [commandQueue.get() commandBuffer];

                    encodeBlit (presentationCommandBuffer, gpuTexture, drawable.texture);

                    [presentationCommandBuffer addScheduledHandler: ^(id<MTLCommandBuffer>)
                    {
                        [drawable present];
                    }];

                    [presentationCommandBuffer commit];
                }
            }];

            [memoryBlitCommandBuffer.get() commit];
        }

        dirtyRegions.clear();
        return dirtyRegions;
    }

private:
    //==============================================================================
    explicit CoreGraphicsMetalLayerRenderer (ObjCObjectHandle<id<MTLDevice>> mtlDevice)
        : device (mtlDevice),
          commandQueue ([device.get() newCommandQueue])
    {
    }

    //==============================================================================
    static auto alignTo (size_t n, size_t alignment)
    {
        return ((n + alignment - 1) / alignment) * alignment;
    }

    //==============================================================================
    class GpuTexturePool
    {
    public:
        GpuTexturePool (id<MTLDevice> metalDevice, MTLTextureDescriptor* descriptor)
        {
            for (auto& t : textureCache)
                t.reset ((descriptor.width != 0 && descriptor.height != 0) ? [metalDevice newTextureWithDescriptor: descriptor]
                                                                           : nullptr);
        }

        id<MTLTexture> take() const
        {
            auto iter = std::find_if (textureCache.begin(), textureCache.end(),
                                      [] (const ObjCObjectHandle<id<MTLTexture>>& t) { return [t.get() retainCount] == 1; });
            return iter == textureCache.end() ? nullptr : (*iter).get();
        }

    private:
        std::array<ObjCObjectHandle<id<MTLTexture>>, 3> textureCache;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GpuTexturePool)
        JUCE_DECLARE_NON_MOVEABLE (GpuTexturePool)
    };

    //==============================================================================
    class Resources
    {
    public:
        Resources (id<MTLDevice> metalDevice, CAMetalLayer* layer)
        {
            const auto bytesPerRow = alignTo ((size_t) layer.drawableSize.width * 4, 256);

            const auto allocationSize = cpuRenderMemory.ensureSize (bytesPerRow * (size_t) layer.drawableSize.height);

            buffer.reset ([metalDevice newBufferWithBytesNoCopy: cpuRenderMemory.get()
                                                         length: allocationSize
                                                        options:
                                                                #if JUCE_MAC
                                                                 MTLResourceStorageModeManaged
                                                                #else
                                                                 MTLResourceStorageModeShared
                                                                #endif
                                                    deallocator: nullptr]);

            auto* textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: layer.pixelFormat
                                                                                   width: (NSUInteger) layer.drawableSize.width
                                                                                  height: (NSUInteger) layer.drawableSize.height
                                                                               mipmapped: NO];
            textureDesc.storageMode =
                                     #if JUCE_MAC
                                      MTLStorageModeManaged;
                                     #else
                                      MTLStorageModeShared;
                                     #endif
            textureDesc.usage = MTLTextureUsageShaderRead;

            sharedTexture.reset ([buffer.get() newTextureWithDescriptor: textureDesc
                                                                 offset: 0
                                                            bytesPerRow: bytesPerRow]);

            cgContext.reset (CGBitmapContextCreate (cpuRenderMemory.get(),
                                                    (size_t) layer.drawableSize.width,
                                                    (size_t) layer.drawableSize.height,
                                                    8, // Bits per component
                                                    bytesPerRow,
                                                    CGColorSpaceCreateWithName (kCGColorSpaceSRGB),
                                                    (uint32_t) kCGImageAlphaPremultipliedFirst | (uint32_t) kCGBitmapByteOrder32Host));

            CGContextTranslateCTM (cgContext.get(), 0, layer.drawableSize.height);
            CGContextScaleCTM (cgContext.get(), layer.contentsScale, -layer.contentsScale);

            textureDesc.storageMode = MTLStorageModePrivate;
            gpuTexturePool = std::make_unique<GpuTexturePool> (metalDevice, textureDesc);
        }

        CGContextRef getCGContext() const noexcept       { return cgContext.get(); }
        id<MTLTexture> getSharedTexture() const noexcept { return sharedTexture.get(); }
        id<MTLTexture> getGpuTexture() noexcept          { return gpuTexturePool == nullptr ? nullptr : gpuTexturePool->take(); }

        void signalBufferModifiedByCpu()
        {
           #if JUCE_MAC
            [buffer.get() didModifyRange: { 0, buffer.get().length }];
           #endif
        }

    private:
        class AlignedMemory
        {
        public:
            AlignedMemory() = default;

            void* get()
            {
                return allocation != nullptr ? allocation->data : nullptr;
            }

            size_t ensureSize (size_t newSize)
            {
                const auto alignedSize = alignTo (newSize, pagesize);

                if (alignedSize > size)
                {
                    size = std::max (alignedSize, alignTo ((size_t) ((float) size * growthFactor), pagesize));
                    allocation = std::make_unique<AllocationWrapper> (pagesize, size);
                }

                return size;
            }

        private:
            static constexpr float growthFactor = 1.3f;

            const size_t pagesize = (size_t) getpagesize();

            struct AllocationWrapper
            {
                AllocationWrapper (size_t alignment, size_t allocationSize)
                {
                    if (posix_memalign (&data, alignment, allocationSize) != 0)
                        jassertfalse;
                }

                ~AllocationWrapper()
                {
                    ::free (data);
                }

                void* data = nullptr;
            };

            std::unique_ptr<AllocationWrapper> allocation;
            size_t size = 0;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlignedMemory)
            JUCE_DECLARE_NON_MOVEABLE (AlignedMemory)
        };

        AlignedMemory cpuRenderMemory;

        detail::ContextPtr cgContext;

        ObjCObjectHandle<id<MTLBuffer>> buffer;
        ObjCObjectHandle<id<MTLTexture>> sharedTexture;
        std::unique_ptr<GpuTexturePool> gpuTexturePool;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Resources)
        JUCE_DECLARE_NON_MOVEABLE (Resources)
    };

    //==============================================================================
    std::unique_ptr<Resources> resources;

    ObjCObjectHandle<id<MTLDevice>> device;
    ObjCObjectHandle<id<MTLCommandQueue>> commandQueue;
    ObjCObjectHandle<id<MTLCommandBuffer>> memoryBlitCommandBuffer;

    std::atomic<bool> stopGpuCommandSubmission { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsMetalLayerRenderer)
    JUCE_DECLARE_NON_MOVEABLE (CoreGraphicsMetalLayerRenderer)
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace juce

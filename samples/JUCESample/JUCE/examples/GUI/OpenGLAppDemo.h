/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             OpenGLAppDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple OpenGL application.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra, juce_opengl
 exporters:        xcode_mac, vs2022, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        OpenGLAppDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/WavefrontObjParser.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class OpenGLAppDemo final : public OpenGLAppComponent
{
public:
    //==============================================================================
    OpenGLAppDemo()
    {
        setSize (800, 600);
    }

    ~OpenGLAppDemo() override
    {
        shutdownOpenGL();
    }

    void initialise() override
    {
        createShaders();
    }

    void shutdown() override
    {
        shader    .reset();
        shape     .reset();
        attributes.reset();
        uniforms  .reset();
    }

    Matrix3D<float> getProjectionMatrix() const
    {
        const ScopedLock lock (mutex);

        auto w = 1.0f / (0.5f + 0.1f);
        auto h = w * bounds.toFloat().getAspectRatio (false);

        return Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
    }

    Matrix3D<float> getViewMatrix() const
    {
        auto viewMatrix = Matrix3D<float>::fromTranslation ({ 0.0f, 0.0f, -10.0f });
        auto rotationMatrix = viewMatrix.rotation ({ -0.3f, 5.0f * std::sin ((float) getFrameCounter() * 0.01f), 0.0f });

        return viewMatrix * rotationMatrix;
    }

    void render() override
    {
        using namespace ::juce::gl;

        jassert (OpenGLHelpers::isContextActive());

        auto desktopScale = (float) openGLContext.getRenderingScale();
        OpenGLHelpers::clear (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        {
            const ScopedLock lock (mutex);
            glViewport (0, 0,
                        roundToInt (desktopScale * (float) bounds.getWidth()),
                        roundToInt (desktopScale * (float) bounds.getHeight()));
        }

        shader->use();

        if (uniforms->projectionMatrix != nullptr)
            uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);

        if (uniforms->viewMatrix != nullptr)
            uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);

        shape->draw (*attributes);

        // Reset the element buffers so child Components draw correctly
        glBindBuffer (GL_ARRAY_BUFFER, 0);
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void paint (Graphics& g) override
    {
        // You can add your component specific drawing code here!
        // This will draw over the top of the openGL background.

        g.setColour (getLookAndFeel().findColour (Label::textColourId));
        g.setFont (20);
        g.drawText ("OpenGL Example", 25, 20, 300, 30, Justification::left);
        g.drawLine (20, 20, 170, 20);
        g.drawLine (20, 50, 170, 50);
    }

    void resized() override
    {
        // This is called when this component is resized.
        // If you add any child components, this is where you should
        // update their positions.

        const ScopedLock lock (mutex);
        bounds = getLocalBounds();
    }

    void createShaders()
    {
        vertexShader =
            "attribute vec4 position;\n"
            "attribute vec4 sourceColour;\n"
            "attribute vec2 textureCoordIn;\n"
            "\n"
            "uniform mat4 projectionMatrix;\n"
            "uniform mat4 viewMatrix;\n"
            "\n"
            "varying vec4 destinationColour;\n"
            "varying vec2 textureCoordOut;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    destinationColour = sourceColour;\n"
            "    textureCoordOut = textureCoordIn;\n"
            "    gl_Position = projectionMatrix * viewMatrix * position;\n"
            "}\n";

        fragmentShader =
           #if JUCE_OPENGL_ES
            "varying lowp vec4 destinationColour;\n"
            "varying lowp vec2 textureCoordOut;\n"
           #else
            "varying vec4 destinationColour;\n"
            "varying vec2 textureCoordOut;\n"
           #endif
            "\n"
            "void main()\n"
            "{\n"
           #if JUCE_OPENGL_ES
            "    lowp vec4 colour = vec4(0.95, 0.57, 0.03, 0.7);\n"
           #else
            "    vec4 colour = vec4(0.95, 0.57, 0.03, 0.7);\n"
           #endif
            "    gl_FragColor = colour;\n"
            "}\n";

        std::unique_ptr<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));
        String statusText;

        if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
              && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
              && newShader->link())
        {
            shape     .reset();
            attributes.reset();
            uniforms  .reset();

            shader = std::move (newShader);
            shader->use();

            shape     .reset (new Shape());
            attributes.reset (new Attributes (*shader));
            uniforms  .reset (new Uniforms (*shader));

            statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
        }
        else
        {
            statusText = newShader->getLastError();
        }
    }


private:
    //==============================================================================
    struct Vertex
    {
        float position[3];
        float normal[3];
        float colour[4];
        float texCoord[2];
    };

    //==============================================================================
    // This class just manages the attributes that the shaders use.
    struct Attributes
    {
        explicit Attributes (OpenGLShaderProgram& shaderProgram)
        {
            position      .reset (createAttribute (shaderProgram, "position"));
            normal        .reset (createAttribute (shaderProgram, "normal"));
            sourceColour  .reset (createAttribute (shaderProgram, "sourceColour"));
            textureCoordIn.reset (createAttribute (shaderProgram, "textureCoordIn"));
        }

        void enable()
        {
            using namespace ::juce::gl;

            if (position.get() != nullptr)
            {
                glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), nullptr);
                glEnableVertexAttribArray (position->attributeID);
            }

            if (normal.get() != nullptr)
            {
                glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
                glEnableVertexAttribArray (normal->attributeID);
            }

            if (sourceColour.get() != nullptr)
            {
                glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 6));
                glEnableVertexAttribArray (sourceColour->attributeID);
            }

            if (textureCoordIn.get() != nullptr)
            {
                glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
                glEnableVertexAttribArray (textureCoordIn->attributeID);
            }
        }

        void disable()
        {
            using namespace ::juce::gl;

            if (position != nullptr)       glDisableVertexAttribArray (position->attributeID);
            if (normal != nullptr)         glDisableVertexAttribArray (normal->attributeID);
            if (sourceColour != nullptr)   glDisableVertexAttribArray (sourceColour->attributeID);
            if (textureCoordIn != nullptr) glDisableVertexAttribArray (textureCoordIn->attributeID);
        }

        std::unique_ptr<OpenGLShaderProgram::Attribute> position, normal, sourceColour, textureCoordIn;

    private:
        static OpenGLShaderProgram::Attribute* createAttribute (OpenGLShaderProgram& shader,
                                                                const char* attributeName)
        {
            using namespace ::juce::gl;

            if (glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
                return nullptr;

            return new OpenGLShaderProgram::Attribute (shader, attributeName);
        }
    };

    //==============================================================================
    // This class just manages the uniform values that the demo shaders use.
    struct Uniforms
    {
        explicit Uniforms (OpenGLShaderProgram& shaderProgram)
        {
            projectionMatrix.reset (createUniform (shaderProgram, "projectionMatrix"));
            viewMatrix      .reset (createUniform (shaderProgram, "viewMatrix"));
        }

        std::unique_ptr<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix;

    private:
        static OpenGLShaderProgram::Uniform* createUniform (OpenGLShaderProgram& shaderProgram,
                                                            const char* uniformName)
        {
            using namespace ::juce::gl;

            if (glGetUniformLocation (shaderProgram.getProgramID(), uniformName) < 0)
                return nullptr;

            return new OpenGLShaderProgram::Uniform (shaderProgram, uniformName);
        }
    };

    //==============================================================================
    /** This loads a 3D model from an OBJ file and converts it into some vertex buffers
        that we can draw.
    */
    struct Shape
    {
        Shape()
        {
            if (shapeFile.load (loadEntireAssetIntoString ("teapot.obj")).wasOk())
                for (auto* shapeVertices : shapeFile.shapes)
                    vertexBuffers.add (new VertexBuffer (*shapeVertices));
        }

        void draw (Attributes& glAttributes)
        {
            using namespace ::juce::gl;

            for (auto* vertexBuffer : vertexBuffers)
            {
                vertexBuffer->bind();

                glAttributes.enable();
                glDrawElements (GL_TRIANGLES, vertexBuffer->numIndices, GL_UNSIGNED_INT, nullptr);
                glAttributes.disable();
            }
        }

    private:
        struct VertexBuffer
        {
            explicit VertexBuffer (WavefrontObjFile::Shape& aShape)
            {
                using namespace ::juce::gl;

                numIndices = aShape.mesh.indices.size();

                glGenBuffers (1, &vertexBuffer);
                glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

                Array<Vertex> vertices;
                createVertexListFromMesh (aShape.mesh, vertices, Colours::green);

                glBufferData (GL_ARRAY_BUFFER,
                              static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof (Vertex)),
                              vertices.getRawDataPointer(), GL_STATIC_DRAW);

                glGenBuffers (1, &indexBuffer);
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                              static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof (juce::uint32)),
                              aShape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
            }

            ~VertexBuffer()
            {
                using namespace ::juce::gl;

                glDeleteBuffers (1, &vertexBuffer);
                glDeleteBuffers (1, &indexBuffer);
            }

            void bind()
            {
                using namespace ::juce::gl;

                glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            }

            GLuint vertexBuffer, indexBuffer;
            int numIndices;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
        };

        WavefrontObjFile shapeFile;
        OwnedArray<VertexBuffer> vertexBuffers;

        static void createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, Array<Vertex>& list, Colour colour)
        {
            auto scale = 0.2f;
            WavefrontObjFile::TextureCoord defaultTexCoord { 0.5f, 0.5f };
            WavefrontObjFile::Vertex defaultNormal { 0.5f, 0.5f, 0.5f };

            for (auto i = 0; i < mesh.vertices.size(); ++i)
            {
                const auto& v = mesh.vertices.getReference (i);
                const auto& n = i < mesh.normals.size() ? mesh.normals.getReference (i) : defaultNormal;
                const auto& tc = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i) : defaultTexCoord;

                list.add ({ { scale * v.x, scale * v.y, scale * v.z, },
                            { scale * n.x, scale * n.y, scale * n.z, },
                            { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                            { tc.x, tc.y } });
            }
        }
    };

    const char* vertexShader;
    const char* fragmentShader;

    std::unique_ptr<OpenGLShaderProgram> shader;
    std::unique_ptr<Shape> shape;
    std::unique_ptr<Attributes> attributes;
    std::unique_ptr<Uniforms> uniforms;

    String newVertexShader, newFragmentShader;

    Rectangle<int> bounds;
    CriticalSection mutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLAppDemo)
};

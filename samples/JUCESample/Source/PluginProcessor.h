

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <JuceHeader.h>
#include <LicenseSpring/Configuration.h>
#include <LicenseSpring/LicenseManager.h>
#include "../../AppConfig.h"

#include "utils.h"

class CodaProcessor : public juce::AudioProcessor {
   public:
    CodaProcessor();
    ~CodaProcessor() override;
    
   

    void prepareToPlay(double sampleRate,
                       int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlockBypassed(juce::AudioBuffer<float>& buffer,
                              juce::MidiBuffer& midiMessages) override;
    using AudioProcessor::processBlockBypassed;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    LicenseSpring::LicenseManager::ptr_t licenseManager;
    
    enum
    {
        fftOrder  = 11,             // [1]
        fftSize   = 1 << fftOrder,  // [2]
        scopeSize = 512             // [3]
    };
    
    struct FilterStage {
        /**
         * The type of the IIR coefficient array used, because these
         * `CoefficientPtr`s are completely opaque.
         */
        using Coefficients = std::array<float, 6>;

        std::vector<juce::dsp::IIR::Filter<float>> channels;
        /**
         * The all-pass coefficients for this stage's filters. As explained
         * below, all filters will actually use the first stage's filter's
         * coefficients when the spread has been turned down as an optimization.
         * When adding filter stages in `update_and_swap_filters()`, this should
         * be initialized with some arbitrary `Coefficients` so it can then be
         * reinitialized with the correct coefficients in `processBlock()`.
         */
        juce::dsp::IIR::Filter<float>::CoefficientsPtr coefficients = nullptr;
    };

    /**
     * This contains an arbitrary number of filter stages, which each contains
     * some filter coefficients as well as an IIR filter for each channel.
     */
    struct Filters {
        /**
         * This should be set to `false` when changing the number of filter
         * stages. Then we can initialize the filters during the first
         * processing cycle.
         */
        bool is_initialized = false;

        std::vector<FilterStage> stages;
    };

    /**
     * Reinitialize `filters` with `filter_stages` filters on the next audio
     * processing cycle. The inactive object we're modifying will be swapped
     * with the active object on the next call to `filters.get()`. This should
     * not be called from the audio thread.
     */
    void update_and_swap_filters();

    /**
     * The current processing spec. Needed when adding more filters when the
     * number of stages changes.
     */
    juce::dsp::ProcessSpec current_spec_;

    /**
     * Our all-pass filters. This is essentially a vector of filters indexed by
     * `[filter_idx][channel_idx]` along coefficients per filter. The number of
     * filters and the frequency of the filters is controlled using the
     * `filter_stages` and `filter_frequency` parameters. Filter coefficients
     * are stored along with the filter, but if `filter_spread` is disabled then
     * all filters will use the first filter's coefficients for better cache
     * locality.
     */
    AtomicallySwappable<Filters> filters_;

    juce::AudioProcessorValueTreeState parameters_;

    /**
     * How many samples we should process before updating and smoothing the
     * parameters again. We do this only once every `smoothing_interval` samples
     * because recomputing all of these filter coefficients per-sample becomes
     * pretty expensive.
     *
     * To keep things simple, this value can be negative.
     */
    int next_smooth_in_ = 0;

    juce::AudioParameterInt& filter_stages_;
    juce::AudioParameterInt& filter_frequency_;
    juce::SmoothedValue<float> smoothed_filter_frequency_;
    juce::AudioParameterFloat& filter_resonance_;
    juce::SmoothedValue<float> smoothed_filter_resonance_;
    /**
     * A cutoff spread between the filter stages. When this value is `0`, the
     * same coefficients are used for every filter. Otherwise, the used
     * frequencies are distributed within the range `[-(filter_spread / 2),
     * (filter_spread / 2)]`.
     */
    std::atomic<float>& filter_spread_;
    juce::SmoothedValue<float> smoothed_filter_spread_;
    /**
     * The spread can either be logarithmic or linear. The logarithmic version
     * usually sounds more natural, but surely more options is better, right?
     */
    juce::AudioParameterBool& filter_spread_linear_;
    bool old_filter_spread_linear_;

    /**
     * The interval in samples between parameter smoothing cycles. Recomputing
     * `filter_stages` IIR coefficients every sample while smoothing gets a bit
     * expensive.
     */
    juce::AudioParameterInt& smoothing_interval_;

    /**
     * Will add or remove filters when the number of filter stages changes.
     */
    LambdaAsyncUpdater filter_stages_updater_;
    LambdaParameterListener filter_stages_listener_;
    

    juce::dsp::FFT forwardFFT;                      // [4]
    juce::dsp::WindowingFunction<float> window;     // [5]
 
    float fifo [fftSize];                           // [6]
    float fftData [2 * fftSize];                    // [7]
    int fifoIndex = 0;                              // [8]
    bool nextFFTBlockReady = false;                 // [9]
    float scopeData [scopeSize];                    // [10]
    
    void pushNextSampleIntoFifo (float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next frame should now be rendered..
        if (fifoIndex == fftSize)               // [11]
        {
            if (! nextFFTBlockReady)            // [12]
            {
                juce::zeromem (fftData, sizeof (fftData));
                memcpy (fftData, fifo, sizeof (fifo));
                nextFFTBlockReady = true;
            }
 
            fifoIndex = 0;
        }
 
        fifo[fifoIndex++] = sample;             // [12]
    }
    
    void drawNextFrameOfSpectrum();
    
private:
    AppConfig appConfig { "CODA", "1.0" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CodaProcessor)
};


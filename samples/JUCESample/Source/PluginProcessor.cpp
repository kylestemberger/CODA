

#include "PluginProcessor.h"

#include "PluginEditor.h"


using juce::uint32;

constexpr char filter_settings_group_name[] = "filters";
constexpr char filter_stages_param_name[] = "filter_stages";
constexpr char filter_frequency_param_name[] = "filter_freq";
constexpr char filter_resonance_param_name[] = "filter_res";
constexpr char filter_spread_param_name[] = "filter_spread";
constexpr char filter_spread_linear_param_name[] = "filter_spread_linear";
constexpr char smoothing_interval_param_name[] = "smoothing_interval";

/**
 * When the filter cutoff or resonance parameters change, we'll interpolate
 * between the old and the new values over the course of this time span to
 * prevent clicks.
 */
constexpr float filter_smoothing_secs = 0.1f;

/**
 * The default filter resonance. This value should minimize the amount of
 * resonances. In the GUI we should also be snapping to this value.
 *
 * The actual default neutral Q-value would be `sqrt(2) / 2`, but this value
 * produces slightly less ringing.
 */
constexpr float default_filter_resonance = 0.5f;



CodaProcessor::CodaProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
parameters_(
            *this,
            nullptr,
            "parameters",
            {
                
                
                
                
                std::make_unique<juce::AudioProcessorParameterGroup>(
                                                                     filter_settings_group_name,
                                                                     "Filters",
                                                                     " | ",
                                                                     // TODO: Some combinations of parameters can cause really loud
                                                                     //       resonance. We should limit the resonance and filter
                                                                     //       stages parameter ranges in the GUI until the user
                                                                     //       unlocks.
                                                                     std::make_unique<juce::AudioParameterInt>(
                                                                                                               filter_stages_param_name,
                                                                                                               "Amount",
                                                                                                               0,
                                                                                                               40,
                                                                                                               12),
                                                                     // For some reason Disperser's frequency is a bit off, but
                                                                     // ours is actually correct with respect to 440 Hz = A tuning.
                                                                     // TODO: Figure out some way to get rid of the resonances when
                                                                     //       sweep the frequency down when using a large number of
                                                                     //       stages
                                                                     std::make_unique<juce::AudioParameterInt>(
                                                                                                               filter_frequency_param_name,
                                                                                                               "Center",
                                                                                                               20,
                                                                                                               20000,
                                                                                                               130),
                                                                     // TODO DISPLAY THIS RANGE MORE CLEARLY
                                                                     
                                                                     std::make_unique<juce::AudioParameterFloat>(
                                                                                                                 filter_resonance_param_name,
                                                                                                                 "Focus",
                                                                                                                 juce::NormalisableRange<float>(0.5f, 10.0f, 0.01f, 0.2f),
                                                                                                                 default_filter_resonance),
                                                                     
                                                                     
                                                                     std::make_unique<juce::AudioParameterFloat>(
                                                                                                                 filter_spread_param_name,
                                                                                                                 "Spread Factor",
                                                                                                                 juce::NormalisableRange<
                                                                                                                 float>(-5000.0f, 5000.0f, 1.0f, 0.3f, true),
                                                                                                                 0.0f,
                                                                                                                 " Hz",
                                                                                                                 juce::AudioProcessorParameter::genericParameter,
                                                                                                                 [](float value, int /*max_length*/) -> juce::String {
                                                                                                                     return juce::String(value, 0);
                                                                                                                 }),
                                                                     
                                                                     
                                                                     std::make_unique<juce::AudioParameterBool>(
                                                                                                                filter_spread_linear_param_name,
                                                                                                                "Linear Spread",
                                                                                                                true,
                                                                                                                "",
                                                                                                                [](float value, int /*max_length*/) -> juce::String {
                                                                                                                    return (value >= 0.5) ? "linear" : "logarithmic";
                                                                                                                },
                                                                                                                [](const juce::String& text) -> bool {
                                                                                                                    const auto& lower_case = text.toLowerCase();
                                                                                                                    return lower_case == "linear" || lower_case == "true";
                                                                                                                })),
                
                
                std::make_unique<juce::AudioParameterInt>(
                                                          smoothing_interval_param_name,
                                                          "Smoothing Interval",
                                                          1,
                                                          512,
                                                          128,
                                                          "%",
                                                          [](int value, int /*max_length*/) -> juce::String {
                                                              // Since the exact values don't matter, Coda will display this
                                                              // as a percentage with 100% being the most precision (1
                                                              // sample), and 0% being the least precise. This does mean
                                                              // that the most precise value will be on the left of the
                                                              // range. This all looks off
                                                              // because our lowest integer value is 1.
                                                              return juce::String(
                                                                                  std::round((512 - value) / 511.0f * 100.0f));
                                                          },
                                                          [](const juce::String& text) -> int {
                                                              const float percentage = text.getFloatValue();
                                                              return std::round(512 - ((percentage / 100.0f) * 511.0f));
                                                          }),
                std::make_unique<juce::AudioParameterBool>(
                                                           "please_ignore",
                                                           "Don't touch this",
                                                           true,
                                                           "",
                                                           [](float value, int /*max_length*/) -> juce::String {
                                                               return (value > 0.0) ? "please don't" : "stop it";
                                                           }),
            }),
filter_stages_(*dynamic_cast<juce::AudioParameterInt*>(
                                                       parameters_.getParameter(filter_stages_param_name))),
filter_frequency_(*dynamic_cast<juce::AudioParameterInt*>(
                                                          parameters_.getParameter(filter_frequency_param_name))),

// Easier way to retrieve non-float parameters? (too much memory used)
//
filter_resonance_(*dynamic_cast<juce::AudioParameterFloat*>(
                                                            parameters_.getParameter(filter_resonance_param_name))),
filter_spread_(
               *parameters_.getRawParameterValue(filter_spread_param_name)),
filter_spread_linear_(*dynamic_cast<juce::AudioParameterBool*>(
                                                               parameters_.getParameter(filter_spread_linear_param_name))),
smoothing_interval_(*dynamic_cast<juce::AudioParameterInt*>(
                                                            parameters_.getParameter(smoothing_interval_param_name))),
filter_stages_updater_([&]()
                       {
    //update_and_swap_filters();
    smoothed_filter_spread_.setTargetValue(filter_stages_);
}),
filter_stages_listener_(
                        [&](const juce::String& /*parameter_id*/, float /*new_value*/) {
                            // Resize our filter vector from a background thread
                            filter_stages_updater_.triggerAsyncUpdate();
                        }),
forwardFFT (fftOrder),
window (fftSize, juce::dsp::WindowingFunction<float>::hann) {
              parameters_.addParameterListener(filter_stages_param_name,
                                               &filter_stages_listener_);
    
    auto pConfiguration = appConfig.createLicenseSpringConfig();
    licenseManager = LicenseSpring::LicenseManager::create( appConfig.createLicenseSpringConfig() );
}
   
CodaProcessor::~CodaProcessor() {}

const juce::String CodaProcessor::getName() const {
    return JucePlugin_Name;
}

bool CodaProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool CodaProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool CodaProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double CodaProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int CodaProcessor::getNumPrograms() {
    return 1;
}

int CodaProcessor::getCurrentProgram() {
    return 0;
}

void CodaProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String CodaProcessor::getProgramName(int /*index*/) {
    return "default";
}

void CodaProcessor::changeProgramName(int /*index*/,
                                         const juce::String& /*newName*/) {}

void CodaProcessor::prepareToPlay(double sampleRate,
                                     int maximumExpectedSamplesPerBlock) {
    current_spec_ = juce::dsp::ProcessSpec{
        .sampleRate = sampleRate,
        .maximumBlockSize = static_cast<uint32>(maximumExpectedSamplesPerBlock),
        .numChannels = static_cast<uint32>(getMainBusNumInputChannels())};

    // After initializing the filters we make an explicit call to
    // `filters.get()` to swap the two filters in case we get a parameter change
    // before the first processing cycle. Updating the filters will also set the
    // `is_initialized` flag to `false`, so the filter coefficients will be
    // initialized during the first processing cycle.
    update_and_swap_filters();
    filters_.get();

    // The filter parameter will be smoothed to prevent clicks during automation
    const double compensated_sample_rate = sampleRate / smoothing_interval_;
    smoothed_filter_frequency_.reset(compensated_sample_rate,
                                     filter_smoothing_secs);
    smoothed_filter_resonance_.reset(compensated_sample_rate,
                                     filter_smoothing_secs);
    smoothed_filter_spread_.reset(compensated_sample_rate,
                                  filter_smoothing_secs);
}

void CodaProcessor::releaseResources() {
    filters_.clear([](Filters& filters) {
        filters.stages.clear();
        filters.stages.shrink_to_fit();
        
    });
}

bool CodaProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
    // We can support any number of channels, as long as the main input and
    // output have the same number of channels
    return (layouts.getMainInputChannelSet() ==
            layouts.getMainOutputChannelSet()) &&
           !layouts.getMainInputChannelSet().isDisabled();
}

void CodaProcessor::processBlockBypassed(
    juce::AudioBuffer<float>& /*buffer*/,
    juce::MidiBuffer& /*midiMessages*/) {
    // TODO: The default should be fine if we don't introduce any latency
}

void CodaProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& /*midiMessages*/) {
    juce::AudioBuffer<float> main_buffer = getBusBuffer(buffer, true, 0);
    juce::ScopedNoDenormals noDenormals;

    // TODO: Is there a way to get the VST3 silence flags? Carla, and perhaps
    //        also some other hosts, enable a lot more channels than the user is
    //        likely going to use, so we'll end up wasting a ton of resources on
    //        processing silcence.
    
    const size_t input_channels =
        static_cast<size_t>(getMainBusNumInputChannels());
    const size_t output_channels =
        static_cast<size_t>(getMainBusNumOutputChannels());
    const size_t num_samples = static_cast<size_t>(buffer.getNumSamples());

    for (auto channel = input_channels; channel < output_channels; channel++) {
        buffer.clear(channel, 0.0f, num_samples);
    }

    // Our filter structure gets updated from a background thread whenever the
    // `filter_stages` parameter changes
    Filters& filters = filters_.get();

    smoothed_filter_frequency_.setTargetValue(filter_frequency_);
    smoothed_filter_resonance_.setTargetValue(filter_resonance_);
    //smoothed_filter_spread_.setTargetValue(filter_spread_);
    for (size_t sample_idx = 0; sample_idx < num_samples; sample_idx++) {
        // Recomputing these IIR coefficients every sample is expensive, so to
        // save some cycles we only do it once every `smoothing_interval`
        // samples unless the filters just got reinitialized or some parameter
        // we can't smooth has
        const bool should_apply_smoothing =
            next_smooth_in_ <= 0 && (smoothed_filter_frequency_.isSmoothing() ||
                                     smoothed_filter_resonance_.isSmoothing() ||
                                     smoothed_filter_spread_.isSmoothing());
        const bool should_update_filters =
            !filters.is_initialized ||
            filter_spread_linear_ != old_filter_spread_linear_ ||
            should_apply_smoothing;

        const float current_filter_frequency =
            should_apply_smoothing
                ? smoothed_filter_frequency_.getNextValue()
                : smoothed_filter_frequency_.getCurrentValue();
        const float current_filter_resonance =
            should_apply_smoothing
                ? smoothed_filter_resonance_.getNextValue()
                : smoothed_filter_resonance_.getCurrentValue();
        const float current_filter_spread =
            should_apply_smoothing ? smoothed_filter_spread_.getNextValue()
                                   : smoothed_filter_spread_.getCurrentValue();

        if (should_update_filters && !filters.stages.empty()) {
            // We can use a single set of coefficients as a cache locality
            // optimization if spread has been disabled
            const bool use_single_set_of_coefficients =
                current_filter_spread == 0.0f;
            if (use_single_set_of_coefficients) {
                *filters.stages[0].coefficients =
                    juce::dsp::IIR::ArrayCoefficients<float>::makeAllPass(
                        getSampleRate(), current_filter_frequency,
                        current_filter_resonance);
            }

            // The filter spread can be either linear or logarithmic. The
            // logarithmic version is the default because it sounds a bit more
            // natural. We also need to make sure the spread range stays in the
            // normal ranges to prevent the filters from crapping out. This does
            // cause the range to shift slightly with high spread values and low
            // or high frequency values. Ideally we would want to prevent this
            // in the GUI.
            // TODO: When adding a GUI, prevent spread values that would cause
            //       the frequency range to be shifted
            const float below_nyquist_frequency =
                static_cast<float>(getSampleRate()) / 2.1f;
            const float min_filter_frequency = std::clamp(
                current_filter_frequency - (current_filter_spread / 2.0f), 5.0f,
                below_nyquist_frequency);
            const float max_filter_frequency = std::clamp(
                current_filter_frequency + (current_filter_spread / 2.0f), 5.0f,
                below_nyquist_frequency);
            const float filter_frequency_delta =
                max_filter_frequency - min_filter_frequency;
            const float log_min_filter_frequency =
                std::log(min_filter_frequency);
            const float log_max_filter_frequency =
                std::log(max_filter_frequency);
            const float log_filter_frequency_delta =
                log_max_filter_frequency - log_min_filter_frequency;

            const size_t num_stages = filters.stages.size();
            for (size_t stage_idx = 0; stage_idx < num_stages; stage_idx++) {
                auto& stage = filters.stages[stage_idx];

                if (!use_single_set_of_coefficients) {
                    // TODO: Maybe add back the option for simple linear
                    //       skewing. Or use the same skew scheme JUCE's
                    //       parameter range uses and make the skew factor
                    //       configurable.
                    const float frequency_offset_factor =
                        num_stages == 1 ? 0.5f
                                        : (static_cast<float>(stage_idx) /
                                           static_cast<float>(num_stages - 1));

                    *stage.coefficients =
                        juce::dsp::IIR::ArrayCoefficients<float>::makeAllPass(
                            getSampleRate(),
                            filter_spread_linear_
                                ? (min_filter_frequency +
                                   (filter_frequency_delta *
                                    frequency_offset_factor))
                                : std::exp(log_min_filter_frequency +
                                           (log_filter_frequency_delta *
                                            frequency_offset_factor)),
                            current_filter_resonance);
                }

                const auto& coefficients = use_single_set_of_coefficients
                                               ? filters.stages[0].coefficients
                                               : stage.coefficients;
                for (auto& filter : stage.channels) {
                    filter.coefficients = coefficients;
                }
            }

            next_smooth_in_ = smoothing_interval_;
        }

        next_smooth_in_ -= 1;
        filters.is_initialized = true;
        old_filter_spread_linear_ = filter_spread_linear_;

//        for (auto& stage : filters.stages) 
//        {
//            for (size_t channel = 0; channel < input_channels; channel++) 
//            {
//                // TODO: We should add a dry-wet control, could be useful for
//                //       automation
//                // TODO: Oh and we should _definitely_ have some kind of 'safe
//                //       mode' limiter enabled by default
//                buffer.getArrayOfWritePointers()[channel][sample_idx] =
//                    stage.channels[channel].processSample(
//                                                          buffer.getArrayOfWritePointers()[channel][sample_idx]);
//            }
//        }
        
        if (!smoothed_filter_spread_.isSmoothing())
        {
            for (int i = 0; i < filter_stages_; ++i)
            {
                for (size_t channel = 0; channel < input_channels; channel++)
                {
                    buffer.getArrayOfWritePointers()[channel][sample_idx] =
                    filters.stages[i].channels[channel].processSample(
                                                              buffer.getArrayOfWritePointers()[channel][sample_idx]);
                }
            }
        }
    }
    
    if (buffer.getNumChannels() > 0)
    {
        auto* channelData = buffer.getReadPointer (0);

        for (auto i = 0; i < buffer.getNumSamples(); ++i)
        {
            if (buffer.getMagnitude(0, buffer.getNumSamples() != 0.0f))
            {
                pushNextSampleIntoFifo (channelData[i]);
            }
        }
    }
}

void CodaProcessor::drawNextFrameOfSpectrum()
{
    // first apply a windowing function to our data
    window.multiplyWithWindowingTable (fftData, fftSize);       // [1]

    // then render our FFT data..
    forwardFFT.performFrequencyOnlyForwardTransform (fftData);  // [2]

    auto mindB = -100.0f;
    auto maxdB =    0.0f;

    // Constants for frequency mapping
    const float sampleRate = getSampleRate(); // Adjust if your sample rate is different
    const float maxFrequency = sampleRate / 2.0f;
    const float centerFrequency = 800.0f;
    const float scalingFactor = std::log(maxFrequency / centerFrequency);

    for (int i = 0; i < scopeSize; ++i)                         // [3]
    {
        // Proportion of the way through the scope
        float proportion = (float) i / (float) scopeSize;
        
        // Apply a logarithmic scale to the proportion
        float frequency = centerFrequency * std::exp(scalingFactor * (2.0f * proportion - 1.0f));

        // Find the corresponding index in the FFT data
        int fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(frequency / maxFrequency * (fftSize / 2)));

        // Map the FFT data to decibels and then to the level
        float level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(fftData[fftDataIndex])
                                              - juce::Decibels::gainToDecibels((float) fftSize)),
                                 mindB, maxdB, 0.0f, 1.0f);

        scopeData[i] = level;                                   // [4]
    }
}


bool CodaProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* CodaProcessor::createEditor() {
    return new CodaEditor (*this);
}

void CodaProcessor::getStateInformation(juce::MemoryBlock& destData) {
    const std::unique_ptr<juce::XmlElement> xml =
    parameters_.copyState().createXml();
    copyXmlToBinary(*xml, destData);
}

void CodaProcessor::setStateInformation(const void* data, int sizeInBytes) {
    const auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml && xml->hasTagName(parameters_.state.getType())) {
        parameters_.replaceState(juce::ValueTree::fromXml(*xml));

        // When loading patches from before we added the smoothing interval we
        // should default that to 1 instead of the normal default in case the
        // parameter was being automated
        bool has_smoothing_interval = false;
        for (const auto* child : xml->getChildWithTagNameIterator("PARAM")) {
            if (child->compareAttribute("id", smoothing_interval_param_name)) {
                has_smoothing_interval = true;
                break;
            }
        }

        if (!has_smoothing_interval) {
            smoothing_interval_ = 1;
        }
    }
}

void CodaProcessor::update_and_swap_filters() 
{
    
    auto newCoeffs = new juce::dsp::IIR::Coefficients<float>(FilterStage::Coefficients{});
    filters_.modify_and_swap([this, &newCoeffs](Filters& filters) 
    {
        filters.is_initialized = false;
        filters.stages.resize(static_cast<size_t>(40));

        for (auto& stage : filters.stages) {
            // The actual coefficients for each stage are initialized on the
            // next processing cycle thanks to `filters.is_initialized`
            if (!stage.coefficients) {
                // The actual values here don't matter and we can just use
                // any 6 length array
                stage.coefficients = newCoeffs;
            }

            stage.channels.resize(
                static_cast<size_t>(getMainBusNumOutputChannels()));
            for (auto& filter : stage.channels) {
                filter.prepare(current_spec_);
                filter.coefficients = stage.coefficients;
                filter.reset();
            }
        }
    });
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new CodaProcessor();
}


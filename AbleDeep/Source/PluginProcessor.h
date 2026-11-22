#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "AbleDeepEngine.h"

class AbleDeepProcessor : public juce::AudioProcessor
{
public:
    AbleDeepProcessor();
    ~AbleDeepProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AbleDeep"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override                       { return (int) presets.size(); }
    int getCurrentProgram() override                    { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    const std::vector<abledeep::Preset>& getPresets() const { return presets; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void handleMidi (const juce::MidiMessage& m);
    void pullParams();

    abledeep::Engine engine;
    abledeep::Params params;
    std::vector<abledeep::Preset> presets;
    std::vector<std::pair<std::atomic<float>*, float abledeep::Params::*>> paramLinks;
    int currentProgram = 0;
    std::vector<float> monoScratch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AbleDeepProcessor)
};

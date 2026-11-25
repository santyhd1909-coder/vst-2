#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Synth.h"
#include "Presets.h"

class DaluProcessor : public juce::AudioProcessor
{
public:
    DaluProcessor();
    ~DaluProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "DALUmagic"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadPreset (int index);

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;

    // datos para el parlante animado (se leen desde la interfaz)
    std::atomic<float> outLevel { 0.f }, outBass { 0.f }, outHigh { 0.f }, noteKick { 0.f };
    std::atomic<int>   currentPreset { 0 };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    SynthParams readParams() const;

    DaluEngine engine;
    double sr = 44100.0;
    float lpB = 0.f, lpH = 0.f, aB = 0.01f, aH = 0.3f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DaluProcessor)
};

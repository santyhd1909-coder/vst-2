#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

using namespace juce;

//==============================================================================
// Estilo visual Frutiger Metro: rojo / negro / blanco
class DaluLook : public LookAndFeel_V4
{
public:
    DaluLook();
    void drawRotarySlider (Graphics&, int x, int y, int w, int h, float pos, float startAngle, float endAngle, Slider&) override;
    void drawButtonBackground (Graphics&, Button&, const Colour&, bool highlighted, bool down) override;
    void drawComboBox (Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, ComboBox&) override;
};

//==============================================================================
class Knob : public Component
{
public:
    Knob (AudioProcessorValueTreeState& apvts, const String& paramId, const String& caption);
    void resized() override;
    Slider slider;
    Label label;
private:
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> att;
};

//==============================================================================
// Parlante estilo tinta que reacciona al audio
class SpeakerComponent : public Component, private Timer
{
public:
    explicit SpeakerComponent (DaluProcessor& p);
    void paint (Graphics&) override;
private:
    void timerCallback() override;
    void drawDriver (Graphics&, float cx, float cy, float R, float exc, float cap);
    void drawSpike (Graphics&, float cx, float cy, float R, float a, float len, float w);

    struct Spike { float a, len, w, phase, speed; };
    struct Dot   { float a, d, r, phase; bool red; };
    struct Ripple { float r, alpha; };

    DaluProcessor& proc;
    std::vector<Spike> spikesW, spikesT;
    std::vector<Dot> dots;
    std::vector<Ripple> ripples;
    float lvl = 0, bass = 0, high = 0, prevBass = 0, kick = 0, t = 0;
};

//==============================================================================
class DaluEditor : public AudioProcessorEditor,
                   private AudioProcessorValueTreeState::Listener,
                   private AsyncUpdater,
                   private Timer
{
public:
    explicit DaluEditor (DaluProcessor&);
    ~DaluEditor() override;
    void paint (Graphics&) override;
    void resized() override;

private:
    void parameterChanged (const String&, float) override { triggerAsyncUpdate(); }
    void handleAsyncUpdate() override { resized(); repaint(); }
    void timerCallback() override;

    void rebuildPresetRows();
    void pickPreset (int idx);
    void stepPreset (int dir);
    void showFxMenu();
    void setFxOn (int i, bool on);
    void drawPanel (Graphics&, Rectangle<int>, const String& title, bool white = false);
    static Image makeBackground (int w, int h);

    struct PresetModel : public ListBoxModel
    {
        std::vector<int> rows;
        std::function<void (int)> onPick;
        int current = -1;
        int getNumRows() override { return (int) rows.size(); }
        void paintListBoxItem (int row, Graphics& g, int w, int h, bool) override;
        void listBoxItemClicked (int row, const MouseEvent&) override
        {
            if (row >= 0 && row < (int) rows.size() && onPick) onPick (rows[(size_t) row]);
        }
    };

    struct ComboUI
    {
        ComboBox box;
        Label caption;
        std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> att;
    };

    struct FxUI
    {
        const char* key = ""; const char* title = "";
        std::vector<std::unique_ptr<Knob>> knobs;
        TextButton close { "x" };
        Rectangle<int> rect;
        bool on = false;
    };

    DaluProcessor& proc;
    DaluLook look;

    Image titleImage;
    TextButton prevBtn { "<" }, nextBtn { ">" }, addFxBtn;
    ComboBox catBox;
    PresetModel model;
    ListBox presetList { "presets", &model };
    int lastPreset = -1;

    std::array<ComboUI, 6> combos;                  // w1, w2, o2, uni, vow, leg
    std::vector<std::unique_ptr<Knob>> oscKnobs, filterKnobs, envKnobs, modKnobs;
    std::unique_ptr<Knob> volKnob;
    std::array<FxUI, 5> fx;

    SpeakerComponent speaker;
    MidiKeyboardComponent keyboard;

    Rectangle<int> presetRect, oscRect, filtRect, envRect, fxRect, lcdRect, modRect;
    Image bgImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DaluEditor)
};

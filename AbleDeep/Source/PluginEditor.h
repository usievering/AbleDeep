#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

namespace abledeep::ui
{
namespace colours
{
    const juce::Colour bg        { 0xff15120f };
    const juce::Colour panel     { 0xff211c18 };
    const juce::Colour panelEdge { 0xff352d26 };
    const juce::Colour text      { 0xffe9dfd3 };
    const juce::Colour textDim   { 0xff9a8c7e };
    const juce::Colour accent    { 0xffe3a24f };   // warmes Bernstein
    const juce::Colour accent2   { 0xff5fb8a8 };   // gedecktes Petrol
    const juce::Colour track     { 0xff3a3029 };
}

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (juce::FontOptions (13.0f)); }
    juce::Font getPopupMenuFont() override { return juce::Font (juce::FontOptions (14.0f)); }
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
};

struct Control
{
    std::unique_ptr<juce::Slider> slider;
    std::unique_ptr<juce::ComboBox> combo;
    std::unique_ptr<juce::ToggleButton> toggle;
    std::unique_ptr<juce::Label> label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bAtt;
    juce::Component* main() const
    {
        if (slider) return slider.get();
        if (combo) return combo.get();
        return toggle.get();
    }
};

struct Section
{
    juce::String title;
    int cols = 2;
    int row = 0;
    bool dimmed = false;
    std::vector<Control*> controls;
    juce::Rectangle<int> bounds;
    juce::Colour colour;
};
} // namespace abledeep::ui

class AbleDeepEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit AbleDeepEditor (AbleDeepProcessor&);
    ~AbleDeepEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void layoutSection (abledeep::ui::Section&);

    AbleDeepProcessor& proc;
    abledeep::ui::LookAndFeel lnf;

    std::vector<std::unique_ptr<abledeep::ui::Control>> controls;
    std::vector<abledeep::ui::Section> sections;

    juce::ComboBox presetBox;
    juce::TextButton prevButton { "<" }, nextButton { ">" };
    int shownProgram = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AbleDeepEditor)
};

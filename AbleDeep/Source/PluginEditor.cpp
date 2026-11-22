#include "PluginEditor.h"

using namespace abledeep;
using namespace abledeep::ui;

static constexpr int kCellW = 70, kCellH = 82, kTitleH = 26, kPad = 8, kGap = 10, kHeaderH = 64, kFooterH = 24;

// =============================================================================
//  LookAndFeel
// =============================================================================
LookAndFeel::LookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, colours::track);
    setColour (juce::ComboBox::textColourId, colours::text);
    setColour (juce::ComboBox::outlineColourId, colours::panelEdge);
    setColour (juce::ComboBox::arrowColourId, colours::accent);
    setColour (juce::PopupMenu::backgroundColourId, colours::panel);
    setColour (juce::PopupMenu::textColourId, colours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, colours::accent.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, colours::text);
    setColour (juce::Label::textColourId, colours::textDim);
    setColour (juce::TextButton::buttonColourId, colours::track);
    setColour (juce::TextButton::textColourOffId, colours::text);
    setColour (juce::BubbleComponent::backgroundColourId, colours::panel);
    setColour (juce::BubbleComponent::outlineColourId, colours::accent);
    setColour (juce::TooltipWindow::textColourId, colours::text);
}

void LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                    float startAngle, float endAngle, juce::Slider& s)
{
    const auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const auto fill = s.findColour (juce::Slider::rotarySliderFillColourId);

    // Knopfkörper
    const float bodyR = radius * 0.68f;
    juce::ColourGradient grad (juce::Colour (0xff3b322b), centre.x, centre.y - bodyR,
                               juce::Colour (0xff1c1714), centre.x, centre.y + bodyR, false);
    g.setGradientFill (grad);
    g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

    // Spur
    const float arcR = radius - 2.0f;
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (colours::track);
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Wert (bipolare Regler starten in der Mitte)
    const bool bipolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;
    const float from = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;
    juce::Path val;
    val.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, juce::jmin (from, angle), juce::jmax (from, angle), true);
    g.setColour (fill);
    g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Zeiger
    const juce::Point<float> p1 = centre.getPointOnCircumference (bodyR * 0.25f, angle);
    const juce::Point<float> p2 = centre.getPointOnCircumference (bodyR * 0.85f, angle);
    g.setColour (colours::text);
    g.drawLine ({ p1, p2 }, 2.0f);
}

void LookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box)
{
    const auto r = juce::Rectangle<int> (0, 0, w, h).toFloat().reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (box.hasKeyboardFocus (false) ? colours::accent : colours::panelEdge);
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    juce::Path arrow;
    const float ax = (float) w - 11.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (colours::accent);
    g.fillPath (arrow);
}

void LookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    const auto r = b.getLocalBounds().toFloat().reduced (6.0f, 4.0f);
    const bool on = b.getToggleState();
    g.setColour (on ? colours::accent : colours::track);
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    if (highlighted) { g.setColour (juce::Colours::white.withAlpha (0.08f)); g.fillRoundedRectangle (r, r.getHeight() * 0.5f); }
    g.setColour (on ? colours::bg : colours::textDim);
    g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
    g.drawText (on ? "ON" : "OFF", r, juce::Justification::centred);
}

void LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool hi, bool down)
{
    const auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (down ? colours::accent.withAlpha (0.4f) : (hi ? colours::panelEdge : colours::track));
    g.fillRoundedRectangle (r, 4.0f);
}

// =============================================================================
//  Editor
// =============================================================================
AbleDeepEditor::AbleDeepEditor (AbleDeepProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);

    struct SecDef { const char* group; const char* title; int cols; int row; juce::Colour col; };
    const SecDef defs[] {
        { "Engine",  "ENGINE",   1, 0, colours::accent },
        { "Analog",  "ANALOG",   4, 0, colours::accent },
        { "E-Piano", "E-PIANO",  2, 0, colours::accent2 },
        { "Organ",   "ORGAN",    3, 0, colours::accent2 },
        { "Play",    "PLAY",     2, 0, colours::accent },
        { "Filter",  "FILTER",   4, 1, colours::accent },
        { "Amp",     "AMP",      3, 1, colours::accent },
        { "LFO",     "LFO",      2, 1, colours::accent2 },
        { "FX",      "EFFECTS",  6, 1, colours::accent2 },
    };

    for (const auto& d : defs)
    {
        Section s; s.title = d.title; s.cols = d.cols; s.colour = d.col; s.row = d.row;
        sections.push_back (s);
    }

    const Params defaults;
    for (const auto& info : paramTable())
    {
        auto c = std::make_unique<Control>();
        Section* sec = nullptr;
        for (size_t i = 0; i < std::size (defs); ++i)
            if (juce::String (defs[i].group) == info.group) sec = &sections[i];

        c->label = std::make_unique<juce::Label> (juce::String(), info.name);
        c->label->setJustificationType (juce::Justification::centred);
        c->label->setFont (juce::Font (juce::FontOptions (11.5f)));
        c->label->setInterceptsMouseClicks (false, false);
        addAndMakeVisible (*c->label);

        if (info.type == PType::Choice)
        {
            c->combo = std::make_unique<juce::ComboBox>();
            int id = 1;
            for (const auto& item : *info.choices) c->combo->addItem (item, id++);
            addAndMakeVisible (*c->combo);
            c->cAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, info.id, *c->combo);
        }
        else if (info.type == PType::Bool)
        {
            c->toggle = std::make_unique<juce::ToggleButton>();
            addAndMakeVisible (*c->toggle);
            c->bAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, info.id, *c->toggle);
        }
        else
        {
            c->slider = std::make_unique<juce::Slider> (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox);
            c->slider->setPopupDisplayEnabled (true, true, this);
            c->slider->setColour (juce::Slider::rotarySliderFillColourId, sec != nullptr ? sec->colour : colours::accent);
            c->slider->setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
            addAndMakeVisible (*c->slider);
            c->sAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, info.id, *c->slider);
            c->slider->setDoubleClickReturnValue (true, (double) (defaults.*(info.ptr)));
        }

        if (sec != nullptr) sec->controls.push_back (c.get());
        controls.push_back (std::move (c));
    }

    // Preset-Auswahl
    const auto& presets = proc.getPresets();
    std::string lastCategory;
    for (int i = 0; i < (int) presets.size(); ++i)
    {
        const auto& pr = presets[(size_t) i];
        if (pr.category != lastCategory)
        {
            if (i > 0) presetBox.addSeparator();
            presetBox.addSectionHeading (juce::String::fromUTF8 (pr.category.c_str()));
            lastCategory = pr.category;
        }
        presetBox.addItem (juce::String::fromUTF8 (pr.name.c_str()), i + 1);
    }
    presetBox.setColour (juce::ComboBox::backgroundColourId, colours::panel);
    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0 && idx != proc.getCurrentProgram()) { proc.setCurrentProgram (idx); shownProgram = idx; }
    };
    addAndMakeVisible (presetBox);

    auto step = [this] (int d)
    {
        const int n = proc.getNumPrograms();
        proc.setCurrentProgram ((proc.getCurrentProgram() + d + n) % n);
    };
    prevButton.onClick = [step] { step (-1); };
    nextButton.onClick = [step] { step (+1); };
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);

    // Fenstergröße aus dem Layout berechnen
    int row1 = 0, row2 = 0, n1 = 0, n2 = 0;
    for (auto& s : sections) { (s.row == 0 ? row1 : row2) += s.cols * kCellW + 2 * kPad; (s.row == 0 ? n1 : n2)++; }
    const int width = juce::jmax (row1 + (n1 + 1) * kGap, row2 + (n2 + 1) * kGap);
    const int secH = kTitleH + 2 * kCellH + kPad;
    setSize (width, kHeaderH + 2 * secH + 3 * kGap + kFooterH);

    timerCallback();
    startTimerHz (8);
}

AbleDeepEditor::~AbleDeepEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void AbleDeepEditor::timerCallback()
{
    // Sektionen der nicht aktiven Engines abdunkeln
    const int eng = (int) proc.apvts.getRawParameterValue ("engine")->load();
    const char* engineSections[] { "ANALOG", "E-PIANO", "ORGAN" };
    bool changed = false;
    for (auto& s : sections)
        for (int e = 0; e < 3; ++e)
            if (s.title == engineSections[e])
            {
                const bool dim = (e != eng);
                if (dim != s.dimmed)
                {
                    s.dimmed = dim; changed = true;
                    for (auto* ctl : s.controls) { ctl->main()->setAlpha (dim ? 0.4f : 1.0f); ctl->label->setAlpha (dim ? 0.4f : 1.0f); }
                }
            }
    if (changed) repaint();

    const int cur = proc.getCurrentProgram();
    if (cur != shownProgram)
    {
        shownProgram = cur;
        presetBox.setSelectedId (cur + 1, juce::dontSendNotification);
    }
}

void AbleDeepEditor::paint (juce::Graphics& g)
{
    g.fillAll (colours::bg);

    // Header
    auto header = getLocalBounds().removeFromTop (kHeaderH);
    juce::ColourGradient hg (juce::Colour (0xff2a211a), 0.0f, 0.0f, colours::bg, 0.0f, (float) kHeaderH, false);
    g.setGradientFill (hg);
    g.fillRect (header);

    auto logo = header.reduced (kGap + 4, 0);
    g.setFont (juce::Font (juce::FontOptions (30.0f, juce::Font::bold)));
    g.setColour (colours::text);
    const int ableW = juce::GlyphArrangement::getStringWidthInt (g.getCurrentFont(), "Able");
    g.drawText ("Able", logo.removeFromLeft (ableW + 2), juce::Justification::centredLeft);
    g.setColour (colours::accent);
    const int deepW = juce::GlyphArrangement::getStringWidthInt (g.getCurrentFont(), "Deep");
    g.drawText ("Deep", logo.removeFromLeft (deepW + 4), juce::Justification::centredLeft);
    g.setColour (colours::textDim);
    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.drawText (juce::String::fromUTF8 ("  Deep House · Soulful House · Detroit Techno"), logo.removeFromLeft (340), juce::Justification::centredLeft);

    g.setColour (colours::textDim);
    g.drawText ("PRESET", prevButton.getBounds().translated (-62, 0).withWidth (56), juce::Justification::centredRight);

    // Sektionen
    for (const auto& s : sections)
    {
        const auto r = s.bounds.toFloat();
        g.setColour (s.dimmed ? colours::panel.darker (0.35f) : colours::panel);
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (colours::panelEdge);
        g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);

        g.setColour (s.colour);
        g.fillRoundedRectangle (r.getX() + kPad, r.getY() + 9.0f, 3.0f, 10.0f, 1.5f);
        g.setColour (colours::text);
        g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
        g.drawText (s.title, (int) r.getX() + kPad + 8, (int) r.getY() + 4, 160, 20, juce::Justification::centredLeft);
    }

    // Footer
    g.setColour (colours::textDim.withAlpha (0.8f));
    g.setFont (juce::Font (juce::FontOptions (11.5f)));
    g.drawText (juce::String::fromUTF8 ("Mod-Wheel öffnet das Filter  |  Pitch-Bend ±2  |  Sustain-Pedal  |  Doppelklick setzt Regler zurück  |  v1.2"),
                getLocalBounds().removeFromBottom (kFooterH).reduced (kGap, 0), juce::Justification::centredLeft);
}

void AbleDeepEditor::layoutSection (Section& s)
{
    auto area = s.bounds.reduced (kPad, 0);
    area.removeFromTop (kTitleH);
    const int totalCols = s.cols;
    const int cellW = area.getWidth() / totalCols;

    for (size_t i = 0; i < s.controls.size(); ++i)
    {
        const int col = (int) i % totalCols, row = (int) i / totalCols;
        juce::Rectangle<int> cell (area.getX() + col * cellW, area.getY() + row * kCellH, cellW, kCellH);
        auto* c = s.controls[i];
        auto labelArea = cell.removeFromBottom (18);
        c->label->setBounds (labelArea);

        if (c->slider)
            c->slider->setBounds (cell.withSizeKeepingCentre (juce::jmin (cell.getWidth(), 62), juce::jmin (cell.getHeight(), 62)));
        else if (c->combo)
            c->combo->setBounds (cell.withSizeKeepingCentre (cellW - 6, 24).translated (0, 8));
        else if (c->toggle)
            c->toggle->setBounds (cell.withSizeKeepingCentre (juce::jmin (cellW - 8, 60), 28).translated (0, 6));
    }
}

void AbleDeepEditor::resized()
{
    auto header = getLocalBounds().removeFromTop (kHeaderH).reduced (kGap + 4, 16);
    auto right = header.removeFromRight (360);
    nextButton.setBounds (right.removeFromRight (30));
    right.removeFromRight (4);
    presetBox.setBounds (right.removeFromRight (260));
    right.removeFromRight (4);
    prevButton.setBounds (right.removeFromRight (30));

    const int secH = kTitleH + 2 * kCellH + kPad;
    for (int row = 0; row < 2; ++row)
    {
        std::vector<Section*> rs;
        int natural = 0;
        for (auto& s : sections)
            if (s.row == row) { rs.push_back (&s); natural += s.cols * kCellW + 2 * kPad; }

        const int avail = getWidth() - (int) (rs.size() + 1) * kGap;
        const float scale = (float) avail / (float) natural;
        int x = kGap;
        const int y = kHeaderH + kGap + row * (secH + kGap);
        for (size_t i = 0; i < rs.size(); ++i)
        {
            int w = (int) std::round ((float) (rs[i]->cols * kCellW + 2 * kPad) * scale);
            if (i == rs.size() - 1) w = getWidth() - kGap - x;
            rs[i]->bounds = { x, y, w, secH };
            layoutSection (*rs[i]);
            x += w + kGap;
        }
    }
}

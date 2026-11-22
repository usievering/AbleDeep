#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace abledeep;

static juce::String formatValue (const ParamInfo& info, float v)
{
    const juce::String unit (info.unit);
    if (unit == "Hz")  return v >= 1000.0f ? juce::String (v / 1000.0f, 2) + " kHz" : juce::String (v, v < 10.0f ? 2 : 0) + " Hz";
    if (unit == "s")   return v < 1.0f ? juce::String (juce::roundToInt (v * 1000.0f)) + " ms" : juce::String (v, 2) + " s";
    if (unit == "dB")  return juce::String (v, 1) + " dB";
    if (unit == "ct")  return juce::String (v, 1) + " ct";
    if (info.minV < 0.0f) return juce::String (juce::roundToInt (v * 100.0f)) + " %";
    return juce::String (juce::roundToInt (v * 100.0f)) + " %";
}

juce::AudioProcessorValueTreeState::ParameterLayout AbleDeepProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    const Params defaults;

    for (const auto& info : paramTable())
    {
        const juce::ParameterID pid { info.id, 1 };
        const float def = defaults.*(info.ptr);

        switch (info.type)
        {
            case PType::Choice:
            {
                juce::StringArray items;
                for (const auto& c : *info.choices) items.add (c);
                layout.add (std::make_unique<juce::AudioParameterChoice> (pid, info.name, items, (int) def));
                break;
            }
            case PType::Bool:
                layout.add (std::make_unique<juce::AudioParameterBool> (pid, info.name, def > 0.5f));
                break;
            case PType::Int:
                layout.add (std::make_unique<juce::AudioParameterInt> (pid, info.name, (int) info.minV, (int) info.maxV, (int) def,
                                                                      juce::AudioParameterIntAttributes().withLabel (info.unit)));
                break;
            case PType::Float:
            default:
            {
                juce::NormalisableRange<float> range (info.minV, info.maxV);
                if (info.skewCentre > 0.0f) range.setSkewForCentre (info.skewCentre);
                const ParamInfo* ip = &info;
                layout.add (std::make_unique<juce::AudioParameterFloat> (
                    pid, info.name, range, def,
                    juce::AudioParameterFloatAttributes()
                        .withStringFromValueFunction ([ip] (float v, int) { return formatValue (*ip, v); })));
                break;
            }
        }
    }
    return layout;
}

AbleDeepProcessor::AbleDeepProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "AbleDeepState", createLayout()),
      presets (makePresets())
{
    for (const auto& info : paramTable())
        paramLinks.push_back ({ apvts.getRawParameterValue (info.id), info.ptr });

    setCurrentProgram (0);
}

bool AbleDeepProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void AbleDeepProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    monoScratch.assign ((size_t) juce::jmax (1, samplesPerBlock) * 2, 0.0f);
    pullParams();
    engine.setParams (params);
    engine.prepare (sampleRate);
}

void AbleDeepProcessor::pullParams()
{
    for (auto& link : paramLinks)
        params.*(link.second) = link.first->load (std::memory_order_relaxed);
}

void AbleDeepProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())                 engine.noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())           engine.noteOff (m.getNoteNumber());
    else if (m.isPitchWheel())        engine.pitchBend ((float) (m.getPitchWheelValue() - 8192) / 8192.0f);
    else if (m.isSustainPedalOn())    engine.sustainPedal (true);
    else if (m.isSustainPedalOff())   engine.sustainPedal (false);
    else if (m.isController() && m.getControllerNumber() == 1)
                                      engine.modWheel ((float) m.getControllerValue() / 127.0f);
    else if (m.isAllNotesOff() || m.isAllSoundOff())
                                      engine.allNotesOff();
}

void AbleDeepProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    pullParams();
    engine.setParams (params);

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                engine.setTempo (*bpm);

    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    if (numCh == 0) return;

    float* L = buffer.getWritePointer (0);
    float* R = numCh > 1 ? buffer.getWritePointer (1) : nullptr;

    // Mono-Ausgang: rechter Kanal in Scratch-Puffer
    if (R == nullptr)
    {
        if ((int) monoScratch.size() < numSamples) monoScratch.resize ((size_t) numSamples);
        R = monoScratch.data();
    }

    int pos = 0;
    for (const auto meta : midi)
    {
        const int evPos = juce::jlimit (0, numSamples, meta.samplePosition);
        if (evPos > pos) { engine.render (L + pos, R + pos, evPos - pos); pos = evPos; }
        handleMidi (meta.getMessage());
    }
    if (pos < numSamples) engine.render (L + pos, R + pos, numSamples - pos);

    if (numCh == 1)
        for (int i = 0; i < numSamples; ++i) L[i] = 0.5f * (L[i] + R[i]);

    for (int ch = 2; ch < numCh; ++ch) buffer.clear (ch, 0, numSamples);
}

void AbleDeepProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= (int) presets.size()) return;
    currentProgram = index;
    const Params& p = presets[(size_t) index].p;

    for (const auto& info : paramTable())
        if (auto* param = apvts.getParameter (info.id))
            param->setValueNotifyingHost (param->convertTo0to1 (p.*(info.ptr)));
}

const juce::String AbleDeepProcessor::getProgramName (int index)
{
    if (index < 0 || index >= (int) presets.size()) return {};
    return presets[(size_t) index].name;
}

void AbleDeepProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void AbleDeepProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            currentProgram = juce::jlimit (0, (int) presets.size() - 1, (int) tree.getProperty ("program", 0));
            apvts.replaceState (tree);
        }
}

juce::AudioProcessorEditor* AbleDeepProcessor::createEditor()
{
    return new AbleDeepEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AbleDeepProcessor();
}

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

namespace
{
    const int kO2[8]  = { -12, -5, 0, 5, 7, 12, 19, 24 };
    const int kUni[5] = { 1, 3, 5, 7, 9 };
}

AudioProcessorValueTreeState::ParameterLayout DaluProcessor::createLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto addF = [&] (const char* id, const char* name, float mn, float mx, float def, bool logSkew = false)
    {
        NormalisableRange<float> r (mn, mx);
        if (logSkew) r.setSkewForCentre (std::sqrt (mn * mx));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, r, def));
    };
    auto addC = [&] (const char* id, const char* name, StringArray items, int def)
    {
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, items, def));
    };
    auto addB = [&] (const char* id, const char* name)
    {
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, false));
    };

    addF ("vol", "Volumen", 0.f, 1.f, 0.7f);

    StringArray waves { "Seno", "Triangulo", "Sierra", "Cuadrada" };
    addC ("w1", "Onda 1", waves, 2);
    addC ("w2", "Onda 2", waves, 2);
    addC ("o2", "Osc2 semitonos", StringArray { "-12", "-5", "0", "+5", "+7", "+12", "+19", "+24" }, 2);
    addC ("uni", "Unison", StringArray { "1", "3", "5", "7", "9" }, 0);
    addF ("det", "Detune", 0.f, 100.f, 8.f);
    addF ("mix", "Mix Osc2", 0.f, 1.f, 0.5f);
    addF ("sub", "Sub", 0.f, 1.f, 0.f);
    addF ("nz", "Ruido", 0.f, 0.4f, 0.f);

    addC ("vow", "Formante", StringArray { "Off", "Ah", "Oh", "Ee", "Oo" }, 0);
    addF ("cut", "Cutoff", 60.f, 18000.f, 5000.f, true);
    addF ("res", "Reso", 0.1f, 18.f, 1.f);
    addF ("fe", "Env Filtro", -4.f, 6.f, 0.f);
    addF ("fd", "Dec Filtro", 0.03f, 3.f, 0.4f, true);

    addF ("A", "Attack", 0.002f, 4.f, 0.01f, true);
    addF ("D", "Decay", 0.02f, 4.f, 0.3f, true);
    addF ("S", "Sustain", 0.f, 1.f, 0.7f);
    addF ("R", "Release", 0.03f, 5.f, 0.4f, true);
    addF ("vib", "Vibrato", 0.f, 60.f, 0.f);
    addC ("leg", "Modo", StringArray { "Poly", "Legato" }, 0);
    addF ("glide", "Glide", 0.005f, 1.f, 0.08f, true);

    addB ("on_dr", "Drive ON");   addF ("dr_amt", "Drive", 0.f, 1.f, 0.3f);
    addB ("on_ch", "Chorus ON");  addF ("ch_mix", "Chorus Mix", 0.f, 1.f, 0.5f);  addF ("ch_rate", "Chorus Rate", 0.1f, 3.f, 0.6f, true);
    addB ("on_tr", "Tremolo ON"); addF ("tr_depth", "Tremolo Prof", 0.f, 1.f, 0.4f); addF ("tr_rate", "Tremolo Rate", 0.5f, 14.f, 5.f, true);
    addB ("on_dl", "Delay ON");   addF ("dl_mix", "Delay Mix", 0.f, 1.f, 0.3f); addF ("dl_time", "Delay Tiempo", 0.05f, 0.9f, 0.375f); addF ("dl_fb", "Delay Feedback", 0.f, 0.85f, 0.35f);
    addB ("on_rv", "Reverb ON");  addF ("rv_mix", "Reverb Mix", 0.f, 1.f, 0.35f); addF ("rv_size", "Reverb Tamano", 0.4f, 5.f, 2.f);

    return { p.begin(), p.end() };
}

DaluProcessor::DaluProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "DALUMAGIC", createLayout())
{
    loadPreset (0);
}

void DaluProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    engine.prepare (sr);
    aB = 1.f - std::exp (-dsp_util::kTwoPi * 200.f / (float) sr);
    aH = 1.f - std::exp (-dsp_util::kTwoPi * 3000.f / (float) sr);
    lpB = lpH = 0.f;
}

bool DaluProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == AudioChannelSet::stereo();
}

SynthParams DaluProcessor::readParams() const
{
    auto F = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };
    SynthParams P;
    P.w1 = (int) F ("w1"); P.w2 = (int) F ("w2"); P.vow = (int) F ("vow");
    P.uni = kUni[jlimit (0, 4, (int) F ("uni"))];
    P.o2semi = (float) kO2[jlimit (0, 7, (int) F ("o2"))];
    P.det = F ("det"); P.mix = F ("mix"); P.sub = F ("sub"); P.nz = F ("nz");
    P.cut = F ("cut"); P.res = F ("res"); P.fe = F ("fe"); P.fd = F ("fd");
    P.A = F ("A"); P.D = F ("D"); P.S = F ("S"); P.R = F ("R");
    P.vib = F ("vib"); P.vol = F ("vol");
    P.leg = (int) F ("leg") == 1; P.glide = F ("glide");
    P.onDr = F ("on_dr") > 0.5f; P.onCh = F ("on_ch") > 0.5f; P.onTr = F ("on_tr") > 0.5f;
    P.onDl = F ("on_dl") > 0.5f; P.onRv = F ("on_rv") > 0.5f;
    P.drAmt = F ("dr_amt"); P.chMix = F ("ch_mix"); P.chRate = F ("ch_rate");
    P.trDepth = F ("tr_depth"); P.trRate = F ("tr_rate");
    P.dlMix = F ("dl_mix"); P.dlTime = F ("dl_time"); P.dlFb = F ("dl_fb");
    P.rvMix = F ("rv_mix"); P.rvSize = F ("rv_size");
    return P;
}

void DaluProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (buffer.getNumChannels() < 2) return;

    keyboardState.processNextMidiBuffer (midi, 0, n, true);
    const SynthParams P = readParams();
    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    int pos = 0;
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        const int t = jlimit (0, n, meta.samplePosition);
        if (t > pos) { engine.render (L + pos, R + pos, t - pos, P); pos = t; }

        if (msg.isNoteOn())            { engine.noteOn (msg.getNoteNumber(), msg.getFloatVelocity(), P); noteKick.store (1.f); }
        else if (msg.isNoteOff())      engine.noteOff (msg.getNoteNumber(), P);
        else if (msg.isAllNotesOff() || msg.isAllSoundOff()) engine.allOff();
    }
    if (pos < n) engine.render (L + pos, R + pos, n - pos, P);

    float e = 0.f, eb = 0.f, eh = 0.f;
    for (int i = 0; i < n; ++i)
    {
        const float x = L[i];
        e += x * x;
        lpB += aB * (x - lpB); eb += lpB * lpB;
        lpH += aH * (x - lpH); const float h = x - lpH; eh += h * h;
    }
    const float inv = 1.f / (float) std::max (1, n);
    outLevel.store (std::sqrt (e * inv));
    outBass.store (std::sqrt (eb * inv));
    outHigh.store (std::sqrt (eh * inv));
}

void DaluProcessor::loadPreset (int idx)
{
    const auto& pre = getPresets();
    if (idx < 0 || idx >= (int) pre.size()) return;

    for (auto* prm : getParameters())
        if (auto* r = dynamic_cast<RangedAudioParameter*> (prm))
            if (r->paramID != "vol")
                r->setValueNotifyingHost (r->getDefaultValue());

    for (auto& kv : pre[(size_t) idx].values)
        if (auto* r = apvts.getParameter (kv.first))
            r->setValueNotifyingHost (r->convertTo0to1 (kv.second));

    currentPreset.store (idx);
}

void DaluProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("preset", currentPreset.load(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void DaluProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = ValueTree::fromXml (*xml);
            apvts.replaceState (vt);
            currentPreset.store ((int) vt.getProperty ("preset", 0));
        }
}

AudioProcessorEditor* DaluProcessor::createEditor() { return new DaluEditor (*this); }

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DaluProcessor(); }

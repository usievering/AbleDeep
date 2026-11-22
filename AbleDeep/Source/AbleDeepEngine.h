// =============================================================================
//  AbleDeep – Deep House / Soulful House Instrument
//  Klang-Engine (framework-unabhängig, reines C++17)
//
//  Drei Klangerzeuger:
//    0  Analog   – 2 PolyBLEP-Oszillatoren + Sub + Noise, Unison, 4-Pol-Ladder-Filter
//    1  E-Piano  – FM-Rhodes mit Tine-Anschlag, "Bark" und tastaturabhängigem Decay
//    2  Organ    – 9-Zugriegel-Orgel mit Percussion und Key-Click (M1-Orgelbass!)
//  Dazu: Chord Memory (m7/m9/m11 …), Mono/Glide, LFO, Drive, Chorus, Tremolo,
//  Ping-Pong-Delay (tempo-synchron), Hall, Vinyl-Dust.
// =============================================================================
#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>
#include <array>
#include <algorithm>
#include <string>

namespace abledeep
{
static constexpr float kPi = 3.14159265358979f;
static constexpr float kTwoPi = 6.28318530717959f;

// -----------------------------------------------------------------------------
//  Parameter-Set
// -----------------------------------------------------------------------------
struct Params
{
    // Engine
    float engine = 0;          // 0 Analog, 1 E-Piano, 2 Organ
    float octave = 0;          // -2 .. +2

    // Analog
    float osc1Wave = 0;        // 0 Saw, 1 Square, 2 Triangle, 3 Sine
    float osc2Wave = 0;
    float osc2Semi = 0;        // -24 .. 24
    float osc2Fine = 7;        // cents
    float oscMix = 0.5f;
    float subLevel = 0.0f;
    float noiseLevel = 0.0f;
    float unison = 0.25f;      // Detune-Breite

    // E-Piano
    float epTone = 0.5f;
    float epTine = 0.4f;
    float epBark = 0.3f;

    // Organ
    float orgLow = 0.8f;
    float orgMid = 0.5f;
    float orgHigh = 0.2f;
    float orgPerc = 0.3f;
    float orgClick = 0.3f;

    // Filter
    float cutoff = 8000.0f;    // Hz
    float resonance = 0.1f;
    float filterEnv = 0.2f;    // -1 .. 1
    float keyTrack = 0.5f;
    float fltA = 0.005f, fltD = 0.4f, fltS = 0.3f, fltR = 0.4f;

    // Amp
    float ampA = 0.003f, ampD = 0.6f, ampS = 0.8f, ampR = 0.3f;
    float velSens = 0.6f;

    // LFO
    float lfoRate = 0.3f;
    float lfoFilter = 0.0f;
    float lfoPitch = 0.0f;

    // Spielhilfen
    float chordMode = 0;       // siehe chordNames()
    float mono = 0;
    float glide = 0.05f;

    // Effekte
    float drive = 0.2f;
    float chorus = 0.3f;
    float tremRate = 4.0f;
    float tremDepth = 0.0f;
    float delayMix = 0.0f;
    float delayTime = 2;       // Index in delayNames()
    float delayFb = 0.35f;
    float reverbMix = 0.15f;
    float reverbSize = 0.6f;
    float dust = 0.0f;
    float gain = -6.0f;        // dB
};

inline const std::vector<std::string>& engineNames()  { static std::vector<std::string> v { "Analog", "E-Piano", "Organ" }; return v; }
inline const std::vector<std::string>& waveNames()    { static std::vector<std::string> v { "Saw", "Square", "Triangle", "Sine" }; return v; }
inline const std::vector<std::string>& chordNames()   { static std::vector<std::string> v { "Off", "min7", "min9", "min11", "maj7", "maj9", "dom9", "7sus4", "Deep Stack" }; return v; }
inline const std::vector<std::string>& voiceModeNames() { static std::vector<std::string> v { "Poly", "Mono" }; return v; }
inline const std::vector<std::string>& delayNames()   { static std::vector<std::string> v { "1/16", "1/8", "3/16", "1/4", "3/8", "1/2" }; return v; }
inline float delayBeats (int idx) { static const float b[] { 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f }; return b[std::clamp (idx, 0, 5)]; }

inline const std::vector<int>& chordIntervals (int mode)
{
    static const std::vector<std::vector<int>> c {
        { 0 },
        { 0, 3, 7, 10 },            // min7
        { 0, 3, 7, 10, 14 },        // min9  – der Klassiker
        { 0, 7, 10, 14, 15, 17 },   // min11 (offene Lage)
        { 0, 4, 7, 11 },            // maj7
        { 0, 4, 7, 11, 14 },        // maj9
        { 0, 4, 10, 14 },           // dom9
        { 0, 5, 7, 10 },            // 7sus4
        { 0, 7, 10, 14, 17 },       // Deep Stack (Quarten-Feeling, ohne Terz)
    };
    return c[(size_t) std::clamp (mode, 0, (int) c.size() - 1)];
}

// -----------------------------------------------------------------------------
//  Parameter-Tabelle (wird vom Plugin-Wrapper benutzt)
// -----------------------------------------------------------------------------
enum class PType { Float, Choice, Bool, Int };

struct ParamInfo
{
    const char* id;
    const char* name;
    PType type;
    float minV, maxV;
    float skewCentre;                       // 0 = linear
    const std::vector<std::string>* choices;
    float Params::* ptr;
    const char* unit;
    const char* group;
};

inline const std::vector<ParamInfo>& paramTable()
{
    static const std::vector<ParamInfo> t {
        { "engine",    "Engine",        PType::Choice, 0, 2, 0, &engineNames(), &Params::engine, "", "Engine" },
        { "octave",    "Octave",        PType::Int,   -2, 2, 0, nullptr, &Params::octave, "", "Engine" },

        { "osc1Wave",  "Osc 1 Wave",    PType::Choice, 0, 3, 0, &waveNames(), &Params::osc1Wave, "", "Analog" },
        { "osc2Wave",  "Osc 2 Wave",    PType::Choice, 0, 3, 0, &waveNames(), &Params::osc2Wave, "", "Analog" },
        { "osc2Semi",  "Osc 2 Semi",    PType::Int,  -24, 24, 0, nullptr, &Params::osc2Semi, "st", "Analog" },
        { "osc2Fine",  "Osc 2 Fine",    PType::Float,-50, 50, 0, nullptr, &Params::osc2Fine, "ct", "Analog" },
        { "oscMix",    "Osc Mix",       PType::Float,  0, 1, 0, nullptr, &Params::oscMix, "", "Analog" },
        { "subLevel",  "Sub",           PType::Float,  0, 1, 0, nullptr, &Params::subLevel, "", "Analog" },
        { "noiseLevel","Noise",         PType::Float,  0, 1, 0, nullptr, &Params::noiseLevel, "", "Analog" },
        { "unison",    "Unison",        PType::Float,  0, 1, 0, nullptr, &Params::unison, "", "Analog" },

        { "epTone",    "EP Tone",       PType::Float,  0, 1, 0, nullptr, &Params::epTone, "", "E-Piano" },
        { "epTine",    "EP Tine",       PType::Float,  0, 1, 0, nullptr, &Params::epTine, "", "E-Piano" },
        { "epBark",    "EP Bark",       PType::Float,  0, 1, 0, nullptr, &Params::epBark, "", "E-Piano" },

        { "orgLow",    "Drawbar Low",   PType::Float,  0, 1, 0, nullptr, &Params::orgLow, "", "Organ" },
        { "orgMid",    "Drawbar Mid",   PType::Float,  0, 1, 0, nullptr, &Params::orgMid, "", "Organ" },
        { "orgHigh",   "Drawbar High",  PType::Float,  0, 1, 0, nullptr, &Params::orgHigh, "", "Organ" },
        { "orgPerc",   "Percussion",    PType::Float,  0, 1, 0, nullptr, &Params::orgPerc, "", "Organ" },
        { "orgClick",  "Key Click",     PType::Float,  0, 1, 0, nullptr, &Params::orgClick, "", "Organ" },

        { "cutoff",    "Cutoff",        PType::Float, 30, 18000, 1200, nullptr, &Params::cutoff, "Hz", "Filter" },
        { "resonance", "Resonance",     PType::Float,  0, 1, 0, nullptr, &Params::resonance, "", "Filter" },
        { "filterEnv", "Env Amount",    PType::Float, -1, 1, 0, nullptr, &Params::filterEnv, "", "Filter" },
        { "keyTrack",  "Key Track",     PType::Float,  0, 1, 0, nullptr, &Params::keyTrack, "", "Filter" },
        { "fltA",      "Flt Attack",    PType::Float, 0.001f, 5, 0.3f, nullptr, &Params::fltA, "s", "Filter" },
        { "fltD",      "Flt Decay",     PType::Float, 0.01f, 8, 0.6f, nullptr, &Params::fltD, "s", "Filter" },
        { "fltS",      "Flt Sustain",   PType::Float,  0, 1, 0, nullptr, &Params::fltS, "", "Filter" },
        { "fltR",      "Flt Release",   PType::Float, 0.01f, 8, 0.6f, nullptr, &Params::fltR, "s", "Filter" },

        { "ampA",      "Attack",        PType::Float, 0.001f, 5, 0.3f, nullptr, &Params::ampA, "s", "Amp" },
        { "ampD",      "Decay",         PType::Float, 0.01f, 10, 0.8f, nullptr, &Params::ampD, "s", "Amp" },
        { "ampS",      "Sustain",       PType::Float,  0, 1, 0, nullptr, &Params::ampS, "", "Amp" },
        { "ampR",      "Release",       PType::Float, 0.01f, 8, 0.6f, nullptr, &Params::ampR, "s", "Amp" },
        { "velSens",   "Velocity",      PType::Float,  0, 1, 0, nullptr, &Params::velSens, "", "Amp" },

        { "lfoRate",   "LFO Rate",      PType::Float, 0.02f, 12, 1.0f, nullptr, &Params::lfoRate, "Hz", "LFO" },
        { "lfoFilter", "LFO > Filter",  PType::Float,  0, 1, 0, nullptr, &Params::lfoFilter, "", "LFO" },
        { "lfoPitch",  "LFO > Pitch",   PType::Float,  0, 1, 0, nullptr, &Params::lfoPitch, "", "LFO" },

        { "chordMode", "Chord",         PType::Choice, 0, 8, 0, &chordNames(), &Params::chordMode, "", "Play" },
        { "mono",      "Voice Mode",    PType::Choice, 0, 1, 0, &voiceModeNames(), &Params::mono, "", "Play" },
        { "glide",     "Glide",         PType::Float,  0, 1, 0.15f, nullptr, &Params::glide, "s", "Play" },

        { "drive",     "Warmth",        PType::Float,  0, 1, 0, nullptr, &Params::drive, "", "FX" },
        { "chorus",    "Chorus",        PType::Float,  0, 1, 0, nullptr, &Params::chorus, "", "FX" },
        { "tremRate",  "Trem Rate",     PType::Float, 0.1f, 10, 3.0f, nullptr, &Params::tremRate, "Hz", "FX" },
        { "tremDepth", "Trem Depth",    PType::Float,  0, 1, 0, nullptr, &Params::tremDepth, "", "FX" },
        { "delayMix",  "Delay",         PType::Float,  0, 1, 0, nullptr, &Params::delayMix, "", "FX" },
        { "delayTime", "Delay Time",    PType::Choice, 0, 5, 0, &delayNames(), &Params::delayTime, "", "FX" },
        { "delayFb",   "Feedback",      PType::Float,  0, 0.95f, 0, nullptr, &Params::delayFb, "", "FX" },
        { "reverbMix", "Reverb",        PType::Float,  0, 1, 0, nullptr, &Params::reverbMix, "", "FX" },
        { "reverbSize","Room Size",     PType::Float,  0, 1, 0, nullptr, &Params::reverbSize, "", "FX" },
        { "dust",      "Vinyl Dust",    PType::Float,  0, 1, 0, nullptr, &Params::dust, "", "FX" },
        { "gain",      "Volume",        PType::Float, -36, 6, 0, nullptr, &Params::gain, "dB", "FX" },
    };
    return t;
}

// -----------------------------------------------------------------------------
//  Presets – Deep House (Kerri Chandler, Atjazz, Jimpster …) und Detroit Techno
// -----------------------------------------------------------------------------
struct Preset { std::string name; Params p; std::string category; };

inline std::vector<Preset> makePresets()
{
    std::vector<Preset> v;
    std::string category = "Deep & Soulful House";
    auto add = [&v, &category] (const char* n, Params p) { v.push_back ({ n, p, category }); };

    { // Kerri-Style: warmer Moll-9-Stab aus verstimmten Saws, schließender Filter
        Params p; p.engine = 0; p.osc1Wave = 0; p.osc2Wave = 0; p.osc2Semi = 0; p.osc2Fine = 9; p.oscMix = 0.5f;
        p.unison = 0.35f; p.subLevel = 0.0f;
        p.cutoff = 650; p.resonance = 0.22f; p.filterEnv = 0.42f; p.keyTrack = 0.4f;
        p.fltA = 0.001f; p.fltD = 0.32f; p.fltS = 0.12f; p.fltR = 0.25f;
        p.ampA = 0.002f; p.ampD = 0.9f; p.ampS = 0.0f; p.ampR = 0.25f; p.velSens = 0.55f;
        p.chordMode = 2; p.drive = 0.35f; p.chorus = 0.45f;
        p.delayMix = 0.22f; p.delayTime = 2; p.delayFb = 0.38f; p.reverbMix = 0.18f; p.reverbSize = 0.55f;
        p.gain = 4;
        add ("Chandler Stab m9", p);
    }
    { // Velvet Rhodes – Allround-E-Piano mit Tremolo
        Params p; p.engine = 1; p.epTone = 0.42f; p.epTine = 0.45f; p.epBark = 0.35f;
        p.cutoff = 9000; p.resonance = 0.0f; p.filterEnv = 0.0f; p.keyTrack = 0.0f;
        p.ampA = 0.001f; p.ampD = 3.5f; p.ampS = 0.0f; p.ampR = 0.35f; p.velSens = 0.7f;
        p.drive = 0.25f; p.chorus = 0.25f; p.tremRate = 4.2f; p.tremDepth = 0.35f;
        p.reverbMix = 0.2f; p.reverbSize = 0.55f; p.gain = -12.2;
        add ("Velvet Rhodes", p);
    }
    { // Atjazz-Style: jazzige m11-Rhodes-Akkorde, weich & breit
        Params p; p.engine = 1; p.epTone = 0.35f; p.epTine = 0.3f; p.epBark = 0.2f;
        p.cutoff = 5200; p.resonance = 0.05f; p.keyTrack = 0.3f; p.filterEnv = 0.0f;
        p.ampA = 0.002f; p.ampD = 4.0f; p.ampS = 0.0f; p.ampR = 0.6f; p.velSens = 0.6f;
        p.chordMode = 3; p.drive = 0.2f; p.chorus = 0.55f; p.tremRate = 3.2f; p.tremDepth = 0.2f;
        p.delayMix = 0.15f; p.delayTime = 4; p.delayFb = 0.3f; p.reverbMix = 0.3f; p.reverbSize = 0.7f;
        p.gain = -3.7;
        add ("Soul Keys m11", p);
    }
    { // Dusty Keys – Lo-Fi Rhodes mit Vinyl-Knistern, Dur 9
        Params p; p.engine = 1; p.epTone = 0.55f; p.epTine = 0.35f; p.epBark = 0.5f;
        p.cutoff = 2600; p.resonance = 0.1f; p.keyTrack = 0.5f;
        p.ampA = 0.001f; p.ampD = 3.0f; p.ampS = 0.0f; p.ampR = 0.3f;
        p.chordMode = 5; p.drive = 0.55f; p.chorus = 0.35f; p.tremRate = 5.0f; p.tremDepth = 0.15f;
        p.reverbMix = 0.22f; p.reverbSize = 0.5f; p.dust = 0.45f; p.gain = -5.9;
        add ("Dusty Keys maj9", p);
    }
    { // Der berühmte M1-Orgelbass – perkussiv, rund, mono
        Params p; p.engine = 2; p.octave = -1;
        p.orgLow = 1.0f; p.orgMid = 0.35f; p.orgHigh = 0.0f; p.orgPerc = 0.75f; p.orgClick = 0.45f;
        p.cutoff = 2200; p.resonance = 0.1f; p.filterEnv = 0.25f; p.fltA = 0.001f; p.fltD = 0.18f; p.fltS = 0.2f;
        p.keyTrack = 0.3f;
        p.ampA = 0.001f; p.ampD = 0.5f; p.ampS = 0.55f; p.ampR = 0.07f; p.velSens = 0.5f;
        p.mono = 1; p.glide = 0.0f; p.drive = 0.3f; p.chorus = 0.0f;
        p.reverbMix = 0.0f; p.gain = -2;
        add ("M1 Organ Bass", p);
    }
    { // Gospel-Orgel für Soulful House
        Params p; p.engine = 2;
        p.orgLow = 0.85f; p.orgMid = 0.65f; p.orgHigh = 0.45f; p.orgPerc = 0.35f; p.orgClick = 0.3f;
        p.cutoff = 7000; p.resonance = 0.0f; p.filterEnv = 0.0f;
        p.ampA = 0.004f; p.ampD = 1.0f; p.ampS = 1.0f; p.ampR = 0.12f;
        p.drive = 0.45f; p.chorus = 0.6f; p.tremRate = 6.2f; p.tremDepth = 0.25f;
        p.reverbMix = 0.22f; p.reverbSize = 0.6f; p.gain = -12;
        add ("Sunday Gospel Organ", p);
    }
    { // Orgel-Stab mit m7-Chord-Memory – NY-Garage-Style
        Params p; p.engine = 2;
        p.orgLow = 0.7f; p.orgMid = 0.6f; p.orgHigh = 0.3f; p.orgPerc = 0.8f; p.orgClick = 0.5f;
        p.cutoff = 3800; p.resonance = 0.15f; p.filterEnv = 0.2f; p.fltD = 0.2f; p.fltS = 0.3f;
        p.ampA = 0.001f; p.ampD = 0.35f; p.ampS = 0.0f; p.ampR = 0.15f;
        p.chordMode = 1; p.drive = 0.4f; p.chorus = 0.3f;
        p.delayMix = 0.2f; p.delayTime = 2; p.delayFb = 0.35f; p.reverbMix = 0.15f; p.gain = -1.1;
        add ("Garage Organ Stab m7", p);
    }
    { // Tiefer, sauberer Sub-Bass
        Params p; p.engine = 0; p.octave = -1; p.osc1Wave = 3; p.osc2Wave = 2; p.osc2Semi = 0; p.osc2Fine = 0;
        p.oscMix = 0.2f; p.subLevel = 0.35f; p.unison = 0.0f;
        p.cutoff = 380; p.resonance = 0.05f; p.filterEnv = 0.12f; p.fltD = 0.15f; p.fltS = 0.0f; p.keyTrack = 0.6f;
        p.ampA = 0.003f; p.ampD = 0.6f; p.ampS = 0.8f; p.ampR = 0.08f; p.velSens = 0.3f;
        p.mono = 1; p.glide = 0.035f; p.drive = 0.25f; p.chorus = 0.0f; p.reverbMix = 0.0f; p.gain = -2.5;
        add ("Deep Sub", p);
    }
    { // Warmer, gezupfter Analog-Bass
        Params p; p.engine = 0; p.octave = -1; p.osc1Wave = 0; p.osc2Wave = 1; p.osc2Semi = -12; p.osc2Fine = 0;
        p.oscMix = 0.4f; p.subLevel = 0.3f; p.unison = 0.05f;
        p.cutoff = 220; p.resonance = 0.35f; p.filterEnv = 0.45f; p.fltA = 0.001f; p.fltD = 0.22f; p.fltS = 0.05f;
        p.keyTrack = 0.5f;
        p.ampA = 0.001f; p.ampD = 0.45f; p.ampS = 0.45f; p.ampR = 0.08f; p.velSens = 0.5f;
        p.mono = 1; p.glide = 0.02f; p.drive = 0.4f; p.chorus = 0.0f; p.reverbMix = 0.0f; p.gain = 3.5;
        add ("Warm Pluck Bass", p);
    }
    { // Jimpster-Style: atmosphärisches Pad mit langsamer Filterbewegung
        Params p; p.engine = 0; p.osc1Wave = 0; p.osc2Wave = 0; p.osc2Semi = 12; p.osc2Fine = 6; p.oscMix = 0.35f;
        p.unison = 0.6f; p.noiseLevel = 0.03f;
        p.cutoff = 1500; p.resonance = 0.15f; p.filterEnv = 0.1f; p.fltA = 1.2f; p.fltD = 2.0f; p.fltS = 0.6f; p.fltR = 1.5f;
        p.keyTrack = 0.3f;
        p.ampA = 0.9f; p.ampD = 2.0f; p.ampS = 0.85f; p.ampR = 1.8f; p.velSens = 0.3f;
        p.lfoRate = 0.12f; p.lfoFilter = 0.3f; p.lfoPitch = 0.02f;
        p.drive = 0.2f; p.chorus = 0.75f; p.reverbMix = 0.45f; p.reverbSize = 0.85f; p.gain = -8.9;
        add ("Midnight Pad", p);
    }
    { // Breites String-Ensemble zum Andicken von Refrains
        Params p; p.engine = 0; p.osc1Wave = 0; p.osc2Wave = 0; p.osc2Semi = 0; p.osc2Fine = -12; p.oscMix = 0.5f;
        p.unison = 0.75f;
        p.cutoff = 4200; p.resonance = 0.05f; p.filterEnv = 0.05f; p.keyTrack = 0.5f;
        p.ampA = 0.6f; p.ampD = 1.0f; p.ampS = 0.9f; p.ampR = 1.2f; p.velSens = 0.4f;
        p.lfoRate = 5.0f; p.lfoPitch = 0.04f;
        p.drive = 0.1f; p.chorus = 0.85f; p.reverbMix = 0.4f; p.reverbSize = 0.75f; p.gain = -9.3;
        add ("Disco Strings", p);
    }
    { // Kurzer, jazziger Pluck mit punktiertem Delay
        Params p; p.engine = 0; p.osc1Wave = 1; p.osc2Wave = 0; p.osc2Semi = 12; p.osc2Fine = 4; p.oscMix = 0.3f;
        p.unison = 0.15f;
        p.cutoff = 900; p.resonance = 0.3f; p.filterEnv = 0.5f; p.fltA = 0.001f; p.fltD = 0.16f; p.fltS = 0.0f; p.keyTrack = 0.6f;
        p.ampA = 0.001f; p.ampD = 0.35f; p.ampS = 0.0f; p.ampR = 0.2f;
        p.drive = 0.25f; p.chorus = 0.4f; p.delayMix = 0.32f; p.delayTime = 4; p.delayFb = 0.45f;
        p.reverbMix = 0.3f; p.reverbSize = 0.65f; p.gain = 0;
        add ("Jazz Pluck", p);
    }
    { // Detroit-Stab: härter, resonanter m7-Akkord
        Params p; p.engine = 0; p.osc1Wave = 1; p.osc2Wave = 0; p.osc2Fine = 12; p.oscMix = 0.5f; p.unison = 0.2f;
        p.cutoff = 500; p.resonance = 0.5f; p.filterEnv = 0.55f; p.fltA = 0.001f; p.fltD = 0.2f; p.fltS = 0.05f; p.keyTrack = 0.4f;
        p.ampA = 0.001f; p.ampD = 0.4f; p.ampS = 0.0f; p.ampR = 0.2f;
        p.chordMode = 1; p.drive = 0.5f; p.chorus = 0.2f;
        p.delayMix = 0.25f; p.delayTime = 2; p.delayFb = 0.5f; p.reverbMix = 0.2f; p.gain = 2.2;
        add ("Detroit Stab m7", p);
    }
    { // Lange, offene Deep-Stack-Fläche mit Filter-Swell
        Params p; p.engine = 0; p.osc1Wave = 0; p.osc2Wave = 2; p.osc2Semi = 0; p.osc2Fine = 5; p.oscMix = 0.4f;
        p.unison = 0.45f;
        p.cutoff = 450; p.resonance = 0.25f; p.filterEnv = 0.35f; p.fltA = 0.8f; p.fltD = 2.5f; p.fltS = 0.3f; p.fltR = 1.2f;
        p.ampA = 0.25f; p.ampD = 2.0f; p.ampS = 0.7f; p.ampR = 1.5f; p.velSens = 0.4f;
        p.lfoRate = 0.2f; p.lfoFilter = 0.15f;
        p.chordMode = 8; p.drive = 0.3f; p.chorus = 0.6f; p.reverbMix = 0.4f; p.reverbSize = 0.8f;
        p.delayMix = 0.12f; p.delayTime = 5; p.delayFb = 0.35f; p.gain = 0.8;
        add ("Deep Stack Swell", p);
    }
    { // Sonniger Dur-9-Rhodes-Chord – Soulful Sunday
        Params p; p.engine = 1; p.epTone = 0.5f; p.epTine = 0.5f; p.epBark = 0.25f;
        p.cutoff = 7000; p.keyTrack = 0.2f;
        p.ampA = 0.001f; p.ampD = 3.5f; p.ampS = 0.0f; p.ampR = 0.5f;
        p.chordMode = 5; p.drive = 0.2f; p.chorus = 0.4f; p.tremRate = 4.6f; p.tremDepth = 0.3f;
        p.reverbMix = 0.28f; p.reverbSize = 0.65f; p.gain = -3.5;
        add ("Sunshine Rhodes maj9", p);
    }
    { // Schlicht: Init-Sound zum Selberbauen
        Params p; p.gain = -7; p.chorus = 0.0f; p.reverbMix = 0.0f; p.drive = 0.0f;
        category = "Basis";
        add ("Init", p);
    }

    // =========================================================================
    //  Detroit Techno – Juan Atkins, Derrick May, Underground Resistance,
    //  Jeff Mills, Carl Craig, Drexciya, Robert Hood
    //  (neue Presets werden hinten angehängt, damit gespeicherte Projekte stimmen)
    // =========================================================================
    category = "Detroit Techno";

    { // Hell angeschlagener Piano-Akkord à la Derrick May / Rhythim Is Rhythim
        Params p; p.engine = 1; p.epTone = 0.85f; p.epTine = 0.75f; p.epBark = 0.55f;
        p.cutoff = 9000; p.keyTrack = 0.2f;
        p.ampA = 0.001f; p.ampD = 1.4f; p.ampS = 0.0f; p.ampR = 0.25f; p.velSens = 0.5f;
        p.chordMode = 1; p.drive = 0.35f; p.chorus = 0.2f;
        p.delayMix = 0.2f; p.delayTime = 2; p.delayFb = 0.35f; p.reverbMix = 0.25f; p.reverbSize = 0.6f;
        p.gain = -5.4;
        add ("DT Piano Stab", p);
    }
    { // Kurze, helle Streicher-Stabs mit Akkord – Detroit-Klassiker der späten 80er
        Params p; p.engine = 0; p.osc1Wave = 0; p.osc2Wave = 0; p.osc2Semi = 12; p.osc2Fine = -8; p.oscMix = 0.4f;
        p.unison = 0.7f;
        p.cutoff = 2600; p.resonance = 0.08f; p.filterEnv = 0.25f; p.fltA = 0.005f; p.fltD = 0.3f; p.fltS = 0.4f; p.keyTrack = 0.5f;
        p.ampA = 0.012f; p.ampD = 0.45f; p.ampS = 0.35f; p.ampR = 0.35f; p.velSens = 0.5f;
        p.lfoRate = 5.5f; p.lfoPitch = 0.03f;
        p.chordMode = 1; p.drive = 0.2f; p.chorus = 0.8f; p.reverbMix = 0.3f; p.reverbSize = 0.7f;
        p.gain = 3.7;
        add ("DT Stab Strings m7", p);
    }
    { // Großes, gespieltes String-Ensemble für Läufe und Melodien
        Params p; p.engine = 0; p.osc1Wave = 0; p.osc2Wave = 0; p.osc2Semi = 0; p.osc2Fine = 14; p.oscMix = 0.5f;
        p.unison = 0.85f; p.noiseLevel = 0.02f;
        p.cutoff = 5200; p.resonance = 0.05f; p.filterEnv = 0.08f; p.keyTrack = 0.5f;
        p.ampA = 0.08f; p.ampD = 1.0f; p.ampS = 0.85f; p.ampR = 0.6f; p.velSens = 0.4f;
        p.lfoRate = 5.2f; p.lfoPitch = 0.05f;
        p.drive = 0.15f; p.chorus = 0.9f; p.reverbMix = 0.35f; p.reverbSize = 0.75f; p.gain = -8.7;
        add ("DT Motor City Strings", p);
    }
    { // UR-"Hi-Tech-Jazz"-Bläser: Saw-Brass mit Filter-Anstieg, dom9
        Params p; p.engine = 0; p.osc1Wave = 0; p.osc2Wave = 0; p.osc2Semi = 0; p.osc2Fine = 6; p.oscMix = 0.5f;
        p.unison = 0.25f;
        p.cutoff = 600; p.resonance = 0.12f; p.filterEnv = 0.55f; p.fltA = 0.04f; p.fltD = 0.45f; p.fltS = 0.35f; p.fltR = 0.2f;
        p.keyTrack = 0.5f;
        p.ampA = 0.015f; p.ampD = 0.6f; p.ampS = 0.6f; p.ampR = 0.18f; p.velSens = 0.6f;
        p.lfoRate = 5.0f; p.lfoPitch = 0.025f;
        p.chordMode = 6; p.drive = 0.45f; p.chorus = 0.35f; p.reverbMix = 0.22f; p.reverbSize = 0.6f;
        p.gain = 1.8;
        add ("DT Hi-Tech Brass dom9", p);
    }
    { // Minimaler Dub-Chord mit langem Delay-Echo (Robert Hood / Basic Channel)
        Params p; p.engine = 0; p.osc1Wave = 0; p.osc2Wave = 1; p.osc2Semi = 0; p.osc2Fine = 5; p.oscMix = 0.45f;
        p.unison = 0.2f;
        p.cutoff = 700; p.resonance = 0.3f; p.filterEnv = 0.35f; p.fltA = 0.001f; p.fltD = 0.15f; p.fltS = 0.05f; p.keyTrack = 0.4f;
        p.ampA = 0.001f; p.ampD = 0.25f; p.ampS = 0.0f; p.ampR = 0.12f;
        p.chordMode = 1; p.drive = 0.3f; p.chorus = 0.3f;
        p.delayMix = 0.45f; p.delayTime = 2; p.delayFb = 0.72f; p.reverbMix = 0.35f; p.reverbSize = 0.75f;
        p.gain = 5.1;
        add ("DT Dub Chord m7", p);
    }
    { // Hypnotischer, resonanter Loop-Pluck (Jeff Mills / Purpose Maker)
        Params p; p.engine = 0; p.osc1Wave = 1; p.osc2Wave = 0; p.osc2Semi = 7; p.osc2Fine = 0; p.oscMix = 0.25f;
        p.unison = 0.0f;
        p.cutoff = 900; p.resonance = 0.6f; p.filterEnv = 0.45f; p.fltA = 0.001f; p.fltD = 0.09f; p.fltS = 0.0f; p.keyTrack = 0.5f;
        p.ampA = 0.001f; p.ampD = 0.18f; p.ampS = 0.0f; p.ampR = 0.08f; p.velSens = 0.4f;
        p.lfoRate = 0.25f; p.lfoFilter = 0.25f;
        p.drive = 0.4f; p.chorus = 0.0f; p.delayMix = 0.2f; p.delayTime = 1; p.delayFb = 0.4f; p.reverbMix = 0.1f;
        p.gain = -0.8;
        add ("DT Hypnotic Pluck", p);
    }
    { // Kosmisches Pad mit wanderndem Filter (Carl Craig / Planet E)
        Params p; p.engine = 0; p.osc1Wave = 0; p.osc2Wave = 2; p.osc2Semi = 7; p.osc2Fine = 4; p.oscMix = 0.4f;
        p.unison = 0.7f; p.noiseLevel = 0.04f;
        p.cutoff = 900; p.resonance = 0.35f; p.filterEnv = 0.2f; p.fltA = 2.0f; p.fltD = 3.0f; p.fltS = 0.5f; p.fltR = 2.0f;
        p.keyTrack = 0.3f;
        p.ampA = 1.5f; p.ampD = 2.0f; p.ampS = 0.9f; p.ampR = 2.5f; p.velSens = 0.2f;
        p.lfoRate = 0.07f; p.lfoFilter = 0.4f; p.lfoPitch = 0.02f;
        p.chordMode = 8; p.drive = 0.25f; p.chorus = 0.8f;
        p.delayMix = 0.15f; p.delayTime = 5; p.delayFb = 0.5f; p.reverbMix = 0.55f; p.reverbSize = 0.92f;
        p.gain = -2.8;
        add ("DT Cosmic Pad", p);
    }
    { // Hohe Bleeps und Sequenzer-Melodien (Juan Atkins / Model 500)
        Params p; p.engine = 0; p.osc1Wave = 3; p.osc2Wave = 1; p.osc2Semi = 12; p.osc2Fine = 0; p.oscMix = 0.3f;
        p.unison = 0.0f;
        p.cutoff = 3500; p.resonance = 0.2f; p.filterEnv = 0.3f; p.fltA = 0.001f; p.fltD = 0.1f; p.fltS = 0.2f; p.keyTrack = 0.6f;
        p.ampA = 0.001f; p.ampD = 0.22f; p.ampS = 0.0f; p.ampR = 0.1f; p.velSens = 0.4f;
        p.drive = 0.2f; p.chorus = 0.2f; p.delayMix = 0.35f; p.delayTime = 4; p.delayFb = 0.5f; p.reverbMix = 0.2f;
        p.gain = 0.7;
        add ("DT Bleep Lead", p);
    }
    { // Electro-Bass mit knurrender Resonanz (Drexciya)
        Params p; p.engine = 0; p.octave = -1; p.osc1Wave = 1; p.osc2Wave = 0; p.osc2Semi = -12; p.osc2Fine = 0;
        p.oscMix = 0.35f; p.subLevel = 0.35f; p.unison = 0.0f;
        p.cutoff = 260; p.resonance = 0.55f; p.filterEnv = 0.55f; p.fltA = 0.001f; p.fltD = 0.16f; p.fltS = 0.1f; p.keyTrack = 0.5f;
        p.ampA = 0.001f; p.ampD = 0.3f; p.ampS = 0.6f; p.ampR = 0.06f; p.velSens = 0.5f;
        p.mono = 1; p.glide = 0.06f; p.drive = 0.6f; p.chorus = 0.0f; p.reverbMix = 0.0f;
        p.gain = 1.1;
        add ("DT Electro Bass", p);
    }
    { // Acid-Linie: Saw, hohe Resonanz, Slides – mit Mod-Wheel aufdrehen
        Params p; p.engine = 0; p.octave = -1; p.osc1Wave = 0; p.osc2Wave = 0; p.oscMix = 0.0f; p.unison = 0.0f;
        p.cutoff = 240; p.resonance = 0.8f; p.filterEnv = 0.6f; p.fltA = 0.001f; p.fltD = 0.2f; p.fltS = 0.0f; p.keyTrack = 0.3f;
        p.ampA = 0.001f; p.ampD = 0.3f; p.ampS = 0.7f; p.ampR = 0.05f; p.velSens = 0.6f;
        p.mono = 1; p.glide = 0.05f; p.drive = 0.55f; p.chorus = 0.0f;
        p.delayMix = 0.12f; p.delayTime = 2; p.delayFb = 0.3f; p.reverbMix = 0.0f;
        p.gain = 0.9;
        add ("DT Acid Line", p);
    }
    { // Tiefer, satter Rumpel-Bass unter der 909-Kick
        Params p; p.engine = 0; p.octave = -1; p.osc1Wave = 3; p.osc2Wave = 0; p.osc2Semi = 0; p.osc2Fine = 0;
        p.oscMix = 0.3f; p.subLevel = 0.5f; p.unison = 0.0f;
        p.cutoff = 300; p.resonance = 0.15f; p.filterEnv = 0.25f; p.fltA = 0.001f; p.fltD = 0.12f; p.fltS = 0.2f; p.keyTrack = 0.6f;
        p.ampA = 0.002f; p.ampD = 0.35f; p.ampS = 0.5f; p.ampR = 0.07f; p.velSens = 0.3f;
        p.mono = 1; p.glide = 0.0f; p.drive = 0.5f; p.chorus = 0.0f; p.reverbMix = 0.0f;
        p.gain = 1.6;
        add ("DT Rumble Bass", p);
    }
    { // Kalte, perkussive Roboter-Orgel mit 7sus4
        Params p; p.engine = 2;
        p.orgLow = 0.3f; p.orgMid = 0.7f; p.orgHigh = 0.8f; p.orgPerc = 1.0f; p.orgClick = 0.7f;
        p.cutoff = 3000; p.resonance = 0.35f; p.filterEnv = 0.3f; p.fltD = 0.12f; p.fltS = 0.2f;
        p.ampA = 0.001f; p.ampD = 0.2f; p.ampS = 0.0f; p.ampR = 0.1f;
        p.chordMode = 7; p.drive = 0.35f; p.chorus = 0.15f;
        p.delayMix = 0.3f; p.delayTime = 2; p.delayFb = 0.55f; p.reverbMix = 0.2f;
        p.gain = 2.4;
        add ("DT Robot Organ 7sus4", p);
    }
    return v;
}

// -----------------------------------------------------------------------------
//  Hilfsbausteine
// -----------------------------------------------------------------------------
struct SineTable
{
    static constexpr int N = 4096;
    float t[N + 1];
    SineTable() { for (int i = 0; i <= N; ++i) t[i] = std::sin (kTwoPi * (float) i / (float) N); }
    // phase 0..1 (beliebig, wird gewrappt)
    inline float operator() (float ph) const
    {
        ph -= std::floor (ph);
        float x = ph * (float) N;
        int i = (int) x;
        float f = x - (float) i;
        return t[i] + f * (t[i + 1] - t[i]);
    }
};
inline const SineTable& sineTab() { static SineTable s; return s; }

struct Rng
{
    uint32_t s = 0x12345678u;
    inline float next()   { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (float) (s & 0xFFFFFF) / 16777216.0f; } // 0..1
    inline float bipolar() { return next() * 2.0f - 1.0f; }
};

inline float polyBlep (float t, float dt)
{
    if (t < dt)        { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

inline float oscWave (int wave, float ph, float dt)
{
    switch (wave)
    {
        case 0: return 2.0f * ph - 1.0f - polyBlep (ph, dt);
        case 1: { float p2 = ph + 0.5f; p2 -= std::floor (p2);
                  float v = ph < 0.5f ? 1.0f : -1.0f;
                  return v + polyBlep (ph, dt) - polyBlep (p2, dt); }
        case 2: return 4.0f * std::fabs (ph - 0.5f) - 1.0f;
        default: return sineTab() (ph);
    }
}

struct ADSR
{
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    Stage stage = Idle;
    float value = 0, sr = 44100;
    float a = 0.01f, d = 0.3f, s = 0.7f, r = 0.3f;
    float aInc = 0.01f, dCoef = 0.99f, rCoef = 0.99f;

    void set (float A, float D, float S, float R)
    {
        a = A; d = D; s = S; r = R;
        aInc  = 1.0f / std::max (1.0f, a * sr);
        dCoef = std::exp (-4.6f / std::max (1.0f, d * sr));
        rCoef = std::exp (-4.6f / std::max (1.0f, r * sr));
    }
    void trigger() { stage = Attack; }
    void release() { if (stage != Idle) stage = Release; }
    void reset()   { stage = Idle; value = 0; }
    bool active() const { return stage != Idle; }

    inline float process()
    {
        switch (stage)
        {
            case Attack:
                value += aInc;
                if (value >= 1.0f) { value = 1.0f; stage = Decay; }
                break;
            case Decay: {
                value = s + (value - s) * dCoef;
                if (std::fabs (value - s) < 1.0e-4f) { value = s; stage = Sustain; }
                if (s <= 0.0f && value < 1.0e-4f) { value = 0; stage = Idle; }
                break; }
            case Sustain: value = s; if (s <= 0.0f) stage = Idle; break;
            case Release: {
                value *= rCoef;
                if (value < 1.0e-4f) { value = 0; stage = Idle; }
                break; }
            case Idle:
            default: value = 0; break;
        }
        return value;
    }
};

// 4-Pol Ladder-Tiefpass (ZDF/TPT, mit tanh-Sättigung im Feedback-Pfad)
inline float fastTanh (float x)
{
    x = std::clamp (x, -3.0f, 3.0f);
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

struct Ladder
{
    float s[4] { 0, 0, 0, 0 };
    void reset() { std::fill (s, s + 4, 0.0f); }
    inline float process (float x, float g, float k)
    {
        const float G = g / (1.0f + g);
        const float beta = 1.0f / (1.0f + g);
        const float G2 = G * G, G3 = G2 * G, G4 = G3 * G;
        const float S = G3 * beta * s[0] + G2 * beta * s[1] + G * beta * s[2] + beta * s[3];
        float y4 = (G4 * x + S) / (1.0f + k * G4);
        float u = fastTanh (x - k * y4);
        float in = u;
        for (int i = 0; i < 4; ++i)
        {
            float v = (in - s[i]) * G;
            float y = v + s[i];
            s[i] = y + v;
            in = y;
        }
        return in;
    }
};

struct OnePole
{
    float z = 0;
    inline float lp (float x, float c) { z += c * (x - z); return z; }
    static float coef (float hz, float sr) { return 1.0f - std::exp (-kTwoPi * hz / sr); }
};

// -----------------------------------------------------------------------------
//  Voice
// -----------------------------------------------------------------------------
struct Voice
{
    bool  active = false;
    int   note = 60;          // tatsächlich klingende Note
    int   trigger = -1;       // gedrückte Taste (für Chord Memory)
    int   chordIdx = 0;
    float vel = 1;
    uint64_t age = 0;
    bool  released = false;

    float freq = 261.6f, targetFreq = 261.6f;
    float ph1a = 0, ph1b = 0, ph2a = 0, ph2b = 0, phSub = 0;
    float phC = 0, phM = 0;                // FM
    float phO[9] {};                      // Orgel
    float modEnv = 0, tineEnv = 0, percEnv = 0, clickEnv = 0;
    float pan = 0;
    float epDecayScale = 1;
    float chordGainCache = 1;
    float fltG = 0.5f;
    uint32_t ctrl = 0;

    ADSR amp, flt;
    Ladder ladL, ladR;
    OnePole clickLP;
    Rng rng;
};

// -----------------------------------------------------------------------------
//  Effekte
// -----------------------------------------------------------------------------
struct DelayLine
{
    std::vector<float> buf; int w = 0;
    void init (int n) { buf.assign ((size_t) std::max (4, n), 0.0f); w = 0; }
    void clear() { std::fill (buf.begin(), buf.end(), 0.0f); }
    inline void push (float x) { buf[(size_t) w] = x; if (++w >= (int) buf.size()) w = 0; }
    inline float read (float delaySamples) const   // linear interpoliert
    {
        const int n = (int) buf.size();
        float rp = (float) w - delaySamples - 1.0f;
        while (rp < 0) rp += (float) n;
        int i0 = (int) rp; float f = rp - (float) i0;
        int i1 = i0 + 1; if (i1 >= n) i1 = 0;
        return buf[(size_t) i0] + f * (buf[(size_t) i1] - buf[(size_t) i0]);
    }
};

struct Comb
{
    std::vector<float> b; int i = 0; float store = 0;
    void init (int n) { b.assign ((size_t) std::max (1, n), 0.0f); i = 0; store = 0; }
    inline float process (float x, float fb, float damp)
    {
        float y = b[(size_t) i];
        store = y * (1.0f - damp) + store * damp;
        b[(size_t) i] = x + store * fb;
        if (++i >= (int) b.size()) i = 0;
        return y;
    }
};
struct Allpass
{
    std::vector<float> b; int i = 0;
    void init (int n) { b.assign ((size_t) std::max (1, n), 0.0f); i = 0; }
    inline float process (float x)
    {
        float bo = b[(size_t) i];
        float y = -x + bo;
        b[(size_t) i] = x + bo * 0.5f;
        if (++i >= (int) b.size()) i = 0;
        return y;
    }
};

struct Reverb   // Freeverb-artiger Hall
{
    Comb cL[8], cR[8]; Allpass aL[4], aR[4];
    float hpL = 0, hpR = 0;
    void init (float sr)
    {
        const int ct[8] { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
        const int at[4] { 556, 441, 341, 225 };
        const float sc = sr / 44100.0f;
        for (int k = 0; k < 8; ++k) { cL[k].init ((int) (ct[k] * sc)); cR[k].init ((int) ((ct[k] + 23) * sc)); }
        for (int k = 0; k < 4; ++k) { aL[k].init ((int) (at[k] * sc)); aR[k].init ((int) ((at[k] + 23) * sc)); }
    }
    inline void process (float inL, float inR, float size, float& outL, float& outR, float hpC)
    {
        // Hochpass vor dem Hall: hält den Bass sauber
        hpL += hpC * (inL - hpL); hpR += hpC * (inR - hpR);
        float x = ((inL - hpL) + (inR - hpR)) * 0.015f;
        const float fb = 0.70f + size * 0.27f, damp = 0.35f;
        float l = 0, r = 0;
        for (int k = 0; k < 8; ++k) { l += cL[k].process (x, fb, damp); r += cR[k].process (x, fb, damp); }
        for (int k = 0; k < 4; ++k) { l = aL[k].process (l); r = aR[k].process (r); }
        outL = l; outR = r;
    }
};

// -----------------------------------------------------------------------------
//  Engine
// -----------------------------------------------------------------------------
class Engine
{
public:
    static constexpr int kMaxVoices = 32;

    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        for (auto& v : voices) { v = Voice(); v.amp.sr = sr; v.flt.sr = sr; v.rng.s = 0x9E3779B9u ^ (uint32_t) (&v - voices.data()) * 2654435761u; }
        chorusL.init ((int) (0.05f * sr)); chorusR.init ((int) (0.05f * sr));
        delL.init ((int) (4.0f * sr)); delR.init ((int) (4.0f * sr));
        reverb.init (sr);
        curDelaySamples = 0.5f * sr;
        smoothCutoff = p.cutoff;
        smoothGain = dbToGain (p.gain);
    }

    void setParams (const Params& np) { p = np; }
    const Params& params() const { return p; }
    void setTempo (double b) { if (b > 20.0 && b < 400.0) bpm = (float) b; }
    void pitchBend (float v) { bend = std::clamp (v, -1.0f, 1.0f); }
    void modWheel (float v)  { mod = std::clamp (v, 0.0f, 1.0f); }
    void sustainPedal (bool down)
    {
        sustain = down;
        if (! down)
            for (int n : sustainedNotes) releaseTrigger (n);
        if (! down) sustainedNotes.clear();
    }

    void allNotesOff()
    {
        for (auto& v : voices) { v.amp.release(); v.flt.release(); v.released = true; }
        monoStack.clear(); sustainedNotes.clear();
    }

    void noteOn (int key, float velocity)
    {
        velocity = std::clamp (velocity, 0.0f, 1.0f);
        const auto& iv = chordIntervals ((int) p.chordMode);

        if (p.mono > 0.5f)
        {
            monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), key), monoStack.end());
            monoStack.push_back (key);
            bool legato = false;
            for (size_t i = 0; i < iv.size() && i < (size_t) kMaxVoices; ++i)
                if (voices[i].active && ! voices[i].released) legato = true;

            for (size_t i = 0; i < (size_t) kMaxVoices; ++i)
            {
                auto& v = voices[i];
                if (i < iv.size())
                {
                    int n = key + iv[i];
                    if (legato && v.active && ! v.released) { v.note = n; v.targetFreq = noteToHz (n); v.trigger = key; }
                    else { bool hadFreq = v.active; startVoice (v, n, key, (int) i, velocity, (int) iv.size());
                           if (hadFreq && p.glide > 0.001f) { /* gleitet vom alten Ton */ } else v.freq = v.targetFreq; }
                }
                else if (v.active) { v.amp.release(); v.flt.release(); v.released = true; }
            }
            return;
        }

        for (size_t i = 0; i < iv.size(); ++i)
        {
            Voice& v = allocVoice();
            startVoice (v, key + iv[i], key, (int) i, velocity, (int) iv.size());
            v.freq = v.targetFreq;
        }
    }

    void noteOff (int key)
    {
        if (p.mono > 0.5f)
        {
            monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), key), monoStack.end());
            if (! monoStack.empty())
            {
                int k2 = monoStack.back();
                const auto& iv = chordIntervals ((int) p.chordMode);
                for (size_t i = 0; i < iv.size() && i < (size_t) kMaxVoices; ++i)
                    if (voices[i].active && ! voices[i].released)
                    { voices[i].note = k2 + iv[i]; voices[i].targetFreq = noteToHz (k2 + iv[i]); voices[i].trigger = k2; }
                return;
            }
        }
        if (sustain) { sustainedNotes.push_back (key); return; }
        releaseTrigger (key);
    }

    // Rendert n Samples (überschreibt L/R)
    void render (float* L, float* R, int n)
    {
        const int engine = (int) p.engine;
        const float lfoInc = p.lfoRate / sr;
        const float glideC = p.glide > 0.001f ? std::exp (-1.0f / (p.glide * 0.35f * sr)) : 0.0f;
        const float bendSemis = bend * 2.0f;
        const float res = p.resonance * 3.9f;
        const float cutC = OnePole::coef (30.0f, sr);
        const float gainTarget = dbToGain (p.gain);

        // Orgel-Zugriegel: 16' 8' 5⅓' 4' 2⅔' 2' 1⅗' 1⅓' 1'
        static const float orgRatio[9] { 0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f };
        float orgLvl[9] { p.orgLow * 0.8f, p.orgLow, p.orgMid * 0.6f, p.orgMid, p.orgMid * 0.7f,
                          p.orgHigh, p.orgHigh * 0.5f, p.orgHigh * 0.6f, p.orgHigh * 0.5f };
        float orgNorm = 0; for (float l : orgLvl) orgNorm += l;
        orgNorm = 1.0f / std::max (1.0f, orgNorm * 0.7f);

        for (auto& v : voices)
            if (v.active)
            {
                v.amp.set (p.ampA, engine == 1 ? p.ampD * v.epDecayScale : p.ampD, p.ampS, p.ampR);
                v.flt.set (p.fltA, p.fltD, p.fltS, p.fltR);
            }

        // Delay-Zeit
        const float targetDelay = std::min (3.9f * sr, delayBeats ((int) p.delayTime) * 60.0f / bpm * sr);
        const float dlyFbLP = OnePole::coef (3200.0f, sr), dlyFbHP = OnePole::coef (180.0f, sr);
        const float revHP = OnePole::coef (220.0f, sr);
        const float tremInc = p.tremRate / sr;
        const float driveK = 1.0f + p.drive * 4.0f;
        const float driveNorm = 0.5f / std::tanh (driveK * 0.5f);
        const float driveBias = 0.15f * p.drive;
        const float driveLP = OnePole::coef (18000.0f - p.drive * 9000.0f, sr);

        for (int s = 0; s < n; ++s)
        {
            lfoPh += lfoInc; if (lfoPh >= 1.0f) lfoPh -= 1.0f;
            const float lfo = sineTab() (lfoPh);
            smoothCutoff += cutC * (p.cutoff - smoothCutoff);
            const float pitchMod = std::exp2 ((bendSemis + lfo * p.lfoPitch * 0.6f) / 12.0f);

            float mixL = 0, mixR = 0;
            for (auto& v : voices)
            {
                if (! v.active) continue;

                if (glideC > 0.0f) v.freq = v.targetFreq + (v.freq - v.targetFreq) * glideC;
                else v.freq = v.targetFreq;
                const float f = v.freq * pitchMod;
                const float dt = std::min (0.45f, f / sr);

                const float a = v.amp.process();
                const float fe = v.flt.process();
                if (! v.amp.active()) { v.active = false; continue; }

                float sigL = 0, sigR = 0;
                if (engine == 0)          // ---------- Analog
                {
                    const float det = p.unison * 0.012f;          // bis ±0.7 % (≈ 20 ct)
                    const float d1a = dt * (1.0f + det), d1b = dt * (1.0f - det);
                    const float r2 = std::exp2 ((p.osc2Semi + p.osc2Fine * 0.01f) / 12.0f);
                    const float d2a = std::min (0.45f, dt * r2 * (1.0f - det * 0.7f));
                    const float d2b = std::min (0.45f, dt * r2 * (1.0f + det * 0.7f));
                    adv (v.ph1a, d1a); adv (v.ph1b, d1b); adv (v.ph2a, d2a); adv (v.ph2b, d2b); adv (v.phSub, dt * 0.5f);
                    const int w1 = (int) p.osc1Wave, w2 = (int) p.osc2Wave;
                    const float o1a = oscWave (w1, v.ph1a, d1a), o1b = oscWave (w1, v.ph1b, d1b);
                    const float o2a = oscWave (w2, v.ph2a, d2a), o2b = oscWave (w2, v.ph2b, d2b);
                    const float m1 = 1.0f - p.oscMix, m2 = p.oscMix;
                    const float sub = sineTab() (v.phSub) * p.subLevel;
                    const float nz = v.rng.bipolar() * p.noiseLevel * 0.5f;
                    const float spread = std::min (0.6f, p.unison * 1.2f);
                    const float wA = 0.5f + 0.5f * spread, wB = 0.5f - 0.5f * spread;
                    sigL = (m1 * (o1a * wA + o1b * wB) + m2 * (o2a * wB + o2b * wA)) + sub + nz;
                    sigR = (m1 * (o1a * wB + o1b * wA) + m2 * (o2a * wA + o2b * wB)) + sub + nz;
                    sigL *= 0.6f; sigR *= 0.6f;
                }
                else if (engine == 1)     // ---------- FM-Rhodes
                {
                    adv (v.phC, dt); adv (v.phM, dt);
                    v.modEnv *= modEnvC; v.tineEnv *= tineEnvC;
                    const float idx = p.epTone * 5.0f * (0.25f + 0.75f * v.vel) * (0.2f + 0.8f * v.modEnv);
                    float x = sineTab() (v.phC + idx * sineTab() (v.phM) / kTwoPi);
                    // Tine: hoher, schnell verklingender Anschlag
                    x += p.epTine * v.vel * 0.45f * v.tineEnv * sineTab() (v.phC * 14.0f);
                    // Bark: asymmetrische Sättigung, anschlagsabhängig
                    const float k = 1.0f + p.epBark * v.vel * 3.5f;
                    const float bias = 0.25f * p.epBark * v.vel;
                    x = (fastTanh (x * k + bias) - fastTanh (bias)) / fastTanh (k);
                    sigL = sigR = x * 0.7f;
                }
                else                      // ---------- Orgel
                {
                    float x = 0;
                    for (int h = 0; h < 9; ++h)
                    {
                        const float dh = dt * orgRatio[h];
                        if (dh >= 0.48f) continue;
                        adv (v.phO[h], dh);
                        x += orgLvl[h] * sineTab() (v.phO[h]);
                    }
                    x *= orgNorm;
                    v.percEnv *= percEnvC;
                    x += p.orgPerc * v.percEnv * (0.7f * sineTab() (v.phO[3]) + 0.5f * sineTab() (v.phO[4]));
                    v.clickEnv *= clickEnvC;
                    if (v.clickEnv > 1.0e-4f)
                    {
                        float c = v.rng.bipolar();
                        c = c - v.clickLP.lp (c, 0.3f);
                        x += c * v.clickEnv * p.orgClick * 0.35f;
                    }
                    sigL = sigR = x * 0.7f;
                }

                // Filter
                if ((v.ctrl++ & 7u) == 0u)   // Filter-Koeffizient im Kontroll-Takt (alle 8 Samples)
                {
                    float fc = smoothCutoff
                             * std::exp2 (p.keyTrack * (float) (v.note - 60) / 12.0f
                                          + p.filterEnv * fe * (0.5f + 0.5f * v.vel) * 7.0f
                                          + lfo * p.lfoFilter * 2.0f
                                          + mod * 3.5f);
                    fc = std::clamp (fc, 20.0f, sr * 0.45f);
                    v.fltG = std::tan (kPi * fc / sr);
                }
                const float g = v.fltG;
                const float comp = 1.0f + res * 0.45f;
                sigL = v.ladL.process (sigL * comp, g, res);
                sigR = engine == 0 ? v.ladR.process (sigR * comp, g, res) : sigL;   // E-Piano/Orgel sind mono

                const float velAmp = 1.0f - p.velSens + p.velSens * v.vel;
                const float gnL = a * velAmp * voiceGain * (1.0f - std::max (0.0f, v.pan));
                const float gnR = a * velAmp * voiceGain * (1.0f + std::min (0.0f, v.pan));
                mixL += sigL * gnL * v.chordGainCache;
                mixR += sigR * gnR * v.chordGainCache;
            }

            // ---------- Drive / Warmth
            if (p.drive > 0.001f)
            {
                float dl = (fastTanh (mixL * driveK + driveBias) - fastTanh (driveBias)) * driveNorm;
                float dr = (fastTanh (mixR * driveK + driveBias) - fastTanh (driveBias)) * driveNorm;
                mixL = mixL + p.drive * (driveLPL.lp (dl, driveLP) - mixL);
                mixR = mixR + p.drive * (driveLPR.lp (dr, driveLP) - mixR);
            }

            // ---------- Chorus (Juno-Style)
            chorusL.push (mixL); chorusR.push (mixR);
            if (p.chorus > 0.001f)
            {
                chPh += 0.55f / sr; if (chPh >= 1.0f) chPh -= 1.0f;
                const float tri = 4.0f * std::fabs (chPh - 0.5f) - 1.0f;
                const float base = 0.0065f * sr, depth = 0.0028f * sr;
                const float wl = chorusL.read (base + depth * tri);
                const float wr = chorusR.read (base - depth * tri);
                const float m = p.chorus * 0.75f;
                mixL = mixL * (1.0f - 0.3f * p.chorus) + wl * m;
                mixR = mixR * (1.0f - 0.3f * p.chorus) + wr * m;
            }

            // ---------- Tremolo / Auto-Pan
            if (p.tremDepth > 0.001f)
            {
                trPh += tremInc; if (trPh >= 1.0f) trPh -= 1.0f;
                const float t = sineTab() (trPh);
                mixL *= 1.0f - p.tremDepth * 0.5f * (1.0f + t);
                mixR *= 1.0f - p.tremDepth * 0.5f * (1.0f - t);
                const float makeup = 1.0f + p.tremDepth * 0.5f;
                mixL *= makeup; mixR *= makeup;
            }

            // ---------- Ping-Pong-Delay
            curDelaySamples += 0.0005f * (targetDelay - curDelaySamples);
            {
                const float dl = delL.read (curDelaySamples);
                const float dr = delR.read (curDelaySamples);
                float fbL = dlyLP_L.lp (dr, dlyFbLP); fbL -= dlyHP_L.lp (fbL, dlyFbHP);
                float fbR = dlyLP_R.lp (dl, dlyFbLP); fbR -= dlyHP_R.lp (fbR, dlyFbHP);
                delL.push ((mixL + mixR) * 0.5f * (p.delayMix > 0.001f ? 1.0f : 0.0f) + fbL * p.delayFb);
                delR.push (fbR * p.delayFb);
                mixL += dl * p.delayMix;
                mixR += dr * p.delayMix;
            }

            // ---------- Hall
            {
                float rl, rr;
                reverb.process (mixL, mixR, p.reverbSize, rl, rr, revHP);
                mixL = mixL * (1.0f - 0.35f * p.reverbMix) + rl * p.reverbMix * 1.6f;
                mixR = mixR * (1.0f - 0.35f * p.reverbMix) + rr * p.reverbMix * 1.6f;
            }

            // ---------- Vinyl Dust
            if (p.dust > 0.001f)
            {
                float crackle = 0;
                if (dustRng.next() < 0.00035f * p.dust * (44100.0f / sr) * 3.0f)
                    crackle = dustRng.bipolar() * (0.3f + 0.7f * dustRng.next());
                float c = crackle - dustHP.lp (crackle, 0.25f);
                float hiss = dustLP.lp (dustRng.bipolar(), 0.15f) * 0.012f;
                const float d = p.dust * 0.25f;
                mixL += (c + hiss) * d; mixR += (c * 0.8f + hiss) * d;
            }

            // ---------- Ausgang
            smoothGain += 0.001f * (gainTarget - smoothGain);
            mixL *= smoothGain; mixR *= smoothGain;
            mixL -= dcL.lp (mixL, OnePole::coef (12.0f, sr));
            mixR -= dcR.lp (mixR, OnePole::coef (12.0f, sr));
            L[s] = softClip (mixL);
            R[s] = softClip (mixR);
        }
    }

private:
    static inline void adv (float& ph, float d) { ph += d; if (ph >= 1.0f) ph -= 1.0f; }
    static inline float dbToGain (float db) { return std::pow (10.0f, db / 20.0f); }
    static inline float softClip (float x)
    {
        const float t = 0.85f;
        if (x > t)  return t + (1.0f - t) * std::tanh ((x - t) / (1.0f - t));
        if (x < -t) return -t - (1.0f - t) * std::tanh ((-x - t) / (1.0f - t));
        return x;
    }
    inline float noteToHz (int n) const
    {
        return 440.0f * std::exp2 ((float) (n + 12 * (int) p.octave - 69) / 12.0f);
    }

    Voice& allocVoice()
    {
        Voice* best = nullptr;
        for (auto& v : voices) if (! v.active) return v;
        // bevorzugt freigegebene, leiseste Stimme
        for (auto& v : voices)
            if (v.released && (best == nullptr || v.amp.value < best->amp.value)) best = &v;
        if (best) return *best;
        for (auto& v : voices) if (best == nullptr || v.age < best->age) best = &v;
        return *best;
    }

    void startVoice (Voice& v, int n, int key, int idx, float vel, int chordSize)
    {
        const bool wasActive = v.active;
        v.active = true; v.released = false;
        v.note = n; v.trigger = key; v.chordIdx = idx; v.vel = vel; v.age = ++ageCounter;
        v.targetFreq = noteToHz (n);
        v.chordGainCache = 1.0f / std::sqrt ((float) chordSize);
        // Akkordtöne leicht im Stereobild verteilen
        v.pan = chordSize > 1 ? ((float) idx / (float) (chordSize - 1) - 0.5f) * 0.5f : 0.0f;
        v.epDecayScale = std::clamp (std::exp2 (-(float) (n + 12 * (int) p.octave - 60) / 24.0f), 0.25f, 2.5f);
        if (! wasActive)
        {
            const float r = v.rng.next();
            v.ph1a = v.ph1b = r; v.ph2a = v.ph2b = v.rng.next(); v.phSub = 0;
            v.phC = v.phM = 0;
            for (auto& ph : v.phO) ph = v.rng.next();
            v.ladL.reset(); v.ladR.reset();
            v.amp.value = 0; v.flt.value = 0;
        }
        v.ctrl = 0;
        v.modEnv = 1.0f; v.tineEnv = 1.0f; v.percEnv = 1.0f; v.clickEnv = 1.0f;
        v.amp.trigger(); v.flt.trigger();
        modEnvC  = std::exp (-1.0f / (0.45f * sr));
        tineEnvC = std::exp (-1.0f / (0.018f * sr));
        percEnvC = std::exp (-1.0f / (0.16f * sr));
        clickEnvC = std::exp (-1.0f / (0.004f * sr));
    }

    void releaseTrigger (int key)
    {
        for (auto& v : voices)
            if (v.active && v.trigger == key && ! v.released)
            { v.amp.release(); v.flt.release(); v.released = true; }
    }

    Params p;
    float sr = 44100, bpm = 122;
    float bend = 0, mod = 0;
    bool sustain = false;
    std::vector<int> sustainedNotes, monoStack;
    uint64_t ageCounter = 0;
    float lfoPh = 0, chPh = 0, trPh = 0;
    float smoothCutoff = 8000, smoothGain = 0.5f;
    float modEnvC = 0.9999f, tineEnvC = 0.999f, percEnvC = 0.999f, clickEnvC = 0.99f;
    static constexpr float voiceGain = 0.84f;

    std::array<Voice, kMaxVoices> voices;
    DelayLine chorusL, chorusR, delL, delR;
    float curDelaySamples = 22050;
    OnePole dlyLP_L, dlyLP_R, dlyHP_L, dlyHP_R, driveLPL, driveLPR, dcL, dcR, dustHP, dustLP;
    Reverb reverb;
    Rng dustRng;
};

} // namespace abledeep

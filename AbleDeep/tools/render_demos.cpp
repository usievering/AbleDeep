// Offline-Renderer: erzeugt Hörproben aller AbleDeep-Presets (ohne JUCE)
//   g++ -O2 -std=c++17 -I../Source render_demos.cpp -o render_demos && ./render_demos out/
#include "AbleDeepEngine.h"
#include <cstdio>
#include <fstream>
#include <map>

using namespace abledeep;

static const float SR = 44100.0f;
static float BPM = 122.0f;
static float STEP = 60.0f / BPM / 4.0f; // 16tel in Sekunden
static void setBpm (float b) { BPM = b; STEP = 60.0f / b / 4.0f; }

struct Ev { float t, len; int note; float vel; };

static void writeWav (const std::string& path, const std::vector<float>& L, const std::vector<float>& R)
{
    std::ofstream f (path, std::ios::binary);
    const uint32_t n = (uint32_t) L.size(), dataBytes = n * 4, sr = (uint32_t) SR, br = sr * 4;
    const uint16_t ch = 2, bits = 16, ba = 4, fmt = 1; const uint32_t fmtLen = 16, riff = 36 + dataBytes;
    f.write ("RIFF", 4); f.write ((const char*) &riff, 4); f.write ("WAVEfmt ", 8);
    f.write ((const char*) &fmtLen, 4); f.write ((const char*) &fmt, 2); f.write ((const char*) &ch, 2);
    f.write ((const char*) &sr, 4); f.write ((const char*) &br, 4); f.write ((const char*) &ba, 2); f.write ((const char*) &bits, 2);
    f.write ("data", 4); f.write ((const char*) &dataBytes, 4);
    for (uint32_t i = 0; i < n; ++i)
    {
        int16_t a = (int16_t) std::lrint (std::clamp (L[i], -1.0f, 1.0f) * 32767.0f);
        int16_t b = (int16_t) std::lrint (std::clamp (R[i], -1.0f, 1.0f) * 32767.0f);
        f.write ((const char*) &a, 2); f.write ((const char*) &b, 2);
    }
}

static void renderEvents (const Params& p, const std::vector<Ev>& evs, float seconds,
                          std::vector<float>& L, std::vector<float>& R)
{
    Engine e; e.prepare (SR); e.setParams (p); e.setTempo (BPM);
    const int total = (int) (seconds * SR);
    L.assign ((size_t) total, 0); R.assign ((size_t) total, 0);
    struct M { int s; bool on; int note; float vel; };
    std::vector<M> ms;
    for (auto& ev : evs) { ms.push_back ({ (int) (ev.t * SR), true, ev.note, ev.vel });
                           ms.push_back ({ (int) ((ev.t + ev.len) * SR), false, ev.note, 0 }); }
    std::stable_sort (ms.begin(), ms.end(), [] (const M& a, const M& b) { return a.s < b.s || (a.s == b.s && ! a.on && b.on); });
    size_t mi = 0; int pos = 0; const int block = 64;
    while (pos < total)
    {
        while (mi < ms.size() && ms[mi].s <= pos)
        { if (ms[mi].on) e.noteOn (ms[mi].note, ms[mi].vel); else e.noteOff (ms[mi].note); ++mi; }
        int n = std::min (block, total - pos);
        e.render (&L[(size_t) pos], &R[(size_t) pos], n);
        pos += n;
    }
}

// ----- Muster -----------------------------------------------------------------
static std::vector<Ev> stabPattern (int bars)
{   // Synkopierte Akkord-Stabs (ein Ton pro Anschlag -> Chord Memory)
    std::vector<Ev> v; const int roots[4] { 57, 62, 60, 55 };
    const int steps[5] { 0, 3, 6, 10, 13 }; const float vel[5] { 0.95f, 0.7f, 0.85f, 0.75f, 0.6f };
    for (int b = 0; b < bars; ++b)
        for (int i = 0; i < 5; ++i)
            v.push_back ({ (b * 16 + steps[i]) * STEP, 1.6f * STEP, roots[b % 4], vel[i] });
    return v;
}
static std::vector<Ev> chordHoldPattern (int bars, bool singleKey)
{
    std::vector<Ev> v;
    const std::vector<std::vector<int>> voic {
        { 45, 55, 60, 64, 67, 71 },  // Am9
        { 50, 60, 65, 69, 72, 76 },  // Dm9
        { 48, 55, 59, 62, 64, 67 },  // Cmaj9
        { 43, 53, 57, 59, 62, 64 },  // G9sus
    };
    const int roots[4] { 57, 62, 60, 55 };
    for (int b = 0; b < bars; ++b)
    {
        const float t0 = b * 16 * STEP;
        const int hits[3] { 0, 7, 10 }; const float lens[3] { 6.5f, 2.5f, 5.5f };
        for (int h = 0; h < 3; ++h)
        {
            float vel = h == 0 ? 0.9f : 0.65f;
            if (singleKey) v.push_back ({ t0 + hits[h] * STEP, lens[h] * STEP, roots[b % 4], vel });
            else for (int n : voic[(size_t) b % 4]) v.push_back ({ t0 + hits[h] * STEP + 0.004f * (n % 5), lens[h] * STEP, n, vel });
        }
    }
    return v;
}
static std::vector<Ev> padPattern (int bars, bool singleKey)
{
    std::vector<Ev> v;
    const std::vector<std::vector<int>> voic { { 57, 60, 64, 67, 71 }, { 53, 57, 60, 64, 67 }, { 55, 59, 62, 64, 67 }, { 52, 55, 59, 62, 66 } };
    const int roots[4] { 57, 53, 55, 52 };
    for (int b = 0; b < bars; b += 2)
    {
        const float t0 = b * 16 * STEP;
        if (singleKey) v.push_back ({ t0, 31.0f * STEP, roots[(b / 2) % 4], 0.8f });
        else for (int n : voic[(size_t) (b / 2) % 4]) v.push_back ({ t0, 31.0f * STEP, n, 0.8f });
    }
    return v;
}
static std::vector<Ev> bassPattern (int bars)
{
    std::vector<Ev> v;
    struct B { int step; int off; float len; float vel; };
    const B pat[] { { 0, 0, 2.5f, 1.0f }, { 3, 0, 0.8f, 0.7f }, { 6, 7, 1.0f, 0.85f }, { 8, 12, 0.8f, 0.6f },
                    { 10, 0, 2.0f, 0.9f }, { 13, 3, 0.8f, 0.7f }, { 14, 5, 1.6f, 0.75f } };
    const int roots[4] { 45, 50, 48, 43 };
    for (int b = 0; b < bars; ++b)
        for (auto& x : pat) v.push_back ({ (b * 16 + x.step) * STEP, x.len * STEP, roots[b % 4] + x.off, x.vel });
    return v;
}
static std::vector<Ev> melodyPattern (int bars)
{
    std::vector<Ev> v;
    const int mel[16] { 69, -1, 72, 76, -1, 74, 72, -1, 69, -1, 67, 69, -1, 72, -1, 64 };
    for (int b = 0; b < bars; ++b)
        for (int i = 0; i < 16; ++i) if (mel[i] > 0 && ((i + b) % 3 != 2 || i == 0))
            v.push_back ({ (b * 16 + i) * STEP, 1.2f * STEP, mel[i] - (b % 2 ? 2 : 0), 0.6f + 0.3f * (i % 4 == 0) });
    return v;
}


// ----- Detroit-Techno-Muster (130 BPM) -----------------------------------------
static std::vector<Ev> technoStabs (int bars)
{   // Stabs auf Offbeats + Synkope, Akkord wechselt alle 2 Takte (Am / Fm)
    std::vector<Ev> v; const int roots[2] { 57, 53 };
    const int steps[4] { 2, 6, 10, 13 }; const float vel[4] { 0.9f, 0.75f, 0.85f, 0.65f };
    for (int b = 0; b < bars; ++b)
        for (int i = 0; i < 4; ++i)
            v.push_back ({ (b * 16 + steps[i]) * STEP, 1.2f * STEP, roots[(b / 2) % 2], vel[i] });
    return v;
}
static std::vector<Ev> technoSeq (int bars)
{   // hypnotische 16tel-Sequenz
    std::vector<Ev> v; const int seq[16] { 69, -1, 72, 69, -1, 76, -1, 72, 69, -1, 74, -1, 72, 69, -1, 67 };
    for (int b = 0; b < bars; ++b)
        for (int i = 0; i < 16; ++i) if (seq[i] > 0)
            v.push_back ({ (b * 16 + i) * STEP, 0.6f * STEP, seq[i] - (b % 4 == 3 ? 2 : 0), i % 4 == 0 ? 0.9f : 0.6f });
    return v;
}
static std::vector<Ev> technoBass (int bars, bool acid)
{
    std::vector<Ev> v;
    if (acid)
    {   // 303-artig, überlappende Noten = Slide
        const int n[16] { 45, 45, 57, 45, 48, 45, 55, 45, 45, 57, 45, 52, 45, 50, 57, 45 };
        const bool slide[16] { 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0 };
        const bool accent[16] { 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0 };
        for (int b = 0; b < bars; ++b)
            for (int i = 0; i < 16; ++i)
                v.push_back ({ (b * 16 + i) * STEP, (slide[i] ? 1.15f : 0.55f) * STEP, n[i], accent[i] ? 1.0f : 0.6f });
        return v;
    }
    // Electro/Rumble: Off-Beat-Bass zwischen den Kicks
    const int st[6] { 2, 6, 7, 10, 14, 15 }; const int off[6] { 0, 0, 12, 0, 0, 3 };
    const int roots[2] { 45, 41 };
    for (int b = 0; b < bars; ++b)
        for (int i = 0; i < 6; ++i)
            v.push_back ({ (b * 16 + st[i]) * STEP, 0.9f * STEP, roots[(b / 2) % 2] + off[i], i % 3 == 0 ? 1.0f : 0.7f });
    return v;
}

// ----- simple Drums für die Groove-Demo ---------------------------------------
static void addDrums (std::vector<float>& L, std::vector<float>& R, int bars)
{
    Rng rng;
    for (int b = 0; b < bars; ++b)
        for (int st = 0; st < 16; ++st)
        {
            const int s0 = (int) ((b * 16 + st) * STEP * SR);
            if (st % 4 == 0) // Kick
                for (int i = 0; i < (int) (0.4f * SR) && s0 + i < (int) L.size(); ++i)
                {
                    float t = i / SR; float f = 48.0f + 110.0f * std::exp (-t * 30.0f);
                    static float ph; if (i == 0) ph = 0; ph += f / SR;
                    float x = std::sin (kTwoPi * ph) * std::exp (-t * 7.0f) * 0.75f;
                    L[(size_t) (s0 + i)] += x; R[(size_t) (s0 + i)] += x;
                }
            if (st % 4 == 2) // offene Hihat
                { float hp = 0; for (int i = 0; i < (int) (0.12f * SR) && s0 + i < (int) L.size(); ++i)
                  { float t = i / SR; float n = rng.bipolar(); hp += 0.5f * (n - hp); float x = (n - hp) * std::exp (-t * 30.0f) * 0.12f;
                    L[(size_t) (s0 + i)] += x * 0.8f; R[(size_t) (s0 + i)] += x; } }
            if (st % 2 == 1) // geschlossene Hihat
                { float hp = 0; for (int i = 0; i < (int) (0.03f * SR) && s0 + i < (int) L.size(); ++i)
                  { float t = i / SR; float n = rng.bipolar(); hp += 0.5f * (n - hp); float x = (n - hp) * std::exp (-t * 120.0f) * 0.05f;
                    L[(size_t) (s0 + i)] += x; R[(size_t) (s0 + i)] += x * 0.8f; } }
            if (st == 4 || st == 12) // Clap
                { for (int i = 0; i < (int) (0.2f * SR) && s0 + i < (int) L.size(); ++i)
                  { float t = i / SR; float env = std::exp (-t * 25.0f) * (t < 0.02f ? (std::fmod (t, 0.007f) < 0.002f ? 1.0f : 0.3f) : 1.0f);
                    float x = rng.bipolar() * env * 0.1f; L[(size_t) (s0 + i)] += x; R[(size_t) (s0 + i)] += x; } }
        }
}

int main (int argc, char** argv)
{
    std::string out = argc > 1 ? argv[1] : "./";
    std::string only = argc > 2 ? argv[2] : "";   // optional: nur Presets mit diesem Namensanfang
    auto presets = makePresets();
    std::map<std::string, Params> byName;
    for (auto& pr : presets) byName[pr.name] = pr.p;

    const int bars = 4;
    int idx = 1;
    for (auto& pr : presets)
    {
        const Params& p = pr.p;
        const bool chordMem = p.chordMode > 0.5f;
        std::vector<Ev> ev;
        const std::string& n = pr.name;
        const bool detroit = pr.category == "Detroit Techno";
        setBpm (detroit ? 130.0f : 122.0f);
        const float secsNow = bars * 16 * STEP + 2.5f;
        auto has = [&n] (const char* k) { return n.find (k) != std::string::npos; };
        const bool padLike = has ("Pad") || (has ("Strings") && ! has ("Stab")) || has ("Swell");
        if (p.mono > 0.5f)                                   ev = detroit ? technoBass (bars, has ("Acid")) : bassPattern (bars);
        else if (has ("Stab") || has ("Chord") || has ("Brass") || has ("Organ 7")) ev = detroit ? technoStabs (bars) : stabPattern (bars);
        else if (has ("Pluck") || has ("Lead"))              ev = detroit ? technoSeq (bars) : melodyPattern (bars);
        else if (padLike)                                    ev = padPattern (bars * 2, chordMem);
        else                                                  ev = chordHoldPattern (bars, chordMem);
        float s = padLike ? bars * 2 * 16 * STEP + 3.0f : secsNow;
        std::vector<float> L, R;
        renderEvents (p, ev, s, L, R);
        float peak = 0, rms = 0;
        for (size_t i = 0; i < L.size(); ++i) { peak = std::max ({ peak, std::fabs (L[i]), std::fabs (R[i]) }); rms += L[i] * L[i] + R[i] * R[i]; }
        rms = std::sqrt (rms / (2.0f * (float) L.size()));
        char fn[256]; std::snprintf (fn, sizeof fn, "%s/%02d %s.wav", out.c_str(), idx++, n.c_str());
        if (only.size() && n.rfind (only, 0) != 0) continue;
        writeWav (fn, L, R);
        std::printf ("%-24s peak %.2f  rms %5.1f dBFS\n", n.c_str(), peak, 20.0f * std::log10 (rms + 1e-9f));
    }

    // Groove-Demo: Bass + Stabs + Pad + Drums
    {
        const int gb = 8; const float gs = gb * 16 * STEP + 2.0f;
        std::vector<float> L, R, l2, r2;
        renderEvents (byName["M1 Organ Bass"], bassPattern (gb), gs, L, R);
        auto mix = [&] (const std::string& name, const std::vector<Ev>& ev, float g)
        { renderEvents (byName[name], ev, gs, l2, r2); for (size_t i = 0; i < L.size(); ++i) { L[i] += l2[i] * g; R[i] += r2[i] * g; } };
        mix ("Chandler Stab m9", stabPattern (gb), 0.7f);
        mix ("Midnight Pad", padPattern (gb, false), 0.5f);
        addDrums (L, R, gb);
        float peak = 0; for (size_t i = 0; i < L.size(); ++i) peak = std::max ({ peak, std::fabs (L[i]), std::fabs (R[i]) });
        const float g = 0.89f / peak; for (size_t i = 0; i < L.size(); ++i) { L[i] *= g; R[i] *= g; }
        writeWav (out + "/00 AbleDeep Groove Demo.wav", L, R);
        std::printf ("Groove demo written (norm %.2f)\n", g);
    }
    {   // Detroit-Groove: Electro-Bass + Hi-Tech-Brass + Cosmic Pad + Bleeps + Drums @130
        setBpm (130.0f);
        const int gb = 8; const float gs = gb * 16 * STEP + 2.5f;
        std::vector<float> L, R, l2, r2;
        renderEvents (byName["DT Electro Bass"], technoBass (gb, false), gs, L, R);
        auto mix = [&] (const std::string& name, const std::vector<Ev>& ev, float g)
        { renderEvents (byName[name], ev, gs, l2, r2); for (size_t i = 0; i < L.size(); ++i) { L[i] += l2[i] * g; R[i] += r2[i] * g; } };
        mix ("DT Hi-Tech Brass dom9", technoStabs (gb), 0.65f);
        mix ("DT Cosmic Pad", padPattern (gb, true), 0.55f);
        { auto seq = technoSeq (gb); std::vector<Ev> second; for (auto& e : seq) if (e.t >= 4 * 16 * STEP) second.push_back (e);
          mix ("DT Bleep Lead", second, 0.45f); }
        addDrums (L, R, gb);
        float peak = 0; for (size_t i = 0; i < L.size(); ++i) peak = std::max ({ peak, std::fabs (L[i]), std::fabs (R[i]) });
        const float g = 0.89f / peak; for (size_t i = 0; i < L.size(); ++i) { L[i] *= g; R[i] *= g; }
        writeWav (out + "/00 AbleDeep Detroit Groove Demo.wav", L, R);
        std::printf ("Detroit groove demo written (norm %.2f)\n", g);
    }
    return 0;
}

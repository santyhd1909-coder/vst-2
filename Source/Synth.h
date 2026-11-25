#pragma once
// DALUmagic - motor de sintesis (osciladores, filtro, envolventes, efectos)
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>

struct SynthParams
{
    int   w1 = 2, w2 = 2, vow = 0, uni = 1;       // 0 sine, 1 tri, 2 saw, 3 square
    float o2semi = 0.f;
    float det = 8.f, mix = 0.5f, sub = 0.f, nz = 0.f;
    float cut = 5000.f, res = 1.f, fe = 0.f, fd = 0.4f;
    float A = 0.01f, D = 0.3f, S = 0.7f, R = 0.4f, vib = 0.f, vol = 0.7f;
    bool  onDr = false, onCh = false, onTr = false, onDl = false, onRv = false;
    float drAmt = 0.3f, chMix = 0.5f, chRate = 0.6f, trDepth = 0.4f, trRate = 5.f;
    float dlMix = 0.3f, dlTime = 0.375f, dlFb = 0.35f, rvMix = 0.35f, rvSize = 2.f;
    bool  leg = false;      // modo legato: las notas superpuestas se deslizan
    float glide = 0.08f;    // tiempo de slide en segundos
};

namespace dsp_util
{
    constexpr float kPi = 3.14159265358979323846f;
    constexpr float kTwoPi = 6.28318530717958647692f;

    inline float polyBlep (float t, float dt)
    {
        if (t < dt)         { t /= dt; return t + t - t * t - 1.f; }
        if (t > 1.f - dt)   { t = (t - 1.f) / dt; return t * t + t + t + 1.f; }
        return 0.f;
    }

    // type: 0 seno, 1 triangulo, 2 sierra, 3 cuadrada
    inline float osc (int type, double& ph, double inc)
    {
        const float t = (float) ph, dt = (float) inc;
        float v = 0.f;
        switch (type)
        {
            case 0:  v = std::sin (kTwoPi * t); break;
            case 1:  v = 4.f * std::abs (t - 0.5f) - 1.f; break;
            case 2:  v = 2.f * t - 1.f; v -= polyBlep (t, dt); break;
            default:
            {
                v = t < 0.5f ? 1.f : -1.f;
                v += polyBlep (t, dt);
                float t2 = t + 0.5f; if (t2 >= 1.f) t2 -= 1.f;
                v -= polyBlep (t2, dt);
                break;
            }
        }
        ph += inc; if (ph >= 1.0) ph -= 1.0;
        return v;
    }

    struct Biquad
    {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
        void peaking (double sr, double f, double q, double gainDb)
        {
            const double A = std::pow (10.0, gainDb / 40.0);
            const double w0 = 2.0 * 3.14159265358979 * f / sr;
            const double alpha = std::sin (w0) / (2.0 * q);
            const double c = std::cos (w0);
            const double a0 = 1.0 + alpha / A;
            b0 = (float) ((1.0 + alpha * A) / a0);
            b1 = (float) ((-2.0 * c) / a0);
            b2 = (float) ((1.0 - alpha * A) / a0);
            a1 = (float) ((-2.0 * c) / a0);
            a2 = (float) ((1.0 - alpha / A) / a0);
        }
        float process (float x)
        {
            const float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
        void reset() { z1 = z2 = 0; }
    };

    struct DelayLine
    {
        std::vector<float> b; int w = 0;
        void init (int n) { b.assign ((size_t) std::max (4, n), 0.f); w = 0; }
        float read (float d) const
        {
            const int n = (int) b.size();
            float rp = (float) w - d;
            while (rp < 0.f) rp += (float) n;
            int i0 = (int) rp; if (i0 >= n) i0 -= n;
            int i1 = i0 + 1; if (i1 >= n) i1 = 0;
            const float fr = rp - (float) i0;
            return b[(size_t) i0] * (1.f - fr) + b[(size_t) i1] * fr;
        }
        void push (float x) { b[(size_t) w] = x; if (++w >= (int) b.size()) w = 0; }
    };
}

class Voice
{
public:
    bool active = false, gate = false;
    int note = -1;
    uint64_t age = 0;

    void start (int n, float v, const SynthParams& P, double sr, uint64_t stamp)
    {
        active = true; gate = true; note = n; age = stamp;
        freq = 440.0 * std::pow (2.0, (n - 69) / 12.0);
        targetFreq = freq; glideCoef = 1.0;
        uni = std::max (1, std::min (9, P.uni));
        w1 = P.w1; w2 = P.w2;
        for (int k = 0; k < uni; ++k)
        {
            const double d = uni == 1 ? -P.det : -P.det + 2.0 * P.det * k / (uni - 1);
            ratio[(size_t) k] = std::pow (2.0, d / 1200.0);
            ph[(size_t) k] = uni == 1 ? 0.0 : (double) ((k * 37 + n * 11) % 100) / 100.0;
        }
        ratio2 = std::pow (2.0, P.o2semi / 12.0) * std::pow (2.0, P.det / 1200.0);
        ph2 = 0.0; phSub = 0.0;
        g1 = (1.f - P.mix) / std::sqrt ((float) uni);
        g2 = P.mix; gs = P.sub; gn = P.nz;

        pk = v * 0.9f;
        amp = 0.f; stage = 0;
        attackInc = pk / (float) std::max (1.0, (double) P.A * sr);
        sus = P.S * pk;
        decCoef = (float) std::exp (-1.0 / (std::max (0.005, (double) P.D / 3.0) * sr));
        fenv = 0.f; fenvRising = true;
        fenvInc = (float) (1.0 / (std::max (0.004, (double) P.A) * sr));
        fdCoef = (float) std::exp (-1.0 / (std::max (0.01, (double) P.fd / 3.0) * sr));
        keyTrack = (float) std::pow (freq / 261.6, 0.35);
        ic1 = ic2 = 0.f; ctl = 0;
    }

    void glideTo (int n, const SynthParams& P, double sr)
    {
        note = n;
        targetFreq = 440.0 * std::pow (2.0, (n - 69) / 12.0);
        glideCoef = 1.0 - std::exp (-1.0 / (std::max (0.003, (double) P.glide / 3.0) * sr));
    }

    void release (const SynthParams& P, double sr)
    {
        if (! gate) return;
        gate = false; stage = 2;
        relCoef = (float) std::exp (-1.0 / (std::max (0.01, (double) P.R / 4.0) * sr));
    }

    void fastKill (double sr)
    {
        gate = false; stage = 2;
        relCoef = (float) std::exp (-1.0 / (0.01 * sr));
    }

    float render (const SynthParams& P, float vibMul, double sr, float noise)
    {
        switch (stage)
        {
            case 0: amp += attackInc; if (amp >= pk) { amp = pk; stage = 1; } break;
            case 1:
                amp = sus + (amp - sus) * decCoef;
                if (sus < 1e-4f && amp < 1e-4f) { active = false; return 0.f; }
                break;
            default:
                amp *= relCoef;
                if (amp < 1e-4f) { active = false; return 0.f; }
                break;
        }

        if (fenvRising) { fenv += fenvInc; if (fenv >= 1.f) { fenv = 1.f; fenvRising = false; } }
        else fenv *= fdCoef;

        if (--ctl <= 0)
        {
            ctl = 8;
            if (freq != targetFreq) keyTrack = (float) std::pow (freq / 261.6, 0.35);
            float fc = P.cut * keyTrack * std::exp2 (P.fe * fenv);
            fc = std::max (30.f, std::min (fc, std::min (20000.f, (float) (0.45 * sr))));
            const float g = std::tan (dsp_util::kPi * fc / (float) sr);
            const float k = 1.f / std::max (0.1f, P.res);
            a1 = 1.f / (1.f + g * (g + k));
            a2 = g * a1;
            a3 = g * a2;
        }

        if (freq != targetFreq)
        {
            freq += (targetFreq - freq) * glideCoef;
            if (std::abs (targetFreq - freq) < targetFreq * 1e-5) freq = targetFreq;
        }
        const double base = freq * (double) vibMul / sr;
        float s = 0.f;
        for (int k = 0; k < uni; ++k)
            s += dsp_util::osc (w1, ph[(size_t) k], base * ratio[(size_t) k]) * g1;
        s += dsp_util::osc (w2, ph2, base * ratio2) * g2;
        if (gs > 0.f) s += dsp_util::osc (0, phSub, base * 0.5) * gs;
        if (gn > 0.f) s += noise * gn;

        const float v3 = s - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.f * v1 - ic1;
        ic2 = 2.f * v2 - ic2;
        return v2 * amp;
    }

private:
    double freq = 440.0, targetFreq = 440.0, glideCoef = 1.0, ratio2 = 1.0, ph2 = 0.0, phSub = 0.0;
    std::array<double, 9> ratio {}, ph {};
    int uni = 1, w1 = 2, w2 = 2, stage = 0, ctl = 0;
    float g1 = 1, g2 = 0, gs = 0, gn = 0;
    float pk = 0, amp = 0, attackInc = 0, sus = 0, decCoef = 0, relCoef = 0;
    float fenv = 0, fenvInc = 0, fdCoef = 0; bool fenvRising = true;
    float keyTrack = 1, a1 = 0, a2 = 0, a3 = 0, ic1 = 0, ic2 = 0;
};

class DaluEngine
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        chL.init ((int) (sr * 0.1)); chR.init ((int) (sr * 0.1));
        dlL.init ((int) (sr * 1.6));  dlR.init ((int) (sr * 1.6));
        reverb.setSampleRate (sr);
        reverb.reset();
        f1.reset(); f2.reset();
        allOff();
    }

    void allOff()
    {
        for (auto& v : voices) { v.active = false; v.gate = false; }
        legStack.clear(); legVoice = nullptr;
    }

    void noteOn (int n, float vel, const SynthParams& P)
    {
        if (P.leg)
        {
            legStack.erase (std::remove (legStack.begin(), legStack.end(), n), legStack.end());
            legStack.push_back (n);
            if (legVoice != nullptr && legVoice->active && legVoice->gate)
            {
                legVoice->glideTo (n, P, sr);   // nota superpuesta: slide sin retrigger
                return;
            }
        }
        Voice* target = nullptr;
        for (auto& v : voices) if (v.active && v.note == n && v.gate) { target = &v; break; }
        if (! target) for (auto& v : voices) if (! v.active) { target = &v; break; }
        if (! target)
        {
            target = &voices[0];
            for (auto& v : voices) if (v.age < target->age) target = &v;
        }
        target->start (n, vel, P, sr, ++stamp);
        if (P.leg) legVoice = target;
    }

    void noteOff (int n, const SynthParams& P)
    {
        if (P.leg && legVoice != nullptr)
        {
            legStack.erase (std::remove (legStack.begin(), legStack.end(), n), legStack.end());
            if (! legVoice->active || ! legVoice->gate) { legVoice = nullptr; return; }
            if (! legStack.empty()) legVoice->glideTo (legStack.back(), P, sr);
            else { legVoice->release (P, sr); legVoice = nullptr; }
            return;
        }
        for (auto& v : voices) if (v.active && v.gate && v.note == n) v.release (P, sr);
    }

    void render (float* L, float* R, int n, const SynthParams& P)
    {
        using namespace dsp_util;
        if (P.vow != lastVow)
        {
            lastVow = P.vow;
            static const double fr[5][2] = { { 0, 0 }, { 800, 1150 }, { 450, 800 }, { 270, 2300 }, { 300, 870 } };
            if (P.vow > 0) { f1.peaking (sr, fr[P.vow][0], 5.0, 14.0); f2.peaking (sr, fr[P.vow][1], 5.0, 11.0); }
        }
        if (! P.onRv && wasReverb) { reverb.reset(); }
        wasReverb = P.onRv;

        // drive
        const float dAmt = P.onDr ? P.drAmt : 0.f;
        const float dk = 1.f + dAmt * 80.f;
        const float dw = std::min (1.f, dAmt * 3.f);
        // chorus
        const float cm = P.onCh ? P.chMix : 0.f;
        // tremolo
        const float td = P.onTr ? P.trDepth : 0.f;
        // delay
        const float dm = P.onDl ? P.dlMix : 0.f;
        const float dlFb = P.dlFb;
        const float fbA = 1.f - std::exp (-kTwoPi * 3200.f / (float) sr);

        for (int i = 0; i < n; ++i)
        {
            const float lfo = std::sin (vibPh);
            vibPh += kTwoPi * 5.4f / (float) sr; if (vibPh > kTwoPi) vibPh -= kTwoPi;
            const float vibMul = P.vib > 0.f ? std::exp2 (P.vib * lfo / 1200.f) : 1.f;
            const float noise = nextNoise();

            float s = 0.f;
            for (auto& v : voices) if (v.active) s += v.render (P, vibMul, sr, noise);
            s *= 0.3f;
            if (P.vow > 0) s = f2.process (f1.process (s));

            // drive
            if (dAmt > 0.f)
            {
                const float wet = (1.f + dk) * s / (1.f + dk * std::abs (s)) * 0.8f;
                s = s * (1.f - dw * 0.7f) + wet * (dw * 0.8f);
            }

            float l = s, r = s;

            // chorus
            if (cm > 0.f)
            {
                cPh1 += kTwoPi * P.chRate / (float) sr;        if (cPh1 > kTwoPi) cPh1 -= kTwoPi;
                cPh2 += kTwoPi * P.chRate * 1.27f / (float) sr; if (cPh2 > kTwoPi) cPh2 -= kTwoPi;
                const float d1 = (0.022f + 0.0045f * std::sin (cPh1)) * (float) sr;
                const float d2 = (0.031f + 0.0045f * std::sin (cPh2)) * (float) sr;
                const float wl = chL.read (d1), wr = chR.read (d2);
                chL.push (s); chR.push (s);
                l = s * (1.f - cm * 0.3f) + wl * cm * 0.9f;
                r = s * (1.f - cm * 0.3f) + wr * cm * 0.9f;
            }

            // tremolo
            if (td > 0.f)
            {
                tPh += kTwoPi * P.trRate / (float) sr; if (tPh > kTwoPi) tPh -= kTwoPi;
                const float g = 1.f - td * 0.5f + td * 0.5f * std::sin (tPh);
                l *= g; r *= g;
            }

            // delay
            dlTimeSm += (P.dlTime - dlTimeSm) * 0.0008f;
            if (dm > 0.f || dlActive)
            {
                const float tl = dlTimeSm * (float) sr, tr = (dlTimeSm + 0.007f) * (float) sr;
                const float wl = dlL.read (tl), wr = dlR.read (tr);
                fbL += fbA * (wl - fbL); fbR += fbA * (wr - fbR);
                dlL.push (l + fbL * dlFb);
                dlR.push (r + fbR * dlFb);
                l += wl * dm; r += wr * dm;
                dlActive = dm > 0.f || std::abs (wl) > 1e-5f;
            }

            L[i] = l; R[i] = r;
        }

        if (P.onRv)
        {
            juce::Reverb::Parameters rp;
            rp.roomSize = std::min (0.95f, 0.45f + P.rvSize * 0.09f);
            rp.damping = 0.45f; rp.width = 1.f; rp.freezeMode = 0.f;
            rp.wetLevel = P.rvMix * 0.35f;
            rp.dryLevel = 0.5f * (1.f - P.rvMix * 0.35f);
            reverb.setParameters (rp);
            reverb.processStereo (L, R, n);
        }

        const float vol = P.vol;
        for (int i = 0; i < n; ++i)
        {
            L[i] = std::tanh (L[i] * vol * 1.4f);
            R[i] = std::tanh (R[i] * vol * 1.4f);
        }
    }

private:
    float nextNoise()
    {
        seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
        return (float) (seed & 0xFFFFFF) / 8388608.f - 1.f;
    }

    double sr = 44100.0;
    std::array<Voice, 16> voices;
    uint64_t stamp = 0;
    std::vector<int> legStack;
    Voice* legVoice = nullptr;
    uint32_t seed = 2463534242u;
    float vibPh = 0, cPh1 = 0, cPh2 = 0, tPh = 0, dlTimeSm = 0.375f, fbL = 0, fbR = 0;
    bool dlActive = false, wasReverb = false;
    int lastVow = -1;
    dsp_util::Biquad f1, f2;
    dsp_util::DelayLine chL, chR, dlL, dlR;
    juce::Reverb reverb;
};

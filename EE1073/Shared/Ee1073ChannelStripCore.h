#pragma once

#include <algorithm>
#include <cmath>

// Shared DSP core for the EE-1073 (Neve 1073-style mic preamp + 3-band EQ
// channel strip) emulation. Used by both the JUCE VST3/AU plugin
// (EE1073/Source/) and the Max for Live external (EE1073/Max/). Keep this
// header free of JUCE- or Max-specific types so it compiles identically in
// both targets.
//
// Behavioral model informed by the project blueprint ("From Circuit
// Simulation to DAW Integration..."):
//  - Input and output transformer stages: asymmetric soft saturation with a
//    one-pole "memory" term standing in for magnetic hysteresis (the
//    core resisting changes in magnetization), so the saturation lags the
//    instantaneous signal rather than reacting sample-instantly.
//  - Class-A discrete gain stage: soft-clipping saturation with a shallow
//    dip in generated harmonic energy around 2 kHz, applied via a fixed
//    shaping filter ahead of the nonlinearity. Both this stage and the
//    transformer stages grow more asymmetric (more even-harmonic content)
//    as the Input knob is driven harder.
//  - Three-band EQ (low shelf, mid peak, high shelf) plus switchable HPF,
//    implemented as RBJ-cookbook biquads. The mid band's Q narrows/widens
//    with the amount of boost/cut applied, approximating the 1073's
//    documented interactive (non-fixed-Q) mid-band behavior.
//
// This is a simplified behavioral model, not the full WDF/MNA transformer
// and transistor circuit simulation the blueprint identifies as the path to
// maximum fidelity -- see the blueprint PDF for what that would require.
namespace ee1073
{

struct Parameters
{
    float inputGainDb = 0.0f;

    bool hpfEnabled = false;
    float hpfFreqHz = 80.0f; // real unit: 50 / 80 / 160 / 300 Hz switch positions

    float lowShelfFreqHz = 60.0f; // real unit: 35 / 60 / 110 / 220 Hz switch positions
    float lowShelfGainDb = 0.0f;  // +/-16 dB

    float midFreqHz = 1000.0f; // real unit: ~360 Hz .. 7.2 kHz switch positions
    float midGainDb = 0.0f;    // +/-18 dB

    float highShelfGainDb = 0.0f; // fixed at 12 kHz on the real unit

    bool eqEnabled = true;
    float outputGainDb = 0.0f;

    bool phaseInvert = false;
};

inline bool operator== (const Parameters& a, const Parameters& b)
{
    return a.inputGainDb == b.inputGainDb
        && a.hpfEnabled == b.hpfEnabled
        && a.hpfFreqHz == b.hpfFreqHz
        && a.lowShelfFreqHz == b.lowShelfFreqHz
        && a.lowShelfGainDb == b.lowShelfGainDb
        && a.midFreqHz == b.midFreqHz
        && a.midGainDb == b.midGainDb
        && a.highShelfGainDb == b.highShelfGainDb
        && a.eqEnabled == b.eqEnabled
        && a.outputGainDb == b.outputGainDb
        && a.phaseInvert == b.phaseInvert;
}

// Minimal RBJ-cookbook biquad (Direct Form I). Coefficients are recomputed
// only when parameters change, not per-sample.
class Biquad
{
public:
    void setBypass() { b0 = 1.0f; b1 = b2 = a1 = a2 = 0.0f; }

    void setHighpass (float freqHz, float q, double sr)
    {
        const float w0 = static_cast<float> (2.0 * M_PI * freqHz / sr);
        const float cosw0 = std::cos (w0);
        const float alpha = std::sin (w0) / (2.0f * q);
        const float a0 = 1.0f + alpha;

        b0 = ((1.0f + cosw0) / 2.0f) / a0;
        b1 = (-(1.0f + cosw0)) / a0;
        b2 = ((1.0f + cosw0) / 2.0f) / a0;
        a1 = (-2.0f * cosw0) / a0;
        a2 = (1.0f - alpha) / a0;
    }

    void setLowShelf (float freqHz, float gainDb, double sr)
    {
        const float A = std::pow (10.0f, gainDb / 40.0f);
        const float w0 = static_cast<float> (2.0 * M_PI * freqHz / sr);
        const float cosw0 = std::cos (w0);
        const float sinw0 = std::sin (w0);
        const float alpha = sinw0 / 2.0f * std::sqrt ((A + 1.0f / A) * (1.0f / kShelfSlope - 1.0f) + 2.0f);
        const float twoSqrtAAlpha = 2.0f * std::sqrt (A) * alpha;

        const float a0 = (A + 1.0f) + (A - 1.0f) * cosw0 + twoSqrtAAlpha;
        b0 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 + twoSqrtAAlpha)) / a0;
        b1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
        b2 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 - twoSqrtAAlpha)) / a0;
        a1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
        a2 = ((A + 1.0f) + (A - 1.0f) * cosw0 - twoSqrtAAlpha) / a0;
    }

    void setHighShelf (float freqHz, float gainDb, double sr)
    {
        const float A = std::pow (10.0f, gainDb / 40.0f);
        const float w0 = static_cast<float> (2.0 * M_PI * freqHz / sr);
        const float cosw0 = std::cos (w0);
        const float sinw0 = std::sin (w0);
        const float alpha = sinw0 / 2.0f * std::sqrt ((A + 1.0f / A) * (1.0f / kShelfSlope - 1.0f) + 2.0f);
        const float twoSqrtAAlpha = 2.0f * std::sqrt (A) * alpha;

        const float a0 = (A + 1.0f) - (A - 1.0f) * cosw0 + twoSqrtAAlpha;
        b0 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 + twoSqrtAAlpha)) / a0;
        b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
        b2 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 - twoSqrtAAlpha)) / a0;
        a1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
        a2 = ((A + 1.0f) - (A - 1.0f) * cosw0 - twoSqrtAAlpha) / a0;
    }

    void setPeaking (float freqHz, float gainDb, float q, double sr)
    {
        const float A = std::pow (10.0f, gainDb / 40.0f);
        const float w0 = static_cast<float> (2.0 * M_PI * freqHz / sr);
        const float cosw0 = std::cos (w0);
        const float alpha = std::sin (w0) / (2.0f * q);

        const float a0 = 1.0f + alpha / A;
        b0 = (1.0f + alpha * A) / a0;
        b1 = (-2.0f * cosw0) / a0;
        b2 = (1.0f - alpha * A) / a0;
        a1 = (-2.0f * cosw0) / a0;
        a2 = (1.0f - alpha / A) / a0;
    }

    float process (float x)
    {
        const float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        return y;
    }

    void reset() { x1 = x2 = y1 = y2 = 0.0f; }

private:
    static constexpr float kShelfSlope = 1.0f;

    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;
};

class ChannelStripCore
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        reset();
        dirty = true;
    }

    void reset()
    {
        hpf.reset();
        lowShelf.reset();
        midPeak.reset();
        highShelf.reset();
        harmonicShapeFilter.reset();
        inputHysteresis = 0.0f;
        outputHysteresis = 0.0f;
    }

    void setParameters (const Parameters& p)
    {
        if (! (p == params))
            dirty = true;
        params = p;
    }

    float processSample (float x)
    {
        if (dirty)
        {
            updateFilters();
            dirty = false;
        }

        float y = x * dbToGain (params.inputGainDb);

        // Even-harmonic content grows with the Input knob: driving the
        // preamp/transformers harder biases them further from their
        // symmetric linear region, so the saturation asymmetry (and thus
        // 2nd-harmonic energy) scales with input drive.
        const float inputDrive01 = std::clamp ((params.inputGainDb + 20.0f) / 50.0f, 0.0f, 1.0f);
        const float transformerAsymmetry = lerp (0.03f, 0.16f, inputDrive01);
        const float classAAsymmetry = lerp (0.0f, 0.12f, inputDrive01);

        y = transformerStage (y, inputHysteresis, transformerAsymmetry);

        if (params.hpfEnabled)
            y = hpf.process (y);

        // Shallow dip in generated harmonic energy around 2 kHz, applied
        // ahead of the Class-A stage's nonlinearity (see blueprint).
        y = harmonicShapeFilter.process (y);
        y = classAStage (y, classAAsymmetry);

        if (params.eqEnabled)
        {
            y = lowShelf.process (y);
            y = midPeak.process (y);
            y = highShelf.process (y);
        }

        y = transformerStage (y, outputHysteresis, transformerAsymmetry);
        y *= dbToGain (params.outputGainDb);

        if (params.phaseInvert)
            y = -y;

        return y;
    }

private:
    static float dbToGain (float db) { return std::pow (10.0f, db / 20.0f); }
    static float lerp (float a, float b, float t) { return a + (b - a) * t; }

    // Soft-clipping curve biased by `asymmetry` to add even-harmonic
    // content. The shift-then-subtract form guarantees
    // asymmetricSoftClip(0, ...) == 0 for any asymmetry, and asymmetry == 0
    // reduces to a plain symmetric (odd-harmonics-only) tanh soft clip.
    static float asymmetricSoftClip (float x, float drive, float asymmetry)
    {
        const float shifted = x + asymmetry;
        return std::tanh (shifted * drive) / std::tanh (drive) - std::tanh (asymmetry * drive) / std::tanh (drive);
    }

    // Saturation with a lagging "memory" term standing in for transformer
    // core hysteresis: the effective drive blends the instantaneous curve
    // with a slow-following version of itself.
    static float transformerStage (float x, float& hysteresisState, float asymmetry)
    {
        constexpr float drive = 1.3f;
        constexpr float hysteresisCoeff = 0.15f; // lag amount, not sample-rate normalized (simplified)

        const float instant = asymmetricSoftClip (x, drive, asymmetry);

        hysteresisState += hysteresisCoeff * (instant - hysteresisState);
        return 0.6f * instant + 0.4f * hysteresisState;
    }

    // Class-A discrete gain stage with soft clipping and 2 kHz harmonic dip
    // applied via the shaping filter before the nonlinearity (per blueprint)
    static float classAStage (float x, float asymmetry)
    {
        constexpr float drive = 1.15f;
        return asymmetricSoftClip (x, drive, asymmetry);
    }

    void updateFilters()
    {
        const float hpfQ = 0.707f;
        hpf.setHighpass (std::max (10.0f, params.hpfFreqHz), hpfQ, sr);

        lowShelf.setLowShelf (params.lowShelfFreqHz, params.lowShelfGainDb, sr);
        highShelf.setHighShelf (12000.0f, params.highShelfGainDb, sr);

        // Interactive mid-band: Q narrows for small boosts/cuts near unity
        // and widens as the amount of boost/cut increases, approximating the
        // 1073's documented non-fixed-Q mid-band behavior. This is a
        // simplified stand-in for the real interconnected passive network.
        // The Q adjustment follows: Q = Q_base / (1 + |gain| * factor)
        // Small adjustments near 0 dB keep Q wide (~1.4), large adjustments
        // narrow the Q as expected for the 1073's interactive behavior.
        const float gainMagnitude = std::abs (params.midGainDb);
        const float qBase = 1.4f;
        const float qAdjustmentFactor = 0.1f;
        const float q = std::clamp (qBase / (1.0f + gainMagnitude * qAdjustmentFactor), 0.35f, 1.4f);
        midPeak.setPeaking (params.midFreqHz, params.midGainDb, q, sr);

        harmonicShapeFilter.setPeaking (2000.0f, -2.0f, 1.2f, sr);
    }

    double sr = 44100.0;
    bool dirty = true;
    Parameters params;

    Biquad hpf, lowShelf, midPeak, highShelf, harmonicShapeFilter;
    float inputHysteresis = 0.0f;
    float outputHysteresis = 0.0f;
};

} // namespace ee1073

// Plugin parameter validation helper
inline bool validateParameters (const ee1073::Parameters& p)
{
    // Validate frequency ranges are positive
    return p.hpfFreqHz > 0.0f
        && p.lowShelfFreqHz > 0.0f
        && p.midFreqHz > 0.0f
        && p.highShelfGainDb >= -16.0f && p.highShelfGainDb <= 16.0f;
}


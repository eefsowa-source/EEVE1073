#pragma once

#include <algorithm>
#include <cmath>

// Shared DSP core for the EE-1176 (1176-style FET compressor) emulation.
// Used by both the JUCE VST3/AU plugin (EE1176/Source/) and the Max for
// Live external (EE1176/Max/). Keep this header free of JUCE- or Max-specific
// types so it compiles identically in both targets.
//
// Behavioral model informed by the project PDF blueprint:
//  - Feedback-topology detector (the FET gain-reduction stage is inside the
//    detector's loop on real 1176 hardware, so the envelope follows the
//    *output* rather than the input; this is the main source of the unit's
//    program-dependent feel).
//  - Ratio-dependent internal threshold (the Input knob drives a fixed
//    reference level, and changing ratio shifts the detector's effective
//    bias point rather than acting as a simple post-threshold slope).
//  - Soft-knee gain computation in the dB domain.
//  - FET/output-stage saturation that scales with gain-reduction depth, so
//    heavier compression naturally adds more harmonic content, and whose
//    even-harmonic asymmetry grows with the Input knob (more drive biases
//    the FET further from its symmetric linear region).
//  - A dedicated "British mode" (all-buttons-in) state: fixed ~16:1
//    effective ratio, faster/tighter time constants, and increased,
//    asymmetric saturation.
//
// This is still a simplified behavioral model, not a full WDF/MNA circuit
// simulation of the FET and diode network -- see the blueprint PDF for what
// a circuit-accurate implementation would additionally require.
namespace ee1176
{

enum class Ratio
{
    r4to1,
    r8to1,
    r12to1,
    r20to1,
    allButtonsIn // "British mode"
};

struct Parameters
{
    float inputGainDb = 0.0f;
    float outputGainDb = 0.0f;
    float attackMs = 0.4f;    // ~20us..800us in the real unit
    float releaseMs = 300.0f; // ~50ms..1.1s
    Ratio ratio = Ratio::r4to1;
};

class CompressorCore
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        reset();
    }

    void reset()
    {
        envelopeDb = silenceDb;
        previousOutput = 0.0f;
    }

    void setParameters (const Parameters& p) { params = p; }

    // Processes one sample of a single channel. Call once per channel per
    // sample, or extend to block/multi-channel processing as needed.
    float processSample (float x)
    {
        const float inputGain = dbToGain (params.inputGainDb);
        const float outputGain = dbToGain (params.outputGainDb);
        const bool british = params.ratio == Ratio::allButtonsIn;

        const float xIn = x * inputGain;

        // --- Feedback-style detector: level tracked on the previous
        // output sample, not the incoming signal, matching the 1176's
        // detector-after-gain-element topology.
        const float detectLevel = std::abs (previousOutput);
        const float detectDb = gainToDb (detectLevel);

        const float attackCoeff = timeToCoeff (british ? params.attackMs * 0.6f : params.attackMs);
        const float releaseCoeff = timeToCoeff (british ? params.releaseMs * 0.5f : params.releaseMs);
        const float coeff = detectDb > envelopeDb ? attackCoeff : releaseCoeff;
        envelopeDb += coeff * (detectDb - envelopeDb);

        // --- Ratio-dependent internal threshold: higher ratios push the
        // detector's effective bias point up, so the unit engages later
        // but harder on strong peaks (per blueprint).
        const float ratioValue = british ? britishRatioValue() : ratioToValue (params.ratio);
        const float thresholdDb = baseThresholdDb + ratioThresholdShiftDb (params.ratio);

        // --- Soft-knee downward compression in the dB domain.
        const float overDb = envelopeDb - thresholdDb;
        float gainReductionDb = 0.0f;
        if (overDb > -kneeWidthDb * 0.5f)
        {
            if (overDb <= kneeWidthDb * 0.5f)
            {
                // Quadratic knee blend between 0 dB and full-ratio slope.
                const float x2 = overDb + kneeWidthDb * 0.5f;
                gainReductionDb = -((1.0f / ratioValue - 1.0f) * (x2 * x2) / (2.0f * kneeWidthDb));
            }
            else
            {
                const float kneeReductionAtEdge = -((1.0f / ratioValue - 1.0f) * kneeWidthDb / 2.0f);
                gainReductionDb = kneeReductionAtEdge + (overDb - kneeWidthDb * 0.5f) * (1.0f / ratioValue - 1.0f);
            }
        }

        const float gainLinear = dbToGain (gainReductionDb);
        const float driven = xIn * gainLinear;

        // --- FET / output-stage saturation, scaled by how hard the gain
        // element is being driven -- heavier gain reduction biases the FET
        // further from its linear region, adding more harmonics.
        const float driveAmount = std::clamp (-gainReductionDb / 20.0f, 0.0f, 1.0f);
        const float saturationDrive = 1.0f + driveAmount * (british ? 3.0f : 1.2f);

        // Even-harmonic content grows with the Input knob: pushing more
        // signal into the FET biases it further from its symmetric linear
        // region, so the asymmetry (and thus 2nd-harmonic energy) of the
        // saturation curve scales with input drive, not just how hard the
        // gain-reduction stage happens to be working at this instant.
        const float inputDrive01 = std::clamp ((params.inputGainDb + 20.0f) / 60.0f, 0.0f, 1.0f);
        const float asymmetry = british
                                     ? lerp (0.12f, 0.30f, inputDrive01)  // already asymmetric; input pushes further
                                     : lerp (0.0f, 0.16f, inputDrive01);  // clean at low input, more even harmonics as it's driven

        float y = asymmetricSoftClip (driven, saturationDrive, asymmetry);

        y *= outputGain;
        previousOutput = y;
        lastGainReductionDb = gainReductionDb;
        return y;
    }

    // Gain reduction from the most recently processed sample, in dB
    // (negative or zero). For UI metering only -- not used internally.
    float getGainReductionDb() const { return lastGainReductionDb; }

private:
    static float dbToGain (float db) { return std::pow (10.0f, db / 20.0f); }
    static float gainToDb (float g) { return 20.0f * std::log10 (std::max (g, 1.0e-6f)); }

    static float lerp (float a, float b, float t) { return a + (b - a) * t; }

    // Soft-clipping curve biased by `asymmetry` to add even-harmonic
    // content (standing in for the shifted bias points and "sharper
    // knees" the blueprint attributes to all-buttons-in mode, and more
    // generally for a FET pushed harder from its linear region). The
    // shift-then-subtract form guarantees asymmetricSoftClip(0, ...) == 0
    // for any asymmetry, and asymmetry == 0 reduces to a plain symmetric
    // (odd-harmonics-only) tanh soft clip.
    static float asymmetricSoftClip (float x, float drive, float asymmetry)
    {
        const float shifted = x + asymmetry;
        return std::tanh (shifted * drive) / std::tanh (drive) - std::tanh (asymmetry * drive) / std::tanh (drive);
    }

    float timeToCoeff (float timeMs) const
    {
        const float t = std::max (0.001f, timeMs) * 0.001f;
        return 1.0f - std::exp (-1.0f / (static_cast<float> (sr) * t));
    }

    static float ratioToValue (Ratio r)
    {
        switch (r)
        {
            case Ratio::r4to1:  return 4.0f;
            case Ratio::r8to1:  return 8.0f;
            case Ratio::r12to1: return 12.0f;
            case Ratio::r20to1: return 20.0f;
            case Ratio::allButtonsIn: return 16.0f;
        }
        return 4.0f;
    }

    static float britishRatioValue() { return 16.0f; } // ~12:1..20:1 per blueprint

    // Higher ratios raise the internal threshold -- the compressor engages
    // only on stronger peaks, per the blueprint's description of the
    // detector's shifting bias point.
    static float ratioThresholdShiftDb (Ratio r)
    {
        switch (r)
        {
            case Ratio::r4to1:  return 0.0f;
            case Ratio::r8to1:  return 2.0f;
            case Ratio::r12to1: return 3.5f;
            case Ratio::r20to1: return 5.0f;
            case Ratio::allButtonsIn: return 4.0f;
        }
        return 0.0f;
    }

    static constexpr float silenceDb = -100.0f;
    static constexpr float baseThresholdDb = -18.0f; // fixed internal reference; Input knob drives signal against it
    static constexpr float kneeWidthDb = 6.0f;

    double sr = 44100.0;
    float envelopeDb = silenceDb;
    float previousOutput = 0.0f;
    float lastGainReductionDb = 0.0f;
    Parameters params;
};

} // namespace ee1176

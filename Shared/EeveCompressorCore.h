#pragma once

#include <algorithm>
#include <cmath>

// Shared DSP core for the EEVE1073 (1176-style FET compressor) emulation.
// Used by both the JUCE VST3/AU plugin (Source/) and the Max for Live
// external (Max/source/). Keep this header free of JUCE- or Max-specific
// types so it compiles identically in both targets.
//
// This is a placeholder behavioral model (see the project PDF blueprint for
// the full circuit-accurate design target: FET gain-reduction stage,
// program-dependent attack/release, and "British mode" / all-buttons-in
// emulation). Replace processSample() with the real WDF/behavioral model.
namespace eeve
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
    float attackMs = 0.4f;   // ~20us..800us in the real unit; placeholder ms range
    float releaseMs = 300.0f; // ~50ms..1.1s
    Ratio ratio = Ratio::r4to1;
};

class CompressorCore
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        envelope = 0.0f;
    }

    void reset() { envelope = 0.0f; }

    void setParameters (const Parameters& p) { params = p; }

    // Processes one sample of a single channel. Call once per channel per
    // sample, or extend to block/multi-channel processing as needed.
    float processSample (float x)
    {
        const float inputGain = dbToGain (params.inputGainDb);
        const float outputGain = dbToGain (params.outputGainDb);

        const float xIn = x * inputGain;

        const float rectified = std::abs (xIn);
        const float attackCoeff = timeToCoeff (params.attackMs);
        const float releaseCoeff = timeToCoeff (params.releaseMs);
        const float coeff = rectified > envelope ? attackCoeff : releaseCoeff;
        envelope += coeff * (rectified - envelope);

        const float ratioValue = ratioToValue (params.ratio);
        const float thresholdLinear = 1.0f; // fixed internal reference (Input knob drives level instead)
        float gainReduction = 1.0f;
        if (envelope > thresholdLinear)
        {
            const float over = envelope / thresholdLinear;
            gainReduction = std::pow (over, (1.0f / ratioValue) - 1.0f);
        }

        float y = xIn * gainReduction;

        if (params.ratio == Ratio::allButtonsIn)
            y = std::tanh (y * 1.5f) / 1.5f; // rough extra saturation stand-in for British mode

        return y * outputGain;
    }

private:
    static float dbToGain (float db) { return std::pow (10.0f, db / 20.0f); }

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
            case Ratio::allButtonsIn: return 16.0f; // ~12:1 to 20:1 per blueprint
        }
        return 4.0f;
    }

    double sr = 44100.0;
    float envelope = 0.0f;
    Parameters params;
};

} // namespace eeve

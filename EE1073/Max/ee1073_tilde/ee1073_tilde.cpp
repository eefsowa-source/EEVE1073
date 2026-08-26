// ee1073~ — Max/MSP external wrapping the shared EE-1073 channel-strip core
// (Neve 1073-style mic preamp + 3-band EQ), for use inside a Max for Live
// device in Ableton Live.
//
// Built with Cycling '74's min-devkit (https://github.com/Cycling74/min-devkit).
// See EE1073/Max/CMakeLists.txt for build setup.

#include "c74_min.h"
#include "../../Shared/Ee1073ChannelStripCore.h"
#include "../../../Shared/EeFourTimesOversampler.h"

using namespace c74::min;

class ee1073_tilde : public object<ee1073_tilde>, public sample_operator<1, 1>
{
public:
    MIN_DESCRIPTION { "EE-1073 mic preamp + 3-band EQ channel strip (Neve 1073 emulation core)" };
    MIN_TAGS { "audio, eq, preamp" };
    MIN_AUTHOR { "EON Audio" };
    MIN_RELATED { "ee1176~, biquad~" };

    inlet<> in { this, "(signal) audio in" };
    outlet<> out { this, "(signal) audio out", "signal" };

    attribute<number> input { this, "input", 0.0,
        description { "Input gain in dB" } };
    attribute<bool> hpf_on { this, "hpf_on", false,
        description { "High-pass filter on/off" } };
    attribute<number> hpf_freq { this, "hpf_freq", 80.0,
        description { "High-pass filter frequency in Hz (50/80/160/300)" } };
    attribute<number> low_freq { this, "low_freq", 60.0,
        description { "Low shelf frequency in Hz (35/60/110/220)" } };
    attribute<number> low_gain { this, "low_gain", 0.0,
        description { "Low shelf gain in dB" } };
    attribute<number> mid_freq { this, "mid_freq", 1000.0,
        description { "Mid peak frequency in Hz" } };
    attribute<number> mid_gain { this, "mid_gain", 0.0,
        description { "Mid peak gain in dB" } };
    attribute<number> high_gain { this, "high_gain", 0.0,
        description { "High shelf (12kHz) gain in dB" } };
    attribute<bool> eq_on { this, "eq_on", true,
        description { "EQ section in/out" } };
    attribute<number> output { this, "output", 0.0,
        description { "Output gain in dB" } };
    attribute<bool> phase_invert { this, "phase_invert", false,
        description { "Invert output polarity" } };

    ee1073_tilde() { core.prepare (c74::max::sys_getsr() * 4.0); }

    void setup (double sampleRate)
    {
        core.prepare (sampleRate * 4.0);
        oversampler.reset();
    }

    sample operator()(sample x)
    {
        ee1073::Parameters p;
        p.inputGainDb = static_cast<float> (input.get());
        p.hpfEnabled = hpf_on.get();
        p.hpfFreqHz = static_cast<float> (hpf_freq.get());
        p.lowShelfFreqHz = static_cast<float> (low_freq.get());
        p.lowShelfGainDb = static_cast<float> (low_gain.get());
        p.midFreqHz = static_cast<float> (mid_freq.get());
        p.midGainDb = static_cast<float> (mid_gain.get());
        p.highShelfGainDb = static_cast<float> (high_gain.get());
        p.eqEnabled = eq_on.get();
        p.outputGainDb = static_cast<float> (output.get());
        p.phaseInvert = phase_invert.get();
        core.setParameters (p);

        return oversampler.process (static_cast<float> (x), [this] (float oversampled) {
            return core.processSample (oversampled);
        });
    }

private:
    ee1073::ChannelStripCore core;
    ee_audio::FourTimesOversampler oversampler;
};

MIN_EXTERNAL (ee1073_tilde);

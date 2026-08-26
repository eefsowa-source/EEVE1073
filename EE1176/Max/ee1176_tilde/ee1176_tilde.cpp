// ee1176~ — Max/MSP external wrapping the shared EE-1176 compressor core,
// for use inside a Max for Live device in Ableton Live.
//
// Built with Cycling '74's min-devkit (https://github.com/Cycling74/min-devkit).
// See Max/CMakeLists.txt for build setup.

#include "c74_min.h"
#include "../../Shared/Ee1176CompressorCore.h"

using namespace c74::min;

class ee1176_tilde : public object<ee1176_tilde>, public sample_operator<1, 1>
{
public:
    MIN_DESCRIPTION { "EE-1176 FET-style compressor (1176 emulation core)" };
    MIN_TAGS { "audio, dynamics" };
    MIN_AUTHOR { "EON Audio" };
    MIN_RELATED { "comp~, gain~" };

    inlet<> in { this, "(signal) audio in" };
    outlet<> out { this, "(signal) audio out", "signal" };

    attribute<number> input { this, "input", 0.0,
        description { "Input gain in dB" } };
    attribute<number> output { this, "output", 0.0,
        description { "Output gain in dB" } };
    attribute<number> attack { this, "attack", 0.4,
        description { "Attack time in ms" } };
    attribute<number> release { this, "release", 300.0,
        description { "Release time in ms" } };
    attribute<int> ratio { this, "ratio", 0,
        description { "0=4:1 1=8:1 2=12:1 3=20:1 4=All(British)" } };

    void setup (double sampleRate)
    {
        core.prepare (sampleRate);
    }

    ee1176_tilde() { core.prepare (c74::max::sys_getsr()); }

    sample operator()(sample x)
    {
        ee1176::Parameters p;
        p.inputGainDb = static_cast<float> (input.get());
        p.outputGainDb = static_cast<float> (output.get());
        p.attackMs = static_cast<float> (attack.get());
        p.releaseMs = static_cast<float> (release.get());
        p.ratio = static_cast<ee1176::Ratio> (ratio.get());
        core.setParameters (p);

        return core.processSample (static_cast<float> (x));
    }

private:
    ee1176::CompressorCore core;
};

MIN_EXTERNAL (ee1176_tilde);

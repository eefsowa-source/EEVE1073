#include "../EE1073/Shared/Ee1073ChannelStripCore.h"
#include "../EE1176/Shared/Ee1176CompressorCore.h"
#include <cmath>
#include <iostream>

int main()
{
    ee1073::ChannelStripCore channel;
    channel.prepare (48000.0);
    ee1073::Parameters invalid;
    invalid.hpfEnabled = true;
    invalid.hpfFreqHz = 1.0e30f;
    invalid.lowShelfFreqHz = -1.0e30f;
    invalid.midFreqHz = 1.0e30f;
    invalid.lowShelfGainDb = 1.0e30f;
    invalid.midGainDb = -1.0e30f;
    channel.setParameters (invalid);
    for (int i = 0; i < 256; ++i)
        if (! std::isfinite (channel.processSample (0.25f)))
            return 1;

    ee1176::CompressorCore compressor;
    compressor.prepare (192000.0);
    ee1176::Parameters badCompressor;
    badCompressor.inputGainDb = 1.0e30f;
    badCompressor.attackMs = -1.0e30f;
    badCompressor.releaseMs = 1.0e30f;
    compressor.setParameters (badCompressor);
    for (int i = 0; i < 256; ++i)
        if (! std::isfinite (compressor.processSample (0.25f)))
            return 2;

    std::cout << "DSP sanity OK\n";
    return 0;
}

#include "../EE1073/Shared/Ee1073ChannelStripCore.h"

#include <cmath>
#include <iostream>

int main()
{
    constexpr double sampleRate = 192000.0;
    constexpr double frequencyHz = 1000.0;
    constexpr double amplitude = 0.12589254117941673; // -18 dBFS peak
    constexpr int totalSamples = static_cast<int> (sampleRate * 2.0);
    constexpr int settleSamples = static_cast<int> (sampleRate * 0.5);

    ee1073::ChannelStripCore channel;
    channel.prepare (sampleRate);
    channel.setParameters ({});

    double sum = 0.0;
    int measured = 0;
    for (int sample = 0; sample < totalSamples; ++sample)
    {
        const auto input = static_cast<float> (
            amplitude * std::sin (2.0 * M_PI * frequencyHz * sample / sampleRate));
        const auto output = channel.processSample (input);
        if (! std::isfinite (output))
            return 2;
        if (sample >= settleSamples)
        {
            sum += output;
            ++measured;
        }
    }

    const auto dcOffset = sum / measured;
    if (std::abs (dcOffset) > 0.001)
    {
        std::cerr << "EE-1073 DC offset exceeds 0.001: " << dcOffset << "\n";
        return 1;
    }
    return 0;
}

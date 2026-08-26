#pragma once

namespace ee_audio
{
class FourTimesOversampler
{
public:
    void reset (float value = 0.0f) noexcept
    {
        previousInput = value;
        previousOutput = value;
    }

    template <typename Processor>
    float process (float input, Processor&& processor) noexcept
    {
        const auto delta = (input - previousInput) * 0.25f;
        float outputSum = 0.0f;

        for (int phase = 1; phase <= 4; ++phase)
            outputSum += processor (previousInput + delta * static_cast<float> (phase));

        previousInput = input;
        previousOutput = outputSum * 0.25f;
        return previousOutput;
    }

private:
    float previousInput = 0.0f;
    float previousOutput = 0.0f;
};
}

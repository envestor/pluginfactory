#include "../saturation/SaturationStage.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace
{

std::vector<float> makeSine (float freqHz, float sampleRate, int numSamples)
{
    std::vector<float> out (static_cast<size_t> (numSamples));
    for (int i = 0; i < numSamples; ++i)
        out[static_cast<size_t> (i)] = std::sin (2.0f * static_cast<float> (M_PI) * freqHz * static_cast<float> (i) / sampleRate);
    return out;
}

// Runs `settleSamples` of silence through the stage first so the parameter
// smoothers reach their targets, then processes `input` and returns the
// result (same length as input, no offset).
std::vector<float> processSettled (bdpo::dsp::SaturationStage& stage, const std::vector<float>& input, int settleSamples)
{
    for (int i = 0; i < settleSamples; ++i)
        stage.processSample (0.0f);

    std::vector<float> out;
    out.reserve (input.size());
    for (float x : input)
        out.push_back (stage.processSample (x));
    return out;
}

} // namespace

int main()
{
    constexpr float sampleRate = 48000.0f;
    constexpr int numSamples = 4800; // 100 ms
    constexpr int settleSamples = 2000; // let the 5 ms smoother settle before measuring

    int failures = 0;

    // --- Test 1: unity gain at drive = 0 -----------------------------------
    {
        bdpo::dsp::SaturationStage stage;
        stage.prepare (sampleRate);
        stage.setDrive (0.0f);
        stage.setOutputTrimDb (0.0f);
        stage.setMix (1.0f);

        auto sine = makeSine (440.0f, sampleRate, numSamples);
        auto out = processSettled (stage, sine, settleSamples);

        float maxAbsError = 0.0f;
        for (size_t i = 0; i < out.size(); ++i)
            maxAbsError = std::max (maxAbsError, std::fabs (out[i] - sine[i]));

        const float tolerance = 0.01f; // k ~= 0.001 at drive=0, so tanh(kx)/tanh(k) ~= x
        if (maxAbsError > tolerance)
        {
            std::fprintf (stderr, "FAIL: unity gain at drive=0, max abs error %.6f > tolerance %.6f\n", maxAbsError, tolerance);
            ++failures;
        }
        else
        {
            std::printf ("PASS: unity gain at drive=0 (max abs error %.6f)\n", maxAbsError);
        }
    }

    // --- Test 2: no NaN / inf, across a null signal and a driven sine -----
    {
        bdpo::dsp::SaturationStage stage;
        stage.prepare (sampleRate);
        stage.setDrive (1.0f);
        stage.setOutputTrimDb (6.0f);
        stage.setMix (1.0f);

        std::vector<float> nullSignal (static_cast<size_t> (numSamples), 0.0f);
        auto nullOut = processSettled (stage, nullSignal, settleSamples);

        auto sine = makeSine (1000.0f, sampleRate, numSamples);
        auto sineOut = processSettled (stage, sine, settleSamples);

        bool anyBad = false;
        for (float y : nullOut)
            anyBad |= (std::isnan (y) || std::isinf (y));
        for (float y : sineOut)
            anyBad |= (std::isnan (y) || std::isinf (y));

        if (anyBad)
        {
            std::fprintf (stderr, "FAIL: NaN or Inf found in output\n");
            ++failures;
        }
        else
        {
            std::printf ("PASS: no NaN or Inf at drive=1, output trim=+6dB\n");
        }

        // Null signal through a symmetric shaper must stay at zero: no DC
        // offset can appear from silence.
        float maxAbsNullOut = 0.0f;
        for (float y : nullOut)
            maxAbsNullOut = std::max (maxAbsNullOut, std::fabs (y));
        if (maxAbsNullOut > 1.0e-6f)
        {
            std::fprintf (stderr, "FAIL: null signal produced non-zero output, max abs %.9f\n", maxAbsNullOut);
            ++failures;
        }
        else
        {
            std::printf ("PASS: null signal stays at zero (max abs %.9f)\n", maxAbsNullOut);
        }
    }

    // --- Test 3: no DC offset drift on a driven sine (symmetric curve) ----
    {
        bdpo::dsp::SaturationStage stage;
        stage.prepare (sampleRate);
        stage.setDrive (0.8f);
        stage.setOutputTrimDb (0.0f);
        stage.setMix (1.0f);

        auto sine = makeSine (300.0f, sampleRate, numSamples);
        auto out = processSettled (stage, sine, settleSamples);

        double sum = 0.0;
        for (float y : out)
            sum += y;
        const double mean = sum / static_cast<double> (out.size());

        const double tolerance = 1.0e-4; // ~ -80 dBFS DC floor target
        if (std::fabs (mean) > tolerance)
        {
            std::fprintf (stderr, "FAIL: DC offset drift %.9f > tolerance %.9f\n", mean, tolerance);
            ++failures;
        }
        else
        {
            std::printf ("PASS: no DC offset drift at drive=0.8 (mean %.9f)\n", mean);
        }
    }

    if (failures == 0)
    {
        std::printf ("All SaturationStage offline tests passed.\n");
        return 0;
    }

    std::fprintf (stderr, "%d SaturationStage offline test(s) failed.\n", failures);
    return 1;
}

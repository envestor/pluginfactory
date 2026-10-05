#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace bdpo::dsp
{

// Drive -> waveshaper -> output trim -> dry/wet mix.
//
// Oversampling is deliberately NOT implemented here. Every nonlinear stage
// we ship must be oversampled before release (Pirkle ch. 22), but this task
// is scoped to the bare waveshaper spike only. The hook point is marked
// below with TODO(SAT-oversampling) so the plugin wiring task can insert an
// upsample/process/downsample loop around processSample()/processBlock()
// without changing this class's public interface.
class SaturationStage
{
public:
    // Selects the waveshaping curve. Only Tanh is implemented in this spike;
    // the enum exists so later curves (Atan, FExp1, asymmetric tape curves)
    // can be added without changing callers.
    enum class Curve
    {
        Tanh
    };

    SaturationStage() = default;

    // Call once before processing starts, and again whenever the sample
    // rate changes. Resets the parameter smoothers so a stale ramp from the
    // previous rate can't leak into the new one.
    void prepare (double newSampleRate) noexcept
    {
        sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;

        // ~5 ms smoothing time constant, independent of sample rate, so
        // drive/output/mix changes don't zipper but still feel immediate.
        const double smoothingSeconds = 0.005;
        smoothingCoeff = static_cast<float> (std::exp (-1.0 / (smoothingSeconds * sampleRate)));

        reset();
    }

    // Clears smoother state to the current target values. Does not allocate.
    void reset() noexcept
    {
        smoothedDrive = targetDrive;
        smoothedOutputGain = targetOutputGain;
        smoothedMix = targetMix;
    }

    void setCurve (Curve newCurve) noexcept { curve = newCurve; }

    // normalizedDrive in [0, 1]. 0 maps to (near-)unity gain through the
    // shaper; 1 maps to the heaviest drive this stage offers.
    void setDrive (float normalizedDrive) noexcept
    {
        targetDrive = std::clamp (normalizedDrive, 0.0f, 1.0f);
    }

    // outputTrimDb in decibels, applied after the waveshaper.
    void setOutputTrimDb (float outputTrimDb) noexcept
    {
        targetOutputGain = dbToGain (outputTrimDb);
    }

    // dryWetMix in [0, 0 = fully dry, 1 = fully wet].
    void setMix (float dryWetMix) noexcept
    {
        targetMix = std::clamp (dryWetMix, 0.0f, 1.0f);
    }

    // Real-time safe: no allocation, no locks, no I/O, no logging.
    float processSample (float x) noexcept
    {
        // Per-sample one-pole smoothing (Pirkle Sec 2.5.2): cheap enough to
        // run every sample, so the drive/mix controls never zipper.
        smoothedDrive = smoothingCoeff * smoothedDrive + (1.0f - smoothingCoeff) * targetDrive;
        smoothedOutputGain = smoothingCoeff * smoothedOutputGain + (1.0f - smoothingCoeff) * targetOutputGain;
        smoothedMix = smoothingCoeff * smoothedMix + (1.0f - smoothingCoeff) * targetMix;

        // TODO(SAT-oversampling): this is where processSamplesUp() /
        // processSamplesDown() belong once oversampling is wired in. The
        // shaper below is transcendental (tanh), so it aliases at drive
        // unless it runs at an oversampled rate (Pirkle Sec 19.1, 19.4).
        const float wet = shape (x) * smoothedOutputGain;

        return smoothedMix * wet + (1.0f - smoothedMix) * x;
    }

    void processBlock (float* samples, int numSamples) noexcept
    {
        for (int i = 0; i < numSamples; ++i)
            samples[i] = processSample (samples[i]);
    }

private:
    static float dbToGain (float db) noexcept
    {
        return std::pow (10.0f, db / 20.0f);
    }

    // Normalized-drive -> waveshaper curvature k. k is kept away from exact
    // zero to avoid a 0/0 in the tanh normalisation below; as k -> 0 the
    // curve's limit is the identity function, which is what gives the
    // "unity gain at drive = 0" behaviour the offline test checks for.
    float driveToK (float normalizedDrive) const noexcept
    {
        constexpr float kMin = 0.001f;
        constexpr float kMax = 10.0f;
        return kMin + normalizedDrive * (kMax - kMin);
    }

    float shape (float x) const noexcept
    {
        switch (curve)
        {
            case Curve::Tanh:
            default:
                return tanhShaper (x, driveToK (smoothedDrive));
        }
    }

    // Normalised TANH curve (Pirkle Table 19.1): y = tanh(k*x) / tanh(k).
    // Symmetric, so it produces only odd harmonics and no DC offset, and it
    // maps [-1, 1] -> [-1, 1] for any k > 0.
    static float tanhShaper (float x, float k) noexcept
    {
        const float denom = std::tanh (k);
        if (denom == 0.0f)
            return x;
        return std::tanh (k * x) / denom;
    }

    double sampleRate { 44100.0 };
    float smoothingCoeff { 0.0f };

    Curve curve { Curve::Tanh };

    float targetDrive { 0.0f };
    float targetOutputGain { 1.0f };
    float targetMix { 1.0f };

    float smoothedDrive { 0.0f };
    float smoothedOutputGain { 1.0f };
    float smoothedMix { 1.0f };
};

} // namespace bdpo::dsp

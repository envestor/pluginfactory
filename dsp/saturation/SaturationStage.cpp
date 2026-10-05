#include "SaturationStage.h"

// SaturationStage is header-only (all methods are small and noexcept, and
// are expected to be inlined on the audio thread). This translation unit
// exists so the dsp library has a .cpp to compile and link, giving CI a
// concrete build target and giving future non-header-only growth (extra
// curves, a bigger lookup table) a home without an API change.

namespace bdpo::dsp
{
static_assert (sizeof (SaturationStage) > 0, "SaturationStage must be a complete type");
}

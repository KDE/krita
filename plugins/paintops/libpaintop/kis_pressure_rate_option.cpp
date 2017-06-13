#include "kis_pressure_rate_option.h"

KisPressureRateOption::KisPressureRateOption()
    : KisCurveOption("Rate", KisPaintOpOption::GENERAL, false)
{
}

double KisPressureRateOption::apply(const KisPaintInformation &info) const
{
    return isChecked() ? computeSizeLikeValue(info) : 1.0;
}

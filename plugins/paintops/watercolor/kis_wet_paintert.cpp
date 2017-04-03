#include "kis_wet_paintert.h"

struct KisWetPainter::Private
{
    Private(KisPaintDeviceSP _device, const KoColor &_color) : device(_device), color(_color) {}

    KisPaintDeviceSP device;
    const KoColor &color;
};

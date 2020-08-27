#ifndef KISFILTERFASTCOLOROVERLAY_H
#define KISFILTERFASTCOLOROVERLAY_H

#include "filter/kis_filter.h"


class KisFilterFastColorOverlay : public KisFilter
{
public:
    KisFilterFastColorOverlay();

    void processImpl(KisPaintDeviceSP device,
                     const QRect& rect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;

    static inline KoID id() {
        return KoID("fastcoloroverlay", i18n("Fast Color Overlay"));
    }

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP defaultConfiguration() const override;
};

#endif // KISFILTERFASTCOLOROVERLAY_H

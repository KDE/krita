#ifndef KIS_EDGE_DETECTION_FILTER_H
#define KIS_EDGE_DETECTION_FILTER_H

#include "filter/kis_filter.h"

#include <Eigen/Core>

class KisEdgeDetectionFilter : public KisFilter
{
public:
    KisEdgeDetectionFilter();
    void processImpl(KisPaintDeviceSP device,
                     const QRect& rect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    static inline KoID id() {
        return KoID("edge detection", i18n("Edge Detection"));
    }

    KisFilterConfigurationSP factoryConfiguration() const override;
public:
    //KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const override;
    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
};

#endif // KIS_EDGE_DETECTION_FILTER_H

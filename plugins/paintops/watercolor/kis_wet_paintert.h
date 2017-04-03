#ifndef KIS_WET_PAINTERT_H
#define KIS_WET_PAINTERT_H

#include <QScopedPointer>

#include "kis_types.h"

#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisWetPainter
{
public:
    KisMarkerPaintert(KisPaintDeviceSP device);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif /* KIS_WET_PAINTERT_H */

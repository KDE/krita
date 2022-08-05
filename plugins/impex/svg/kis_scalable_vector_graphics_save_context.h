#ifndef KISSCALABLEVECTORGRAPHICSSAVECONTEXT_H
#define KISSCALABLEVECTORGRAPHICSSAVECONTEXT_H

#include <kis_types.h>
#include <kis_meta_data_entry.h>

class QDomDocument;
class KoStore;

class KisScalableVectorGraphicsSaveContext
{
public:
    KisScalableVectorGraphicsSaveContext(KoStore *store);
    QString saveDeviceData(KisPaintDeviceSP dev, KisMetaData::Store *metaData, const QRect &imageRect, qreal xRes, qreal yRes);
    void saveStack(const QDomDocument& doc);

private:
    int m_id;
    KoStore *m_store;
};
#endif // KISSCALABLEVECTORGRAPHICSSAVECONTEXT_H

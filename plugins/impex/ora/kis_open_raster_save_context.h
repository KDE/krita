/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_OPEN_RASTER_SAVE_CONTEXT_H_
#define _KIS_OPEN_RASTER_SAVE_CONTEXT_H_

#include <kis_types.h>

class QDomDocument;
class KoStore;

#include <kis_meta_data_entry.h>

class KisOpenRasterSaveContext
{
public:
    KisOpenRasterSaveContext(KoStore *store);
    QString saveDeviceData(KisPaintDeviceSP dev, KisMetaData::Store *metaData, const QRect &imageRect, qreal xRes, qreal yRes);
    void saveStack(const QDomDocument& doc);
private:
    int m_id;
    KoStore *m_store;

};


#endif

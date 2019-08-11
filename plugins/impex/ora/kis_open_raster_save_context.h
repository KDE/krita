/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

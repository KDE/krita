/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _ORA_SAVE_CONTEXT_H_
#define _ORA_SAVE_CONTEXT_H_

class KoStore;
#include <kis_meta_data_entry.h>

#include "kis_open_raster_save_context.h"

class OraSaveContext : public KisOpenRasterSaveContext
{
public:
    OraSaveContext(KoStore* _store);
    ~OraSaveContext() override{}
    QString saveDeviceData(KisPaintDeviceSP dev, KisMetaData::Store *metaData, const QRect &imageRect, const qreal xRes, const qreal yRes) override;
    void saveStack(const QDomDocument& doc) override;
private:
    int m_id;
    KoStore* m_store;
};

#endif

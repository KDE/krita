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

#ifndef _KIS_OPEN_RASTER_LOAD_CONTEXT_H_
#define _KIS_OPEN_RASTER_LOAD_CONTEXT_H_

class QString;
class QDomDocument;
class KoStore;

#include <KoStoreDevice.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include "kis_png_converter.h"
#include <kis_types.h>

class KisOpenRasterLoadContext
{
public:
    KisOpenRasterLoadContext(KoStore *store);
    KisImageSP loadDeviceData(const QString &fileName);
    QDomDocument loadStack();
private:
    KoStore *m_store;
};


#endif

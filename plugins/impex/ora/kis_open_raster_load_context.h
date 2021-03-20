/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

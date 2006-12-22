/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_SOBEL_FILTER_H_
#define _KIS_SOBEL_FILTER_H_

#include "kis_filter.h"
#include "kis_filter_config_widget.h"

class KisSobelFilter : public KisFilter
{
public:
    KisSobelFilter();
public:
    virtual void process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* config);
    static inline KoID id() { return KoID("sobel", i18n("Sobel")); }
    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return true; }
public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
private:
    void prepareRow (const KisPaintDeviceSP src, quint8* data, quint32 x, quint32 y, quint32 w, quint32 h);
};

#endif

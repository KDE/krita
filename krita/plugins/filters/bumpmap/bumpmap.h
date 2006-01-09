/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef BUMPMAP_H
#define BUMPMAP_H

#include <qwidget.h>

#include <kparts/plugin.h>

#include <kis_types.h>
#include <kis_filter.h>
#include "kis_filter_config_widget.h"

class WdgBumpMap;

namespace krita {

    enum enumBumpmapType {
        LINEAR = 0,
        SPHERICAL = 1,
        SINUSOIDAL = 2
    };

}

using namespace krita;



class KritaBumpmap : public KParts::Plugin
{
public:
    KritaBumpmap(QObject *parent, const char *name, const QStringList &);
    virtual ~KritaBumpmap();
};


/**
 * First stab at a bumpmapping filter. For now, this is taken both
 * from the Gimp source and the code from emboss.c:
 *             ANSI C code from the article
 *             "Fast Embossing Effects on Raster Image Data"
 *             by John Schlag, jfs@kerner.com
 *             in "Graphics Gems IV", Academic Press, 1994
 */
class KisFilterBumpmap : public KisFilter
{
public:
    KisFilterBumpmap();
public:
    virtual void process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration*, const QRect&);
    static inline KisID id() { return KisID("bumpmap", i18n("Bumpmap")); };
    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return false; }
    virtual bool supportsIncrementalPainting() { return false; }

    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceImplSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*);

};



class KisBumpmapConfiguration : public KisFilterConfiguration {

public:

    KisBumpmapConfiguration();

    QString bumpmap();
    void setBumpmap(QString bumpmap);

    double  azimuth();
    void setAzimuth(double azimuth);

    double  elevation();
    void setElevation(double);

    double  depth();
    void setDepth(double depth);

    Q_INT32 xofs();
    void setXofs(Q_INT32 xofs);

    Q_INT32 yofs();
    void setYofs(Q_INT32 yofs);

    Q_INT32 waterlevel();
    void setWaterlevel(Q_INT32 waterlevel);

    Q_INT32 ambient();
    void setAmbient(Q_INT32 ambient);

    bool compensate();
    void setCompensate(bool compensate);

    bool invert();
    void setInvert(bool invert);

    bool tiled();
    void setTiled(bool tiled);

    enumBumpmapType type();
    void setType(enumBumpmapType type);

};


class KisBumpmapConfigWidget : public KisFilterConfigWidget {

    Q_OBJECT

public:
    KisBumpmapConfigWidget(KisFilter * filter, KisPaintDeviceImplSP dev, QWidget * parent, const char * name = 0, WFlags f = 0 );
    virtual ~KisBumpmapConfigWidget() {};

    KisBumpmapConfiguration * config();

    WdgBumpmap * m_page;

private:

    KisFilter * m_filter;
    KisPaintDeviceImplSP m_device;

};

#endif

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

#include "ui_wdgbumpmap.h"

class WdgBumpmap : public QWidget, public Ui::WdgBumpmap
{
    Q_OBJECT

    public:
        WdgBumpmap(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

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
    virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&);
    virtual ColorSpaceIndependence colorSpaceIndependence() { return TO_LAB16; };
    static inline KisID id() { return KisID("bumpmap", i18n("Bumpmap")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual bool supportsIncrementalPainting() { return true; }

    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration * configuration(QWidget*);
    virtual KisFilterConfiguration * configuration();

};



class KisBumpmapConfiguration : public KisFilterConfiguration {

public:

    KisBumpmapConfiguration();
    virtual void fromXML( const QString&  );
    virtual QString toString();

public:

    QString bumpmap;
    double  azimuth;
    double  elevation;
    double  depth;
    qint32 xofs;
    qint32 yofs;
    qint32 waterlevel;
    qint32 ambient;
    bool    compensate;
    bool    invert;
    bool    tiled;
    enumBumpmapType type;

};


class KisBumpmapConfigWidget : public KisFilterConfigWidget {

    Q_OBJECT

public:
    KisBumpmapConfigWidget(KisFilter * filter, KisPaintDeviceSP dev, QWidget * parent, const char * name = 0, Qt::WFlags f = 0 );
    virtual ~KisBumpmapConfigWidget() {};

    KisBumpmapConfiguration * config();
    void setConfiguration(KisFilterConfiguration * config);

    WdgBumpmap * m_page;

private:

    KisFilter * m_filter;
    KisPaintDeviceSP m_device;

};

#endif

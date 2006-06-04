/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_BRIGHTNESS_CONTRAST_FILTER_H_
#define _KIS_BRIGHTNESS_CONTRAST_FILTER_H_

#include <QList>

#include "kis_filter.h"
#include "kis_filter_config_widget.h"
#include "ui_wdg_brightness_contrast.h"

class QWidget;
class KoColorAdjustment;

class WdgBrightnessContrast : public QWidget, public Ui::WdgBrightnessContrast
{
    Q_OBJECT

    public:
        WdgBrightnessContrast(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class KisBrightnessContrastFilterConfiguration : public KisFilterConfiguration {

public:

    KisBrightnessContrastFilterConfiguration();
    virtual ~KisBrightnessContrastFilterConfiguration();
    virtual void fromXML( const QString&  );
    virtual QString toString();

public:
    quint16 transfer[256];
    QList<QPair<double,double> >  curve;
    KoColorAdjustment * m_adjustment;
};

/**
 * This class affect Intensity Y of the image
 */
class KisBrightnessContrastFilter : public KisFilter
{

public:

    KisBrightnessContrastFilter();

public:

    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration * configuration(QWidget *);
    virtual KisFilterConfiguration * configuration() { return new KisBrightnessContrastFilterConfiguration(); };
    virtual void process(KisPaintDeviceSP, KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    static inline KoID id() { return KoID("brightnesscontrast", i18n("Brightness / Contrast")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP dev);

    virtual ColorSpaceIndependence colorSpaceIndependence() { return TO_LAB16; };
    virtual bool workWith(KoColorSpace* cs);
};


class KisBrightnessContrastConfigWidget : public KisFilterConfigWidget {

public:
    KisBrightnessContrastConfigWidget(QWidget * parent, KisPaintDeviceSP dev, const char * name = 0, Qt::WFlags f = 0 );
    virtual ~KisBrightnessContrastConfigWidget() {};

    KisBrightnessContrastFilterConfiguration * config();
    void setConfiguration( KisFilterConfiguration * config );
    WdgBrightnessContrast * m_page;
};

#endif

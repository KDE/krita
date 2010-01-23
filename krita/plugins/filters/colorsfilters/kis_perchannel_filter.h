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

#ifndef _KIS_PERCHANNEL_FILTER_H_
#define _KIS_PERCHANNEL_FILTER_H_

#include <QPair>
#include <QList>

#include "filter/kis_color_transformation_filter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_config_widget.h"

#include "ui_wdg_perchannel.h"

class WdgPerChannel : public QWidget, public Ui::WdgPerChannel
{
    Q_OBJECT

public:
    WdgPerChannel(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class KisPerChannelFilterConfiguration
        : public KisFilterConfiguration
{
public:
    KisPerChannelFilterConfiguration(int n);
    ~KisPerChannelFilterConfiguration();

    using KisFilterConfiguration::fromXML;
    using KisFilterConfiguration::toXML;
    using KisFilterConfiguration::toLegacyXML;
    using KisFilterConfiguration::fromLegacyXML;

    virtual void fromLegacyXML(const QDomElement& root);
    virtual void toLegacyXML(QDomDocument& doc, QDomElement& root) const;

    virtual void fromXML(const QDomElement& e);
    virtual void toXML(QDomDocument& doc, QDomElement& root) const;

    void setCurves(QList<KisCubicCurve> &curves);
    static inline void initDefaultCurves(QList<KisCubicCurve> &curves, int nCh);
    bool isCompatible(const KisPaintDeviceSP) const;

public:
    QList<KisCubicCurve> m_curves;
    const KoColorSpace* oldCs;

};


/**
 * This class is generic for filters that affect channel separately
 */
class KisPerChannelFilter
        : public KisColorTransformationFilter
{
public:
    KisPerChannelFilter();
public:
    virtual KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image = 0) const;
    virtual KisFilterConfiguration * factoryConfiguration(const KisPaintDeviceSP) const;

    virtual KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const;

    static inline KoID id() {
        return KoID("perchannel", i18n("Color Adjustment"));
    }
private:
};

class KisPerChannelConfigWidget : public KisConfigWidget
{
    Q_OBJECT

public:
    KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WFlags f = 0);
    virtual ~KisPerChannelConfigWidget() {}

    virtual void setConfiguration(const KisPropertiesConfiguration* config);
    virtual KisPropertiesConfiguration * configuration() const;

private slots:
    virtual void setActiveChannel(int ch);
    void setPreview(int state);

private:

    // private routines
    inline QPixmap getHistogram();
    inline QPixmap createGradient(Qt::Orientation orient /*, int invert (not used now) */);

    // members
    WdgPerChannel * m_page;
    KisPaintDeviceSP m_dev;
    KisHistogram *m_histogram;
    mutable QList<KisCubicCurve> m_curves;
    int m_activeCh;

    // scales for displaying color numbers
    double m_scale;
    double m_shift;
};

#endif

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

#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_config_widget.h"

#include "ui_wdg_perchannel.h"

class WdgPerChannel : public QWidget, public Ui::WdgPerChannel
{
    Q_OBJECT

    public:
        WdgPerChannel(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

typedef QList<QPointF> KisCurve;

class KisPerChannelFilterConfiguration
    : public KisFilterConfiguration
{
public:
    KisPerChannelFilterConfiguration(int n);
    ~KisPerChannelFilterConfiguration();

    virtual void fromXML( const QString&  );
    virtual QString toString();

    bool isCompatible(const KisPaintDeviceSP) const;

public:
    QList<KisCurve> curves;
    quint16 **transfers;
    quint16 nTransfers;
    // Caching of adjustment
    bool dirty;
    KoColorSpace* oldCs;
    KoColorTransformation* adjustment;
};


/**
 * This class is generic for filters that affect channel separately
 */
class KisPerChannelFilter
    : public KisFilter
{
public:
    KisPerChannelFilter();
public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    void process(KisFilterConstantProcessingInformation src,
                 KisFilterProcessingInformation dst,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater = 0
        ) const;
    static inline KoID id() { return KoID("perchannel", i18n("Color Adjustment")); }
private:
};

class KisPerChannelConfigWidget : public KisFilterConfigWidget {

    typedef KisFilterConfigWidget super;
    Q_OBJECT

public:
    KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WFlags f = 0 );
    virtual ~KisPerChannelConfigWidget() {}

    virtual void setConfiguration(KisFilterConfiguration * config);
    virtual KisFilterConfiguration * configuration() const;

private slots:
    virtual void setActiveChannel(int ch);

private:
    WdgPerChannel * m_page;
    KisPaintDeviceSP m_dev;
    KisHistogram *m_histogram;
    mutable QList<KisCurve> m_curves;
    int m_activeCh;
};

#endif

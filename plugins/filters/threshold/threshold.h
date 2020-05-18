/*
 * This file is part of Krita
 *
 * Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef THRESHOLD_H
#define THRESHOLD_H

#include <QObject>
#include <QVariant>
#include <filter/kis_filter.h>
#include <kis_filter_configuration.h>
#include <kis_config_widget.h>

#include "ui_wdg_threshold.h"

class WdgThreshold;
class QWidget;
class KisHistogram;



class KritaThreshold : public QObject
{
    Q_OBJECT
public:
    KritaThreshold(QObject *parent, const QVariantList &);
    ~KritaThreshold() override;
};

class KisFilterThreshold : public KisFilter
{
public:
    KisFilterThreshold();
public:

    static inline KoID id() {
        return KoID("threshold", i18n("Threshold"));
    }

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater *progressUpdater) const override;

    KisFilterConfigurationSP defaultConfiguration() const override;

    KisConfigWidget *createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

};

class KisThresholdConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisThresholdConfigWidget(QWidget *parent, KisPaintDeviceSP dev);
    ~KisThresholdConfigWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    Ui::WdgThreshold m_page;

private Q_SLOTS:
    void slotDrawHistogram(bool logarithmic = false);

    void slotSetThreshold(int);


protected:
    QScopedPointer<KisHistogram> m_histogram;
    bool m_histlog;
};


#endif


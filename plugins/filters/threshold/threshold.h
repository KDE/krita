/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

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


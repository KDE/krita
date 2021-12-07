/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HALFTONE_CONFIG_PAGE_WIDGET_H
#define KIS_HALFTONE_CONFIG_PAGE_WIDGET_H

#include <QStringList>

#include <kis_paint_device.h>

#include "ui_KisHalftoneConfigPageWidget.h"
#include "KisHalftoneFilterConfiguration.h"

class KisViewManager;

class KisHalftoneFilterConfiguration;

class KoCanvasResourcesInterface;
using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;

class KisHalftoneConfigPageWidget : public QWidget
{
    Q_OBJECT
public:
    KisHalftoneConfigPageWidget(QWidget *parent, const KisPaintDeviceSP dev);
    ~KisHalftoneConfigPageWidget();

    void showColors();
    void hideColors();
    void setColorsVisible(bool show);

    void setConfiguration(const KisHalftoneFilterConfigurationSP config, const QString & prefix);
    void configuration(KisHalftoneFilterConfigurationSP config, const QString & prefix) const;

    void setView(KisViewManager *view);
    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);

private:
    Ui_HalftoneConfigPageWidget m_ui;
    const KisPaintDeviceSP m_paintDevice;
    QStringList m_generatorIds;
    KisConfigWidget *m_generatorWidget;
    KisViewManager *m_view;
    KoCanvasResourcesInterfaceSP m_canvasResourcesInterface;

    const Ui_HalftoneConfigPageWidget* ui() const;
    Ui_HalftoneConfigPageWidget* ui();

    void setGenerator(const QString & generatorId, const KisFilterConfigurationSP config);
    
Q_SIGNALS:
    void signal_configurationUpdated();

private Q_SLOTS:
    void slot_comboBoxGenerator_currentIndexChanged(int index);
};

#endif

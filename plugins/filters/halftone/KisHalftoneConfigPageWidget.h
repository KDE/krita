/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_HALFTONE_CONFIG_PAGE_WIDGET_H
#define KIS_HALFTONE_CONFIG_PAGE_WIDGET_H

#include <QStringList>

#include <kis_paint_device.h>

#include "ui_KisHalftoneConfigPageWidget.h"
#include "KisHalftoneFilterConfiguration.h"

class KisHalftoneFilterConfiguration;

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

private:
    Ui_HalftoneConfigPageWidget m_ui;
    const KisPaintDeviceSP m_paintDevice;
    QStringList m_generatorIds;
    KisConfigWidget *m_generatorWidget;

    const Ui_HalftoneConfigPageWidget* ui() const;
    Ui_HalftoneConfigPageWidget* ui();

    void setGenerator(const QString & generatorId, const KisFilterConfigurationSP config);
    
Q_SIGNALS:
    void signal_configurationUpdated();

private Q_SLOTS:
    void slot_comboBoxGenerator_currentIndexChanged(int index);
};

#endif

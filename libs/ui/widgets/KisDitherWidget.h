/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2019 Carl Olsson <carl.olsson@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DITHER_WIDGET_H
#define KIS_DITHER_WIDGET_H

#include <kritaui_export.h>
#include <QWidget>
#include "ui_KisDitherWidget.h"

class KisResourceItemChooser;
class KisPropertiesConfiguration;
class KisFilterConfiguration;

class KRITAUI_EXPORT KisDitherWidget : public QWidget, public Ui::KisDitherWidget
{
    Q_OBJECT
public:
    KisDitherWidget(QWidget* parent = 0);
    void setConfiguration(const KisFilterConfiguration &config, const QString &prefix = "");
    void configuration(KisPropertiesConfiguration &config, const QString &prefix = "") const;
    static void factoryConfiguration(KisPropertiesConfiguration &config, const QString &prefix = "");
    static QList<KoResourceSP> prepareLinkedResources(const KisFilterConfiguration &config, const QString &prefix, KisResourcesInterfaceSP resourcesInterface);

Q_SIGNALS:
    void sigConfigurationItemChanged();
private:
    KisResourceItemChooser* m_ditherPatternWidget;
};

#endif

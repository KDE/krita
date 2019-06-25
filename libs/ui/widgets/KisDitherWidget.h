/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2019 Carl Olsson <carl.olsson@gmail.com>
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

#ifndef KIS_DITHER_WIDGET_H
#define KIS_DITHER_WIDGET_H

#include <kritaui_export.h>
#include <QWidget>
#include "ui_KisDitherWidget.h"

class KisResourceItemChooser;
class KisPropertiesConfiguration;

class KRITAUI_EXPORT KisDitherWidget : public QWidget, public Ui::KisDitherWidget
{
    Q_OBJECT
public:
    KisDitherWidget(QWidget* parent = 0);
    void setConfiguration(const KisPropertiesConfiguration &config, const QString &prefix = "");
    void configuration(KisPropertiesConfiguration &config, const QString &prefix = "") const;
    static void factoryConfiguration(KisPropertiesConfiguration &config, const QString &prefix = "");
Q_SIGNALS:
    void sigConfigurationItemChanged();
private:
    KisResourceItemChooser* m_ditherPatternWidget;
};

#endif

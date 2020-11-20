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

#ifndef KISGRADIENTGENERATORCONFIGWIDGET_H
#define KISGRADIENTGENERATORCONFIGWIDGET_H

#include <kis_config_widget.h>
#include <KoAbstractGradient.h>

#include "ui_KisGradientGeneratorConfigWidget.h"

class KisViewManager;

class KisGradientGeneratorConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisGradientGeneratorConfigWidget(QWidget* parent = 0);
    ~KisGradientGeneratorConfigWidget() override;
    
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

    void setView(KisViewManager *view) override;

private:
    Ui_GradientGeneratorConfigWidget m_ui;
    KisViewManager *m_view;
    KoAbstractGradientSP m_gradient;

private Q_SLOTS:
    void slot_widgetGradientChooser_resourceSelected(KoResource *resource);
    void slot_radioButtonEndPositionCartesianCoordinates_toggled(bool enabled);
    void slot_radioButtonEndPositionPolarCoordinates_toggled(bool enabled);

};

#endif

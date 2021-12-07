/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);

private:
    Ui_GradientGeneratorConfigWidget m_ui;
    KisViewManager *m_view;

private Q_SLOTS:
    void slot_radioButtonEndPositionCartesianCoordinates_toggled(bool enabled);
    void slot_radioButtonEndPositionPolarCoordinates_toggled(bool enabled);

};

#endif

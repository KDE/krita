/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONECONFIGWIDGET_H
#define KISSCREENTONECONFIGWIDGET_H

#include <kis_config_widget.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "ui_KisScreentoneConfigWidget.h"

class Ui_WdgScreentoneOptions;

class KisScreentoneConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisScreentoneConfigWidget(QWidget* parent = 0, const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8());
    ~KisScreentoneConfigWidget() override;
public:
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

private:
    Ui_ScreentoneConfigWidget m_ui;
    const KoColorSpace *m_colorSpace;

    void setupPatternComboBox();
    void setupShapeComboBox();
    void setupInterpolationComboBox();

private Q_SLOTS:
    void slot_comboBoxPattern_currentIndexChanged(int);
    void slot_comboBoxShape_currentIndexChanged(int);
    void slot_sliderSizeX_valueChanged(qreal value);
    void slot_sliderSizeY_valueChanged(qreal value);
    void slot_buttonKeepSizeSquare_keepAspectRatioChanged(bool keep);

};

#endif

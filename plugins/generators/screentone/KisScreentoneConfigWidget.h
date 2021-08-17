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

class KisViewManager;
class Ui_WdgScreentoneOptions;

class KisScreentoneConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisScreentoneConfigWidget(QWidget* parent = 0, const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8());
    ~KisScreentoneConfigWidget() override;

    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

    void setView(KisViewManager *view) override;

private:
    Ui_ScreentoneConfigWidget m_ui;
    KisViewManager *m_view;
    const KoColorSpace *m_colorSpace;
    QString m_lastSelectedInterpolationText;

    void setupPatternComboBox();
    void setupShapeComboBox();
    void setupInterpolationComboBox();

    int shapeToComboIndex(int pattern, int shape) const;
    int comboIndexToShape(int patterIndex, int shapeIndex) const;

private Q_SLOTS:
    void slot_comboBoxPattern_currentIndexChanged(int);
    void slot_comboBoxShape_currentIndexChanged(int);
    
    void slot_buttonSimpleTransformation_toggled(bool checked);
    void slot_buttonAdvancedTransformation_toggled(bool checked);
    void slot_comboBoxUnits_currentIndexChanged(int index);
    void slot_buttonResolutionFromImage_clicked();
    void slot_sliderFrequencyX_valueChanged(qreal value);
    void slot_sliderFrequencyY_valueChanged(qreal value);
    void slot_buttonConstrainFrequency_keepAspectRatioChanged(bool keep);
    void slot_sliderSizeX_valueChanged(qreal value);
    void slot_sliderSizeY_valueChanged(qreal value);
    void slot_buttonConstrainSize_keepAspectRatioChanged(bool keep);
    void slot_sliderAlignToPixelGridX_valueChanged(int value);
    void slot_sliderAlignToPixelGridY_valueChanged(int value);

    void slot_setAdvancedFromSimpleTransformation();
    void slot_setSimpleFromAdvancedTransformation();
};

#endif

/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CONVOLVEMATRIXEFFECTCONFIGWIDGET_H
#define CONVOLVEMATRIXEFFECTCONFIGWIDGET_H

#include "KoFilterEffectConfigWidgetBase.h"

class QDoubleSpinBox;
class KoFilterEffect;
class ConvolveMatrixEffect;
class KComboBox;
class QSpinBox;
class QCheckBox;
class MatrixDataModel;

class ConvolveMatrixEffectConfigWidget : public KoFilterEffectConfigWidgetBase
{
    Q_OBJECT
public:
    explicit ConvolveMatrixEffectConfigWidget(QWidget *parent = 0);

    /// reimplemented from KoFilterEffectConfigWidgetBase
    virtual bool editFilterEffect(KoFilterEffect *filterEffect);

private Q_SLOTS:
    void orderChanged(int value);
    void targetChanged(int value);
    void divisorChanged(double divisor);
    void biasChanged(double bias);
    void edgeModeChanged(int mode);
    void preserveAlphaChanged(bool checked);
    void editKernel();
    void kernelChanged();
private:
    ConvolveMatrixEffect *m_effect;
    KComboBox *m_edgeMode;
    QSpinBox *m_orderX;
    QSpinBox *m_orderY;
    QSpinBox *m_targetX;
    QSpinBox *m_targetY;
    QDoubleSpinBox *m_divisor;
    QDoubleSpinBox *m_bias;
    QCheckBox *m_preserveAlpha;
    MatrixDataModel *m_matrixModel;
};

#endif // CONVOLVEMATRIXEFFECTCONFIGWIDGET_H

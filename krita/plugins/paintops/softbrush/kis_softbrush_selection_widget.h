/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_SOFTBRUSH_SELECTION_WIDGET_H
#define KIS_SOFTBRUSH_SELECTION_WIDGET_H

class QTabWidget;

#include <QWidget>

#include <kis_cubic_curve.h>

#include "ui_wdgsoftoptions.h"
#include "ui_wdgsoftcurveoptions.h"
class KisSoftOpOptionsWidget: public QWidget, public Ui::WdgSoftOptions
{
public:
    KisSoftOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class KisSoftCurveOptionsWidget: public QWidget, public Ui::WdgSoftCurveOptions
{
public:
    KisSoftCurveOptionsWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
        KisCubicCurve topLeftBottomRightLinearCurve;
        topLeftBottomRightLinearCurve.setPoint(0, QPointF(0.0,1.0));
        topLeftBottomRightLinearCurve.setPoint(1, QPointF(1.0,0.0));
        softCurve->setCurve(topLeftBottomRightLinearCurve);
    }
};


/**
 * Compound widget that contain Curve soft brush and Gaussian soft brush
 */
class KisSoftBrushSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    KisSoftBrushSelectionWidget(QWidget * parent = 0);

    ~KisSoftBrushSelectionWidget();

    void setCurveBrush(bool on);
    void setGaussianBrush(bool on);

    void setCurrentBrushSize(qreal size);
    qreal currentBrushSize();

public:
    QTabWidget * m_brushesTab;
    KisSoftOpOptionsWidget * m_gaussBrushTip;
    KisSoftCurveOptionsWidget * m_curveBrushTip;
};

#endif

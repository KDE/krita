/*
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintCurveOptionRangeControlsStrategy.h"

#include <klocalizedstring.h>

#include <MyPaintCurveRangeModel.h>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <kis_slider_spin_box.h>
#include <KisWidgetConnectionUtils.h>
#include <KisMpl.h>
#include <kis_debug.h>

MyPaintCurveOptionRangeControlsStrategy::MyPaintCurveOptionRangeControlsStrategy(KisCurveRangeModelInterface *rangeInterface, QWidget *rangeControlsPlaceholder)
    : m_rangeModel(dynamic_cast<MyPaintCurveRangeModel*>(rangeInterface))
    , m_xValueSuffix(m_rangeModel->xValueSuffix())
    , m_yValueSuffix(m_rangeModel->yValueSuffix())
{
    using namespace KisWidgetConnectionUtils;
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_rangeModel);

    KisDoubleSliderSpinBox *yLimitSlider = new KisDoubleSliderSpinBox(rangeControlsPlaceholder);
    yLimitSlider->setPrefix(i18n("Y limit: "));
    yLimitSlider->setRange(0.0, m_rangeModel->maxYRange(), 2);
    m_yValueSuffix.bind(std::bind(&KisDoubleSliderSpinBox::setSuffix, yLimitSlider, std::placeholders::_1));
    connectControl(yLimitSlider, m_rangeModel, "yLimit");

    KisDoubleSliderSpinBox *xMin = new KisDoubleSliderSpinBox(rangeControlsPlaceholder);
    xMin->setPrefix(i18n("X min: "));
    m_xValueSuffix.bind(std::bind(&KisDoubleSliderSpinBox::setSuffix, xMin, std::placeholders::_1));
    connectControlState(xMin, m_rangeModel, "xMinState", "xMin");

    KisDoubleSliderSpinBox *xMax = new KisDoubleSliderSpinBox(rangeControlsPlaceholder);
    xMax->setPrefix(i18n("X max: "));
    m_xValueSuffix.bind(std::bind(&KisDoubleSliderSpinBox::setSuffix, xMax, std::placeholders::_1));
    connectControlState(xMax, m_rangeModel, "xMaxState", "xMax");

    QHBoxLayout *xRangeLayout = new QHBoxLayout();
    xRangeLayout->addWidget(xMin);
    xRangeLayout->addWidget(xMax);

    QVBoxLayout *layout = new QVBoxLayout(rangeControlsPlaceholder);
    layout->addWidget(yLimitSlider);
    layout->addLayout(xRangeLayout);

}

MyPaintCurveOptionRangeControlsStrategy::~MyPaintCurveOptionRangeControlsStrategy()
{

}

KisCurveOptionRangeControlsStrategyFactory MyPaintCurveOptionRangeControlsStrategy::factory()
{
    return [] (KisCurveRangeModelInterface *rangeInterface, QWidget *rangeControlsPlaceholder) {
        return new MyPaintCurveOptionRangeControlsStrategy(rangeInterface, rangeControlsPlaceholder);
    };
}

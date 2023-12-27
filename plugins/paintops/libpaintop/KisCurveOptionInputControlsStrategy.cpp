/*
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCurveOptionInputControlsStrategy.h"

#include <QHBoxLayout>
#include <QFont>

#include <QSpinBox>
#include <QDoubleSpinBox>

#include <kis_algebra_2d.h>
#include <KisMpl.h>

#include <kis_curve_widget.h>
#include <KisCurveWidgetControlsManager.h>
#include <KisCurveRangeModelInterface.h>



template<typename SpinBox>
KisCurveOptionInputControlsStrategy<SpinBox>::
KisCurveOptionInputControlsStrategy(KisCurveRangeModelInterface *rangeInterface,
                                    KisCurveWidget *curveWidget,
                                    QWidget *inPlaceholder, QWidget *outPlaceholder)
{
    this->curveWidget = curveWidget;

    inSpinBox = new SpinBox(inPlaceholder);
    outSpinBox = new SpinBox(outPlaceholder);

    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(inSpinBox->sizePolicy().hasHeightForWidth());
    inSpinBox->setSizePolicy(sizePolicy);
    outSpinBox->setSizePolicy(sizePolicy);

    inSpinBox->setMinimumSize(QSize(0, 0));
    outSpinBox->setMinimumSize(QSize(0, 0));

    QFont font;
    font.setPointSize(9);
    inSpinBox->setFont(font);
    outSpinBox->setFont(font);

    QHBoxLayout *inLayout = new QHBoxLayout(inPlaceholder);
    inLayout->addWidget(inSpinBox);
    inLayout->setMargin(0);

    QHBoxLayout *outLayout = new QHBoxLayout(outPlaceholder);
    outLayout->addWidget(outSpinBox);
    outLayout->setMargin(0);

    xValueSuffix = rangeInterface->xValueSuffix();
    yValueSuffix = rangeInterface->yValueSuffix();
    rangeValues = lager::with(rangeInterface->xMinValue(), rangeInterface->xMaxValue(),
                              rangeInterface->yMinValue(), rangeInterface->yMaxValue());

    xValueSuffix.bind(std::bind(&SpinBox::setSuffix, inSpinBox, std::placeholders::_1));
    yValueSuffix.bind(std::bind(&SpinBox::setSuffix, outSpinBox, std::placeholders::_1));
    rangeValues.bind(
                kismpl::unzip_wrapper(std::bind(&KisCurveOptionInputControlsStrategy::updateCurveLabels, this,
                                                std::placeholders::_1, std::placeholders::_2,
                                                std::placeholders::_3, std::placeholders::_4)));

}

template<typename SpinBox>
KisCurveOptionInputControlsStrategy<SpinBox>::~KisCurveOptionInputControlsStrategy()
{
}

template<typename SpinBox>
KisCurveOptionInputControlsStrategyFactory KisCurveOptionInputControlsStrategy<SpinBox>::factory()
{
    return
        [] (KisCurveRangeModelInterface *rangeInterface,
            KisCurveWidget *curveWidget,
            QWidget *inPlaceholder, QWidget *outPlaceholder) {
            return new KisCurveOptionInputControlsStrategy<SpinBox>(rangeInterface,
                                                                    curveWidget,
                                                                    inPlaceholder,
                                                                    outPlaceholder);

        };
}

template<typename SpinBox>
void KisCurveOptionInputControlsStrategy<SpinBox>::updateCurveLabels(qreal xMin, qreal xMax, qreal yMin, qreal yMax)
{
    using ValueType = typename KisCurveWidgetControlsManager<SpinBox>::ValueType;

    curveControlsManager.reset(
                new KisCurveWidgetControlsManager<SpinBox>(
                    curveWidget,
                    inSpinBox, outSpinBox,
                    KisAlgebra2D::lazyRound<ValueType>(xMin),
                    KisAlgebra2D::lazyRound<ValueType>(xMax),
                    KisAlgebra2D::lazyRound<ValueType>(yMin),
                    KisAlgebra2D::lazyRound<ValueType>(yMax)));
}

template class PAINTOP_EXPORT_INSTANCE KisCurveOptionInputControlsStrategy<QSpinBox>;
template class PAINTOP_EXPORT_INSTANCE KisCurveOptionInputControlsStrategy<QDoubleSpinBox>;

/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCurveWidgetControlsManager.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <kis_curve_widget.h>
#include <kis_signals_blocker.h>
#include "kis_debug.h"

#include <KisSpinBoxSplineUnitConverter.h>

namespace detail {
qreal io2sp(int x, int min, int max)
{
    KisSpinBoxSplineUnitConverter unitConverter;
    return unitConverter.io2sp(x, min, max);
}

int sp2io(qreal x, int min, int max)
{
    KisSpinBoxSplineUnitConverter unitConverter;
    return unitConverter.sp2io(x, min, max);
}

qreal io2sp(qreal x, qreal min, qreal max)
{
    const qreal rangeLen = max - min;
    return !qFuzzyIsNull(rangeLen) ? (x - min) / rangeLen : 0.0;
}

qreal sp2io(qreal x, qreal min, qreal max)
{
    const qreal rangeLen = max - min;
    return x * rangeLen + min;
}

void setupStep(QSpinBox *spinBox, int min, int max) {
    Q_UNUSED(min);
    Q_UNUSED(max);
    spinBox->setSingleStep(1);
}

void setupStep(QDoubleSpinBox *spinBox, qreal min, qreal max) {
    if (qAbs(max - min) > 10) {
        spinBox->setSingleStep(1.0);
    } else {
        spinBox->setSingleStep(0.1);
    }
}

bool willChangeSpinBox(const QSpinBox *spinBox, int newValue)
{
    return spinBox->value() != newValue;
}

bool willChangeSpinBox(const QDoubleSpinBox *spinBox, qreal newValue)
{
    return qRound(spinBox->value() * spinBox->decimals()) !=
        qRound(newValue * spinBox->decimals());
}

} // namespace detail

KisCurveWidgetControlsManagerBase::KisCurveWidgetControlsManagerBase(KisCurveWidget *curveWidget)
    : QObject(curveWidget)
    , m_curveWidget(curveWidget)
{
    connect(m_curveWidget, &KisCurveWidget::shouldSyncIOControls, this, &KisCurveWidgetControlsManagerBase::syncIOControls);
    connect(m_curveWidget, &KisCurveWidget::shouldFocusIOControls, this, &KisCurveWidgetControlsManagerBase::focusIOControls);
}

KisCurveWidgetControlsManagerBase::~KisCurveWidgetControlsManagerBase()
{
}


template <typename SpinBox>
KisCurveWidgetControlsManager<SpinBox>::
KisCurveWidgetControlsManager(KisCurveWidget *curveWidget)
    : KisCurveWidgetControlsManagerBase(curveWidget)
{

}

template <typename SpinBox>
KisCurveWidgetControlsManager<SpinBox>::
KisCurveWidgetControlsManager(KisCurveWidget *curveWidget,
                                 SpinBox *in, SpinBox *out,
                                 ValueType inMin, ValueType inMax,
                                 ValueType outMin, ValueType outMax)
    : KisCurveWidgetControlsManager(curveWidget)
{
    setupInOutControls(in, out, inMin, inMax, outMin, outMax);
}

template <typename SpinBox>
KisCurveWidgetControlsManager<SpinBox>::
~KisCurveWidgetControlsManager()
{
}

template <typename SpinBox>
void KisCurveWidgetControlsManager<SpinBox>::
setupInOutControls(SpinBox *in, SpinBox *out,
                   ValueType inMin, ValueType inMax,
                   ValueType outMin, ValueType outMax)
{
    dropInOutControls();

    m_intIn = in;
    m_intOut = out;

    if (!m_intIn || !m_intOut)
        return;

    m_inMin = inMin;
    m_inMax = inMax;
    m_outMin = outMin;
    m_outMax = outMax;

    ValueType realInMin = qMin(inMin, inMax); // tilt elevation has range (90, 0), which QSpinBox can't handle
    ValueType realInMax = qMax(inMin, inMax);

    m_intIn->setRange(realInMin, realInMax);
    m_intOut->setRange(m_outMin, m_outMax);

    detail::setupStep(m_intIn, realInMin, realInMax);
    detail::setupStep(m_intOut, m_outMin, m_outMax);

    connect(m_intIn, qOverload<ValueType>(&SpinBox::valueChanged), this, &KisCurveWidgetControlsManager::inOutChanged, Qt::UniqueConnection);
    connect(m_intOut, qOverload<ValueType>(&SpinBox::valueChanged), this, &KisCurveWidgetControlsManager::inOutChanged, Qt::UniqueConnection);

    syncIOControls();
}

template <typename SpinBox>
void KisCurveWidgetControlsManager<SpinBox>::
dropInOutControls()
{
    if (!m_intIn || !m_intOut)
        return;

    disconnect(m_intIn, qOverload<ValueType>(&SpinBox::valueChanged), this, &KisCurveWidgetControlsManager::inOutChanged);
    disconnect(m_intOut, qOverload<ValueType>(&SpinBox::valueChanged), this, &KisCurveWidgetControlsManager::inOutChanged);

    m_intIn = m_intOut = nullptr;

}

template <typename SpinBox>
void KisCurveWidgetControlsManager<SpinBox>::
inOutChanged()
{
    QPointF pt;

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_curveWidget->currentPoint());

    pt.setX(detail::io2sp(m_intIn->value(), m_inMin, m_inMax));
    pt.setY(detail::io2sp(m_intOut->value(), m_outMin, m_outMax));

    if (m_curveWidget->setCurrentPoint(pt)) {
        syncIOControls();
    }
}

template <typename SpinBox>
void KisCurveWidgetControlsManager<SpinBox>::
syncIOControls()
{
    if (!m_intIn || !m_intOut)
        return;

    std::optional<QPointF> currentPoint = m_curveWidget->currentPoint();

    m_intIn->setEnabled(currentPoint.has_value());
    m_intOut->setEnabled(currentPoint.has_value());

    if (currentPoint) {
        KisSignalsBlocker b(m_intIn, m_intOut);

        const ValueType inValue = detail::sp2io(currentPoint->x(), m_inMin, m_inMax);
        const ValueType outValue = detail::sp2io(currentPoint->y(), m_outMin, m_outMax);

        if (detail::willChangeSpinBox(m_intIn, inValue)) {
            m_intIn->setValue(inValue);
        }

        if (detail::willChangeSpinBox(m_intOut, outValue)) {
            m_intOut->setValue(outValue);
        }
    } else {
        /*FIXME: Ideally, these controls should hide away now */
    }
}

template <typename SpinBox>
void KisCurveWidgetControlsManager<SpinBox>::
focusIOControls()
{
    if (m_intIn) {
        m_intIn->setFocus();
    }
}

template class KRITAUI_EXPORT_INSTANCE KisCurveWidgetControlsManager<QSpinBox>;
template class KRITAUI_EXPORT_INSTANCE KisCurveWidgetControlsManager<QDoubleSpinBox>;

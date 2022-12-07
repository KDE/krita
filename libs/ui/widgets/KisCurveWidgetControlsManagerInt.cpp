/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCurveWidgetControlsManagerInt.h"

#include <kis_curve_widget.h>
#include <kis_signals_blocker.h>

#include <QSpinBox>
#include <KisSpinBoxSplineUnitConverter.h>

KisCurveWidgetControlsManagerInt::KisCurveWidgetControlsManagerInt(KisCurveWidget *curveWidget)
    : QObject(curveWidget)
    , m_curveWidget(curveWidget)
{
    connect(m_curveWidget, SIGNAL(shouldSyncIOControls()), this, SLOT(syncIOControls()));
    connect(m_curveWidget, SIGNAL(shouldFocusIOControls()), this, SLOT(focusIOControls()));
}

KisCurveWidgetControlsManagerInt::KisCurveWidgetControlsManagerInt(KisCurveWidget *curveWidget, QSpinBox *in, QSpinBox *out, int inMin, int inMax, int outMin, int outMax)
    : KisCurveWidgetControlsManagerInt(curveWidget)
{
    setupInOutControls(in, out, inMin, inMax, outMin, outMax);
}

KisCurveWidgetControlsManagerInt::~KisCurveWidgetControlsManagerInt()
{
}

void KisCurveWidgetControlsManagerInt::setupInOutControls(QSpinBox *in, QSpinBox *out, int inMin, int inMax, int outMin, int outMax)
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

    int realInMin = qMin(inMin, inMax); // tilt elevation has range (90, 0), which QSpinBox can't handle
    int realInMax = qMax(inMin, inMax);

    m_intIn->setRange(realInMin, realInMax);
    m_intOut->setRange(m_outMin, m_outMax);

    connect(m_intIn, SIGNAL(valueChanged(int)), this, SLOT(inOutChanged()), Qt::UniqueConnection);
    connect(m_intOut, SIGNAL(valueChanged(int)), this, SLOT(inOutChanged()), Qt::UniqueConnection);

    syncIOControls();
}

void KisCurveWidgetControlsManagerInt::dropInOutControls()
{
    if (!m_intIn || !m_intOut)
        return;

    disconnect(m_intIn, SIGNAL(valueChanged(int)), this, SLOT(inOutChanged()));
    disconnect(m_intOut, SIGNAL(valueChanged(int)), this, SLOT(inOutChanged()));

    m_intIn = m_intOut = 0;

}

void KisCurveWidgetControlsManagerInt::inOutChanged()
{
    QPointF pt;

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_curveWidget->currentPoint());

    pt.setX(io2sp(m_intIn->value(), m_inMin, m_inMax));
    pt.setY(io2sp(m_intOut->value(), m_outMin, m_outMax));

    if (m_curveWidget->setCurrentPoint(pt)) {
        syncIOControls();
    }
}

void KisCurveWidgetControlsManagerInt::syncIOControls()
{
    if (!m_intIn || !m_intOut)
        return;

    std::optional<QPointF> currentPoint = m_curveWidget->currentPoint();

    m_intIn->setEnabled(currentPoint.has_value());
    m_intOut->setEnabled(currentPoint.has_value());

    if (currentPoint) {
        KisSignalsBlocker b(m_intIn, m_intOut);

        m_intIn->setValue(sp2io(currentPoint->x(), m_inMin, m_inMax));
        m_intOut->setValue(sp2io(currentPoint->y(), m_outMin, m_outMax));
    } else {
        /*FIXME: Ideally, these controls should hide away now */
    }
}

void KisCurveWidgetControlsManagerInt::focusIOControls()
{
    if (m_intIn) {
        m_intIn->setFocus();
    }
}

double KisCurveWidgetControlsManagerInt::io2sp(int x, int min, int max)
{
    KisSpinBoxSplineUnitConverter unitConverter;
    return unitConverter.io2sp(x, min, max);
}

int KisCurveWidgetControlsManagerInt::sp2io(double x, int min, int max)
{
    KisSpinBoxSplineUnitConverter unitConverter;
    return unitConverter.sp2io(x, min, max);
}


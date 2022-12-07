/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURVEWIDGETCONTROLSMANAGERINT_H
#define KISCURVEWIDGETCONTROLSMANAGERINT_H

#include <kritaui_export.h>
#include <QObject>

class QSpinBox;
class KisCurveWidget;

class KRITAUI_EXPORT KisCurveWidgetControlsManagerInt : public QObject
{
    Q_OBJECT
public:
    KisCurveWidgetControlsManagerInt(KisCurveWidget *curveWidget);
    KisCurveWidgetControlsManagerInt(KisCurveWidget *curveWidget, QSpinBox *in, QSpinBox *out, int inMin, int inMax, int outMin, int outMax);
    ~KisCurveWidgetControlsManagerInt();

    void setupInOutControls(QSpinBox *in, QSpinBox *out, int inMin, int inMax, int outMin, int outMax);
    void dropInOutControls();

private Q_SLOTS:
    void inOutChanged();
    void syncIOControls();
    void focusIOControls();

private:
    double io2sp(int x, int min, int max);
    int sp2io(double x, int min, int max);

private:
    /* In/Out controls */
    QSpinBox *m_intIn {nullptr};
    QSpinBox *m_intOut {nullptr};

    /* Working range of them */
    int m_inMin {0};
    int m_inMax {0};
    int m_outMin {0};
    int m_outMax {0};

    KisCurveWidget *m_curveWidget {nullptr};
};

#endif // KISCURVEWIDGETCONTROLSMANAGERINT_H

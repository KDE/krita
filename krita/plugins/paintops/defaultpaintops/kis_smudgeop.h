/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_SMUDGEOP_H_
#define KIS_SMUDGEOP_H_

#include "kis_paintop.h"
#include <qobject.h>
        
class QWidget;
class QCheckBox;
class QLabel;
class QSlider;
class KisPoint;
class KisPainter;
class KCurve;
class WdgBrushCurveControl;

class KisSmudgeOpFactory : public KisPaintOpFactory  {

public:
    KisSmudgeOpFactory() {}
    virtual ~KisSmudgeOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter);
    virtual KisID id() { return KisID("smudge", i18n("Smudge Brush")); }
    virtual QString pixmap() { return "paintbrush.png"; }
    virtual KisPaintOpSettings *settings(QWidget * parent, const KisInputDevice& inputDevice);
};

class KisSmudgeOpSettings : public QObject, public KisPaintOpSettings {
    Q_OBJECT
    typedef KisPaintOpSettings super;
public:
    KisSmudgeOpSettings(QWidget *parent, bool isTablet);

    int rate() const;
    bool varySize() const;
    bool varyOpacity() const;

    bool customSize() const { return m_customSize; }
    bool customOpacity() const { return m_customOpacity; }
    const double* sizeCurve() const { return m_sizeCurve; }
    const double* opacityCurve() const { return m_opacityCurve; }

    virtual QWidget *widget() const { return m_optionsWidget; }
private slots:
    void slotCustomCurves();
private:
    void transferCurve(KCurve* curve, double* target);
    QWidget *m_optionsWidget;
    QLabel* m_rateLabel;
    QSlider* m_rateSlider;
    QLabel * m_pressureVariation;
    QCheckBox * m_size;
    QCheckBox * m_opacity;
    WdgBrushCurveControl* m_curveControl;

    bool m_customSize;
    bool m_customOpacity;
    double m_sizeCurve[256];
    double m_opacityCurve[256];
};

class KisSmudgeOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisSmudgeOp(const KisSmudgeOpSettings *settings, KisPainter * painter);
    virtual ~KisSmudgeOp();

    void paintAt(const KisPoint &pos, const KisPaintInformation& info);

    int rate() { return (m_rate * 255) / 100; }
private:
    KisPaintDeviceSP m_target, m_srcdev;
    inline double scaleToCurve(double pressure, double* curve) const {
        int offset = CLAMP(int(255.0 * pressure), 0, 255);
        return curve[offset];
    }
    bool m_firstRun;
    int m_rate;
    bool m_pressureSize;
    bool m_pressureOpacity;
    bool m_customSize;
    bool m_customOpacity;
    double m_sizeCurve[256];
    double m_opacityCurve[256];
};

#endif // KIS_BRUSHOP_H_

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

#include <QDialog>

#include "kis_brush_based_paintop.h"
#include <qobject.h>
#include <klocale.h>

class QWidget;
class QCheckBox;
class QLabel;
class QSlider;
class KisPoint;
class KisPainter;
class KCurve;

class KisSmudgeOpSettings;

namespace Ui
{
class WdgBrushCurveControl;
}


class KisSmudgeOpFactory : public KisPaintOpFactory
{

public:
    KisSmudgeOpFactory() {}
    virtual ~KisSmudgeOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image);
    virtual QString id() const {
        return "smudge";
    }
    virtual QString name() const {
        return i18n("Smudge Brush");
    }
    virtual QString pixmap() {
        return "paintbrush.png";
    }
    virtual KisPaintOpSettingsSP settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image);
    virtual KisPaintOpSettingsSP settings(KisImageSP image);
};

class KisSmudgeOp : public KisBrushBasedPaintOp
{

public:

    KisSmudgeOp(const KisSmudgeOpSettings *settings, KisPainter * painter);
    virtual ~KisSmudgeOp();

    void paintAt(const KisPaintInformation& info);
    double paintLine(const KisPaintInformation &pi1,
                     const KisPaintInformation &pi2,
                     double savedDist = -1);

    int rate() {
        return (m_rate * 255) / 100;
    }
private:
    KisPaintDeviceSP m_target, m_srcdev;
    inline double scaleToCurve(double pressure, double* curve) const {
        int offset = int(255.0 * pressure);
        if (offset < 0)
            offset = 0;
        if (offset > 255)
            offset =  255; // Was: clamp(..., 0, 255);
        return curve[offset];
    }
    bool m_firstRun;
    int m_rate;
    bool m_pressureRate;
    bool m_pressureSize;
    bool m_pressureOpacity;
    bool m_customRate;
    bool m_customSize;
    bool m_customOpacity;
    double m_rateCurve[256];
    double m_sizeCurve[256];
    double m_opacityCurve[256];
};

#endif // KIS_BRUSHOP_H_

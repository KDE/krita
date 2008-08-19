/*
 *  Copyright (c) 2008 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KIS_COMPLEXOP_H_
#define KIS_COMPLEXOP_H_

#include "kis_brush_based_paintop.h"
#include <klocale.h>
#include <QDialog>
#include <KoColorSpace.h>

#include <kis_paintop_settings.h>

class QWidget;
class QCheckBox;
class QLabel;
class QPointF;
class KisPainter;
class KCurve;
namespace Ui { class WdgBrushCurveControl; }

class KisComplexOpFactory : public KisPaintOpFactory  {

public:
    KisComplexOpFactory() {}
    virtual ~KisComplexOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image);
    virtual QString id() const { return "paintcomplex"; }
    virtual QString name() const { return i18n("Complex Brush"); }
    virtual QString pixmap() { return ""; }
    virtual KisPaintOpSettingsSP settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image);
    virtual KisPaintOpSettingsSP settings(KisImageSP image);
};

class KisComplexOpSettings : public QObject, public KisPaintOpSettings {
    Q_OBJECT

public:
    KisComplexOpSettings(QWidget *parent);
    virtual KisPaintOpSettingsSP clone() const;

    bool varySize() const;
    bool varyOpacity() const;
    bool varyDarken() const;

    bool customSize() const { return m_customSize; }
    bool customOpacity() const { return m_customOpacity; }
    bool customDarken() const { return m_customDarken; }
    const double* sizeCurve() const { return m_sizeCurve; }
    const double* opacityCurve() const { return m_opacityCurve; }
    const double* darkenCurve() const { return m_darkenCurve; }

    virtual QWidget *widget() const { return m_optionsWidget; }

    virtual void fromXML(const QDomElement&);
    virtual void toXML(QDomDocument&, QDomElement&) const;
public:

private slots:
    void slotCustomCurves();

private:
    void transferCurve(KCurve* curve, double* target);
    QWidget *m_optionsWidget;
    QLabel * m_pressureVariation;
    QCheckBox * m_size;
    QCheckBox * m_opacity;
    QCheckBox * m_darken;
    Ui::WdgBrushCurveControl* m_curveControl;
    QDialog* m_curveControlWidget;

    bool m_customSize;
    bool m_customOpacity;
    bool m_customDarken;
    double m_sizeCurve[256];
    double m_opacityCurve[256];
    double m_darkenCurve[256];
};

class KisComplexOp : public KisBrushBasedPaintOp {

public:

    KisComplexOp(const KisComplexOpSettings *settings, KisPainter * painter);
    virtual ~KisComplexOp();

    void paintAt(const KisPaintInformation& info);
    virtual double paintLine(const KisPaintInformation &pi1,
                             const KisPaintInformation &pi2,
                             double savedDist = -1);

private:
    inline double scaleToCurve(double pressure, double* curve) const {
        int offset = int(255.0 * pressure);
        if (offset < 0)
            offset = 0;
        if (offset > 255)
            offset =  255; // Was: clamp(..., 0, 255);
        return curve[offset];
    }
    bool m_pressureSize;
    bool m_pressureOpacity;
    bool m_pressureDarken;
    bool m_customSize;
    bool m_customOpacity;
    bool m_customDarken;
    double m_sizeCurve[256];
    double m_opacityCurve[256];
    double m_darkenCurve[256];
};

#endif // KIS_COMPLEXOP_H_

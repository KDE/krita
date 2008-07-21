/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef KIS_SUMIPAINTOP_H_
#define KIS_SUMIPAINTOP_H_

#include <QColor>
#include <QMutex>

#include <klocale.h>
#include <kis_paintop.h>
#include <kis_types.h>

#include "lines.h"
#include "brush.h"
#include "ui_wdgsumieoptions.h"
#include "widgets/kis_popup_button.h"

class QPointF;
class KisPainter;

class KisSumiPaintOpFactory : public KisPaintOpFactory {

public:
    KisSumiPaintOpFactory() {}
    virtual ~KisSumiPaintOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image);
    virtual QString id() const { return "sumibrush"; }
    virtual QString name() const { return i18n("Sumi-e brush"); }
    virtual QString pixmap() { return "krita-sumi.png"; }
    virtual KisPaintOpSettingsSP settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image);
    virtual KisPaintOpSettingsSP settings(KisImageSP image);

};

class KisSumiPaintOpSettings : public QObject, public KisPaintOpSettings {
    Q_OBJECT

public:
    KisSumiPaintOpSettings(QWidget * paren );
    virtual ~KisSumiPaintOpSettings() {}

    virtual KisPaintOpSettingsSP clone() const;

    QWidget * widget() const { return m_optionsWidget; }

    using KisPropertiesConfiguration::fromXML;
    using KisPropertiesConfiguration::toXML;

    virtual void fromXML(const QDomElement&);
    virtual void toXML(QDomDocument&, QDomElement&) const;

	QList<float> * curve() const;
	int radius() const;
	double sigma() const;
	int brushDimension() const;
	int inkAmount() const;

private:
    Ui::WdgSumieOptions* m_options;
    KisPopupButton * m_optionsWidget;
    QWidget * m_popupWidget;

	int m_curveSamples;
	QList<QPointF> m_curveData;
};


class KisSumiPaintOp : public KisPaintOp {

public:

    KisSumiPaintOp(const KisSumiPaintOpSettings *settings, KisPainter * painter, KisImageSP image);
    virtual ~KisSumiPaintOp();

    // Do we want to spray even when no movement?
    virtual bool incremental() { return false; }

    void paintAt(const KisPaintInformation& info);
    //double paintLine(const KisPaintInformation &pi1,const KisPaintInformation &pi2,double savedDist);


private:
    QColor c;
    QPointF m_previousPoint;
    KisImageSP m_image;
    bool newStrokeFlag;
    //Stroke stroke;
    KisPaintDeviceSP dab;
    QMutex m_mutex;
    Brush m_brush;
};

#endif // KIS_SUMIPAINTOP_H_

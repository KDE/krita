/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_CPAINTOP_H_
#define KIS_CPAINTOP_H_

#include <QImage>

#include <QVector>

#include <KoColor.h>
#include <kis_paintop.h>
#include <kis_paintop_settings.h>
#include "ui_wdgcpaintoptions.h"

class QPointF;
class KisPainter;
class Brush;
class Stroke;
class KisConfigWidget;

class KisCPaintOpFactory : public KisPaintOpFactory
{

public:
    KisCPaintOpFactory();
    virtual ~KisCPaintOpFactory();

    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image);
    virtual QString id() const {
        return "paintCPaint";
    }
    virtual QString name() const {
        return i18n("Chinese Brush");
    }
    virtual KisPaintOpSettingsSP settings(const KoInputDevice& inputDevice, KisImageWSP image);
    virtual KisPaintOpSettingsSP settings(KisImageWSP image);
    virtual KisPaintOpSettingsWidget* settingsWidget(QWidget* parent);

private:

    QVector<Brush*> m_brushes;
};



class KisCPaintOpSettings : public QObject, public KisPaintOpSettings
{

    Q_OBJECT

public:
    KisCPaintOpSettings(QWidget * parent,  QVector<Brush*> m_brushes);
    virtual ~KisCPaintOpSettings() {}

    virtual KisPaintOpSettingsSP clone() const;

    int brush() const;
    int ink() const;
    int water() const;

    KisConfigWidget * widget() const {
        return m_options;
    }

    using KisPaintOpSettings::fromXML;
    virtual void fromXML(const QDomElement&);

    using KisPaintOpSettings::toXML;
    virtual void toXML(QDomDocument&, QDomElement&) const;


private slots:

    void resetCurrentBrush();

private:

    QVector<Brush*> m_brushes;
    Ui::WdgCPaintOptions * m_options;
    KisConfigWidget * m_options;
};



class KisCPaintOp : public KisPaintOp
{

public:

    KisCPaintOp(Brush * brush, const KisCPaintOpSettings * settings, KisPainter * painter);
    virtual ~KisCPaintOp();

    void paintAt(const KisPaintInformation& info);

    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const {
        Q_UNUSED(xSpacing);
        Q_UNUSED(ySpacing);
        Q_UNUSED(pressure1);
        Q_UNUSED(pressure2);
        // XXX: this is wrong!
        return 0.5;
    }


private:

    Brush * m_currentBrush;
    KoColor m_color;
    int m_ink;
    int m_water;

    QPointF m_lastPoint;

    int sampleCount;
    bool newStrokeFlag;
    Stroke * m_stroke;

};

#endif // KIS_CPAINTOP_H_

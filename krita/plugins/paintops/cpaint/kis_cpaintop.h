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

#include <qvaluevector.h>

#include <kis_color.h>
#include <kis_paintop.h>

#include "wdgcpaintoptions.h"

class KisPoint;
class KisPainter;
class Brush;
class Stroke;

class KisCPaintOpFactory : public KisPaintOpFactory  {

public:
    KisCPaintOpFactory();
    virtual ~KisCPaintOpFactory();

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter);
    virtual KisID id() { return KisID("paintCPaint", i18n("Chinese Brush")); }
    virtual KisPaintOpSettings *settings(QWidget * parent, const KisInputDevice& inputDevice);

private:

    QValueVector<Brush*> m_brushes;
};



class KisCPaintOpSettings : public KisPaintOpSettings {

public:
    KisCPaintOpSettings(QWidget * parent,  QValueVector<Brush*> m_brushes);
    virtual ~KisCPaintOpSettings() {};

    int brush() const;
    int ink() const;
    int water() const;

    QWidget * widget() const { return m_options; };

private:

    void resetCurrentBrush();

private:

    QValueVector<Brush*> m_brushes;
    WdgCPaintOptions * m_options;
};



class KisCPaintOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisCPaintOp(Brush * brush, const KisCPaintOpSettings * settings, KisPainter * painter);
    virtual ~KisCPaintOp();

    void paintAt(const KisPoint &pos, const KisPaintInformation& info);

private:

    Brush * m_currentBrush;
    KisColor m_color;
    int m_ink;
    int m_water;

    int pressure;
    short int tiltx, tilty;

    int lastx, lasty;

    int sampleCount;
    bool newStrokeFlag;

    Stroke * curStroke;
};

#endif // KIS_CPAINTOP_H_

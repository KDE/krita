/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_BRUSHOP_H_
#define KIS_BRUSHOP_H_

#include "kis_paintop.h"

class QWidget;
class QCheckBox;
class QLabel;
class KisPoint;
class KisPainter;

class KisBrushOpFactory : public KisPaintOpFactory  {

public:
    KisBrushOpFactory() {}
    virtual ~KisBrushOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter);
    virtual KoID id() { return KoID("paintbrush", i18n("Pixel Brush")); }
    virtual QString pixmap() { return "paintbrush.png"; }
    virtual KisPaintOpSettings *settings(QWidget * parent, const KisInputDevice& inputDevice);
};

class KisBrushOpSettings : public KisPaintOpSettings {
    typedef KisPaintOpSettings super;
public:
    KisBrushOpSettings(QWidget *parent);

    bool varySize() const;
    bool varyOpacity() const;
    bool varyDarken() const;

    virtual QWidget *widget() const { return m_optionsWidget; }

private:
    QWidget *m_optionsWidget;
    QLabel * m_pressureVariation;
    QCheckBox * m_size;
    QCheckBox * m_opacity;
    QCheckBox * m_darken;
};

class KisBrushOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisBrushOp(const KisBrushOpSettings *settings, KisPainter * painter);
    virtual ~KisBrushOp();

    void paintAt(const KisPoint &pos, const KisPaintInformation& info);

private:

    bool m_pressureSize;
    bool m_pressureOpacity;
    bool m_pressureDarken;
};

#endif // KIS_BRUSHOP_H_

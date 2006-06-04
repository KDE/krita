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

#ifndef KIS_WETOP_H_
#define KIS_WETOP_H_

#include "kis_paintop.h"
#include "kis_types.h"
#include "KoColorSpace.h"
#include "ui_wdgpressure.h"

class KisPoint;
class KisPainter;
class KisInputDevice;

class WetPaintOptions : public QWidget, public Ui::WetPaintOptions
{
    Q_OBJECT

    public:
        WetPaintOptions(QWidget *parent, const char *name) : QWidget(parent) { setObjectName(name); setupUi(this); }
};

class KisWetOpFactory : public KisPaintOpFactory  {
public:
    KisWetOpFactory() {}
    virtual ~KisWetOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter);
    virtual KoID id() { return KoID("wetbrush", i18n("Watercolor Brush")); }
    virtual bool userVisible(KoColorSpace* cs) { return cs->id() == KoID("WET", ""); }
    virtual KisPaintOpSettings *settings(QWidget * parent, const KisInputDevice& inputDevice);
};

class KisWetOpSettings : public KisPaintOpSettings {
    typedef KisPaintOpSettings super;
public:
    KisWetOpSettings(QWidget *parent);

    bool varySize() const;
    bool varyWetness() const;
    bool varyStrength() const;

    virtual QWidget *widget() const { return m_options; }

private:
    WetPaintOptions *m_options;
};

class KisWetOp : public KisPaintOp {

    typedef KisPaintOp super;
    bool m_size;
    bool m_wetness;
    bool m_strength;

public:

    KisWetOp(const KisWetOpSettings *settings, KisPainter * painter);
    virtual ~KisWetOp();

    void paintAt(const KisPoint &pos, const KisPaintInformation& info);

};

#endif // KIS_WETOP_H_

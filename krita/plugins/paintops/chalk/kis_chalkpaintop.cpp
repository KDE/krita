/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_chalkpaintop.h"
#include <cmath>

#include <QRect>
#include <QColor>
#include <QMutexLocker>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_random_accessor.h>

#include <kis_paintop_settings.h>
#include <kis_paintop_options_widget.h>


class KisChalkOpSettings : public KisPaintOpSettings
{
public:

    KisChalkOpSettings(QWidget * widget)
            : KisPaintOpSettings() {
        m_optionsWidget = new KisPaintOpOptionsWidget();
    }

    KisPaintOpSettingsSP clone() const {
        KisPaintOpSettings * c = new KisChalkOpSettings(0);
        c->fromXML(toXML());
        return c;
    }

    QWidget * widget() const {
        return m_optionsWidget;
    }

private:
    KisPaintOpOptionsWidget *m_optionsWidget;
};

KisPaintOp * KisChalkPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image)
{
    Q_UNUSED(settings);
    KisPaintOp * op = new KisChalkPaintOp(painter, image);
    Q_CHECK_PTR(op);
    return op;
}


KisPaintOpSettingsSP KisChalkPaintOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP /*image*/)
{
    return new KisChalkOpSettings(parent);
}

KisPaintOpSettingsSP KisChalkPaintOpFactory::settings(KisImageSP image)
{
    Q_UNUSED(image);
    return new KisChalkOpSettings(0);
}



KisChalkPaintOp::KisChalkPaintOp(KisPainter * painter, KisImageSP image)
        : KisPaintOp(painter)
{
    newStrokeFlag = true;
    m_image = image;
}

KisChalkPaintOp::~KisChalkPaintOp()
{
}

void KisChalkPaintOp::paintAt(const KisPaintInformation& info)
{
    QMutexLocker locker(&m_mutex);
    if (!painter()) return;
    // read, write pixel data
    KisPaintDeviceSP device = painter()->device();
    if (!device) return;

    if (newStrokeFlag) {
        newStrokeFlag = false;
    } else {
        dab = cachedDab();
        dab->clear();

        float x1, y1;

        x1 = info.pos().x();
        y1 = info.pos().y();

        m_mybrush.paint(dab, x1, y1, painter()->paintColor());

        QRect rc = dab->extent();
        painter()->bitBlt(rc.topLeft(), dab, rc);
    }
}

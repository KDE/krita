/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_paintop_settings.h"

#include <QWidget>
#include <QImage>
#include <QColor>

#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_node.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_paintop_registry.h"
#include "kis_paint_information.h"
#include "kis_paintop_settings_widget.h"

struct KisPaintOpSettings::Private {
    KisNodeSP node;
    KisPaintOpSettingsWidget *settingsWidget;
};

KisPaintOpSettings::KisPaintOpSettings(KisPaintOpSettingsWidget *settingsWidget)
    : d(new Private)
{
    d->settingsWidget = settingsWidget;
}

KisPaintOpSettings::~KisPaintOpSettings()
{
    delete d;
}

KisPaintOpSettingsWidget* KisPaintOpSettings::widget() const
{
    return d->settingsWidget;
}

void KisPaintOpSettings::mousePressEvent(KoPointerEvent *e)
{
    e->ignore();
}

void KisPaintOpSettings::activate()
{
}

void KisPaintOpSettings::setNode(KisNodeSP node)
{
    d->node = node;
}

KisNodeSP KisPaintOpSettings::node() const
{
    return d->node;
}

QImage KisPaintOpSettings::sampleStroke(const QSize& size )
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    int width = size.width();
    int height = size.height();

    KisImageSP img = new KisImage(0, width, height, cs, "temporary for stroke sample");
    KisLayerSP layer = new KisPaintLayer(img, "temporary for stroke sample", OPACITY_OPAQUE, cs);
    
    
    KisPainter painter(layer->paintDevice());
    painter.setPaintColor(KoColor() );

    KisPaintOpPresetSP preset = new KisPaintOpPreset();
    preset->setSettings( this ); // This clones
    preset->settings()->setNode( layer );
    
painter.setPaintOpPreset( preset, img);

    QPointF p1(1.0 / 6.0*width, 2.0 / 3.0*height);
    QPointF p2(2.0 / 6.0*width, 1.0 / 3.0*height);
    QPointF p3(4.0 / 6.0*width, 2.0 / 3.0*height);
    QPointF p4(5.0 / 6.0*width, 1.0 / 3.0*height);

    float pathLength;

    //p2-p1
    float dx = p2.x() - p1.x();
    float dy = p2.y() - p1.y();
    pathLength += sqrt(dx * dx + dy * dy);

    dx = p3.x() - p2.x();
    dy = p3.y() - p2.y();
    pathLength += sqrt(dx * dx + dy * dy);

    dx = p4.x() - p3.x();
    dy = p4.y() - p3.y();
    pathLength += sqrt(dx * dx + dy * dy);

    KisPaintInformation pi1(p1, 0.0);
    KisPaintInformation pi2(p2, 0.95);
    KisPaintInformation pi3(p3, 0.75);
    KisPaintInformation pi4(p4, 0.0);

    QPointF c1(p1.x(), p1.y() - 5);
    QPointF c2(p1.x(), p1.y() + 5);
    painter.paintBezierCurve(pi1, c1, c2, pi2, 0);

    c1.setX(p2.x());
    c1.setY(p2.y() - 5);
    c2.setX(p2.x());
    c2.setY(p2.y() + 5);
    painter.paintBezierCurve(pi2, c1, c2, pi3, 0);

    c1.setX(p3.x());
    c1.setY(p3.y() - 5);
    c2.setX(p3.x());
    c2.setY(p3.y() + 5);
    painter.paintBezierCurve(pi3, c1, c2, pi4, 0);

    return layer->paintDevice()->convertToQImage(0);
}

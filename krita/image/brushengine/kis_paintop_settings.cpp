/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <QImage>
#include <QColor>
#include <QPointer>

#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoViewConverter.h>

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
    QPointer<KisPaintOpSettingsWidget> settingsWidget;
    QString modelName;
};

KisPaintOpSettings::KisPaintOpSettings()
        : d(new Private)
{
}

KisPaintOpSettings::~KisPaintOpSettings()
{
    delete d;
}

void KisPaintOpSettings::setOptionsWidget(KisPaintOpSettingsWidget* widget)
{
    d->settingsWidget = widget;
}

bool KisPaintOpSettings::mousePressEvent(const KisPaintInformation &pos, Qt::KeyboardModifiers modifiers){
    Q_UNUSED(pos);
    Q_UNUSED(modifiers);
    return true; // ignore the event by default
}


KisPaintOpSettingsSP KisPaintOpSettings::clone() const
{
    QString paintopID = getString("paintop");
    if(paintopID.isEmpty())
        return 0;

    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->settings(KoID(paintopID, ""));
    QMapIterator<QString, QVariant> i(getProperties());
    while (i.hasNext()) {
        i.next();
        settings->setProperty(i.key(), QVariant(i.value()));
    }
    return settings;
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

void KisPaintOpSettings::changePaintOpSize(qreal x, qreal y)
{
    if(!d->settingsWidget.isNull()) {
        d->settingsWidget.data()->changePaintOpSize(x, y);
        d->settingsWidget.data()->writeConfiguration(this);
    }
}


QSizeF KisPaintOpSettings::paintOpSize() const
{
    if(!d->settingsWidget.isNull()) {
        return d->settingsWidget.data()->paintOpSize();
    }
    return QSizeF(1.0,1.0);
}


QString KisPaintOpSettings::modelName() const
{
    return d->modelName;
}

void KisPaintOpSettings::setModelName(const QString & modelName)
{
    d->modelName = modelName;
}

KisPaintOpSettingsWidget* KisPaintOpSettings::optionsWidget() const
{
    if(d->settingsWidget.isNull())
        return 0;

    return d->settingsWidget.data();
}

bool KisPaintOpSettings::isValid()
{
    return true;
}

bool KisPaintOpSettings::isLoadable()
{
    return isValid();
}

QPainterPath KisPaintOpSettings::brushOutline(const QPointF& pos, OutlineMode mode, qreal scale, qreal rotation) const
{
    QPainterPath path;
    if (mode == CursorIsOutline){
        QRectF rc(-5,-5, 10, 10);
        path.moveTo(rc.topLeft());
        path.lineTo(rc.bottomRight());
        path.moveTo(rc.topRight());
        path.lineTo(rc.bottomLeft());
        QTransform m;
        m.reset(); m.scale(scale,scale); m.rotateRadians(rotation);
        path = m.map(path);
        path.translate(pos);
    }
    return path;
}

QPainterPath KisPaintOpSettings::ellipseOutline(qreal width, qreal height, qreal scale, qreal rotation) const
{
    QPainterPath path;
    QRectF ellipse(0,0,width * scale,height * scale);
    ellipse.translate(-ellipse.center());
    path.addEllipse(ellipse);

    QTransform m;
    m.reset();
    m.rotate( rotation );
    path = m.map(path);
    return path;
}

void KisPaintOpSettings::setCanvasRotation(qreal angle)
{
    setProperty("runtimeCanvasRotation", angle);
    setPropertyNotSaved("runtimeCanvasRotation");
}

qreal KisPaintOpSettings::canvasRotation() const
{
    return getDouble("runtimeCanvasRotation");
}

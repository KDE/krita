/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_duplicateop_settings.h"
#include "kis_duplicateop_option.h"
#include "kis_duplicateop_settings_widget.h"

#include <QDomElement>
#include <QDomDocument>

#include <KoPointerEvent.h>
#include <KoCompositeOpRegistry.h>

#include <kis_image.h>
#include <kis_brush_option_widget.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_perspective_grid.h>

KisDuplicateOpSettings::KisDuplicateOpSettings(KisImageWSP image)
        : m_image(image)
        , m_isOffsetNotUptodate(false)
{
}

KisDuplicateOpSettings::~KisDuplicateOpSettings()
{
}

bool KisDuplicateOpSettings::paintIncremental()
{
    return false;
}

QString KisDuplicateOpSettings::indirectPaintingCompositeOp() const
{
    return COMPOSITE_COPY;
}

QPointF KisDuplicateOpSettings::offset() const
{
    return m_offset;
}

QPointF KisDuplicateOpSettings::position() const
{
    return m_position;
}

bool KisDuplicateOpSettings::mousePressEvent(const KisPaintInformation &info, Qt::KeyboardModifiers modifiers)
{
    bool ignoreEvent = true;
    if (modifiers == Qt::ControlModifier) {
        m_position = info.pos();
        m_isOffsetNotUptodate = true;
        ignoreEvent = false;
    } else {
        if (m_isOffsetNotUptodate) {
            m_offset = info.pos() - m_position;
            m_isOffsetNotUptodate = false;
        }
        ignoreEvent = true;
    }

    return ignoreEvent;
}


void KisDuplicateOpSettings::activate()
{
    KisDuplicateOpSettingsWidget* options = dynamic_cast<KisDuplicateOpSettingsWidget*>(optionsWidget());
    if(!options)
        return;

    if (m_image && m_image->perspectiveGrid()->countSubGrids() != 1) {
        options->m_duplicateOption->setHealing(false);
        options->m_duplicateOption->setPerspective(false);
    } else {
        options->m_duplicateOption->setPerspective(false);
    }
}

void KisDuplicateOpSettings::fromXML(const QDomElement& elt)
{
    // First, call the parent class fromXML to make sure all the
    // properties are saved to the map
    KisPaintOpSettings::fromXML(elt);

    m_offset.setX(elt.attribute("OffsetX", "0.0").toDouble());
    m_offset.setY(elt.attribute("OffsetY", "0.0").toDouble());
    m_isOffsetNotUptodate = false;
}

void KisDuplicateOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    // Then call the parent class fromXML
    KisPropertiesConfiguration::toXML(doc, rootElt);

    rootElt.setAttribute("OffsetX", QString::number(m_offset.x()));
    rootElt.setAttribute("OffsetY", QString::number(m_offset.y()));
}


KisPaintOpSettingsSP KisDuplicateOpSettings::clone() const
{
    KisPaintOpSettingsSP setting = KisPaintOpSettings::clone();
    KisDuplicateOpSettings* s = dynamic_cast<KisDuplicateOpSettings*>(setting.data());
    s->m_image = m_image;
    s->m_offset = m_offset;
    s->m_isOffsetNotUptodate = m_isOffsetNotUptodate;
    s->m_position = m_position;

    return setting;

}

QPainterPath KisDuplicateOpSettings::brushOutline(const QPointF& pos, KisPaintOpSettings::OutlineMode mode, qreal scale, qreal rotation) const
{
    QPainterPath path;
    path = KisBrushBasedPaintOpSettings::brushOutline(QPointF(), mode, scale, rotation);

    QPainterPath copy(path);
    QRectF rect2 = copy.boundingRect();
    if (m_isOffsetNotUptodate || !getBool(DUPLICATE_MOVE_SOURCE_POINT)) {
        copy.translate(m_position - pos);
    } else {
        copy.translate(-m_offset);
    }

    path.addPath(copy);

    QTransform m;
    m.scale(0.5,0.5);
    rect2 = m.mapRect(rect2);

    path.moveTo(rect2.topLeft());
    path.lineTo(rect2.bottomRight());

    path.moveTo(rect2.topRight());
    path.lineTo(rect2.bottomLeft());

    return path.translated(pos);
}

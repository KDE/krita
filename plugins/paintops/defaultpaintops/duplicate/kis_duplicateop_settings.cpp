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
#include <kis_paintop_settings_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_dom_utils.h>

KisDuplicateOpSettings::KisDuplicateOpSettings()
    : m_isOffsetNotUptodate(false)
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

bool KisDuplicateOpSettings::mousePressEvent(const KisPaintInformation &info, Qt::KeyboardModifiers modifiers, KisNodeWSP currentNode)
{
    bool ignoreEvent = true;

    if (modifiers & Qt::ControlModifier) {
        if (!m_sourceNode || !(modifiers & Qt::AltModifier)) {
            m_sourceNode = currentNode;
        }
        m_position = info.pos();
        m_isOffsetNotUptodate = true;
        ignoreEvent = false;
    }
    else {
        if (m_isOffsetNotUptodate) {
            m_offset = info.pos() - m_position;
            m_isOffsetNotUptodate = false;
        }
        ignoreEvent = true;
    }

    return ignoreEvent;
}

KisNodeWSP KisDuplicateOpSettings::sourceNode() const
{
    return m_sourceNode;
}

void KisDuplicateOpSettings::activate()
{
}

void KisDuplicateOpSettings::fromXML(const QDomElement& elt)
{
    // First, call the parent class fromXML to make sure all the
    // properties are saved to the map
    KisPaintOpSettings::fromXML(elt);

    m_offset.setX(KisDomUtils::toDouble(elt.attribute("OffsetX", "0.0")));
    m_offset.setY(KisDomUtils::toDouble(elt.attribute("OffsetY", "0.0")));
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
    KisPaintOpSettingsSP setting = KisBrushBasedPaintOpSettings::clone();
    KisDuplicateOpSettings* s = dynamic_cast<KisDuplicateOpSettings*>(setting.data());
    s->m_offset = m_offset;
    s->m_isOffsetNotUptodate = m_isOffsetNotUptodate;
    s->m_position = m_position;

    return setting;
}

QPainterPath KisDuplicateOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode)
{
    QPainterPath path;

    OutlineMode forcedMode = mode;

    if (!forcedMode.isVisible) {
        forcedMode.isVisible = true;
        forcedMode.forceCircle = true;
    }

    // clone tool should always show an outline
    path = KisBrushBasedPaintOpSettings::brushOutlineImpl(info, forcedMode, 1.0);

    QPainterPath copy(path);
    QRectF rect2 = copy.boundingRect();
    if (m_isOffsetNotUptodate || !getBool(DUPLICATE_MOVE_SOURCE_POINT)) {
        copy.translate(m_position - info.pos());
    }
    else {
        copy.translate(-m_offset);
    }

    path.addPath(copy);

    qreal dx = rect2.width() / 4.0;
    qreal dy = rect2.height() / 4.0;
    rect2.adjust(dx, dy, -dx, -dy);

    path.moveTo(rect2.topLeft());
    path.lineTo(rect2.bottomRight());

    path.moveTo(rect2.topRight());
    path.lineTo(rect2.bottomLeft());

    return path;
}


#include <brushengine/kis_uniform_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisDuplicateOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_uniformProperties);

    if (props.isEmpty()) {
        {
            KisUniformPaintOpPropertyCallback *prop =
                new KisUniformPaintOpPropertyCallback(
                    KisUniformPaintOpPropertyCallback::Bool,
                    "clone_healing",
                    i18n("Healing"),
                    settings, 0);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisDuplicateOptionProperties option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(option.duplicate_healing);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisDuplicateOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.duplicate_healing = prop->value().toBool();
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(preset()->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisUniformPaintOpPropertyCallback *prop =
                new KisUniformPaintOpPropertyCallback(
                    KisUniformPaintOpPropertyCallback::Bool,
                    "clone_movesource",
                    i18n("Move Source"),
                    settings, 0);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisDuplicateOptionProperties option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(option.duplicate_move_source_point);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisDuplicateOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.duplicate_move_source_point = prop->value().toBool();
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(preset()->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisPaintOpSettings::uniformProperties(settings) + props;
}



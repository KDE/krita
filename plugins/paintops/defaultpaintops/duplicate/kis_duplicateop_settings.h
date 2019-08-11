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

#ifndef KIS_DUPLICATEOP_SETTINGS_H_
#define KIS_DUPLICATEOP_SETTINGS_H_

#include <kis_brush_based_paintop_settings.h>
#include <kis_types.h>
#include <QPointF>

class QDomElement;
class KisDuplicateOpSettings : public KisBrushBasedPaintOpSettings
{

public:
    using KisPaintOpSettings::fromXML;
    using KisPaintOpSettings::toXML;

    KisDuplicateOpSettings();
    ~KisDuplicateOpSettings() override;
    bool paintIncremental() override;
    QString indirectPaintingCompositeOp() const override;

    QPointF offset() const;
    QPointF position() const;
    /**
     * This function is called by a tool when the mouse is pressed.
     * Returns false if picking new origin is in action,
     * and returns true otherwise (i.e. if brush is starting a new stroke).
     * See kis_tool_freehand:tryPickByPaintOp()
     */
    bool mousePressEvent(const KisPaintInformation& pos, Qt::KeyboardModifiers modifiers, KisNodeWSP currentNode) override;
    /**
     * This function is called by a tool when the mouse is released.
     * If the tool is supposed to ignore the event, the paint op should return true
     * and if the tool is supposed to use the event, return false.
     */
    bool mouseReleaseEvent() override;
    void activate() override;

    void fromXML(const QDomElement& elt) override;
    void toXML(QDomDocument& doc, QDomElement& rootElt) const override;

    KisPaintOpSettingsSP clone() const override;
    using KisBrushBasedPaintOpSettings::brushOutline;
    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode) override;

    KisNodeWSP sourceNode() const;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings) override;

public:

    Q_DISABLE_COPY(KisDuplicateOpSettings)

    QPointF m_offset;
    bool m_isOffsetNotUptodate; // true between the act of setting a new origin and the first stroke
    bool m_duringPaintingStroke; // true if the stroke is begin painted now, false otherwise
    QPointF m_position; // Give the position of the last alt-click
    KisNodeWSP m_sourceNode; // Give the node of the source point (origin)
    QList<KisUniformPaintOpPropertyWSP> m_uniformProperties;
};

typedef KisSharedPtr<KisDuplicateOpSettings> KisDuplicateOpSettingsSP;


#endif // KIS_DUPLICATEOP_SETTINGS_H_

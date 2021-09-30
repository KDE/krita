/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    KisDuplicateOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisDuplicateOpSettings() override;
    bool paintIncremental() override;
    QString indirectPaintingCompositeOp() const override;

    QPointF offset() const;
    QPointF position() const;
    /**
     * This function is called by a tool when the mouse is pressed.
     * Returns false if picking new origin is in action,
     * and returns true otherwise (i.e. if brush is starting a new stroke).
     * See kis_tool_freehand:trySampleByPaintOp()
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
    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;

    KisNodeWSP sourceNode() const;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

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

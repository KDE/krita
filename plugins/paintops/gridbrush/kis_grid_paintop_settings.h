/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRID_PAINTOP_SETTINGS_H_
#define KIS_GRID_PAINTOP_SETTINGS_H_

#include <QScopedPointer>

#include <brushengine/kis_paintop_settings.h>
#include <kis_types.h>

#include <kis_outline_generation_policy.h>
#include "kis_grid_paintop_settings_widget.h"



class KisGridPaintOpSettings : public KisOutlineGenerationPolicy<KisPaintOpSettings>
{
public:
    KisGridPaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisGridPaintOpSettings() override;

    void setPaintOpSize(qreal value) override;
    qreal paintOpSize() const override;

    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;
    bool paintIncremental() override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

    bool mousePressEvent(const KisPaintInformation& pos, Qt::KeyboardModifiers modifiers, KisNodeWSP currentNode) override;
    bool mouseReleaseEvent() override;

private:

    struct Private;
    const QScopedPointer<Private> m_d;

public:
    bool m_modifyOffsetWithShortcut;

};

typedef KisSharedPtr<KisGridPaintOpSettings> KisGridPaintOpSettingsSP;

#endif

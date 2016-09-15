/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_SPRAY_PAINTOP_SETTINGS_H_
#define KIS_SPRAY_PAINTOP_SETTINGS_H_

#include <QScopedPointer>

#include <brushengine/kis_no_size_paintop_settings.h>
#include <kis_types.h>

#include <kis_outline_generation_policy.h>
#include "kis_spray_paintop_settings_widget.h"


class KisSprayPaintOpSettings : public KisOutlineGenerationPolicy<KisPaintOpSettings>
{
public:
    KisSprayPaintOpSettings();
    virtual ~KisSprayPaintOpSettings();

    void setPaintOpSize(qreal value) Q_DECL_OVERRIDE;
    qreal paintOpSize() const Q_DECL_OVERRIDE;

    QPainterPath brushOutline(const KisPaintInformation &info, OutlineMode mode) Q_DECL_OVERRIDE;

    QString modelName() const {
        return "airbrush";
    }

    bool paintIncremental();
    bool isAirbrushing() const;
    int rate() const;

protected:

    QList<KisUniformPaintOpPropertySP> uniformProperties();

private:
    Q_DISABLE_COPY(KisSprayPaintOpSettings)

    struct Private;
    const QScopedPointer<Private> m_d;

};

typedef KisSharedPtr<KisSprayPaintOpSettings> KisSprayPaintOpSettingsSP;

#endif

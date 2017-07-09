/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_WATERCOLOR_PAINTOP_SETTINGS_H_
#define KIS_WATERCOLOR_PAINTOP_SETTINGS_H_

#include <brushengine/kis_paintop_settings.h>
#include <QScopedPointer>

class KisWatercolorPaintOpSettings : public KisPaintOpSettings
{
public:
    KisWatercolorPaintOpSettings();
    virtual ~KisWatercolorPaintOpSettings();

    bool paintIncremental();

    void setPaintOpSize(qreal value) override;
    qreal paintOpSize() const override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif

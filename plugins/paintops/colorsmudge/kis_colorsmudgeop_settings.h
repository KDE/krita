/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_COLORSMUDGEOP_SETTINGS_H
#define __KIS_COLORSMUDGEOP_SETTINGS_H

#include <QScopedPointer>
#include <kis_brush_based_paintop_settings.h>


class KisColorSmudgeOpSettings : public KisBrushBasedPaintOpSettings
{
public:
    KisColorSmudgeOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisColorSmudgeOpSettings() override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef KisSharedPtr<KisColorSmudgeOpSettings> KisColorSmudgeOpSettingsSP;

#endif /* __KIS_COLORSMUDGEOP_SETTINGS_H */

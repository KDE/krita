/*
 * SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_WDG_CONVERT_HEIGHT_TO_NORMAL_MAP_H
#define KIS_WDG_CONVERT_HEIGHT_TO_NORMAL_MAP_H
#include <kis_config_widget.h>
#include <QWidget>
#include <KoColorSpace.h>
#include "ui_wdg_convert_height_to_normal_map.h"

class Ui_WidgetConvertHeightToNormalMap;

class KisWdgConvertHeightToNormalMap : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgConvertHeightToNormalMap(QWidget *parent, const KoColorSpace *cs);
    ~KisWdgConvertHeightToNormalMap();

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;

    enum swizzle {
        xPlus,
        xMin,
        yPlus,
        yMin,
        zPlus,
        zMin
    };

private:
    Ui_WidgetConvertHeightToNormalMap *ui;
    QStringList m_types;
    QStringList m_types_translatable;
    const KoColorSpace *m_cs;
private Q_SLOTS:
    void horizontalRadiusChanged(qreal r);
    void verticalRadiusChanged(qreal r);
    void aspectLockChanged(bool v);
};

#endif // KIS_WDG_CONVERT_HEIGHT_TO_NORMAL_MAP_H

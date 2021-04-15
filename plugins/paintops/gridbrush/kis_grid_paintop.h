/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRID_PAINTOP_H_
#define KIS_GRID_PAINTOP_H_

//#define BENCHMARK

#include <klocalizedstring.h>
#include <brushengine/kis_paintop.h>
#include <kis_types.h>
#include <kis_color_option.h>

#include "kis_grid_paintop_settings.h"


class KisPainter;

class KisGridProperties
{
public:
    quint16 diameter;
    quint16 gridWidth;
    quint16 gridHeight;
    qreal horizontalOffset;
    qreal verticalOffset;
    quint16 divisionLevel;
    bool pressureDivision;
    bool randomBorder;
    qreal scale;
    qreal vertBorder;
    qreal horizBorder;

    quint8 shape;
public:
    void readOptionSetting(const KisPropertiesConfigurationSP setting);
    void writeOptionSetting(KisPropertiesConfigurationSP setting);
};

class KisGridPaintOp : public KisPaintOp
{

public:

    KisGridPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisGridPaintOp() override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    KisSpacingInformation computeSpacing(qreal lodScale) const;

private:
    KisGridPaintOpSettingsSP m_settings;
    KisPaintDeviceSP    m_dab;
    KisPainter*         m_painter;
    qreal              m_xSpacing;
    qreal              m_ySpacing;
    qreal              m_spacing;
    KisGridProperties   m_properties;
    KisColorProperties  m_colorProperties;
    KisNodeSP m_node;


#ifdef BENCHMARK
    int m_total;
    int m_count;
#endif

};

#endif // KIS_GRID_PAINTOP_H_

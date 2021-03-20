/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRID_SHAPE_OPTION_H
#define KIS_GRID_SHAPE_OPTION_H

#include <kis_paintop_option.h>

const QString GRIDSHAPE_SHAPE = "GridShape/shape";

class KisShapeOptionsWidget;

class KisGridShapeOption : public KisPaintOpOption
{
public:
    KisGridShapeOption();
    ~KisGridShapeOption() override;

    /// Ellipse, rectangle, line, pixel, anti-aliased pixel
    int shape() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
private:
    KisShapeOptionsWidget * m_options;
};

#endif // KIS_GRID_SHAPE_OPTION_H


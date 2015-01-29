/* This file is part of the KDE project
 * Copyright (C) Timothée Giet <animtim@gmail.com>, (C) 2014
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

#ifndef KIS_PRESSURE_FLOW_OPTION_H
#define KIS_PRESSURE_FLOW_OPTION_H

#include "kis_curve_option.h"
#include <kis_paint_information.h>
#include <krita_export.h>
#include <kis_painter.h>
/**
 * The pressure flow option defines a curve that is used to
 * calculate the effect of pressure on the flow of the dab
 */
class PAINTOP_EXPORT KisPressureFlowOption : public KisCurveOption
{
public:
    KisPressureFlowOption();

    virtual void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    virtual void readOptionSetting(const KisPropertiesConfiguration* setting);

    void apply(KisPainter* painter, const KisPaintInformation &) const;

    void setFlow(qreal flow);
    qreal getFlow() const;

protected:
    qreal m_flow;

};

#endif
 

/* 
 * Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef KIS_PRESSURE_FLOW_OPACITY_OPTION_H
#define KIS_PRESSURE_FLOW_OPACITY_OPTION_H

#include "kis_curve_option.h"
#include <krita_export.h>
#include <kis_types.h>
#include <kis_paintop_settings.h>

class KisPaintInformation;
class KisPainter;

class PAINTOP_EXPORT KisFlowOpacityOption: public KisCurveOption
{
public:
    KisFlowOpacityOption();
    virtual ~KisFlowOpacityOption() { }

    virtual void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    virtual void readOptionSetting(const KisPropertiesConfiguration* setting);
    
    void setFlow(qreal flow);
    void setOpacity(qreal opacity);
    void apply(KisPainter* painter, const KisPaintInformation& info);
    
    qreal getFlow() const;
    qreal getStaticOpacity() const;
    qreal getDynamicOpacity(const KisPaintInformation& info) const;
    
protected:
    qreal m_flow;
    int   m_paintActionType;
};

#endif //KIS_PRESSURE_FLOW_OPACITY_OPTION_H

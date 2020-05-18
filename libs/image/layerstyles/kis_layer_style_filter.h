/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LAYER_STYLE_FILTER_H
#define __KIS_LAYER_STYLE_FILTER_H

#include "kis_types.h"
#include "kis_shared.h"
#include "kritaimage_export.h"
#include "kis_psd_layer_style.h"
#include <QScopedPointer>

class KisLayerStyleFilterEnvironment;
class KisMultipleProjection;
class KisLayerStyleKnockoutBlower;

class KRITAIMAGE_EXPORT KisLayerStyleFilter : public KisShared
{
public:
    KisLayerStyleFilter(const KoID &id);
    virtual ~KisLayerStyleFilter();

    /**
     * \return Unique identifier for this filter
     */
    QString id() const;

    virtual KisLayerStyleFilter* clone() const = 0;

    virtual void processDirectly(KisPaintDeviceSP src,
                                 KisMultipleProjection *dst,
                                 KisLayerStyleKnockoutBlower *blower,
                                 const QRect &applyRect,
                                 KisPSDLayerStyleSP style,
                                 KisLayerStyleFilterEnvironment *env) const = 0;

    /**
     * Some filters need pixels outside the current processing rect to compute the new
     * value (for instance, convolution filters)
     */
    virtual QRect neededRect(const QRect & rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const = 0;

    /**
     * Similar to \ref neededRect: some filters will alter a lot of pixels that are
     * near to each other at the same time. So when you changed a single rectangle
     * in a device, the actual rectangle that will feel the influence of this change
     * might be bigger. Use this function to determine that rect.
     */
    virtual QRect changedRect(const QRect & rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const = 0;

protected:
    KisLayerStyleFilter(const KisLayerStyleFilter &rhs);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LAYER_STYLE_FILTER_H */

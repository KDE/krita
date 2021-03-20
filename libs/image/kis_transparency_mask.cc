/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transparency_mask.h"

#include "kis_debug.h"

#include <KoIcon.h>
#include <kis_icon.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_image.h"

KisTransparencyMask::KisTransparencyMask(KisImageWSP image, const QString &name)
        : KisEffectMask(image, name)
{
}

KisTransparencyMask::KisTransparencyMask(const KisTransparencyMask& rhs)
        : KisEffectMask(rhs)
{
}

KisTransparencyMask::~KisTransparencyMask()
{
}

QRect KisTransparencyMask::decorateRect(KisPaintDeviceSP &src,
                                        KisPaintDeviceSP &dst,
                                        const QRect & rc,
                                        PositionToFilthy maskPos) const
{
    Q_UNUSED(maskPos);

    if (src != dst) {
        KisPainter::copyAreaOptimized(rc.topLeft(), src, dst, rc);
        src->fill(rc, KoColor(Qt::transparent, src->colorSpace()));
    }

    return rc;
}

QRect KisTransparencyMask::extent() const
{
    return parent() ? parent()->extent() : QRect();
}

QRect KisTransparencyMask::exactBounds() const
{
    return parent() ? parent()->exactBounds() : QRect();
}

QRect KisTransparencyMask::changeRect(const QRect &rect, PositionToFilthy pos) const
{
    /**
     * Selection on transparency masks have no special meaning:
     * They do crop both: change and need area
     */
    return KisMask::changeRect(rect, pos);
}

QRect KisTransparencyMask::needRect(const QRect &rect, PositionToFilthy pos) const
{
    /**
     * Selection on transparency masks have no special meaning:
     * They do crop both: change and need area
     */
    return KisMask::needRect(rect, pos);
}

bool KisTransparencyMask::paintsOutsideSelection() const
{
    return true;
}

QIcon KisTransparencyMask::icon() const
{
    return KisIconUtils::loadIcon("transparencyMask");
}

bool KisTransparencyMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisTransparencyMask::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}


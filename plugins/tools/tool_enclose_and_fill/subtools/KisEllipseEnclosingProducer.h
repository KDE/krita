/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISELLIPSEENCLOSINGPRODUCER
#define KISELLIPSEENCLOSINGPRODUCER

#include <kis_tool_ellipse_base.h>
#include <kis_pixel_selection.h>

#include "KisDynamicDelegatedTool.h"

class KisEllipseEnclosingProducer : public KisDynamicDelegateTool<KisToolEllipseBase>
{
    Q_OBJECT

public:
    KisEllipseEnclosingProducer(KoCanvasBase *canvas);
    ~KisEllipseEnclosingProducer() override;
    
    bool hasUserInteractionRunning() const;
    
protected:
    void finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY) override;
    void beginShape() override;
    void endShape() override;

Q_SIGNALS:
    void enclosingMaskProduced(KisPixelSelectionSP enclosingMask);

private:
    bool m_hasUserInteractionRunning {false};
};

#endif

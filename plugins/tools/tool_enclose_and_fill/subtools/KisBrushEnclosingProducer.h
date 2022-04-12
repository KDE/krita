/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBRUSHENCLOSINGPRODUCER
#define KISBRUSHENCLOSINGPRODUCER

#include <kis_pixel_selection.h>

#include "KisToolBasicBrushBase.h"
#include "KisDynamicDelegatedTool.h"

class KisBrushEnclosingProducer : public KisDynamicDelegateTool<KisToolBasicBrushBase>
{
    Q_OBJECT

public:
    KisBrushEnclosingProducer(KoCanvasBase *canvas);
    ~KisBrushEnclosingProducer() override;
    
    bool hasUserInteractionRunning() const;
    
protected:
    void finishStroke(const QPainterPath &stroke) override;
    void beginShape() override;
    void endShape() override;

Q_SIGNALS:
    void enclosingMaskProduced(KisPixelSelectionSP enclosingMask);

private:
    bool m_hasUserInteractionRunning {false};
};

#endif

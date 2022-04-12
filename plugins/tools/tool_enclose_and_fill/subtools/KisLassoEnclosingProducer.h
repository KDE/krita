/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISLASSOENCLOSINGPRODUCER
#define KISLASSOENCLOSINGPRODUCER

#include <kis_pixel_selection.h>
#include <KisToolOutlineBase.h>

#include "KisDynamicDelegatedTool.h"

class KisLassoEnclosingProducer : public KisDynamicDelegateTool<KisToolOutlineBase>
{
    Q_OBJECT

public:
    KisLassoEnclosingProducer(KoCanvasBase *canvas);
    ~KisLassoEnclosingProducer() override;
    
    bool hasUserInteractionRunning() const;
    
protected:
    void finishOutline(const QVector<QPointF> &points) override;
    void beginShape() override;
    void endShape() override;

Q_SIGNALS:
    void enclosingMaskProduced(KisPixelSelectionSP enclosingMask);

private:
    bool m_hasUserInteractionRunning {false};
};

#endif

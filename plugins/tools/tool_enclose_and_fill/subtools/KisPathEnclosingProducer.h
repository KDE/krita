/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPATHENCLOSINGPRODUCER
#define KISPATHENCLOSINGPRODUCER

#include <KoCreatePathTool.h>
#include <kis_pixel_selection.h>
#include <kis_delegated_tool.h>
#include <kis_tool_shape.h>

#include "KisDynamicDelegatedTool.h"

class KisPathEnclosingProducer;

class KisToolPathLocalTool : public KoCreatePathTool {
public:
    KisToolPathLocalTool(KoCanvasBase * canvas, KisPathEnclosingProducer* parentTool);

    void paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter) override;
    void addPathShape(KoPathShape* pathShape) override;
    void beginShape() override;
    void endShape() override;

    using KoCreatePathTool::createOptionWidgets;
    using KoCreatePathTool::endPathWithoutLastPoint;
    using KoCreatePathTool::endPath;
    using KoCreatePathTool::cancelPath;
    using KoCreatePathTool::removeLastPoint;

private:
    KisPathEnclosingProducer* const m_parentTool;
};

typedef KisDelegatedTool<KisToolShape, KisToolPathLocalTool, DeselectShapesActivationPolicy> DelegatedPathTool;

class KisPathEnclosingProducer : public KisDynamicDelegateTool<DelegatedPathTool>
{
    Q_OBJECT

public:
    KisPathEnclosingProducer(KoCanvasBase *canvas);
    ~KisPathEnclosingProducer() override;
    
    bool hasUserInteractionRunning() const;

    void mousePressEvent(KoPointerEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void beginPrimaryAction(KoPointerEvent* event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    // reimplementing KisTool's method because that method calls beginPrimaryAction
    // which now is used to start the path tool.
    void beginPrimaryDoubleClickAction(KoPointerEvent* event) override;

protected:
    void requestStrokeCancellation() override;
    void requestStrokeEnd() override;

    void addPathShape(KoPathShape* pathShape);
    void beginShape() override;
    void endShape() override;

    friend class KisToolPathLocalTool;

Q_SIGNALS:
    void enclosingMaskProduced(KisPixelSelectionSP enclosingMask);

private:
    bool m_hasUserInteractionRunning {false};
};

#endif

/*
 *  SPDX-FileCopyrightText: 2017 Eugene Ingerman
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_SMART_PATCH_H_
#define KIS_TOOL_SMART_PATCH_H_

#include <QScopedPointer>
#include <QPainterPath>

#include "kis_tool_paint.h"

#include "KisToolPaintFactoryBase.h"

#include <flake/kis_node_shape.h>
#include <kis_icon.h>
#include <QKeySequence>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <KoIcon.h>

class KActionCollection;
class KoCanvasBase;
class KisPaintInformation;
class KisSpacingInfomation;


class KisToolSmartPatch : public KisToolPaint
{
    Q_OBJECT
public:
    KisToolSmartPatch(KoCanvasBase * canvas);
    ~KisToolSmartPatch() override;

    QWidget * createOptionWidget() override;

    void activatePrimaryAction() override;
    void deactivatePrimaryAction() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;
    int flags() const override { return KisTool::FLAG_USES_CUSTOM_SIZE | KisTool::FLAG_USES_CUSTOM_PRESET; }

protected Q_SLOTS:
    void resetCursorStyle() override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

private:
    //QRect inpaintImage(KisPaintDeviceSP maskDev, KisPaintDeviceSP imageDev);
    QPainterPath getBrushOutlinePath(const QPointF &documentPos, const KoPointerEvent *event);
    QPainterPath brushOutline();
    void requestUpdateOutline(const QPointF &outlineDocPoint, const KoPointerEvent *event) override;

private:
    struct Private;
    class InpaintCommand;
    const QScopedPointer<Private> m_d;

    void addMaskPath(KoPointerEvent *event);
};


class KisToolSmartPatchFactory : public KisToolPaintFactoryBase
{

public:
    KisToolSmartPatchFactory()
        : KisToolPaintFactoryBase("KritaShape/KisToolSmartPatch")
    {

        setToolTip(i18n("Smart Patch Tool"));

        setSection(ToolBoxSection::Fill);
        setIconName(koIconNameCStr("krita_tool_smart_patch"));
        setPriority(4);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolSmartPatchFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override
    {
        return new KisToolSmartPatch(canvas);
    }

};


#endif // KIS_TOOL_SMART_PATCH_H_

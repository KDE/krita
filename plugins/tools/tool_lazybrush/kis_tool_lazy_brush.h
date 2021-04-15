/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_LAZY_BRUSH_H_
#define KIS_TOOL_LAZY_BRUSH_H_

#include <QScopedPointer>
#include "kis_tool_freehand.h"

#include "KisToolPaintFactoryBase.h"

#include <flake/kis_node_shape.h>
#include <kis_icon.h>
#include <QKeySequence>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <KoIcon.h>

class KActionCollection;
class KoCanvasBase;

class KisToolLazyBrush : public KisToolFreehand
{
    Q_OBJECT
public:
    KisToolLazyBrush(KoCanvasBase * canvas);
    ~KisToolLazyBrush() override;

    QWidget * createOptionWidget() override;

    void activatePrimaryAction() override;
    void deactivatePrimaryAction() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void activateAlternateAction(AlternateAction action) override;
    void deactivateAlternateAction(AlternateAction action) override;

    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void endAlternateAction(KoPointerEvent *event, AlternateAction action) override;

    void explicitUserStrokeEndRequest() override;

protected Q_SLOTS:
    void resetCursorStyle() override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;

private Q_SLOTS:
    void slotCurrentNodeChanged(KisNodeSP node);

Q_SIGNALS:

private:
    bool colorizeMaskActive() const;
    bool canCreateColorizeMask() const;
    bool shouldActivateKeyStrokes() const;
    void tryCreateColorizeMask();

    void tryDisableKeyStrokesOnMask();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


class KisToolLazyBrushFactory : public KisToolPaintFactoryBase
{

public:
    KisToolLazyBrushFactory()
            : KisToolPaintFactoryBase("KritaShape/KisToolLazyBrush") {

        setToolTip(i18n("Colorize Mask Editing Tool"));

        // Temporarily
        setSection(ToolBoxSection::Fill);
        setIconName(koIconNameCStr("krita_tool_lazybrush"));
        //setShortcut(QKeySequence(Qt::Key_Shift + Qt::Key_B));
        setPriority(3);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolLazyBrushFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolLazyBrush(canvas);
    }

};


#endif // KIS_TOOL_LAZY_BRUSH_H_

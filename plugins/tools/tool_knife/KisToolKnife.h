/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_KNIFE_H_
#define KIS_TOOL_KNIFE_H_

#include <QScopedPointer>
#include <QPainterPath>

#include "KoInteractionTool.h"

#include "KisToolPaintFactoryBase.h"

#include <flake/kis_node_shape.h>
#include <kis_icon.h>
#include <QKeySequence>
#include <klocalizedstring.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <KoIcon.h>

class KisKActionCollection;
class KoCanvasBase;
class KisPaintInformation;
class KisSpacingInformation;


class KisToolKnife : public KoInteractionTool
{
    Q_OBJECT
public:
    KisToolKnife(KoCanvasBase * canvas);
    ~KisToolKnife() override;

    QWidget * createOptionWidget() override;

    void paint(QPainter &painter, const KoViewConverter &converter) override;

protected Q_SLOTS:

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void mousePressEventBackup(KoPointerEvent *event);
    void mouseMoveEvent(KoPointerEvent *event)  override;
    void mouseReleaseEvent(KoPointerEvent *event) override;

    KoInteractionStrategy *createStrategy(KoPointerEvent *event) override;
private:
    //friend class KisToolKnifeOptionsWidget;
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


class KisToolKnifeFactory : public KisToolPaintFactoryBase
{

public:
    KisToolKnifeFactory()
        : KisToolPaintFactoryBase("KritaShape/KisToolKnife")
    {

        setToolTip(i18n("Knife Tool"));

        setSection(ToolBoxSection::Main);
        setIconName(koIconNameCStr("tool_comic_panel"));
        setPriority(7);
        setActivationShapeId("flake/always,KoPathShape");
    }

    ~KisToolKnifeFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override
    {
        return new KisToolKnife(canvas);
    }

};


#endif // KIS_TOOL_KNIFE_H_

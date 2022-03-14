/*
 *  kis_tool_select_contiguous.h - part of KImageShop^WKrayon^Krita
 *
 *  SPDX-FileCopyrightText: 1999 Michael Koch <koch@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_SELECT_CONTIGUOUS_H__
#define __KIS_TOOL_SELECT_CONTIGUOUS_H__

#include "KisSelectionToolFactoryBase.h"
#include "kis_tool_select_base.h"
#include <kis_icon.h>
#include <kconfig.h>
#include <kconfiggroup.h>

/**
 * The 'magic wand' selection tool -- in fact just
 * a floodfill that only creates a selection.
 */
class KisToolSelectContiguous : public KisToolSelect
{

    Q_OBJECT

public:
    KisToolSelectContiguous(KoCanvasBase *canvas);
    ~KisToolSelectContiguous() override;

    QWidget* createOptionWidget() override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void resetCursorStyle() override;

protected:

    bool wantsAutoScroll() const override { return false; }

    bool isPixelOnly() const override { return true; }
    bool usesColorLabels() const override { return true; }

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    virtual void slotSetFuzziness(int);
    virtual void slotSetOpacitySpread(int);
    virtual void slotSetSizemod(int);
    virtual void slotSetFeather(int);
    virtual void slotSetUseSelectionAsBoundary(bool);
    //virtual bool antiAliasSelection();

protected:
    using KisToolSelectBase::m_widgetHelper;

private:
    int  m_fuzziness {20};
    int  m_opacitySpread {100};
    int  m_sizemod {0};
    int  m_feather {0};
    bool m_useSelectionAsBoundary {false};
    KConfigGroup m_configGroup;
};

class KisToolSelectContiguousFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectContiguousFactory()
        : KisSelectionToolFactoryBase("KisToolSelectContiguous")
    {
        setToolTip(i18n("Contiguous Selection Tool"));
        setSection(ToolBoxSection::Select);
        setIconName(koIconNameCStr("tool_contiguous_selection"));
        setPriority(4);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolSelectContiguousFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolSelectContiguous(canvas);
    }
};

#endif //__KIS_TOOL_SELECT_CONTIGUOUS_H__

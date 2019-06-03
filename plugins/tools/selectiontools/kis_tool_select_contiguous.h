/*
 *  kis_tool_select_contiguous.h - part of KImageShop^WKrayon^Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
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

    void resetCursorStyle() override;

protected:

    bool wantsAutoScroll() const override { return false; }

public Q_SLOTS:
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    virtual void slotSetFuzziness(int);
    virtual void slotSetSizemod(int);
    virtual void slotSetFeather(int);
    virtual void slotLimitToCurrentLayer(int);
    //virtual bool antiAliasSelection();

protected:
    using KisToolSelectBase::m_widgetHelper;

private:
    int  m_fuzziness;
    int  m_sizemod;
    int  m_feather;
    bool m_limitToCurrentLayer;
    KConfigGroup m_configGroup;
};

class KisToolSelectContiguousFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectContiguousFactory()
        : KisSelectionToolFactoryBase("KisToolSelectContiguous")
    {
        setToolTip(i18n("Contiguous Selection Tool"));
        setSection(TOOL_TYPE_SELECTION);
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

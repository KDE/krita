/*
 *  kis_tool_select_contiguous.h - part of KImageShop^WKrayon^Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <kis_tool.h>
#include <kis_tool_non_paint.h>
#include <kis_tool_factory.h>
#include <kis_selection.h>

class QWidget;
class QVBoxLayout;
class QCheckBox;

class KisSelectionOptions;
class KisCanvasSubject;

/**
 * The 'magic wand' selection tool -- in fact just
 * a floodfill that only creates a selection.
 */
class KisToolSelectContiguous : public KisToolNonPaint {

    typedef KisToolNonPaint super;
    Q_OBJECT

public:
    KisToolSelectContiguous();
    virtual ~KisToolSelectContiguous();

public:
    virtual void update(KisCanvasSubject *subject);

    virtual void setup(KActionCollection *collection);
    virtual quint32 priority() { return 7; }
    virtual enumToolType toolType() { return TOOL_SELECT; };

    virtual QWidget* createOptionWidget(QWidget* parent);
        virtual QWidget* optionWidget();

    virtual void buttonPress(KisButtonPressEvent *event);

public slots:
    virtual void slotSetFuzziness(int);
    virtual void slotSetAction(int);
    virtual void slotSetSampleMerged(int);
    virtual void activate();


private:
    KisCanvasSubject *m_subject;
    KisSelectionOptions * m_optWidget;

    int m_fuzziness;
    enumSelectionMode m_selectAction;
    bool m_sampleMerged;
};

class KisToolSelectContiguousFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolSelectContiguousFactory() : super() {};
    virtual ~KisToolSelectContiguousFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolSelectContiguous();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("contiguousselect", i18n("Contiguous Select Tool")); }
};


#endif //__KIS_TOOL_SELECT_CONTIGUOUS_H__


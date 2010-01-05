class RulerAssistant;
/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_RULER_ASSISTANT_TOOL_H_
#define _KIS_RULER_ASSISTANT_TOOL_H_

#include <kis_tool.h>
#include <KoToolFactory.h>
#include "kis_painting_assistant.h"

class RulerDecoration;
class KisCanvas2;
class ConstraintSolver;

class KisRulerAssistantTool : public KisTool
{
    Q_OBJECT
public:
    KisRulerAssistantTool(KoCanvasBase * canvas);
    virtual ~KisRulerAssistantTool();

    virtual quint32 priority() {
        return 3;
    }
    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

public slots:
    virtual void activate(bool temp = false);
    void deactivate();

protected:

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

protected:
    KisCanvas2* m_canvas;
    QWidget* m_widget;
    QList<KisPaintingAssistantHandleSP> m_handles;
    KisPaintingAssistantHandleSP m_handleDrag;
};


class KisRulerAssistantToolFactory : public KoToolFactory
{
public:
    KisRulerAssistantToolFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisRulerAssistantTool") {
        setToolTip(i18n("Ruler assistant editor tool"));
        setToolType(TOOL_TYPE_VIEW);
        setIcon("krita_tool_ruler_assistant");
        setPriority(0);
    };


    virtual ~KisRulerAssistantToolFactory() {}

    virtual KoTool * createTool(KoCanvasBase * canvas) {
        return new KisRulerAssistantTool(canvas);
    }

};


#endif


/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TOOL_PROXY_H
#define __KIS_TOOL_PROXY_H

#include <KoToolProxy.h>
#include <kis_tool.h>


class KisToolProxy : public KoToolProxy
{
    Q_OBJECT
public:
    enum ActionState {
        BEGIN,  /**< Beginning an action */
        CONTINUE, /**< Continuing an action */
        END /**< Ending an action */
    };

public:
    KisToolProxy(KoCanvasBase *canvas, QObject *parent = 0);
    void initializeImage(KisImageSP image);

    void forwardHoverEvent(QEvent *event);

    /**
     * Forwards the event to the active tool and returns true if the
     * event was not ignored.  That is by default the event is
     * considered accepted, but the tool can explicitly ignore it.
     * @param state beginning, continuing, or ending the action.
     * @param action alternate tool action requested.
     * @param event the event being sent to the tool by the AbstractInputAction.
     * @param originalEvent the original event received by the AbstractInputAction.
     */
    bool forwardEvent(ActionState state, KisTool::ToolAction action, QEvent *event, QEvent *originalEvent);
    bool primaryActionSupportsHiResEvents() const;
    bool alternateActionSupportsHiResEvents(KisTool::AlternateAction action) const;

    void setActiveTool(KoToolBase *tool) override;

    void activateToolAction(KisTool::ToolAction action);
    void deactivateToolAction(KisTool::ToolAction action);

private:
    KoPointerEvent convertEventToPointerEvent(QEvent *event, const QPointF &docPoint, bool *result);
    QPointF tabletToDocument(const QPointF &globalPos);
    void forwardToTool(ActionState state, KisTool::ToolAction action, QEvent *event, const QPointF &docPoint);

protected:
    QPointF widgetToDocument(const QPointF &widgetPoint) const override;

private:
    bool m_isActionActivated;
    KisTool::ToolAction m_lastAction;
};

#endif /* __KIS_TOOL_PROXY_H */

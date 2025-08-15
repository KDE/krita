/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_PROXY_H
#define __KIS_TOOL_PROXY_H

#include <kritaui_export.h>
#include <KoToolProxy.h>
#include <kis_tool.h>


class KRITAUI_EXPORT KisToolProxy : public KoToolProxy
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

    bool supportsPaintingAssistants() const;
Q_SIGNALS:
    void toolPrimaryActionActivated(bool activated);

private:
    KoPointerEvent convertEventToPointerEvent(QEvent *event, const QPointF &docPoint, bool *result);
    QPointF tabletToDocument(const QPointF &globalPos);
    void forwardToTool(ActionState state, KisTool::ToolAction action, QEvent *event, const QPointF &docPoint);

protected:
    QPointF widgetToDocument(const QPointF &widgetPoint) const override;
    QPointF documentToWidget(const QPointF &documentPoint) const override;

private:
    bool m_isActionActivated;
    KisTool::ToolAction m_lastAction;
};

#endif /* __KIS_TOOL_PROXY_H */

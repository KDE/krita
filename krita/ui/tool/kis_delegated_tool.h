/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_DELEGATED_TOOL_H
#define __KIS_DELEGATED_TOOL_H

#include <KoPointerEvent.h>
#include <KoShape.h>

#include <QPointer>

#include "kis_delegated_tool_policies.h"


template <class BaseClass, class DelegateTool, class ActivationPolicy = NoopActivationPolicy>
    class KisDelegatedTool : public BaseClass
{
public:
    KisDelegatedTool(KoCanvasBase *canvas,
                     const QCursor &cursor,
                     DelegateTool *delegateTool)
        : BaseClass(canvas, cursor),
          m_localTool(delegateTool)
    {
    }

    DelegateTool* localTool() const {
        return m_localTool.data();
    }

    void activate(typename BaseClass::ToolActivation toolActivation, const QSet<KoShape*> &shapes)
    {
        BaseClass::activate(toolActivation, shapes);
        m_localTool->activate(toolActivation, shapes);
        ActivationPolicy::onActivate(BaseClass::canvas());
    }

    void deactivate()
    {
        m_localTool->deactivate();
        BaseClass::deactivate();
    }

    void mousePressEvent(KoPointerEvent *event)
    {
        if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                              Qt::LeftButton, Qt::ShiftModifier |
                              Qt::ControlModifier | Qt::AltModifier)) {

            this->setMode(KisTool::PAINT_MODE);

            Q_ASSERT(m_localTool);
            m_localTool->mousePressEvent(event);
        }
        else {
            BaseClass::mousePressEvent(event);
        }
    }

    void mouseDoubleClickEvent(KoPointerEvent *event)
    {
        if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                              Qt::LeftButton, Qt::ShiftModifier |
                              Qt::ControlModifier | Qt::AltModifier)) {

            Q_ASSERT(m_localTool);
            m_localTool->mouseDoubleClickEvent(event);
        }
        else {
            BaseClass::mouseDoubleClickEvent(event);
        }
    }

    void mouseMoveEvent(KoPointerEvent *event)
    {
        Q_ASSERT(m_localTool);
        m_localTool->mouseMoveEvent(event);

        BaseClass::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(KoPointerEvent *event)
    {
        if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
            this->setMode(KisTool::HOVER_MODE);

            Q_ASSERT(m_localTool);
            m_localTool->mouseReleaseEvent(event);
        }
        else {
            BaseClass::mouseReleaseEvent(event);
        }
    }

    void paint(QPainter &painter, const KoViewConverter &converter)
    {
        Q_ASSERT(m_localTool);
        m_localTool->paint(painter, converter);
    }

    QList<QPointer<QWidget> > createOptionWidgets()
    {
        QList<QPointer<QWidget> > list = BaseClass::createOptionWidgets();
        list.append(m_localTool->createOptionWidgets());
        return list;
    }

protected:
    QScopedPointer<DelegateTool> m_localTool;
};

#endif /* __KIS_DELEGATED_TOOL_H */

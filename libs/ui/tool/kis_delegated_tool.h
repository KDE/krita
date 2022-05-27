/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DELEGATED_TOOL_H
#define __KIS_DELEGATED_TOOL_H

#include <KoPointerEvent.h>
#include <KoShape.h>

#include <QLayout>
#include <QPointer>

#include "canvas/kis_canvas2.h"
#include "input/kis_input_manager.h"
#include "kis_delegated_tool_policies.h"
#include "kis_tool.h"
#include <KisOptionCollectionWidget.h>

#define PRESS_CONDITION_OM(_event, _mode, _button, _modifier)           \
    (this->mode() == (_mode) && (_event)->button() == (_button) &&      \
     ((_event)->modifiers() & (_modifier) ||                            \
      (_event)->modifiers() == Qt::NoModifier))

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

    void activate(const QSet<KoShape*> &shapes) override
    {
        BaseClass::activate(shapes);
        m_localTool->activate(shapes);
        ActivationPolicy::onActivate(BaseClass::canvas());

        KisInputManager *inputManager = (static_cast<KisCanvas2*>(BaseClass::canvas()))->globalInputManager();
        if (inputManager) {
            inputManager->attachPriorityEventFilter(this);
        }
    }

    void deactivate() override
    {
        m_localTool->deactivate();
        BaseClass::deactivate();

        KisInputManager *inputManager = (static_cast<KisCanvas2*>(BaseClass::canvas()))->globalInputManager();
        if (inputManager) {
            inputManager->detachPriorityEventFilter(this);
        }
    }

    void mousePressEvent(KoPointerEvent *event) override
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

    void mouseDoubleClickEvent(KoPointerEvent *event) override
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

    void mouseMoveEvent(KoPointerEvent *event) override
    {
        Q_ASSERT(m_localTool);
        m_localTool->mouseMoveEvent(event);

        BaseClass::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(KoPointerEvent *event) override
    {
        if (this->mode() == KisTool::PAINT_MODE && event->button() == Qt::LeftButton) {
            this->setMode(KisTool::HOVER_MODE);

            Q_ASSERT(m_localTool);
            m_localTool->mouseReleaseEvent(event);
        }
        else {
            BaseClass::mouseReleaseEvent(event);
        }
    }

    void paint(QPainter &painter, const KoViewConverter &converter) override
    {
        Q_ASSERT(m_localTool);
        m_localTool->paint(painter, converter);
    }

    QList<QPointer<QWidget> > createOptionWidgets() override
    {
        QList<QPointer<QWidget>> baseWidgetList =
            BaseClass::createOptionWidgets();
        QList<QPointer<QWidget>> localWidgetList =
            m_localTool->createOptionWidgets();

        if (baseWidgetList.size() > 0
            && dynamic_cast<KisOptionCollectionWidget *>(
                baseWidgetList.first().data())) {
            KisOptionCollectionWidget *baseOptionsWidget =
                dynamic_cast<KisOptionCollectionWidget *>(
                    baseWidgetList.first().data());
            for (int i = 0; i < localWidgetList.size(); ++i) {
                QWidget *widget = localWidgetList[i];
                KisOptionCollectionWidgetWithHeader *section =
                    new KisOptionCollectionWidgetWithHeader(
                        widget->windowTitle());
                const QString sectionName = "section" + QString::number(i);
                section->appendWidget(sectionName + "Widget", widget);
                baseOptionsWidget->appendWidget(sectionName, section);
            }
        } else {
            baseWidgetList.append(localWidgetList);
        }
        return baseWidgetList;
    }

protected:
    QScopedPointer<DelegateTool> m_localTool;
};

#endif /* __KIS_DELEGATED_TOOL_H */

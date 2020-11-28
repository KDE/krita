/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "PathToolOptionWidget.h"
#include "KoPathTool.h"
#include <QAction>

#include <KoPathShape.h>
#include <KoParameterShape.h>
#include <KoShapeConfigWidgetBase.h>
#include <QVBoxLayout>
#include <KoCanvasBase.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>
#include <KoUnit.h>
#include "kis_assert.h"

PathToolOptionWidget::PathToolOptionWidget(KoPathTool *tool, QWidget *parent)
    : QWidget(parent),
      m_currentShape(0),
      m_currentPanel(0),
      m_canvas(tool->canvas())

{
    widget.setupUi(this);
    widget.corner->setDefaultAction(tool->action("pathpoint-corner"));
    widget.smooth->setDefaultAction(tool->action("pathpoint-smooth"));
    widget.symmetric->setDefaultAction(tool->action("pathpoint-symmetric"));
    widget.lineSegment->setDefaultAction(tool->action("pathsegment-line"));
    widget.curveSegment->setDefaultAction(tool->action("pathsegment-curve"));
    widget.linePoint->setDefaultAction(tool->action("pathpoint-line"));
    widget.curvePoint->setDefaultAction(tool->action("pathpoint-curve"));
    widget.addPoint->setDefaultAction(tool->action("pathpoint-insert"));
    widget.removePoint->setDefaultAction(tool->action("pathpoint-remove"));
    widget.breakPoint->setDefaultAction(tool->action("path-break-point"));
    widget.breakSegment->setDefaultAction(tool->action("path-break-segment"));
    widget.joinSegment->setDefaultAction(tool->action("pathpoint-join"));
    widget.mergePoints->setDefaultAction(tool->action("pathpoint-merge"));

    widget.wdgShapeProperties->setVisible(false);
    widget.lineShapeProperties->setVisible(false);

    connect(widget.convertToPath, SIGNAL(released()), tool->action("convert-to-path"), SLOT(trigger()));
}

PathToolOptionWidget::~PathToolOptionWidget()
{
}

void PathToolOptionWidget::setSelectionType(int type)
{
    const bool plain = type & PlainPath;
    if (plain)
        widget.stackedWidget->setCurrentIndex(0);
    else
        widget.stackedWidget->setCurrentIndex(1);
}

QString shapeIdFromShape(KoPathShape *pathShape)
{
    if (!pathShape) return QString();

    QString shapeId = pathShape->pathShapeId();

    KoParameterShape *paramShape = dynamic_cast<KoParameterShape *>(pathShape);
    if (paramShape && !paramShape->isParametricShape()) {
        shapeId = paramShape->shapeId();
    }

    return shapeId;
}

void PathToolOptionWidget::setCurrentShape(KoPathShape *pathShape)
{
    const QString newShapeId = shapeIdFromShape(pathShape);
    if (pathShape == m_currentShape && m_currentShapeId == newShapeId) return;

    if (m_currentShape) {
        m_currentShape = 0;
        if (m_currentPanel) {
            m_currentPanel->deleteLater();
            m_currentPanel = 0;
            m_currentShapeId.clear();
        }
    }

    if (pathShape) {
        m_currentShape = pathShape;
        m_currentShapeId = newShapeId;

        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value(m_currentShapeId);
        KIS_SAFE_ASSERT_RECOVER_RETURN(factory);

        QList<KoShapeConfigWidgetBase*> panels = factory->createShapeOptionPanels();
        if (!panels.isEmpty()) {
            KoShapeConfigWidgetBase *activePanel = 0;

            Q_FOREACH (KoShapeConfigWidgetBase *panel, panels) {
                if (!activePanel && panel->showOnShapeSelect()) {
                    activePanel = panel;
                } else {
                    delete panel;
                }
            }

            if (activePanel) {
                KIS_ASSERT_RECOVER_RETURN(m_canvas);
                m_currentPanel = activePanel;
                m_currentPanel->setUnit(m_canvas->unit());

                QLayout *baseLayout = widget.wdgShapeProperties->layout();
                QVBoxLayout *layout = dynamic_cast<QVBoxLayout*>(widget.wdgShapeProperties->layout());

                if (!layout) {
                    KIS_SAFE_ASSERT_RECOVER(!baseLayout) {
                        delete baseLayout;
                    }
                    layout = new QVBoxLayout(widget.wdgShapeProperties);
                }


                KIS_ASSERT_RECOVER_RETURN(widget.wdgShapeProperties->layout());
                layout->addWidget(m_currentPanel);
                connect(m_currentPanel, SIGNAL(propertyChanged()), SLOT(slotShapePropertyChanged()));
                m_currentPanel->open(m_currentShape);
            }
        }
    }

    widget.wdgShapeProperties->setVisible(m_currentPanel);
    widget.lineShapeProperties->setVisible(m_currentPanel);
}

void PathToolOptionWidget::slotShapePropertyChanged()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_currentPanel);

    KUndo2Command *command = m_currentPanel->createCommand();
    if (command) {
        m_canvas->addCommand(command);
    }
}

void PathToolOptionWidget::showEvent(QShowEvent *event)
{
    emit sigRequestUpdateActions();
    QWidget::showEvent(event);
}

/* This file is part of the KDE project

   Copyright (c) 2010 Cyril Oblikov <munknex@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "TreeShape.h"
#include "TreeShapeConfigWidget.h"
#include "TreeChangeStructureCommand.h"
//#include "TreeChangeShapeCommand.h"
#include "TreeChangeConnectionCommand.h"
#include "TreeTool.h"
#include "SelectionDecorator.h"
#include "TreeShapeMoveStrategy.h"
#include "KoGradientBackground.h"

#include <KoPointerEvent.h>
#include <KoToolSelection.h>
#include <KoToolManager.h>
#include <KoSelection.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoShapePaste.h>
#include <KoShapeOdfSaveHelper.h>
#include <KoDrag.h>
#include <KoDocument.h>
#include <KoCanvasBase.h>
#include <KoShapeRubberSelectStrategy.h>
#include <TreeShapeMoveCommand.h>
#include <commands/KoShapeDeleteCommand.h>
#include <commands/KoShapeCreateCommand.h>
#include <KoSnapGuide.h>

#include <QKeyEvent>
#include <QClipboard>

#include "kdebug.h"

class SelectionHandler : public KoToolSelection
{
public:
    SelectionHandler(TreeTool *parent)
        : KoToolSelection(parent), m_selection(parent->koSelection())
    {
        Q_ASSERT(m_selection);
    }

    bool hasSelection() {
        return m_selection->count();
    }

private:
    KoSelection *m_selection;
};

TreeTool::TreeTool(KoCanvasBase *canvas)
    : KoInteractionTool(canvas),
    m_hotPosition(KoFlake::TopLeftCorner),
    m_moveCommand(0),
    m_selectionHandler(new SelectionHandler(this))
{
    KoShapeManager * manager = canvas->shapeManager();
    connect(manager, SIGNAL(selectionChanged()), this, SLOT(grabTrees()));
}

TreeTool::~TreeTool()
{
}

void TreeTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    KoInteractionTool::paint(painter, converter);
    if (currentStrategy() == 0 && koSelection()->count() > 0) {
        SelectionDecorator decorator;
        decorator.setSelection(koSelection());
        decorator.paint(painter, converter);
    }
}

void TreeTool::activate(ToolActivation, const QSet<KoShape*> &)
{
    useCursor(Qt::ArrowCursor);
    koSelection()->deselectAll();
    repaintDecorations();
}

void TreeTool::changeStructure(int index)
{
    kDebug() << index;
    KUndo2Command *command = new KUndo2Command;
    command->setText(i18nc("(qtundo-format)", "Change Tree Structure"));
    foreach(TreeShape *tree, m_selectedTrees) {
        TreeShape::TreeType structure = static_cast<TreeShape::TreeType>(index);
        new TreeChangeStructureCommand(tree, structure, command);
    }

    canvas()->addCommand(command);
}

void TreeTool::changeShape(int index)
{
    kDebug() << index;
//     KUndo2Command *command = new KUndo2Command;
//     command->setText(i18nc("(qtundo-format)", "Change Background Shape"));
//     foreach(TreeShape *tree, m_selectedTrees) {
//         TreeShape::RootType type = static_cast<TreeShape::RootType>(index);
//         new TreeChangeShapeCommand(tree, type, command);
//     }
//
//     canvas()->addCommand(command);
}

void TreeTool::changeConnectionType(int index)
{
    kDebug() << index;
    KUndo2Command *command = new KUndo2Command;
    command->setText(i18nc("(qtundo-format)", "Change Connection Type"));
    foreach(TreeShape *tree, m_selectedTrees) {
        KoConnectionShape::Type type = static_cast<KoConnectionShape::Type>(index);
        new TreeChangeConnectionCommand(tree, type, command);
    }

    canvas()->addCommand(command);
}

void TreeTool::grabTrees()
{
    m_selectedTrees.clear();

    TreeShape *tree;
    foreach (KoShape *shape, canvas()->shapeManager()->selection()->selectedShapes())
        while (shape) {
            tree = dynamic_cast<TreeShape*>(shape->parent());
            if (tree) {
                m_selectedTrees.append(tree);
                shape = 0;
            } else {
                shape = shape->parent();
            }
        }

    emit updateConfigWidget(m_selectedTrees.isEmpty() ? 0 : m_selectedTrees.first());
}

void TreeTool::mousePressEvent(KoPointerEvent *event)
{
    KoInteractionTool::mousePressEvent(event);
}

void TreeTool::mouseMoveEvent(KoPointerEvent *event)
{
    KoInteractionTool::mouseMoveEvent(event);
}

void TreeTool::mouseReleaseEvent(KoPointerEvent *event)
{
    KoInteractionTool::mouseReleaseEvent(event);
}

void TreeTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
    kDebug() << "doubleclick";
}

void TreeTool::keyPressEvent(QKeyEvent *event)
{
    KoInteractionTool::keyPressEvent(event);
    KoShape *root;
    switch (event->key()) {
        case Qt::Key_Tab:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()) {
                TreeShape *tree = dynamic_cast<TreeShape*>(root->parent());
                if (tree) {
                    kDebug() << "Adding child...";
                    KoShapeController *controller = canvas()->shapeController();
                    KUndo2Command *command = new KUndo2Command;
                    foreach(KoShape* shape, tree->addNewChild()) {
                        controller->addShapeDirect(shape, command);
                    }
                    canvas()->addCommand(command);
                }
            }
            event->accept();
            break;
        case Qt::Key_Return:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()) {
                TreeShape *tree = dynamic_cast<TreeShape*>(root->parent());
                if (tree) {
                    tree = dynamic_cast<TreeShape*>(tree->parent());
                    if (tree) {
                        kDebug() << "Adding child...";
                        KoShapeController *controller = canvas()->shapeController();
                        KUndo2Command *command = new KUndo2Command;
                        foreach(KoShape* shape, tree->addNewChild()) {
                            controller->addShapeDirect(shape, command);
                        }
                        canvas()->addCommand(command);
                    }
                }
            }
            event->accept();
            break;
        case Qt::Key_D:
            kDebug() << "Key_D";
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()) {
                TreeShape *tree = dynamic_cast<TreeShape*>(root->parent());
                kDebug() << root->shapeId();
                if (tree) {
                    kDebug() << tree->shapeId();
                    KoShapeController *controller = canvas()->shapeController();
                    KUndo2Command *command = new KUndo2Command;
                    controller->removeShape(tree,command);
                    TreeShape *grandparent = dynamic_cast<TreeShape*>(tree->parent());
                    if (grandparent) {
                        controller->removeShape(grandparent->connector(tree),command);
                    }
                    canvas()->addCommand(command);
                }
            }
            event->accept();
            break;
        case Qt::Key_Z:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()) {
                TreeShape *tree = dynamic_cast<TreeShape*>(root->parent());
                if (tree) {
                    kDebug() << "zIndeces of children...";
                    foreach (KoShape *shape, tree->shapes()) {
                        TreeShape *tmp = dynamic_cast<TreeShape*>(shape);
                        if (tmp)
                            kDebug() << shape->shapeId() << shape->zIndex() << shape->boundingRect()
                                     << tmp->shapes().last()->shapeId()
                                     << tmp->shapes().last()->zIndex()
                                     << tmp->shapes().last()->boundingRect();
                    }
                }
            }
            event->accept();
            break;
        case Qt::Key_1:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()) {
                TreeShape *tree = dynamic_cast<TreeShape*>(root->parent());
                if (tree) {
                    kDebug() << "OrgUp";
                    tree->setStructure(TreeShape::OrgUp);
                    tree->update();
                }
            }
            event->accept();
            break;
        case Qt::Key_2:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()) {
                TreeShape *tree = dynamic_cast<TreeShape*>(root->parent());
                if (tree) {
                    kDebug() << "OrgRight";
                    tree->setStructure(TreeShape::OrgRight);
                    tree->update();
                }
            }
            event->accept();
            break;
        case Qt::Key_3:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()) {
                TreeShape *tree = dynamic_cast<TreeShape*>(root->parent());
                if (tree) {
                    kDebug() << "OrgDown";
                    tree->setStructure(TreeShape::OrgDown);
                    tree->update();
                }
            }
            event->accept();
            break;
        case Qt::Key_4:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()) {
                TreeShape *tree = dynamic_cast<TreeShape*>(root->parent());
                if (tree) {
                    kDebug() << "OrgLeft";
                    tree->setStructure(TreeShape::OrgLeft);
                    tree->update();
                }
            }
            event->accept();
            break;
        default:
            return;
    }
}

KoToolSelection* TreeTool::selection()
{
    return m_selectionHandler;
}

QList<QWidget *> TreeTool::createOptionWidgets()
{
    TreeShapeConfigWidget *widget = new TreeShapeConfigWidget(this);
    connect(this, SIGNAL(updateConfigWidget(TreeShape*)),
            widget, SLOT(updateParameters(TreeShape*)));
    emit updateConfigWidget(0);

    QList<QWidget *> widgets;
    widgets.append(widget);

    return widgets;
}

KoInteractionStrategy *TreeTool::createStrategy(KoPointerEvent *event)
{
    // reset the move by keys when a new strategy is created otherwise we might change the
    // command after a new command was added. This happend when you where faster than the timer.
    m_moveCommand = 0;

    KoShapeManager *shapeManager = canvas()->shapeManager();
    KoSelection *select = shapeManager->selection();

    bool selectMultiple = event->modifiers() & Qt::ControlModifier;
    bool selectNextInStack = event->modifiers() & Qt::ShiftModifier;

//     if ((event->buttons() == Qt::LeftButton) && !(selectMultiple || selectNextInStack)) {
//         const QPainterPath outlinePath = select->transformation().map(select->outline());
//         if (outlinePath.contains(event->point) ||
//             outlinePath.intersects(handlePaintRect(event->point))) {
//                 kDebug() << 1;
//                 return new TreeShapeMoveStrategy(this, event->point);
//         }
//     }

    if ((event->buttons() & Qt::LeftButton) == 0)
        return 0;  // Nothing to do for middle/right mouse button

    KoFlake::ShapeSelection sel;
    sel = selectNextInStack ? KoFlake::NextUnselected : KoFlake::ShapeOnTop;
    KoShape *shape = shapeManager->shapeAt(event->point, sel);

    if (!shape) {
        if (!selectMultiple) {
            repaintDecorations();
            select->deselectAll();
        }
        kDebug() << "KoShapeRubberSelectStrategy(this, event->point)";
        return new KoShapeRubberSelectStrategy(this, event->point);
    }

    if (select->isSelected(shape)) {
        kDebug() << "shape is already selected" << shape->shapeId();
        kDebug() << "shape" << shape << "zIndex" << shape->zIndex();
        if (selectMultiple) {
            repaintDecorations();
            select->deselect(shape);
            kDebug() << "deselecting already selected shape";
        }
    } else { // clicked on shape which is not selected
        repaintDecorations();
        if (!selectMultiple) {
            kDebug() << "deselecting all";
            shapeManager->selection()->deselectAll();
        }
        select->select(shape, selectNextInStack ? false : true);
        kDebug() << "selecting shape and creating TreeShapeMoveStrategy";
        repaintDecorations();
        return new TreeShapeMoveStrategy(this, event->point);
    }
    return 0;
}

void TreeTool::repaintDecorations()
{
    Q_ASSERT(koSelection());
    if (koSelection()->count() > 0)
        canvas()->updateCanvas(koSelection()->boundingRect());
}

void TreeTool::copy() const
{
    QList<KoShape*> shapes;
    foreach (TreeShape *tree, m_selectedTrees)
        shapes.append(dynamic_cast<KoShape*>(tree));

    if (!m_selectedTrees.empty()) {
        KoShapeOdfSaveHelper saveHelper(shapes);
        KoDrag drag;
        drag.setOdf(KoOdf::mimeType(KoOdf::Text), saveHelper);
        drag.addToClipboard();
    }
}

void TreeTool::deleteSelection()
{
    QList<KoShape*> shapes;
    foreach (TreeShape *tree, m_selectedTrees)
        shapes.append(dynamic_cast<KoShape*>(tree));

    if (!m_selectedTrees.empty()) {
        canvas()->addCommand(canvas()->shapeController()->removeShapes(shapes));
    }
}

bool TreeTool::paste()
{
    const QMimeData * data = QApplication::clipboard()->mimeData();

    bool success = false;
    if (data->hasFormat(KoOdf::mimeType(KoOdf::Text))) {
        KoShapeManager * shapeManager = canvas()->shapeManager();
        KoShapePaste paste(canvas(), shapeManager->selection()->activeLayer());
        success = paste.paste(KoOdf::Text, data);
        if (success) {
            shapeManager->selection()->deselectAll();
            foreach(KoShape *shape, paste.pastedShapes()) {
                TreeShape *tree = dynamic_cast<TreeShape*>(shape);
                Q_ASSERT(tree);
                shapeManager->selection()->select(tree->root());
            }
        }
    }
    return success;
}

QStringList TreeTool::supportedPasteMimeTypes() const
{
    QStringList list;
    list << KoOdf::mimeType(KoOdf::Text);
    return list;
}

KoSelection *TreeTool::koSelection()
{
    Q_ASSERT(canvas());
    Q_ASSERT(canvas()->shapeManager());
    return canvas()->shapeManager()->selection();
}

#include <TreeTool.moc>

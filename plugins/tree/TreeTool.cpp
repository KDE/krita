/* This file is part of the KDE project

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
   Boston, MA 02110-1301, USA.
*/

#include "TreeTool.h"
#include "Tree.h"

#include <QToolButton>
#include <QGridLayout>
#include <KLocale>
#include <KIconLoader>
#include <KUrl>
#include <KFileDialog>

#include <KoCanvasBase.h>
#include <KoImageCollection.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoGradientBackground.h>
#include <KoShapeController.h>
#include <QUndoCommand>

#include <kdebug.h>

TreeTool::TreeTool(KoCanvasBase* canvas)
    : KoToolBase(canvas)
{
}

void TreeTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);
    Q_UNUSED(shapes);

    useCursor(Qt::ArrowCursor);
}

void TreeTool::deactivate()
{
}

void TreeTool::keyPressEvent(QKeyEvent *event)
{
    KoShape *root;
    switch (event->key()) {
        case Qt::Key_Tab:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()){
                Tree *tree = dynamic_cast<Tree*>(root->parent());
                if (tree){
                    kDebug() << "Adding child...";
                    KoShapeController *controller = canvas()->shapeController();
                    QUndoCommand *command = new QUndoCommand;
                    foreach(KoShape* shape, tree->addNewChild()){
                        controller->addShapeDirect(shape, command);
                    }
                    canvas()->addCommand(command);
//                     canvas()->updateCanvas(tree->boundingRect().normalized());
//                     kDebug() << tree->boundingRect().normalized();
                }
            }
            event->accept();
            break;
        case Qt::Key_Return:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()){
                Tree *tree = dynamic_cast<Tree*>(root->parent());
                if (tree)
                    if (tree = dynamic_cast<Tree*>(tree->parent())){
                        kDebug() << "Adding child...";
                        KoShapeController *controller = canvas()->shapeController();
                        QUndoCommand *command = new QUndoCommand;
                        foreach(KoShape* shape, tree->addNewChild()){
                            controller->addShapeDirect(shape, command);
                        }
                        canvas()->addCommand(command);
                    }
            }
            event->accept();
            break;
        case Qt::Key_Delete:
            foreach (root, canvas()->shapeManager()->selection()->selectedShapes()){
                Tree *tree = dynamic_cast<Tree*>(root->parent());
                if (tree){
                    KoShapeController *controller = canvas()->shapeController();
                    Tree *grandparent = dynamic_cast<Tree*>(tree->parent());
                    QUndoCommand *command = new QUndoCommand;
                    controller->removeShape(tree,command);
                    controller->removeShape(grandparent->connector(tree),command);
                    canvas()->addCommand(command);
                }
            }
            event->accept();
            break;
        default:
            return;
    }
}

void TreeTool::mousePressEvent(KoPointerEvent* event)
{
    canvas()->shapeManager()->selection()->deselectAll();
    KoShape *root = canvas()->shapeManager()->shapeAt(event->point);
    if (root)
        if (dynamic_cast<Tree*>(root->parent())){
            canvas()->shapeManager()->selection()->select(root,false);
            QRadialGradient *gradient = new QRadialGradient(QPointF(0.5,0.5), 0.5, QPointF(0.25,0.25));
            gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
            gradient->setColorAt(0.0, Qt::white);
            gradient->setColorAt(1.0, Qt::red);
            root->setBackground(new KoGradientBackground(gradient));
            root->update();
            event->accept();
        }
}

void TreeTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
    //shape editing should be here
}

#include <TreeTool.moc>

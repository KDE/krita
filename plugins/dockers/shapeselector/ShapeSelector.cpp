/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ShapeSelector.h"
#include "GroupShape.h"
#include "IconShape.h"
#include "TemplateShape.h"
#include "Canvas.h"

#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoToolManager.h>
#include <KoShapeFactory.h>
#include <KoSelection.h>
#include <KoCreateShapesTool.h>
#include <KoCanvasController.h>

#include <QPainter>
#include <QToolTip>
#include <QTimer>
#include <klocale.h>

// ************** ShapeSelector ************
ShapeSelector::ShapeSelector(QWidget *parent)
: QDockWidget(i18n("Shapes"), parent)
{
    setObjectName("ShapeSelector");
    m_canvas = new Canvas(this);
    setWidget(m_canvas);
    m_shapeManager = new KoShapeManager(m_canvas);
    setMinimumSize(30, 30);

    QTimer::singleShot(0, this, SLOT(loadShapeTypes()));
}

ShapeSelector::~ShapeSelector() {
    delete m_shapeManager;
    delete m_canvas;
}

void ShapeSelector::loadShapeTypes() {
    foreach(QString id, KoShapeRegistry::instance()->keys()) {
        KoShapeFactory *factory = KoShapeRegistry::instance()->value(id);
        bool oneAdded=false;
        foreach(KoShapeTemplate shapeTemplate, factory->templates()) {
            oneAdded=true;
            TemplateShape *shape = new TemplateShape(shapeTemplate);
            add(shape);
        }
        if(!oneAdded)
            add(new GroupShape(factory));
    }
}

void ShapeSelector::itemSelected() {
    KoShape *koShape = m_shapeManager->selection()->firstSelectedShape();
    if(koShape == 0)
        return;
    IconShape *shape= static_cast<IconShape*> (koShape);
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();

    if(canvasController) {
        KoCreateShapesTool * tool = KoToolManager::instance()->shapeCreatorTool( canvasController->canvas() );
        shape->visit( tool );
        KoToolManager::instance()->switchToolRequested(KoCreateShapesTool_ID);
    }
}

void ShapeSelector::add(KoShape *shape) {
    int x=5, y=5; // 5 = gap
    int w = (int) shape->size().width();
    bool ok=true; // lets be optimistic ;)
    do {
        int rowHeight=0;
        ok=true;
        foreach(const KoShape *shape, m_shapeManager->shapes()) {
            if(shape->position().y() > y || shape->position().y() + shape->size().height() < y)
                continue; // other row.
            rowHeight = qMax(rowHeight, qRound(shape->size().height()));
            x = qMax(x, qRound(shape->position().x() + shape->size().width()) + 5); // 5=gap
            if(x + w > width()) { // next row
                y += rowHeight + 5; // 5 = gap
                x = 5;
                ok=false;
                break;
            }
        }
    } while(! ok);
    shape->setPosition(QPointF(x, y));

    m_shapeManager->add(shape);
}

#include "ShapeSelector.moc"

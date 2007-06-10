/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_selection_shape_manager.h"

#include <QList>

#include <KoViewConverter.h>
#include <KoPathShape.h>
#include <KoLineBorder.h>

#include <kdebug.h>

KisSelectionShapeManager::KisSelectionShapeManager(KoCanvasBase *canvas)
    : super(canvas)
{
}

KisSelectionShapeManager::~KisSelectionShapeManager()
{
}

void KoShapeManager::add( KoShape *shape )
{
    KoPathShape* pathShape = dynamic_cast<KoPathShape*>( shape );
    if(pathShape) {
        pathShape->setBorder( new KoLineBorder( 1.0, Qt::lightGray ) );
        add( shape, true );
    }
}

void KisSelectionShapeManager::paintShapeSelection( QPainter &painter, const KoViewConverter &converter, int offset)
{
    paint(painter, converter, false);

    KoShape::applyConversion(painter, converter);

    QList<KoShape*> shapesList = shapes();

    QPainterPathStroker stroker;
    stroker.setWidth(0);

    QVector<qreal> dashes;
    qreal space = 4;
    dashes << 4 << space;

    stroker.setDashPattern(dashes);
    stroker.setDashOffset(offset-4);


    QPainterPath outline;
    KoPathShape* pathShape;
    foreach( KoShape * shape, shapesList )
    {
        pathShape = dynamic_cast<KoPathShape*>( shape );
        if(pathShape) {
            QMatrix matrix;
            matrix.translate(shape->position().x(), shape->position().y());

            outline = outline.united(matrix.map(shape->outline()));
        }
    }
    QPainterPath stroke = stroker.createStroke(outline);

    painter.setRenderHint(QPainter::Antialiasing);
    QColor outlineColor = Qt::blue;
    painter.fillPath(stroke, outlineColor);
}

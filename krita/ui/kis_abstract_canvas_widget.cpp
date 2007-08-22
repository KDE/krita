/*
 * Copyright (C) Adrian Page <adrian@pagenet.plus.com>, (C) 2007
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

#include <QImage>
#include <QPainter>

#include <KoShapeManager.h>
#include <KoViewConverter.h>
#include <KoToolProxy.h>

#include "kis_abstract_canvas_widget.h"
#include "kis_config.h"
#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_grid_drawer.h"
#include "kis_selection_manager.h"

void KisAbstractCanvasWidget::drawDecorations( QPainter & gc, bool ants, bool grids, bool tools,
                                               const QPoint & documentOffset,
                                               const QRect & clipRect,
                                               KisCanvas2 * canvas,
                                               KisGridDrawer * gridDrawer)
{
    // Setup the painter to take care of the offset; all that the
    // classes that do painting need to keep track of is resolution
    gc.setRenderHint( QPainter::Antialiasing );
    gc.setRenderHint( QPainter::SmoothPixmapTransform );
    gc.translate( QPoint( -documentOffset.x(), -documentOffset.y() ) );

    // Paint the shapes (other than the layers)
    gc.save();
    gc.setClipRect( clipRect );
    canvas->globalShapeManager()->paint( gc, *canvas->viewConverter(), false );
    gc.restore();

    //Paint marching ants and selection shapes
    if ( ants ) {
        gc.save();
        canvas->view()->selectionManager()->paint(gc, *canvas->viewConverter() );
        gc.restore();
    }

    // ask the guides, grids, etc to paint themselves
    if ( grids ) {
        gc.save();
        gridDrawer->draw(&gc, canvas->viewConverter()->viewToDocument(clipRect));
        gc.restore();
    }

    // Give the tool a chance to paint its stuff
    if ( tools ) {
        gc.save();
        toolProxy()->paint(gc, *canvas->viewConverter() );
        gc.restore();
    }

}

QImage KisAbstractCanvasWidget::checkImage(qint32 checkSize)
{
    KisConfig cfg;

    QImage tile(checkSize * 2, checkSize * 2, QImage::Format_RGB32);
    QPainter pt(&tile);
    pt.fillRect(tile.rect(), Qt::white);
    pt.fillRect(0, 0, checkSize, checkSize, cfg.checkersColor());
    pt.fillRect(checkSize, checkSize, checkSize, checkSize, cfg.checkersColor());
    pt.end();

    return tile;
}


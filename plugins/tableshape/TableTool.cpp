/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "TableTool.h"
#include "TableShape.h"

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>

TableTool::TableTool( KoCanvasBase* canvas )
    : KoTool( canvas ),
      m_tableshape(0)
{
}

void TableTool::activate (bool temporary)
{
    Q_UNUSED( temporary );

    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach ( KoShape* shape, selection->selectedShapes() )
    {
        m_tableshape = dynamic_cast<TableShape*>( shape );
        if ( m_tableshape )
            break;
    }
    if ( !m_tableshape )
    {
        emit done();
        return;
    }
    useCursor( Qt::ArrowCursor, true );
}

void TableTool::deactivate()
{
  m_tableshape = 0;
}

QWidget * TableTool::createOptionWidget()
{
    return 0;
}

void TableTool::slotChangeUrl()
{
}

#include "TableTool.moc"

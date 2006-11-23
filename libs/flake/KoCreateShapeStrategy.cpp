/* This file is part of the KDE project
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

#include "KoCreateShapeStrategy.h"
#include "KoCreateShapesTool.h"
#include "KoShapeControllerBase.h"
#include "KoShapeRegistry.h"
#include "KoShapeManager.h"
#include "KoCommand.h"
#include "KoCanvasBase.h"
#include "KoShapeConfigWidgetBase.h"
#include "KoShapeConfigFactory.h"
#include "KoShapeFactory.h"

#include <KoProperties.h>

#include <kpagedialog.h>
#include <klocale.h>

KoCreateShapeStrategy::KoCreateShapeStrategy( KoCreateShapesTool *tool, KoCanvasBase *canvas, const QPointF &clicked)
: KoShapeRubberSelectStrategy(tool, canvas, clicked)
{
}

KCommand* KoCreateShapeStrategy::createCommand() {
    KoCreateShapesTool *parent = static_cast<KoCreateShapesTool*>(m_parent);
    KoShapeFactory *factory = KoShapeRegistry::instance()->get(parent->shapeId());
    if(! factory) {
        kWarning(30001) << "Application requested a shape that is not registered '" <<
            parent->shapeId() << "'" << endl;
        return 0;
    }

    const KoProperties *props = parent->shapeProperties();
    KoShape *shape;
    if(props)
        shape = factory->createShape(props);
    else
        shape = factory->createDefaultShape();
    QRectF rect = selectRect();
    shape->setPosition(rect.topLeft());
    QSizeF newSize = rect.size();
    // if the user has dragged when creating the shape,
    // resize the shape to the dragged size
    if(newSize.width() > 1.0 && newSize.height() > 1.0) 
        shape->resize(newSize);

    KCommand * cmd = parent->m_canvas->shapeController()->addShape( shape );
    if ( cmd )
    {
        KoSelection *selection = parent->m_canvas->shapeManager()->selection();
        selection->deselectAll();
        selection->select(shape);

        cmd->execute();
    }
    return cmd;
}

void KoCreateShapeStrategy::finishInteraction( Qt::KeyboardModifiers modifiers ) {
    Q_UNUSED( modifiers );
    m_canvas->updateCanvas(selectRect());
}

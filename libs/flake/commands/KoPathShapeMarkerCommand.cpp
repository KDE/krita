/* This file is part of the KDE project
 * Copyright (C) 2010 Jeremy Lugagne <lugagne.jeremy@gmail.com>
 * Copyright (C) 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#include "KoPathShapeMarkerCommand.h"
#include "KoMarker.h"

#include "flake_export.h"

#include <klocale.h>

KoPathShapeMarkerCommand::KoPathShapeMarkerCommand(const QList<KoPathShape*> &shapes, KoMarker *marker, KoMarkerData::MarkerPosition position, KUndo2Command *parent)
: KUndo2Command(parent)
, m_shapes(shapes)
, m_marker(marker)
, m_position(position)
{
    setText(i18nc("(qtundo-format)", "Set marker"));

    // save old markers
    foreach(KoPathShape *shape, m_shapes) {
        m_oldMarkers.append(shape->marker(position));
    }
}

KoPathShapeMarkerCommand::~KoPathShapeMarkerCommand()
{
}

void KoPathShapeMarkerCommand::redo()
{
    KUndo2Command::redo();
    foreach(KoPathShape *shape, m_shapes) {
        shape->setMarker(m_marker, m_position);
        shape->update();
    }
}

void KoPathShapeMarkerCommand::undo()
{
    KUndo2Command::undo();
    QList<KoMarker*>::iterator markerIt = m_oldMarkers.begin();
    foreach(KoPathShape *shape, m_shapes) {
        shape->setMarker(*markerIt, m_position);
        shape->update();
        ++markerIt;
    }
}

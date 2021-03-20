/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Jeremy Lugagne <lugagne.jeremy@gmail.com>
 * SPDX-FileCopyrightText: 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoPathShapeMarkerCommand.h"
#include "KoMarker.h"
#include "KoPathShape.h"
#include <QExplicitlySharedDataPointer>

#include "kis_command_ids.h"

#include <klocalizedstring.h>

struct Q_DECL_HIDDEN KoPathShapeMarkerCommand::Private
{
    QList<KoPathShape*> shapes;  ///< the shapes to set marker for
    QList<QExplicitlySharedDataPointer<KoMarker>> oldMarkers; ///< the old markers, one for each shape
    QExplicitlySharedDataPointer<KoMarker> marker; ///< the new marker to set
    KoFlake::MarkerPosition position;
    QList<bool> oldAutoFillMarkers;
};

KoPathShapeMarkerCommand::KoPathShapeMarkerCommand(const QList<KoPathShape*> &shapes, KoMarker *marker, KoFlake::MarkerPosition position, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Set marker"), parent),
      m_d(new Private)
{
    m_d->shapes = shapes;
    m_d->marker = marker;
    m_d->position = position;

    // save old markers
    Q_FOREACH (KoPathShape *shape, m_d->shapes) {
        m_d->oldMarkers.append(QExplicitlySharedDataPointer<KoMarker>(shape->marker(position)));
        m_d->oldAutoFillMarkers.append(shape->autoFillMarkers());
    }
}

KoPathShapeMarkerCommand::~KoPathShapeMarkerCommand()
{
}

void KoPathShapeMarkerCommand::redo()
{
    KUndo2Command::redo();
    Q_FOREACH (KoPathShape *shape, m_d->shapes) {
        const QRectF oldDirtyRect = shape->boundingRect();
        shape->setMarker(m_d->marker.data(), m_d->position);

        // we have no GUI for selection auto-filling yet! So just enable it!
        shape->setAutoFillMarkers(true);

        shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
    }
}

void KoPathShapeMarkerCommand::undo()
{
    KUndo2Command::undo();
    auto markerIt = m_d->oldMarkers.begin();
    auto autoFillIt = m_d->oldAutoFillMarkers.begin();
    Q_FOREACH (KoPathShape *shape, m_d->shapes) {
        const QRectF oldDirtyRect = shape->boundingRect();
        shape->setMarker((*markerIt).data(), m_d->position);
        shape->setAutoFillMarkers(*autoFillIt);
        shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
        ++markerIt;
        ++autoFillIt;
    }
}

int KoPathShapeMarkerCommand::id() const
{
    return KisCommandUtils::ChangeShapeMarkersId;
}

bool KoPathShapeMarkerCommand::mergeWith(const KUndo2Command *command)
{
    const KoPathShapeMarkerCommand *other = dynamic_cast<const KoPathShapeMarkerCommand*>(command);

    if (!other ||
        other->m_d->shapes != m_d->shapes ||
        other->m_d->position != m_d->position) {

        return false;
    }

    m_d->marker = other->m_d->marker;
    return true;
}

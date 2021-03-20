/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Jeremy Lugagne <lugagne.jeremy@gmail.com>
 * SPDX-FileCopyrightText: 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KoPathShapeMarkerCommand_H
#define KoPathShapeMarkerCommand_H

#include "kritaflake_export.h"

#include <QScopedPointer>

#include "KoFlake.h"
#include <kundo2command.h>
#include <QList>

class KoPathShape;
class KoMarker;

/// The undo / redo command for setting the shape marker
class KRITAFLAKE_EXPORT KoPathShapeMarkerCommand : public KUndo2Command
{
public:
    /**
     * Command to set a new shape marker.
     * @param shapes a set of all the shapes that should get the new marker.
     * @param marker the new marker, the same for all given shapes
     * @param position the position - start or end - of the marker on the shape
     * @param parent the parent command used for macro commands
     */
    KoPathShapeMarkerCommand(const QList<KoPathShape*> &shapes, KoMarker *marker, KoFlake::MarkerPosition position, KUndo2Command *parent = 0);

    ~KoPathShapeMarkerCommand() override;

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KoPathShapeMarkerCommand_H

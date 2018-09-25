/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOMULTIPATHPOINTMERGECOMMAND_H
#define KOMULTIPATHPOINTMERGECOMMAND_H

#include <kundo2command.h>

#include "kritaflake_export.h"
#include <QScopedPointer>

class KoSelection;
class KoPathShape;
class KoPathPointData;
class KoShapeControllerBase;


class KRITAFLAKE_EXPORT KoMultiPathPointMergeCommand : public KUndo2Command
{
public:
    KoMultiPathPointMergeCommand(const KoPathPointData &pointData1,
                                 const KoPathPointData &pointData2,
                                 KoShapeControllerBase *controller,
                                 KoSelection *selection,
                                 KUndo2Command *parent = 0);
    ~KoMultiPathPointMergeCommand() override;

    void undo() override;
    void redo() override;

    KoPathShape *testingCombinedPath() const;

protected:
    virtual KUndo2Command *createMergeCommand(const KoPathPointData &pointData1,
                                              const KoPathPointData &pointData2);
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOMULTIPATHPOINTMERGECOMMAND_H

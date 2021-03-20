/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

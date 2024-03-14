/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOMERGETEXTPROPERTIESINTOSHAPESCOMMAND_H
#define KOMERGETEXTPROPERTIESINTOSHAPESCOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>

class KoShape;
class KoSvgTextProperties;

class KRITAFLAKE_EXPORT KoShapeMergeTextPropertiesCommand : public KUndo2Command
{
public:
    KoShapeMergeTextPropertiesCommand(const QList<KoShape*> &shapes, const KoSvgTextProperties &props, KUndo2Command *parent = nullptr);
    ~KoShapeMergeTextPropertiesCommand() = default;

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;
private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // KOMERGETEXTPROPERTIESINTOSHAPESCOMMAND_H

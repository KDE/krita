/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CHANGE_GUIDES_COMMAND_H
#define __KIS_CHANGE_GUIDES_COMMAND_H

#include <QScopedPointer>
#include <kundo2command.h>

class KisDocument;
class KUndo2Command;
class KisGuidesConfig;


class KisChangeGuidesCommand : public KUndo2Command
{
public:
    KisChangeGuidesCommand(KisDocument *doc, const KisGuidesConfig &oldGuides, const KisGuidesConfig &newGuides);
    ~KisChangeGuidesCommand() override;

    void undo() override;
    void redo() override;

    int id() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_CHANGE_GUIDES_COMMAND_H */

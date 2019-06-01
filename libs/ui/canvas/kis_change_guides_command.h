/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

/*
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
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

#ifndef KISCHANGEPALETTECOMMAND_H
#define KISCHANGEPALETTECOMMAND_H

#include <QString>

#include <kundo2command.h>
#include <kis_command_ids.h>

class KisChangePaletteCommand : public KUndo2Command
{
public:
    static const char MagicString[];
public:
    KisChangePaletteCommand();
    ~KisChangePaletteCommand() override;
    void undo() override
    {
        // palette modification shouldn't be undone;
    }
    void redo() override
    {
        // palette modification shouldn't be undone;
    }
    bool mergeWith(const KUndo2Command *) override
    {
        // palette modifications' merging doesn't need any action
        return true;
    }
    int id() const override;
};

#endif // KISCHANGEPALETTECOMMAND_H

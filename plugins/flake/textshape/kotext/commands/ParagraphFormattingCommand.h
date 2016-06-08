/* This file is part of the KDE project
 * Copyright (C) 2013 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#ifndef PARAGRAPHFORMATTINGCOMMAND_H
#define PARAGRAPHFORMATTINGCOMMAND_H

#include <KoListLevelProperties.h>

#include "kundo2command.h"

class KoTextEditor;
class QTextCharFormat;
class QTextBlockFormat;

/**
 * This command is used to apply paragraph settings
 */
class ParagraphFormattingCommand : public KUndo2Command
{
public:
    ParagraphFormattingCommand(KoTextEditor *editor,
                               const QTextCharFormat &characterFormat,
                               const QTextBlockFormat &blockFormat,
                               const KoListLevelProperties &llp,
                               KUndo2Command *parent = 0);

    ~ParagraphFormattingCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    bool m_first;
    KoTextEditor *m_editor;
    QTextCharFormat m_charFormat;
    QTextBlockFormat m_blockFormat;
    KoListLevelProperties m_levelProperties;
};

#endif


/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef REPLACETEXTRANGECOMMAND_H
#define REPLACETEXTRANGECOMMAND_H

#include <kundo2command.h>
#include "ArtisticTextTool.h"
#include "ArtisticTextRange.h"
#include <QPointer>

class ArtisticTextShape;

/// Undo command to replace a range of text on an artistic text shape
class ReplaceTextRangeCommand : public KUndo2Command
{
public:
    ReplaceTextRangeCommand(ArtisticTextShape *shape, const QString &text, int from, int count, ArtisticTextTool *tool, KUndo2Command *parent = 0);
    ReplaceTextRangeCommand(ArtisticTextShape *shape, const ArtisticTextRange &text, int from, int count, ArtisticTextTool *tool, KUndo2Command *parent = 0);
    ReplaceTextRangeCommand(ArtisticTextShape *shape, const QList<ArtisticTextRange> &text, int from, int count, ArtisticTextTool *tool, KUndo2Command *parent = 0);

    virtual void redo();
    virtual void undo();

private:
    QPointer<ArtisticTextTool> m_tool;
    ArtisticTextShape *m_shape;
    QList<ArtisticTextRange> m_newFormattedText;
    QList<ArtisticTextRange> m_oldFormattedText;
    int m_from;
    int m_count;
};

#endif // REPLACETEXTRANGECOMMAND_H

/* This file is part of the KDE project
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#ifndef REMOVETEXTRANGECOMMAND_H
#define REMOVETEXTRANGECOMMAND_H

#include <kundo2command.h>
#include <QPointer>
#include "ArtisticTextTool.h"

class ArtisticTextShape;

/// Undo command to remove a range of text from an artistic text shape
class RemoveTextRangeCommand : public KUndo2Command
{
public:
    RemoveTextRangeCommand(ArtisticTextTool *tool, ArtisticTextShape *shape, int from, unsigned int count);

    virtual void redo();
    virtual void undo();

private:
    QPointer<ArtisticTextTool> m_tool;
    ArtisticTextShape *m_shape;
    int m_from;
    int m_count;
    QList<ArtisticTextRange> m_text;
    int m_cursor;
};

#endif // REMOVETEXTRANGECOMMAND_H


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

#ifndef ADDTEXTRANGECOMMAND_H
#define ADDTEXTRANGECOMMAND_H

#include <kundo2command.h>
#include "ArtisticTextTool.h"
#include "ArtisticTextRange.h"
#include <QPointer>

class ArtisticTextShape;

/// Undo command to add a range of text to a artistic text shape
class AddTextRangeCommand : public KUndo2Command
{
public:
    AddTextRangeCommand(ArtisticTextTool *tool, ArtisticTextShape *shape, const QString &text, int from);
    AddTextRangeCommand(ArtisticTextTool *tool, ArtisticTextShape *shape, const ArtisticTextRange &text, int from);

    virtual void redo();
    virtual void undo();

private:
    QPointer<ArtisticTextTool> m_tool;
    ArtisticTextShape *m_shape;
    QString m_plainText;
    ArtisticTextRange m_formattedText;
    QList<ArtisticTextRange> m_oldFormattedText;
    int m_from;
};

#endif // ADDTEXTRANGECOMMAND_H

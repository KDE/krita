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

#ifndef CHANGETEXTFONTCOMMAND_H
#define CHANGETEXTFONTCOMMAND_H

#include "ArtisticTextRange.h"
#include <kundo2command.h>
#include <QFont>

class ArtisticTextShape;

class ChangeTextFontCommand : public KUndo2Command
{
public:
    ChangeTextFontCommand(ArtisticTextShape *shape, const QFont &font, KUndo2Command *parent = 0);
    ChangeTextFontCommand(ArtisticTextShape *shape, int from, int count, const QFont &font, KUndo2Command *parent = 0);
    virtual void undo();
    virtual void redo();

private:
    ArtisticTextShape *m_shape;
    QFont m_newFont;
    QList<ArtisticTextRange> m_oldText;
    QList<ArtisticTextRange> m_newText;
    int m_rangeStart;
    int m_rangeCount;
};

#endif // CHANGETEXTFONTCOMMAND_H

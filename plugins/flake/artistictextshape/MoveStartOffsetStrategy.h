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

#ifndef MOVESTARTOFFSETSTRATEGY_H
#define MOVESTARTOFFSETSTRATEGY_H

#include <KoInteractionStrategy.h>
#include <QList>

class KoPathShape;
class ArtisticTextShape;
class KoToolBase;

/// A strategy to change the offset of a text when put on a path
class MoveStartOffsetStrategy : public KoInteractionStrategy
{
public:
    MoveStartOffsetStrategy(KoToolBase *tool, ArtisticTextShape *text);
    ~MoveStartOffsetStrategy() override;

    // reimplemented from KoInteractionStrategy
    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    // reimplemented from KoInteractionStrategy
    KUndo2Command *createCommand() override;
    // reimplemented from KoInteractionStrategy
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
private:
    ArtisticTextShape *m_text;      ///< the text shape we are working on
    KoPathShape *m_baselineShape;   ///< path shape the text is put on
    qreal m_oldStartOffset;         ///< the initial start offset
    QList<qreal> m_segmentLengths;  ///< cached lengths of baseline path segments
    qreal m_totalLength;            ///< total length of baseline path
};

#endif // MOVESTARTOFFSETSTRATEGY_H

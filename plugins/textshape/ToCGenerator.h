/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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
#ifndef TOCGENERATOR_H
#define TOCGENERATOR_H

#include <QObject>
#include <QList>
#include <QTextBlock>

typedef QPair<QTextBlock, QTextBlock> BlockPair;

class QTextFrame;
class TableOfContent;

class ToCGenerator : public QObject
{
    Q_OBJECT
public:
    explicit ToCGenerator(QTextFrame *tocFrame);
    virtual ~ToCGenerator();
    
    QTextFrame *tocFrame() const { return m_ToCFrame; }

    // TODO API to be called when the shape is printed so we can guarentee
    // the TOC is up-to-date on printing time.

public slots:
    void documentLayoutFinished();
    // TODO a slot to connect the parent QTextDocument changed signal to so we
    // know when the TOC is dirty.

private:
    void generate();
    void update();

    enum State {
        DirtyState,
        NeverGeneratedState,
        WithoutPageNumbersState,
        GeneratedState
    };

    State m_state;
    QTextFrame *m_ToCFrame;
    TableOfContent * m_tocDescription;
    //QList<QTextBlock> m_originalBlocksInToc;
    QList<BlockPair> m_originalBlocksInToc;
};

#endif

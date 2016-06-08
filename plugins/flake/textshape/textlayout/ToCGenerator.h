/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Pavol Korinek <pavol.korinek@ixonos.com>
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 * Copyright (C) 2011 Ko GmbH <cbo@kogmbh.com>
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

#include <QTextBlock>
#include <QObject>

class KoTextRangeManager;
class KoTextDocumentLayout;
class KoTableOfContentsGeneratorInfo;

class QTextDocument;

class ToCGenerator : public QObject
{
    Q_OBJECT
public:
    explicit ToCGenerator(QTextDocument *tocDocument, KoTableOfContentsGeneratorInfo *tocInfo);
    virtual ~ToCGenerator();

    virtual void setBlock(const QTextBlock &block);

    bool generate();

private:
    QString resolvePageNumber(const QTextBlock &headingBlock);
    void generateEntry(int outlineLevel, QTextCursor &cursor, QTextBlock &block, int &blockId);

    QTextDocument *m_ToCDocument;
    KoTableOfContentsGeneratorInfo *m_ToCInfo;
    QTextBlock m_block;
    QTextDocument *m_document;
    KoTextDocumentLayout *m_documentLayout;
    bool m_success;
    bool m_preservePagebreak;

    // Return the ref (name) of the first KoBookmark in the block, if KoBookmark not found, null QString is returned
    QString fetchBookmarkRef(const QTextBlock &block, KoTextRangeManager *textRangeManager);
};

#endif

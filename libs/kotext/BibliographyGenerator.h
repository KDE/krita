/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>

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
#ifndef BIBLIOGRAPHYGENERATOR_H
#define BIBLIOGRAPHYGENERATOR_H

#include <KoBibliographyInfo.h>
#include "kotext_export.h"

#include <QAbstractTextDocumentLayout>
#include <QTextBlock>

class KoInlineTextObjectManager;
class KoTextDocumentLayout;
class QTextFrame;

class KOTEXT_EXPORT BibliographyGenerator : public QObject, public BibliographyGeneratorInterface
{
    Q_OBJECT
public:
    explicit BibliographyGenerator(QTextDocument *bibDocument, QTextBlock block, KoBibliographyInfo *bibInfo);
    virtual ~BibliographyGenerator();


public slots:
    void generate();

private:
    //QString resolvePageNumber(const QTextBlock &headingBlock);
    void generateEntry(QString bibType, QTextCursor &cursor, QTextBlock block, int &blockId);

    QTextDocument *m_document;
    QTextDocument *m_bibDocument;
    KoBibliographyInfo *m_bibInfo;
    QTextBlock m_block;
    qreal m_maxTabPosition;
};

#endif

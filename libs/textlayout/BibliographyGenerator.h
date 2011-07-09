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

#include <QObject>
#include <QList>
#include <QTextBlock>

#include <KoBibliographyInfo.h>
class KoInlineTextObjectManager;

class QTextFrame;

class BibliographyGenerator : public QObject
{
    Q_OBJECT
public:
    explicit BibliographyGenerator(QTextFrame *bibFrame, KoBibliographyInfo *bibInfo);
    virtual ~BibliographyGenerator();


public slots:
    void generate();

private:
    //QString resolvePageNumber(const QTextBlock &headingBlock);

    QTextFrame *m_bibFrame;
    KoBibliographyInfo *m_bibInfo;

    // Return the ref (name) of the first KoBookmark in the block, if KoBookmark not found, null QString is returned
    QString fetchBookmarkRef(QTextBlock block, KoInlineTextObjectManager * inlineTextObjectManager);
};

#endif

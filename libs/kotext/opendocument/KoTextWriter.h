/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#ifndef KOTEXTWRITER_H
#define KOTEXTWRITER_H

#include "kotext_export.h"

#ifdef CHANGETRK
 class KoTextShapeSavingContext;
#else
 class KoShapeSavingContext;
#endif
class KoXmlWriter;
class KoTextDocumentLayout;
class KoStyleManager;
class QTextDocument;
#ifdef CHANGETRK
 class KoChangeTracker;
#endif

#include <QTextBlock>

/**
 * KoTextWriter saves the text ODF of a shape
 */
class KoTextWriter
{
public:
    /**
    * Constructor.
    *
    * @param context The context the KoTextWriter is called in
    */
#ifdef CHANGETRK
    explicit KoTextWriter(KoTextShapeSavingContext &context);
#else
    explicit KoTextWriter(KoShapeSavingContext &context);
#endif

    /**
    * Destructor.
    */
    ~KoTextWriter();

    /**
     * Writes the portion of document contained within 'from' and 'to'
     */
    void write(QTextDocument *document, int from, int to = -1);

private:
    QString saveParagraphStyle(const QTextBlock &block);
    QString saveCharacterStyle(const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat);
    QHash<QTextList *, QString> saveListStyles(QTextBlock block, int to);
    void saveParagraph(const QTextBlock &block, int from, int to);

#ifdef CHANGETRK
    KoTextShapeSavingContext &m_context;
#else
    KoShapeSavingContext &m_context;
#endif
    KoXmlWriter *m_writer;
    KoTextDocumentLayout *m_layout;
    KoStyleManager *m_styleManager;
#ifdef CHANGETRK
    KoChangeTracker *m_changeTracker;
#endif
};

#endif

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

#include "kritatext_export.h"

class KoShapeSavingContext;
class KoStyleManager;
class QTextDocument;
class QTextBlock;
class QTextBlockFormat;
class QTextCharFormat;
class QString;

class KoDocumentRdfBase;


/**
 * KoTextWriter saves the text ODF of a shape
 */
class KRITATEXT_EXPORT KoTextWriter
{
public:
    /**
    * Constructor.
    *
    * @param context The context the KoTextWriter is called in
    * @param rdfData The RDF data
    */
    explicit KoTextWriter(KoShapeSavingContext &context, KoDocumentRdfBase *rdfData = 0);

    /**
    * Destructor.
    */
    ~KoTextWriter();

    /// XXX: APIDOX!
    static void saveOdf(KoShapeSavingContext &context, KoDocumentRdfBase *rdfData, QTextDocument *document, int from, int to);

    /**
     * Save a paragraph style used in a text block
     *
     * This checks if the style is a document style or a automatic style
     * and saves it accordingly.
     *
     * @param block The block form which the style information are taken
     * @param styleManager The used style manager
     * @param context The saving context
     */
    static QString saveParagraphStyle(const QTextBlock &block, KoStyleManager *styleManager, KoShapeSavingContext &context);

    static QString saveParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &charFormat, KoStyleManager *styleManager, KoShapeSavingContext &context);

    /**
     * Writes the portion of document contained within 'from' and 'to'
     *
     * @param document The text document we are saving. There can be more than one
     * text document in the office document, but we don't care
     * @param from the start position in characters from which we save
     * @param to the end position in characters up to which we save. If -1, we save to the end
     */
    void write(const QTextDocument *document, int from, int to = -1);

private:
    class Private;
    Private* const d;
};

#endif

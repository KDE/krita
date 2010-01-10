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

class KoShapeSavingContext;
class KoXmlWriter;
class KoTextDocumentLayout;
class KoStyleManager;
class QTextDocument;
class QTextTable;

class KoChangeTracker;

#include <QTextBlock>

/**
 * KoTextWriter saves the text ODF of a shape
 */
class KOTEXT_EXPORT KoTextWriter
{
public:
    /**
    * Constructor.
    *
    * @param context The context the KoTextWriter is called in
    */
    explicit KoTextWriter(KoShapeSavingContext &context);

    /**
    * Destructor.
    */
    ~KoTextWriter();

    /**
     * Writes the portion of document contained within 'from' and 'to'
     */
    void write(QTextDocument *document, int from, int to = -1);

private:
    class Private;
    Private* const d;
};

#endif

/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2001-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
 * Copyright (C) 2011 Pavol Korinek <pavol.korinek@ixonos.com>
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
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

#ifndef KOTEXTLOADER_H
#define KOTEXTLOADER_H

#include <QObject>

#include "kritatext_export.h"
#include <KoXmlReaderForward.h>

class QTextCursor;
class QTextTable;
class QRect;

class KoShapeLoadingContext;
class KoShape;

/**
 * The KoTextLoader loads is use to load text for one and only one textdocument or shape
 * to the scribe text-engine using QTextCursor objects. So if you have two shapes 2 different
 * KoTextLoader are used for that. Also if you have a frame with text inside a text a new
 * KoTextLoader is used.
 *
 * If you want to use the KoTextLoader for text that needs styles from styles.xml. e.g.
 * test shapes on master pages, you need to set KoOdfLoadingContext::setUseStylesAutoStyles( true ).
 *
 * Don't forget to unset it if you later want to load text that needs content.xml.
 */
class KRITATEXT_EXPORT KoTextLoader : public QObject
{
    Q_OBJECT
public:

    /**
    * Normalizes the whitespaces in the string \p in according to the ODF ruleset
    * for stripping whitespaces and returns the result. If \p leadingSpace is
    * true a leading space is stripped too.
    *
    * This is different from QString::simplifyWhiteSpace() because that one removes
    * leading and trailing whitespace, but such whitespace is significant in ODF.
    * So we use this function to compress sequences of space characters into single
    * spaces.
    */
    static QString normalizeWhitespace(const QString &in, bool leadingSpace);

    /**
    * Constructor.
    * Notice that despite this being a QObject there is no 'parent' available for
    * memory management here.
    *
    * @param context The context the KoTextLoader is called in
    */
    explicit KoTextLoader(KoShapeLoadingContext &context, KoShape *shape = 0);

    /**
    * Destructor.
    */
    ~KoTextLoader();

    enum LoadBodyMode
    {
        LoadMode,
        PasteMode
    };
    /**
    * Load the body from the \p element into the \p cursor .
    *
    * This method got called e.g. at the \a KoTextShapeData::loadOdf() method if a TextShape
    * instance likes to load an ODF element.
    *
    * @param element the element to start loading at
    * @param cursor the text cursor to insert the body after
    * @param pasteMode does special handling of section needed, in case we are pasting text
    */
    void loadBody(const KoXmlElement &element, QTextCursor &cursor, LoadBodyMode pasteMode = LoadMode);

Q_SIGNALS:

    /**
    * This signal is emitted during loading with a percentage within 1-100 range
    * \param percent the progress as a percentage
    */
    void sigProgress(int percent);

private:

    /**
    * Load the paragraph from the \p element into the \p cursor .
    */
    void loadParagraph(const KoXmlElement &element, QTextCursor &cursor);

    /**
    * Load the heading from the \p element into the \p cursor .
    */
    void loadHeading(const KoXmlElement &element, QTextCursor &cursor);

    /**
    * Load the list from the \p element into the \p cursor .
    */
    void loadList(const KoXmlElement &element, QTextCursor &cursor);

    /**
    * Load a list-item into the cursor
    */
    void loadListItem(const KoXmlElement &e, QTextCursor &cursor, int level);

    /**
    * Load the section from the \p element into the \p cursor .
    */
    void loadSection(const KoXmlElement &element, QTextCursor &cursor);

    /**
    * Load the text into the \p cursor .
    */
    void loadText(const QString &text, QTextCursor &cursor,
                  bool *stripLeadingSpace, bool isLastNode);

    /**
    * Load the span from the \p element into the \p cursor .
    */
    void loadSpan(const KoXmlElement &element, QTextCursor &cursor, bool *leadingSpace);

    /**
     * Load the table from the \p element into the \p cursor.
     *
     * The table and its contents are placed in a new shape.
     */
    void loadTable(const KoXmlElement &element, QTextCursor& cursor);

    /**
     * Loads a table column
     */
    void loadTableColumn(const KoXmlElement &element, QTextTable *table, int &columns);

    /**
     * Loads a table-row into the cursor
     */
    void loadTableRow(const KoXmlElement &element, QTextTable *table, QList<QRect> &spanStore, QTextCursor &cursor, int &rows);

    /**
     * Loads a table-cell into the cursor
     */
    void loadTableCell(const KoXmlElement &element, QTextTable *table, QList<QRect> &spanStore, QTextCursor &cursor, int &currentCell);

    /**
     * Load a note \p element into the \p cursor.
     */
    void loadNote(const KoXmlElement &element, QTextCursor& cursor);

    /**
     * Load a citation \p element into the \p cursor.
     */
    void loadCite(const KoXmlElement &element, QTextCursor& cursor);

    /**
    * Load the shape element and assign hyperlink to it \p element into the \p cursor .
    */
    void loadShapeWithHyperLink(const KoXmlElement &element, QTextCursor& cursor);

    /**
    * Load the shape element \p element into the \p cursor .
    */
    KoShape *loadShape(const KoXmlElement &element, QTextCursor& cursor);

    /**
    * Load the table of content element \p element into the \p cursor .
    */
    void loadTableOfContents(const KoXmlElement &element, QTextCursor& cursor);

    /**
    * Load the bibliography element \p element into the \p cursor .
    */
    void loadBibliography(const KoXmlElement &element, QTextCursor& cursor);

    /**
    * This is called in loadBody before reading the body starts.
    */
    void startBody(int total);

    /**
    * This is called in loadBody on each item that is read within the body.
    */
    void processBody();

    /**
    * This is called in loadBody once the body was read.
    */
    void endBody();

    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif

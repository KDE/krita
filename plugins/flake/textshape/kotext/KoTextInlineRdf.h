/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KO_TEXT_INLINE_RDF_H
#define KO_TEXT_INLINE_RDF_H

#include "kritatext_export.h"
// komain
#include <KoXmlReaderForward.h>
#include <KoElementReference.h>
// Qt
#include <QPair>
#include <QMetaType>
#include <QObject>

class KoXmlWriter;
class KoShapeSavingContext;
class KoBookmark;
class KoAnnotation;
class KoTextMeta;
class KoTextEditor;
class KoSection;

class QTextDocument;
class QTextCursor;
class QTextFormat;
class QTextBlock;
class QTextTableCell;

/**
 * @short Store information from xhtml:property etc which are for inline Rdf
 *
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see KoDocumentRdf
 *
 * The easiest way to handle inline Rdf from content.xml is to attach these
 * objects to the document's C++ objects. As you can see from the constructors
 * there are methods which can attach to bookmarks, textmeta, table cells etc.
 *
 * The main reason why the inlineRdf wants these document objects
 * passed in is so that object() can work out what the current value
 * is from the document. For example, when a KoTextInlineRdf is
 * attached to a bookmark-start, then when object() is called the
 * bookmark is inspected to find out the value currently between
 * bookmark-start and bookmark-end.
 *
 * The xmlId() method returns the xml:id that was associated with the
 * inline Rdf if there was one. For example,
 * <bookmark-start xml:id="foo" xhtml:property="uri:baba" ...>
 * the KoTextInlineRdf object will be attached to the KoBookmark
 * for the bookmark-start location and xmlId() will return foo.
 *
 * You can convert one of these to a Soprano::Statement using
 * KoDocumentRdf::toStatement().
 *
 * The attach() and tryToGetInlineRdf() are used by the ODF load and
 * save codepaths respectively. They associate an inlineRdf object
 * with the cursor and fetch back the inline Rdf if one is associated
 * with a text block.
 *
 * FIXME: createXmlId() should consult with the Calligra codebase when
 * generating new xml:id values during save.
 */
class KRITATEXT_EXPORT KoTextInlineRdf : public QObject
{
    Q_OBJECT

public:
    KoTextInlineRdf(const QTextDocument *doc, const QTextBlock &b);
    KoTextInlineRdf(const QTextDocument *doc, KoBookmark *b);
    KoTextInlineRdf(const QTextDocument *doc, KoAnnotation *b);
    KoTextInlineRdf(const QTextDocument *doc, KoTextMeta *b);
    KoTextInlineRdf(const QTextDocument *doc, const QTextTableCell &b);
    KoTextInlineRdf(const QTextDocument *doc, KoSection *s);

    virtual ~KoTextInlineRdf();

    /**
     * The attach() and tryToGetInlineRdf() are used by the ODF load and
     * save codepaths respectively. They associate an inlineRdf object
     * with the cursor and fetch back the inline Rdf if one is associated
     * with a text block.
     */
    static KoTextInlineRdf *tryToGetInlineRdf(QTextCursor &cursor);
    static KoTextInlineRdf *tryToGetInlineRdf(const QTextFormat &tf);
    static KoTextInlineRdf *tryToGetInlineRdf(KoTextEditor *handler);
    /**
     * The attach() and tryToGetInlineRdf() are used by the ODF load and
     * save codepaths respectively. They associate an inlineRdf object
     * with the cursor and fetch back the inline Rdf if one is associated
     * with a text block.
     */
    static void attach(KoTextInlineRdf *inlineRdf, QTextCursor &cursor);

    bool loadOdf(const KoXmlElement &element);
    bool saveOdf(KoShapeSavingContext &context, KoXmlWriter *writer, KoElementReference id = KoElementReference()) const;

    /**
     * Get the RDF subject for this inline RDF
     */
    QString subject() const;
    /**
     * Get the RDF predicate for this inline RDF
     */
    QString predicate() const;
    /**
     * Get the RDF object for this inline RDF
     */
    QString object() const;
    /**
     * Get the type of RDF node (bnode, literal, uri etc) for this inline RDF
     */
    int sopranoObjectType() const;

    /**
     * Because RDF is linked to the xml id attribute of elements in
     * content.xml the xml:id attribute that was read from the
     * content.xml file is available here
     */
    QString xmlId() const;

    /**
     * Find the start and end position of this inline RDF object in the
     * document.
     */
    QPair<int, int> findExtent() const;


    /**
     * Update the xml:id, using during cut and paste as well as document save.
     */
    void setXmlId(const QString &id);

    /**
     * Create a new and unique xml:id
     */
    static QString createXmlId();

private:

    friend class KoRdfSemanticItem;
    friend class KoDocumentRdf;

    class Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KoTextInlineRdf*)
#endif

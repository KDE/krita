/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
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
#ifndef KOTEXTRANGE_H
#define KOTEXTRANGE_H

#include "kritatext_export.h"

#include <QObject>
#include <KoXmlReaderForward.h>

class QTextDocument;
class QTextCursor;

class KoTextRangeManager;
class KoTextRangePrivate;
class KoShapeSavingContext;
class KoTextInlineRdf;
class KoShapeLoadingContext;

/**
 * Base class for all text ranges.
 *
 * They are essentially anchored to a specific position or range in the text
 *
 * @see KoTextRangeManager
 */
class KRITATEXT_EXPORT KoTextRange : public QObject
{
    Q_OBJECT
public:
    enum TagType {StartTag = 0, EndTag = 1};

    /**
     * constructor
     */
    explicit KoTextRange(const QTextCursor &cursor);
    virtual ~KoTextRange();

    /**
     * Will be called by the manager when this variable is added.
     * Remember that inheriting classes should not use the manager() in the constructor, since it will be 0
     * @param manager the object manager for this object.
     */
    void setManager(KoTextRangeManager *manager);

    /**
     * Return the object manager set on this inline object.
     */
    KoTextRangeManager *manager() const;

     /**
     * Return the textdocument the range points to.
     */
   QTextDocument *document() const;

    /**
     * Save the part of this text range corresponding to position as ODF
     * This may save a beginning tag, ending tag, or nothing at all
     * @param context the context for saving.
     * @param position a position in the qtextdocument we are currently saving for.
     * @param tagType the type of tag we are interested in
     */
    virtual void saveOdf(KoShapeSavingContext &context, int position, TagType tagType) const = 0;

    bool positionOnlyMode() const;
    void setPositionOnlyMode(bool m);

    bool hasRange() const;
    int rangeStart() const;
    int rangeEnd() const;

    void setRangeStart(int position);
    void setRangeEnd(int position);

    QString text() const;

    /**
     * A text range might have some Rdf metadata associated with it
     * in content.xml
     * Ownership of the rdf object is taken by the text range, and you should not
     * delete it.
     */
    void setInlineRdf(KoTextInlineRdf *rdf);

    /**
     * Get any Rdf which was stored in content.xml for this text range
     */
    KoTextInlineRdf *inlineRdf() const;

    /**
     * Load a variable from odf.
     *
     * @param element element which represents the shape in odf
     * @param context the KoShapeLoadingContext used for loading
     *
     * @return false if loading failed
     */
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context) =  0;

    void snapshot();
    void restore();

protected:
    KoTextRangePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KoTextRange)
};

KRITATEXT_EXPORT QDebug operator<<(QDebug dbg, const KoTextRange *o);

#endif

/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTSHAPEDATA_H
#define KOTEXTSHAPEDATA_H

#include <KoShapeUserData.h>
#include <KoInsets.h>
#include <kotext_export.h>

class QTextDocument;
class QTextCursor;
class KoXmlWriter;

/**
 * The data store that is held by each KoTextShape instance.
 * This is a separe object to allow KWord proper to use this class' api and
 * access the internals of the text shape.
 * This class holds a Document pointer and is build so multiple shapes (and thus
 * multiple instances of this shape data) can share one document by providing a
 * different view on (a different part of) the document.
 */
class KOTEXT_EXPORT KoTextShapeData : public KoShapeUserData {
    Q_OBJECT
public:
    /// constructor
    KoTextShapeData();
    ~KoTextShapeData();

    /**
     * Replace the QTextDocument this shape will render.
     * @param document the new document. If there was an old document owned, it will be deleted.
     * @param transferOwnership if true then the document will be considered the responsibility
     *    of this data and the doc will be deleted when this shapeData dies.
     */
    void setDocument(QTextDocument *document, bool transferOwnership = true);
    /// return the document
    QTextDocument *document();

    /**
     * return the amount of points into the document (y) this shape will display.
     */
    double documentOffset() const;
    /**
     * Set the amount of points into the document (y direction) that is relevant for this
     * data-shape.  This allows multiple shapes to all use one document at different offsets
     * into the document.
     */
    void setDocumentOffset(double offset);

    /**
     * Return the position in the text-document that this shape shows.
     * It returns -1 if this shape contains no text.
     * Note that the text needs to be layouted separately for this to be updated.
     */
    int position() const;
    /**
     * Set the position in the text-document that this shape shows, or -1 if there is no text.
     * This is set by the text-layout engine.
     * @param position the new position
     */
    void setPosition(int position);
    /**
     * Return the end-position in the text-document that this shape shows.
     * It returns -1 if this shape contains no text.
     * Note that the text needs to be layouted separately for this to be updated.
     */
    int endPosition() const;
    /**
     * Set the end-position in the text-document that this shape shows, or -1 if there is no text.
     * This is set by the text-layout engine.
     * @param position the new position
     */
    void setEndPosition(int position);

    /// mark shape as dirty triggering a re-layout of its text.
    void faul();
    /// mark shape as not-dirty
    void wipe();
    /// return if the shape is marked dirty and its text content needs to be relayout
    bool isDirty() const;

    /// emits a relayout
    void fireResizeEvent();

    /**
     * Set the margins that will make the shapes text area smaller.
     * The shape that owns this textShapeData object will layout text in an area
     * confined by the shape size made smaller by the margins set here.
     * @param margins the margins that shrink the text area.
     */
    void setShapeMargins(const KoInsets &margins);
    /// returns the currently set margins for the shape.
    KoInsets shapeMargins() const;

    void setPageNumber(int page);
    int pageNumber() const;

    void saveOdf(KoXmlWriter *writer, int from = 0, int to = -1) const;

signals:
    /**
     * emitted when the shape thinks it should be relayouted, for example after
     * it has been resized.
     * Note that this event is compressed internally to avoid duplicate layout
     * requests.
     */
    void relayout();

private:
    class Private;
    Private *const d;
};

#endif

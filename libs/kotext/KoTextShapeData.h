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

#include <koffice_export.h>

class QTextDocument;
class QTextCursor;

class KOTEXT_EXPORT KoTextShapeData : public KoShapeUserData {
    Q_OBJECT
public:
    KoTextShapeData();
    ~KoTextShapeData();

    /**
     * Replace the QTextDocument this shape will render.
     * @param document the new document. If there was an old document owned, it will be deleted.
     * @param transferOwnership if true then the document will be considered the responsibility
     *    of this data and the doc will be deleted when it dies.
     */
    void setDocument(QTextDocument *document, bool transferOwnership = true);
    /// return the document
    QTextDocument *document();

    void setTextCursor(QTextCursor *textCursor) { m_textCursor = textCursor; }
    QTextCursor *textCursor() const { return m_textCursor; }

    /**
     * return the amount of points into the document (y) this shape will display.
     */
    double documentOffset() const { return m_offset; }
    /**
     * Set the amount of points into the document (y direction) that is relevant for this
     * datas-shape.  This allows multiple shapes to all use one document at different offsets
     * into the document.
     */
    void setDocumentOffset(double offset) { m_offset = offset; }

    int position() const { return m_position; }
    void setPosition(int position) { m_position = position; }
    int endPosition() const { return m_endPosition; }
    void setEndPosition(int position) { m_endPosition = position; }

    void faul() { m_dirty = true; }
    void wipe() { m_dirty = false; }
    bool isDirty() const { return m_dirty; }

signals:
    void relayout();

private:
    friend class KoTextShape;
    void fireResizeEvent() { emit relayout(); }

private:
    QTextDocument *m_document;
    bool m_ownsDocument, m_dirty;
    QTextCursor *m_textCursor;
    double m_offset;
    int m_position, m_endPosition;
};

#endif

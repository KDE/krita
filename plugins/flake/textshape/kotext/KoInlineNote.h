/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KOINLINENOTE_H
#define KOINLINENOTE_H

#include "KoInlineObject.h"
#include "kritatext_export.h"

class QTextFrame;

/**
 * This object is an inline object, which means it is anchored in the text-flow and it can hold note info.
 * Typical notes that use this are Footnotes, Endnotes and Annotations (also known as comments).
 */
class KRITATEXT_EXPORT KoInlineNote : public KoInlineObject
{
    Q_OBJECT
public:
    /// The type of note specifies how the application will use the text from the note.
    enum Type {
        Footnote,      ///< Notes of this type will have their text placed at the bottom of a shape.
        Endnote,       ///< Notes of this type are used as endnotes in applications that support it.
        Annotation     ///< Notes of this type will have their text placed in the document margin.
    };

    /**
     * Construct a new note to be inserted in the text using KoTextEditor::insertInlineObject() for example.
     * @param type the type of note, which specifies how the application will use the text from the new note.
     */
    explicit KoInlineNote(Type type);
    // destructor
    virtual ~KoInlineNote();

    /**
     * Set the textframe where we will create our own textframe within
     * Our textframe is the one containing the real note contents.
     * @param text the new text
     */
    void setMotherFrame(QTextFrame *text);

    /**
     * Set the label that is shown at the spot this inline note is inserted.
     * @param text the new label
     */
    void setLabel(const QString &text);

    /**
     * Indirectly set the label that is shown at the spot this inline note is inserted.
     * @param autoNumber the number that the label will portray. 0 should be the first
     */
    void setAutoNumber(int autoNumber);

    /// return the current text
    QTextFrame *textFrame() const;

    /// return the current label
    QString label() const;

    /**
     * @return whether the label should be automatically recreated or if the label is static.
     */
    bool autoNumbering() const;

    /**
     * Set whether the label should be automatically recreated.
     * @param on if true then changes in footnote-ordering will recalcualte the label.
     */
    void setAutoNumbering(bool on);

    /// return the type of note.
    Type type() const;

    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    ///reimplemented
    void saveOdf(KoShapeSavingContext &context);

    int getPosInDocument() const;

protected:
    /// reimplemented
    virtual void updatePosition(const QTextDocument *document,
                                int posInDocument, const QTextCharFormat &format);
    /// reimplemented
    virtual void resize(const QTextDocument *document, QTextInlineObject &object,
                        int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented
    virtual void paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
                       const QRectF &rect, const QTextInlineObject &object, int posInDocument, const QTextCharFormat &format);

private:
    friend class InsertNoteCommand;

    // only to be used on subsequent redo of insertion
    void setTextFrame(QTextFrame *textFrame);

    class Private;
    Private * const d;
};

#endif

/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTSHAPE_H
#define KOTEXTSHAPE_H

#include <KoShapeContainer.h>
#include <KoTextShapeData.h>

#include <QTextDocument>
#include <QPainter>
#include <QMutex>
#include <QWaitCondition>

#define TextShape_SHAPEID "TextShapeID"

/**
 * A text shape.
 * The Text shape is capable of drawing structured text.
 * @see KoTextShapeData
 */
class TextShape : public KoShapeContainer {
public:
    TextShape();
    virtual ~TextShape();

    /// reimplemented
    void paintComponent(QPainter &painter, const KoViewConverter &converter);
    /// reimplemented
    void paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas);
    /// reimplemented
    virtual void waitUntilReady() const;

    /// helper method.
    QPointF convertScreenPos(const QPointF &point);

    /**
     * Set the shape's text to be demo text or not.
     * If true, replace the content with an lorem ipsum demo text and don't complain
     *   when there is not enough space at the end
     * If false; remove the demo text again.
     */
    void setDemoText(bool on);
    /// return if the content of this shape is demo text.
    bool demoText() const { return m_demoText; }

    /**
     * From KoShape reimplemented method to load the TextShape from ODF.
     *
     * This method redirects the call to the KoTextShapeData::loadOdf() method which
     * in turn will call the KoTextLoader::loadBody() method that reads the element
     * into a QTextCursor.
     *
     * @param context the KoShapeLoadingContext used for loading.
     * @param element element which represents the shape in odf.
     * @return false if loading failed.
     */
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context );

    /**
     * From KoShape reimplemented method to store the TextShape data as ODF.
     *
     * @param context the KoShapeSavingContext used for saving.
     */
    virtual void saveOdf(KoShapeSavingContext & context) const;

    KoTextShapeData *textShapeData() { return m_textShapeData; }

    bool hasFootnoteDocument() { return m_footnotes != 0; }
    QTextDocument *footnoteDocument();

    void markLayoutDone();

private:
    void shapeChanged(ChangeType type);

    KoTextShapeData *m_textShapeData;
    QTextDocument *m_footnotes;

    bool m_demoText;
    mutable QMutex m_mutex;
    mutable QWaitCondition m_waiter;
};

#endif

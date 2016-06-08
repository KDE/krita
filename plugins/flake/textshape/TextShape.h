/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_calligra@gadz.org>
 * Copyright (C) 2010 KO GmbH <cbo@kogmbh.com>
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
#include <KoFrameShape.h>
#include <KoTextShapeData.h>
#include <KoTextDocument.h>

#include <QTextDocument>
#include <QPainter>

#define TextShape_SHAPEID "TextShapeID"

class KoInlineTextObjectManager;
class KoTextRangeManager;
class KoPageProvider;
class KoImageCollection;
class KoTextDocument;
class TextShape;
class KoTextDocumentLayout;
class KoParagraphStyle;

/**
 * A text shape.
 * The Text shape is capable of drawing structured text.
 * @see KoTextShapeData
 */
class TextShape : public KoShapeContainer, public KoFrameShape
{
public:
    TextShape(KoInlineTextObjectManager *inlineTextObjectManager, KoTextRangeManager *textRangeManager);
    virtual ~TextShape();

    /// reimplemented
    void paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext);
    /// reimplemented
    virtual void waitUntilReady(const KoViewConverter &converter, bool asynchronous) const;

    /// helper method.
    QPointF convertScreenPos(const QPointF &point) const;

    /// reimplemented
    QPainterPath outline() const;

    /// reimplemented
    QRectF outlineRect() const;

    ///reimplemented
    ChildZOrderPolicy childZOrderPolicy()
    {
        return KoShape::ChildZPassThrough;
    }

    /// set the image collection which is needed to draw bullet from images
    void setImageCollection(KoImageCollection *collection)
    {
        m_imageCollection = collection;
    }

    KoImageCollection *imageCollection();

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
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    /**
     * From KoShape reimplemented method to store the TextShape data as ODF.
     *
     * @param context the KoShapeSavingContext used for saving.
     */
    virtual void saveOdf(KoShapeSavingContext &context) const;

    KoTextShapeData *textShapeData()
    {
        return m_textShapeData;
    }

    void updateDocumentData();

    virtual void update() const;
    virtual void update(const QRectF &shape) const;

    // required for kpresenter hack
    void setPageProvider(KoPageProvider *provider)
    {
        m_pageProvider = provider;
    }

    /// reimplemented
    virtual bool loadOdfFrame(const KoXmlElement &element, KoShapeLoadingContext &context);

protected:
    virtual bool loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context);

    /// reimplemented
    virtual void loadStyle(const KoXmlElement &element, KoShapeLoadingContext &context);

    /// reimplemented
    virtual QString saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const;

private:
    void shapeChanged(ChangeType type, KoShape *shape = 0);

    KoTextShapeData *m_textShapeData;
    KoPageProvider *m_pageProvider;
    KoImageCollection *m_imageCollection;
    QRegion m_paintRegion;
    KoParagraphStyle *m_paragraphStyle;
    bool m_clip;
    KoTextDocumentLayout *m_layout;
};

#endif

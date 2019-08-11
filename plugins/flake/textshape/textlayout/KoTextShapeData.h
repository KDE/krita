/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2010 Thomas Zander <zander@kde.org>
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

#include "KoText.h"
#include "kritatextlayout_export.h"

#include <KoTextShapeDataBase.h>
#include <KoXmlReaderForward.h>

class QTextDocument;
class KoShapeLoadingContext;
class KoShapeSavingContext;
class KoTextShapeDataPrivate;
class KoDocumentRdfBase;
class KoTextLayoutRootArea;

/**
 * The data store that is held by each TextShape instance.
 * This is a separate object to allow Words proper to use this class' API and
 * access the internals of the text shape.
 *
 * This class holds a QTextDocument pointer and is built so multiple shapes (and thus
 * multiple instances of this shape data) can share one QTextDocument by providing a
 * different view on (a different part of) the QTextDocument.
 */
class KRITATEXTLAYOUT_EXPORT KoTextShapeData : public KoTextShapeDataBase
{
    Q_OBJECT
public:
    /// constructor
    KoTextShapeData();
    ~KoTextShapeData() override;

    KoShapeUserData* clone() const override;

    /**
     * Replace the QTextDocument this shape will render.
     * @param document the new document. If there was an old document owned, it will be deleted.
     */
    void setDocument(QTextDocument *document);

    /**
     * return the amount of points into the document (y) this shape will display.
     */
    qreal documentOffset() const;

    /// mark shape as dirty triggering a re-layout of its text.
    void setDirty();

    /// return if the shape is marked dirty and its text content needs to be relayout
    bool isDirty() const;

    /// Set the rootArea that is associated to the textshape
    void setRootArea(KoTextLayoutRootArea *rootArea);

    /// the rootArea that is associated to the textshape
    KoTextLayoutRootArea *rootArea();

    void setLeftPadding(qreal padding);
    qreal leftPadding() const;
    void setTopPadding(qreal padding);
    qreal topPadding() const;
    void setRightPadding(qreal padding);
    qreal rightPadding() const;
    void setBottomPadding(qreal padding);
    qreal bottomPadding() const;
    void setPadding(qreal padding);

    /**
    * Load the TextShape from ODF.
    *
    * @see the @a TextShape::loadOdf() method which calls this method.
    * @see the @a KoTextLoader::loadBody() method which got called by this method
    * to load the ODF.
    */
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context, KoDocumentRdfBase *rdfData, KoShape *shape = 0);

    /**
    * Load the TextShape from ODF.
    * Overloaded method provided for your convenience.
    */
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context) override {
        return loadOdf(element, context, 0);
    }

    /**
    * Store the TextShape data as ODF.
    * @see TextShape::saveOdf()
    */
    void saveOdf(KoShapeSavingContext &context, KoDocumentRdfBase *rdfData, int from = 0, int to = -1) const;

    /**
    * Store the TextShape data as ODF.
    * Overloaded method provided for your convenience.
    */
    void saveOdf(KoShapeSavingContext &context, int from = 0, int to  = -1) const override {
        saveOdf(context, 0, from, to);
    }

    // reimplemented
    void loadStyle(const KoXmlElement &element, KoShapeLoadingContext &context) override;

    // reimplemented
    void saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const override;

    /**
     * Set the page direction.
     * The page direction will determine behavior on the insertion of new text and those
     * new paragraphs default direction.
     */
    void setPageDirection(KoText::Direction direction);
    /**
     * Return the direction set on the page.
     * The page direction will determine behavior on the insertion of new text and those
     * new paragraphs default direction.
     */
    KoText::Direction pageDirection() const;

private:
    KoTextShapeData(KoTextShapeDataPrivate *dd);

private:
    Q_DECLARE_PRIVATE(KoTextShapeData)
};

#endif

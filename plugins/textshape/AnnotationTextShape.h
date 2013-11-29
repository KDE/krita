/* This file is part of the KDE project
 * Copyright (C) 2013 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
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

#ifndef ANNOTATIONTEXTSHAPE_H
#define ANNOTATIONTEXTSHAPE_H

#include "TextShape.h"
#include <KoShapeContainer.h>
#include <KoFrameShape.h>
#include <KoTextShapeData.h>
#include <KoTextDocument.h>

#include <QTextDocument>
#include <QPainter>
class KoInlineTextObjectManager;
class KoTextRangeManager;
class KoPageProvider;
class KoImageCollection;
class KoTextDocument;
class TextShape;
class KoTextDocumentLayout;

#define AnnotationShape_SHAPEID "AnnotationTextShapeID"

class AnnotationTextShape : public TextShape
{
public:
    // For now we should give these parameters for TextShape.
    AnnotationTextShape(KoInlineTextObjectManager *inlineTextObjectManager,
                        KoTextRangeManager *textRangeManager);
    virtual ~AnnotationTextShape();

    void setAnnotaionTextData(KoTextShapeData *textShape);

    /// reimplemented
    void paintComponent(QPainter &painter, const KoViewConverter &converter,
                        KoShapePaintingContext &paintcontext);

    /**
     * From KoShape reimplemented method to load the TextShape from ODF.
     *
     * This method redirects the call to the KoTextShapeData::loadOdf() method which
     * in turn will call the KoTextLoader::loadBody() method that reads the element
     * into a QTextCursor.
     *
     * @param element element which represents the shape in odf.
     * @param context the KoShapeLoadingContext used for loading.
     * @return false if loading failed.
     */
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    /**
     * From KoShape reimplemented method to store the TextShape data as ODF.
     *
     * @param context the KoShapeSavingContext used for saving.
     */
    virtual void saveOdf(KoShapeSavingContext &context) const;

private:
    KoTextShapeData *m_textShapeData;
    QRegion m_paintRegion;
    KoParagraphStyle * m_paragraphStyle;
    bool m_clip;
    KoTextDocumentLayout *m_layout;

    QString m_creator;
    QString m_date;
};

#endif // ANNOTATIONTEXTSHAPE_H

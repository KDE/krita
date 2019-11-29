/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOSVGTEXTCHUNKSHAPE_H
#define KOSVGTEXTCHUNKSHAPE_H

#include "kritaflake_export.h"

#include <KoShapeContainer.h>
#include <SvgShape.h>
#include <KoSvgText.h>

class HtmlSavingContext;
class KoSvgTextProperties;
class KoSvgTextChunkShapePrivate;
class KoSvgTextChunkShapeLayoutInterface;

/**
 * KoSvgTextChunkShape is an elementary block of SVG text object.
 *
 * KoSvgTextChunkShape represents either a \<tspan\> or \<text\> element of SVG.
 * The chunk shape uses flake hierarchy to represent the DOM hierarchy of the
 * supplied text. All the attributes of text blocks can be fetched using
 * textProperties() method.
 *
 * KoSvgTextChunkShape uses special text properties object to overcome the
 * flake's "property inheritance" limitation. Basically, flake doesn't support
 * the inheritance of shape properties: every shape stores all the properties
 * that were defined at the stage of loading/creation. KoSvgTextProperties is a
 * wrapper that allows the user to compare the properties of the two shapes and
 * return only the ones that are unique for a child shape. That allows us to
 * generate a correct SVG/markup code that can be edited by the user easily.
 *
 * WARNING: beware the difference between "svg-text-chunk" and
 * KoSvgTextChunkShape! The chunk shape is **not** a "text chunk" in SVG's
 * definition. According to SVG, "text chunk" is a set of characters anchored to
 * a specific absolute position on canvas. And KoSvgTextChunkShape is just one
 * \<tspan\> or \<text\> element. Obviously, one \<tspan\> can contain multiple "text
 * chunks" and, vice versa, a "text chunk" can spread onto multiple \<span\>'s.
 */
class KRITAFLAKE_EXPORT KoSvgTextChunkShape : public KoShapeContainer, public SvgShape
{
public:
    KoSvgTextChunkShape();
    KoSvgTextChunkShape(const KoSvgTextChunkShape &rhs);
    ~KoSvgTextChunkShape() override;

    KoShape* cloneShape() const override;

    QSizeF size() const override;
    void setSize(const QSizeF &size) override;
    QRectF outlineRect() const override;
    QPainterPath outline() const override;

    void paintComponent(QPainter &painter, KoShapePaintingContext &paintContext) override;
    void saveOdf(KoShapeSavingContext &Context) const override;
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &Context) override;

    bool saveHtml(HtmlSavingContext &context);
    /**
     * Reset the text shape into initial state, removing all the child shapes.
     * This method is used by text-updating code to upload the updated text
     * into shape. The uploading code first calls resetTextShape() and then adds
     * new children.
     */
    virtual void resetTextShape();

    bool saveSvg(SvgSavingContext &context) override;
    bool loadSvg(const KoXmlElement &element, SvgLoadingContext &context) override;
    bool loadSvgTextNode(const KoXmlText &text, SvgLoadingContext &context);

    /**
     * Normalize the SVG character transformations
     *
     * In SVG x,y,dx,dy,rotate attributes are inherited from the parent shapes
     * in a curious ways. We do not want to unwind the links on the fly, so we
     * just parse and adjust them right when the loading completed. After
     * normalizeCharTransformations() is finished, all the child shapes will
     * have local transformations set according to the parent's lists.
     */
    void normalizeCharTransformations();

    /**
     * Compress the inheritance of 'fill' and 'stroke'.
     *
     * The loading code makes no difference if the fill or stroke was inherited
     * or not. That is not a problem for normal shapes, but it cannot work for
     * text shapes. The text has different inheritance rules: if the parent
     * text chunk has a gradient fill, its inheriting descendants will
     * **smoothly continue** this gradient. They will not start a new gradient
     * in their local coordinate system.
     *
     * Therefore, after loading, the loading code calls
     * simplifyFillStrokeInheritance() which marks the coinciding strokes and
     * fills so that the rendering code will be able to distinguish them in the
     * future.
     *
     * Proper fill inheritance is also needed for the GUI. When the user
     * changes the color of the parent text chunk, all the inheriting children
     * should update its color automatically, without GUI recursively
     * traversing the shapes.
     *
     */
    void simplifyFillStrokeInheritance();


    /**
     * SVG properties of the text chunk
     * @return the properties object with fill and stroke included as a property
     */
    KoSvgTextProperties textProperties() const;

    /**
     * Return the type of the chunk.
     *
     * The chunk can be either a "text chunk", that contains a text string
     * itself, of an "intermediate chunk" that doesn't contain any text itself,
     * but works as a group for a set of child chunks, which might be either
     * text (leaf) or intermediate chunks. Such groups are needed to define a
     * common text style for a group of '\<tspan\>' objects.
     *
     * @return true if the chunk is a "text chunk" false if it is "intermediate chunk"
     */
    bool isTextNode() const;

    /**
     * A special interface for KoSvgTextShape's layout code. Don't use it
     * unless you are KoSvgTextShape.
     */
    KoSvgTextChunkShapeLayoutInterface* layoutInterface();

    /**
     * WARNING: this propperty is available only if isRootTextNode() is true
     *
     * @return true if the shape should be edited in a rich-text editor
     */
    bool isRichTextPreferred() const;

    /**
     * WARNING: this propperty is available only if isRootTextNode() is true
     *
     * Sets whether the shape should be edited in rich-text editor
     */
    void setRichTextPreferred(bool value);

protected:
    /**
     * Show if the shape is a root of the text hierarchy. Always true for
     * KoSvgTextShape and always false for KoSvgTextChunkShape
     */
    virtual bool isRootTextNode() const;

protected:
    KoSvgTextChunkShape(KoSvgTextChunkShapePrivate *dd);

private:
    KoSvgText::KoSvgCharChunkFormat fetchCharFormat() const;

    void applyParentCharTransformations(const QVector<KoSvgText::CharTransformation> transformations);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif // KOSVGTEXTCHUNKSHAPE_H

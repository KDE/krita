/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef IMAGESHAPE_H
#define IMAGESHAPE_H

#include <QSharedDataPointer>

#include "KoTosContainer.h"
#include <SvgShape.h>

#define ImageShapeId "ImageShape"


class ImageShape : public KoTosContainer, public SvgShape
{
public:
    ImageShape();
    ~ImageShape() override;

    KoShape *cloneShape() const override;

    void paint(QPainter &painter, KoShapePaintingContext &paintContext) const override;

    void setSize(const QSizeF &size) override;

    void saveOdf(KoShapeSavingContext &context) const override;
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context) override;

    bool saveSvg(SvgSavingContext &context) override;
    bool loadSvg(const KoXmlElement &element, SvgLoadingContext &context) override;

private:
    ImageShape(const ImageShape &rhs);

private:
    struct Private;
    QSharedDataPointer<Private> m_d;
};

#endif // IMAGESHAPE_H

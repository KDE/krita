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

#include <KoTosContainer_p.h>

#include "ImageShape.h"
#include "kis_debug.h"

#include <QPainter>
#include <KoViewConverter.h>
#include <SvgLoadingContext.h>
#include <SvgUtil.h>
#include <QFileInfo>
#include <QBuffer>

struct Q_DECL_HIDDEN ImageShape::Private
{
    Private() {}
    Private(const Private &rhs)
        : image(rhs.image),
          ratioParser(rhs.ratioParser ? new SvgUtil::PreserveAspectRatioParser(*rhs.ratioParser) : 0),
          viewBoxTransform(rhs.viewBoxTransform)
    {
    }

    QImage image;
    QScopedPointer<SvgUtil::PreserveAspectRatioParser> ratioParser;
    QTransform viewBoxTransform;
};


ImageShape::ImageShape()
    : m_d(new Private)
{
}

ImageShape::ImageShape(const ImageShape &rhs)
    : KoTosContainer(new KoTosContainerPrivate(*rhs.d_func(), this)),
      m_d(new Private(*rhs.m_d))
{
}

ImageShape::~ImageShape()
{
}

KoShape *ImageShape::cloneShape() const
{
    return new ImageShape(*this);
}

void ImageShape::paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    Q_UNUSED(paintContext);

    const QRectF myrect(QPointF(), size());
    applyConversion(painter, converter);

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setClipRect(QRectF(QPointF(), size()), Qt::IntersectClip);
    painter.setTransform(m_d->viewBoxTransform, true);
    painter.drawImage(QPoint(), m_d->image);
    painter.restore();
}

void ImageShape::setSize(const QSizeF &size)
{
    KoTosContainer::setSize(size);
}

void ImageShape::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

bool ImageShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);

    return false;
}

bool ImageShape::saveSvg(SvgSavingContext &context)
{
    Q_UNUSED(context);
    return false;
}

bool ImageShape::loadSvg(const KoXmlElement &element, SvgLoadingContext &context)
{
    const qreal x = SvgUtil::parseUnitX(context.currentGC(), element.attribute("x"));
    const qreal y = SvgUtil::parseUnitY(context.currentGC(), element.attribute("y"));
    const qreal w = SvgUtil::parseUnitX(context.currentGC(), element.attribute("width"));
    const qreal h = SvgUtil::parseUnitY(context.currentGC(), element.attribute("height"));

    setSize(QSizeF(w, h));
    setPosition(QPointF(x, y));

    if (w == 0.0 || h == 0.0) {
        setVisible(false);
    }

    QString fileName = element.attribute("xlink:href");
    QByteArray data = context.fetchExternalFile(fileName);

    if (!data.isEmpty()) {
        QBuffer buffer(&data);
        m_d->image.load(&buffer, "");
    }

    const QString aspectString = element.attribute("preserveAspectRatio", "xMidYMid meet");
    m_d->ratioParser.reset(new SvgUtil::PreserveAspectRatioParser(aspectString));

    if (!m_d->image.isNull()) {

        m_d->viewBoxTransform =
             QTransform::fromScale(w / m_d->image.width(), h / m_d->image.height());

        SvgUtil::parseAspectRatio(*m_d->ratioParser,
                                  QRectF(QPointF(), size()),
                                  QRect(QPoint(), m_d->image.size()),
                                  &m_d->viewBoxTransform);
        }

    if (m_d->ratioParser->defer) {
        // TODO:
    }

    return true;
}

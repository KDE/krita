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
#include <SvgLoadingContext.h>
#include <SvgSavingContext.h>
#include <SvgUtil.h>
#include <QFileInfo>
#include <QBuffer>
#include <KisMimeDatabase.h>
#include <KoXmlWriter.h>
#include "kis_dom_utils.h"
#include <QRegularExpression>
#include "KisQPainterStateSaver.h"


struct Q_DECL_HIDDEN ImageShape::Private : public QSharedData
{
    Private() {}
    Private(const Private &rhs)
        : QSharedData(),
          image(rhs.image),
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
    : KoTosContainer(rhs),
      m_d(rhs.m_d)
{
}

ImageShape::~ImageShape()
{
}

KoShape *ImageShape::cloneShape() const
{
    return new ImageShape(*this);
}

void ImageShape::paint(QPainter &painter, KoShapePaintingContext &paintContext)
{
    Q_UNUSED(paintContext);
    KisQPainterStateSaver saver(&painter);

    const QRectF myrect(QPointF(), size());

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setClipRect(QRectF(QPointF(), size()), Qt::IntersectClip);
    painter.setTransform(m_d->viewBoxTransform, true);
    painter.drawImage(QPoint(), m_d->image);
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
    const QString uid = context.createUID("image");

    context.shapeWriter().startElement("image");
    context.shapeWriter().addAttribute("id", uid);
    SvgUtil::writeTransformAttributeLazy("transform", transformation(), context.shapeWriter());
    context.shapeWriter().addAttribute("width", QString("%1px").arg(KisDomUtils::toString(size().width())));
    context.shapeWriter().addAttribute("height", QString("%1px").arg(KisDomUtils::toString(size().height())));

    QString aspectString = m_d->ratioParser->toString();
    if (!aspectString.isEmpty()) {
        context.shapeWriter().addAttribute("preserveAspectRatio", aspectString);
    }

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    if (m_d->image.save(&buffer, "PNG")) {
        const QString mimeType = KisMimeDatabase::mimeTypeForSuffix("*.png");
        context.shapeWriter().addAttribute("xlink:href", "data:"+ mimeType + ";base64," + ba.toBase64());
    }

    context.shapeWriter().endElement(); // image

    return true;
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

    QByteArray data;

    if (fileName.startsWith("data:")) {

        QRegularExpression re("data:(.+?);base64,(.+)");
        QRegularExpressionMatch match = re.match(fileName);

        data = match.captured(2).toLatin1();
        data = QByteArray::fromBase64(data);
    } else {
        data = context.fetchExternalFile(fileName);
    }

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

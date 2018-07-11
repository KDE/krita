/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
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

#include "KoOdfGradientBackground.h"

#include "KoShapeBackground_p.h"
#include "KoShapeSavingContext.h"

#include <KoUnit.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoGenStyles.h>
#include <KoOdfLoadingContext.h>
#include <KoStyleStack.h>
#include <KoOdfStylesReader.h>

#include <QPainter>
#include <QColor>
#include <QImage>
#include <qmath.h>

#include "FlakeDebug.h"

class KoOdfGradientBackgroundPrivate : public KoShapeBackgroundPrivate
{
public:
    KoOdfGradientBackgroundPrivate()
        : style()
        , cx(0)
        , cy(0)
        , startColor()
        , endColor()
        , angle(0)
        , border(0)
        , opacity(1.0)
    {}
    ~KoOdfGradientBackgroundPrivate() override{};
    //data
    QString style;
    int cx;
    int cy;
    QColor startColor;
    QColor endColor;
    qreal angle;
    qreal border;
    qreal opacity;
};


KoOdfGradientBackground::KoOdfGradientBackground()
    : KoShapeBackground(*(new KoOdfGradientBackgroundPrivate()))
{

}

KoOdfGradientBackground::~KoOdfGradientBackground()
{

}

bool KoOdfGradientBackground::compareTo(const KoShapeBackground *other) const
{
    Q_UNUSED(other);
    return false;
}


bool KoOdfGradientBackground::loadOdf(const KoXmlElement& e)
{
    Q_D(KoOdfGradientBackground);
    d->style = e.attributeNS(KoXmlNS::draw, "style", QString());
    //TODO: support ellipsoid here too
    if ((d->style != "rectangular") && (d->style != "square")) {
        return false;
    }

    d->cx = KoUnit::parseValue(e.attributeNS(KoXmlNS::draw, "cx", QString()).remove('%'));
    d->cy = KoUnit::parseValue(e.attributeNS(KoXmlNS::draw, "cy", QString()).remove('%'));

    d->border = qBound(0.0,0.01 * e.attributeNS(KoXmlNS::draw, "border", "0").remove('%').toDouble(),1.0);

    d->startColor = QColor(e.attributeNS(KoXmlNS::draw, "start-color", QString()));
    d->startColor.setAlphaF((0.01 * e.attributeNS(KoXmlNS::draw, "start-intensity", "100").remove('%').toDouble()));

    d->endColor = QColor(e.attributeNS(KoXmlNS::draw, "end-color", QString()));
    d->endColor.setAlphaF(0.01 * e.attributeNS(KoXmlNS::draw, "end-intensity", "100").remove('%').toDouble());
    d->angle = e.attributeNS(KoXmlNS::draw, "angle", "0").toDouble() / 10;

    return true;
}


void KoOdfGradientBackground::saveOdf(KoGenStyle& styleFill, KoGenStyles& mainStyles) const
{
    Q_D(const KoOdfGradientBackground);

    KoGenStyle::Type type = styleFill.type();
    KoGenStyle::PropertyType propertyType = (type == KoGenStyle::GraphicStyle || type == KoGenStyle::GraphicAutoStyle ||
                                             type == KoGenStyle::DrawingPageStyle || type == KoGenStyle::DrawingPageAutoStyle )
                                            ? KoGenStyle::DefaultType : KoGenStyle::GraphicType;

    KoGenStyle gradientStyle(KoGenStyle::GradientStyle);

    gradientStyle.addAttribute("draw:style", d->style); // draw:style="square"
    gradientStyle.addAttribute("draw:cx", QString("%1%").arg(d->cx));
    gradientStyle.addAttribute("draw:cy", QString("%1%").arg(d->cy));
    gradientStyle.addAttribute("draw:start-color", d->startColor.name());
    gradientStyle.addAttribute("draw:end-color", d->endColor.name());
    gradientStyle.addAttribute("draw:start-intensity", QString("%1%").arg(qRound(d->startColor.alphaF() * 100)) );
    gradientStyle.addAttribute("draw:end-intensity", QString("%1%").arg(qRound(d->endColor.alphaF() * 100)) );
    gradientStyle.addAttribute("draw:angle", QString("%1").arg(d->angle * 10));
    gradientStyle.addAttribute("draw:border", QString("%1%").arg(qRound(d->border * 100.0)));

    QString gradientStyleName = mainStyles.insert(gradientStyle, "gradient");

    styleFill.addProperty("draw:fill", "gradient", propertyType);
    styleFill.addProperty("draw:fill-gradient-name", gradientStyleName, propertyType);
    if (d->opacity <= 1.0) {
        styleFill.addProperty("draw:opacity", QString("%1%").arg(d->opacity * 100.0), propertyType);
    }
}

void KoOdfGradientBackground::paint(QPainter& painter, const KoViewConverter &/*converter*/, KoShapePaintingContext &/*context*/, const QPainterPath& fillPath) const
{
    Q_D(const KoOdfGradientBackground);

    QImage buffer;

    QRectF targetRect = fillPath.boundingRect();
    QRectF pixels = painter.transform().mapRect(QRectF(0,0,targetRect.width(), targetRect.height()));
    QSize currentSize( qCeil(pixels.size().width()), qCeil(pixels.size().height()) );
    if (buffer.isNull() || buffer.size() != currentSize){
        buffer = QImage(currentSize, QImage::Format_ARGB32_Premultiplied);
        if (d->style == "square") {
            renderSquareGradient(buffer);
        } else {
            renderRectangleGradient(buffer);
        }
    }

    painter.setClipPath(fillPath);

    painter.setOpacity(d->opacity);
    painter.drawImage(targetRect, buffer, QRectF(QPointF(0,0), buffer.size()));
}

void KoOdfGradientBackground::fillStyle(KoGenStyle& style, KoShapeSavingContext& context)
{
    saveOdf(style, context.mainStyles());
}

bool KoOdfGradientBackground::loadStyle(KoOdfLoadingContext& context, const QSizeF& shapeSize)
{
    Q_UNUSED(shapeSize);
    Q_D(KoOdfGradientBackground);

    KoStyleStack &styleStack = context.styleStack();
    if (!styleStack.hasProperty(KoXmlNS::draw, "fill")) {
        return false;
    }

    QString fillStyle = styleStack.property(KoXmlNS::draw, "fill");
    if (fillStyle == "gradient") {

        if (styleStack.hasProperty(KoXmlNS::draw, "opacity")) {
            QString opacity = styleStack.property(KoXmlNS::draw, "opacity");
            if (! opacity.isEmpty() && opacity.right(1) == "%") {
                d->opacity = qMin(opacity.left(opacity.length() - 1).toDouble(), 100.0) / 100;
            }
        }

        QString styleName = styleStack.property(KoXmlNS::draw, "fill-gradient-name");
        KoXmlElement * e = context.stylesReader().drawStyles("gradient")[styleName];
        return loadOdf(*e);
    }

    return false;
}


void KoOdfGradientBackground::renderSquareGradient(QImage& buffer) const
{
    Q_D(const KoOdfGradientBackground);
    buffer.fill(d->startColor.rgba());

    QPainter painter(&buffer);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, false);

    int width = buffer.width();
    int height = buffer.height();

    qreal gradientCenterX = qRound(width * d->cx * 0.01);
    qreal gradientCenterY = qRound(height * d->cy * 0.01);
    qreal centerX = width * 0.5;
    qreal centerY = height * 0.5;

    qreal areaCenterX = qRound(centerX);
    qreal areaCenterY = qRound(centerY);

    QTransform m;
    m.translate(gradientCenterX, gradientCenterY);
    m.rotate(-d->angle);
    m.scale(1.0 - d->border, 1.0 - d->border);
    m.translate(-gradientCenterX, -gradientCenterY);
    m.translate(gradientCenterX - areaCenterX,gradientCenterY - areaCenterY);
    painter.setTransform(m);

    QLinearGradient linearGradient;
    linearGradient.setColorAt(1, d->startColor);
    linearGradient.setColorAt(0, d->endColor);

    // from center going North
    linearGradient.setStart(centerX, centerY);
    linearGradient.setFinalStop(centerX, 0);
    painter.setBrush(linearGradient);
    painter.drawRect(0, 0, width, centerY);

    // from center going South
    linearGradient.setFinalStop(centerX, height);
    painter.setBrush(linearGradient);
    painter.drawRect(0, centerY, width, centerY);

    // clip the East and West portion
    QPainterPath clip;
    clip.moveTo(width, 0);
    clip.lineTo(width, height);
    clip.lineTo(0, 0);
    clip.lineTo(0, height);
    clip.closeSubpath();
    painter.setClipPath(clip);

    // from center going East
    linearGradient.setFinalStop(width, centerY);
    painter.setBrush(linearGradient);
    painter.drawRect(centerX, 0, width, height);

    // from center going West
    linearGradient.setFinalStop( 0, centerY);
    painter.setBrush(linearGradient);
    painter.drawRect(0, 0, centerX, height);
}


void KoOdfGradientBackground::renderRectangleGradient(QImage& buffer) const
{
    Q_D(const KoOdfGradientBackground);
    buffer.fill(d->startColor.rgba());

    QPainter painter(&buffer);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, false);

    int width = buffer.width();
    int height = buffer.height();

    qreal gradientCenterX = qRound(width * d->cx * 0.01);
    qreal gradientCenterY = qRound(height * d->cy * 0.01);
    qreal centerX = width * 0.5;
    qreal centerY = height * 0.5;

    qreal areaCenterY = qRound(centerY);
    qreal areaCenterX = qRound(centerX);

    QTransform m;
    m.translate(gradientCenterX, gradientCenterY);
    // m.rotate(-d->angle); // OOo rotates the gradient differently
    m.scale(1.0 - d->border, 1.0 - d->border);
    m.translate(-gradientCenterX, -gradientCenterY);
    m.translate(gradientCenterX - areaCenterX,gradientCenterY - areaCenterY);
    painter.setTransform(m);

    QLinearGradient linearGradient;
    linearGradient.setColorAt(1, d->startColor);
    linearGradient.setColorAt(0, d->endColor);

    // render background
    QPainterPath clipPath;
    if (width < height) {
        QRectF west(0,0,centerX, height);
        QRectF east(centerX, 0, centerX, height);

        linearGradient.setStart(centerX, centerY);
        linearGradient.setFinalStop(0, centerY);
        painter.setBrush(linearGradient);
        painter.drawRect(west);

        linearGradient.setFinalStop(width, centerY);
        painter.setBrush(linearGradient);
        painter.drawRect(east);

        QRectF north(0,0,width, centerX);
        QRectF south(0,height - centerX, width, centerX);

        clipPath.moveTo(0,0);
        clipPath.lineTo(width, 0);
        clipPath.lineTo(centerX, centerX);
        clipPath.closeSubpath();

        clipPath.moveTo(width, height);
        clipPath.lineTo(0, height);
        clipPath.lineTo(centerX, south.y());
        clipPath.closeSubpath();

        linearGradient.setStart(centerX, centerX);
        linearGradient.setFinalStop(centerX, 0);

        painter.setClipPath(clipPath);
        painter.setBrush(linearGradient);
        painter.drawRect(north);

        linearGradient.setStart(centerX, south.y());
        linearGradient.setFinalStop(centerX, height);

        painter.setBrush(linearGradient);
        painter.drawRect(south);
    } else {
        QRectF north(0,0,width, centerY);
        QRectF south(0, centerY, width, centerY);

        linearGradient.setStart(centerX, centerY);
        linearGradient.setFinalStop(centerX, 0);

        painter.setBrush(linearGradient);
        painter.drawRect(north);

        linearGradient.setFinalStop(centerX, height);
        painter.setBrush(linearGradient);
        painter.drawRect(south);


        QRectF west(0,0,centerY, height);
        QRectF east(width - centerY, 0, centerY, height);

        clipPath.moveTo(0,0);
        clipPath.lineTo(centerY, centerY);
        clipPath.lineTo(0,height);
        clipPath.closeSubpath();

        clipPath.moveTo(width, height);
        clipPath.lineTo(east.x(), centerY);
        clipPath.lineTo(width,0);
        clipPath.closeSubpath();

        linearGradient.setStart(centerY, centerY);
        linearGradient.setFinalStop(0, centerY);

        painter.setClipPath(clipPath);
        painter.setBrush(linearGradient);
        painter.drawRect(west);

        linearGradient.setStart(east.x(), centerY);
        linearGradient.setFinalStop(width, centerY);

        painter.setBrush(linearGradient);
        painter.drawRect(east);
    }
}


void KoOdfGradientBackground::debug() const
{
    Q_D(const KoOdfGradientBackground);
    debugFlake << "cx,cy: "<< d->cx << d->cy;
    debugFlake << "style" << d->style;
    debugFlake << "colors" << d->startColor << d->endColor;
    debugFlake << "angle:" << d->angle;
    debugFlake << "border" << d->border;
}

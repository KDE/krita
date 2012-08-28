/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <ingwa@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Own
#include "Surface.h"

// Qt
#include <QPointF>
#include <QBrush>
#include <QPen>
#include <QDebug>

// Calligra
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoOdfStylesReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoStyleStack.h>
#include <KoOdfGraphicStyles.h>
#include <KoGenStyles.h>
#include <KoOdfWorkaround.h>

#include <KoImageData.h>
#include <KoStore.h>
#include <KoUnit.h>

#include <KDebug>

// KDChart
#include <KDChartCartesianCoordinatePlane>
#include <KDChartBackgroundAttributes>
#include <KDChartFrameAttributes>

// KChart
#include "PlotArea.h"


using namespace KChart;

class Surface::Private
{
public:
    Private(PlotArea *parent);
    ~Private();

    PlotArea *plotArea;
    
    QPointF  position;
    int      width;

    QBrush   brush;
    QPen     framePen;

    KDChart::CartesianCoordinatePlane *kdPlane;
};

Surface::Private::Private(PlotArea *parent)
    : plotArea(parent)
{
}

Surface::Private::~Private()
{
}


// ================================================================


Surface::Surface(PlotArea *parent)
    : d(new Private(parent))
{
    Q_ASSERT(parent);

    // FIXME: Make this class capable of storing floor-specific
    // attributes as well. Right now, it's really only used
    // and designed to load and save the chart's wall.
    d->kdPlane = d->plotArea->kdCartesianPlane();
    Q_ASSERT(d->kdPlane);
}

Surface::~Surface()
{
    delete d;
}


QPointF Surface::position() const
{
    return d->position;
}

void Surface::setPosition(const QPointF &position)
{
    d->position = position;
}

int Surface::width() const
{
    return d->width;
}

void Surface::setWidth(int width)
{
    d->width = width;
}

QBrush Surface::brush() const
{
    return d->brush;
}

void Surface::setBrush(const QBrush &brush)
{
    d->brush = brush;
}

QPen Surface::framePen() const
{
    return d->framePen;
}

void Surface::setFramePen(const QPen &pen)
{
    d->framePen = pen;
}

bool Surface::loadOdf(const KoXmlElement &surfaceElement,
                       KoShapeLoadingContext &context)
{
    // Get the current style stack and save it's state.
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();

    bool brushLoaded = false;
    
    if (surfaceElement.hasAttributeNS(KoXmlNS::chart, "style-name")) {
        KDChart::BackgroundAttributes backgroundAttributes = d->kdPlane->backgroundAttributes();
        KDChart::FrameAttributes frameAttributes = d->kdPlane->frameAttributes();
        
        // Add the chart style to the style stack.
        styleStack.clear();
        context.odfLoadingContext().fillStyleStack(surfaceElement, KoXmlNS::chart, "style-name", "chart");
        
        styleStack.setTypeProperties("graphic");
        
        // If there is a "stroke" property, then get the stroke style
        // and set the pen accordingly.
        if (styleStack.hasProperty(KoXmlNS::draw, "stroke")) {
            frameAttributes.setVisible(true);

            QString  stroke = styleStack.property(KoXmlNS::draw, "stroke");
            QPen pen(Qt::NoPen);
            if (stroke == "solid" || stroke == "dash")
                pen = KoOdfGraphicStyles::loadOdfStrokeStyle(styleStack, stroke,
                                                             context.odfLoadingContext().stylesReader());

            frameAttributes.setPen(pen);
        }
        
        // If there is a "fill" property, then get the fill style, and
        // set the brush for the surface accordingly.
        if (styleStack.hasProperty(KoXmlNS::draw, "fill")) {
            backgroundAttributes.setVisible(true);

            QBrush   brush;
            QString  fill = styleStack.property(KoXmlNS::draw, "fill");
            if (fill == "solid" || fill == "hatch") {
                brushLoaded = true;
                brush = KoOdfGraphicStyles::loadOdfFillStyle(styleStack, fill,
                                                             context.odfLoadingContext().stylesReader());
            }
            else if (fill == "gradient") {
                brushLoaded = true;
                brush = KoOdfGraphicStyles::loadOdfGradientStyle(styleStack, context.odfLoadingContext().stylesReader(), QSizeF(5.0, 60.0));
            }
            else if (fill == "bitmap") {
                brushLoaded = true;
                brush = loadOdfPatternStyle(styleStack, context.odfLoadingContext(), QSizeF(5.0, 60.0));
            }

            backgroundAttributes.setBrush(brush);
        }
        
        // Finally actually set the attributes.
        d->kdPlane->setBackgroundAttributes(backgroundAttributes);
        d->kdPlane->setFrameAttributes(frameAttributes);
    }

#ifndef NWORKAROUND_ODF_BUGS
    if (!brushLoaded) {
        KDChart::BackgroundAttributes backgroundAttributes = d->kdPlane->backgroundAttributes();
        QColor fillColor = KoOdfWorkaround::fixMissingFillColor(surfaceElement, context);
        if (fillColor.isValid()) {
            backgroundAttributes.setVisible(true);
            backgroundAttributes.setBrush(fillColor);
            d->kdPlane->setBackgroundAttributes(backgroundAttributes);
        }
    }
#endif
    
    return true;
}

void Surface::saveOdf(KoShapeSavingContext &context, const char *elementName)
{
    KoXmlWriter  &bodyWriter = context.xmlWriter();
    KoGenStyles  &mainStyles = context.mainStyles();
    KoGenStyle    style      = KoGenStyle(KoGenStyle::GraphicAutoStyle, "chart");

    // elementName is chart:floor or chart:wall
    bodyWriter.startElement(elementName);

    QBrush backgroundBrush;
    if (d->kdPlane->backgroundAttributes().isVisible())
        backgroundBrush = d->kdPlane->backgroundAttributes().brush();
    QPen framePen(Qt::NoPen);
    if (d->kdPlane->frameAttributes().isVisible())
        framePen = d->kdPlane->frameAttributes().pen();

    KoOdfGraphicStyles::saveOdfFillStyle(style, mainStyles, backgroundBrush);
    KoOdfGraphicStyles::saveOdfStrokeStyle(style, mainStyles, framePen);

    bodyWriter.addAttribute("chart:style-name", mainStyles.insert(style, "ch"));

    bodyWriter.endElement(); // chart:floor or chart:wall
}

QBrush Surface::loadOdfPatternStyle(const KoStyleStack &styleStack, 
                                    KoOdfLoadingContext &context, const QSizeF &size)
{
    QString styleName = styleStack.property(KoXmlNS::draw, "fill-image-name");

    KoXmlElement* e = context.stylesReader().drawStyles("fill-image")[styleName];
    if (! e)
        return QBrush();

    const QString href = e->attributeNS(KoXmlNS::xlink, "href", QString());

    if (href.isEmpty())
        return QBrush();

    QString strExtension;
    const int result = href.lastIndexOf(".");
    if (result >= 0) {
        strExtension = href.mid(result + 1); // As we are using KoPicture, the extension should be without the dot.
    }
    QString filename(href);

    KoImageData data;
    data.setImage(href, context.store());
    if (data.errorCode() != KoImageData::Success)
        return QBrush();

    // read the pattern repeat style
    QString style = styleStack.property(KoXmlNS::style, "repeat");
    kDebug(35001) << "pattern style =" << style;

    QSize imageSize = data.image().size();

    if (style == "stretch") {
        imageSize = size.toSize();
    } else {
        // optional attributes which can override original image size
        if (styleStack.hasProperty(KoXmlNS::draw, "fill-image-height") && styleStack.hasProperty(KoXmlNS::draw, "fill-image-width")) {
            QString height = styleStack.property(KoXmlNS::draw, "fill-image-height");
            qreal newHeight = 0.0;
            if (height.endsWith('%'))
                newHeight = 0.01 * height.remove('%').toDouble() * imageSize.height();
            else
                newHeight = KoUnit::parseValue(height);
            QString width = styleStack.property(KoXmlNS::draw, "fill-image-width");
            qreal newWidth = 0.0;
            if (width.endsWith('%'))
                newWidth = 0.01 * width.remove('%').toDouble() * imageSize.width();
            else
                newWidth = KoUnit::parseValue(width);
            if (newHeight > 0.0)
                imageSize.setHeight(static_cast<int>(newHeight));
            if (newWidth > 0.0)
                imageSize.setWidth(static_cast<int>(newWidth));
        }
    }

    kDebug(35001) << "shape size =" << size;
    kDebug(35001) << "original image size =" << data.image().size();
    kDebug(35001) << "resulting image size =" << imageSize;

    QBrush resultBrush(QPixmap::fromImage(data.image()).scaled(imageSize));

    if (style == "repeat") {
        QTransform matrix;
        if (styleStack.hasProperty(KoXmlNS::draw, "fill-image-ref-point")) {
            // align pattern to the given size
            QString align = styleStack.property(KoXmlNS::draw, "fill-image-ref-point");
            kDebug(35001) << "pattern align =" << align;
            if (align == "top-left")
                matrix.translate(0, 0);
            else if (align == "top")
                matrix.translate(0.5*size.width(), 0);
            else if (align == "top-right")
                matrix.translate(size.width(), 0);
            else if (align == "left")
                matrix.translate(0, 0.5*size.height());
            else if (align == "center")
                matrix.translate(0.5*size.width(), 0.5*size.height());
            else if (align == "right")
                matrix.translate(size.width(), 0.5*size.height());
            else if (align == "bottom-left")
                matrix.translate(0, size.height());
            else if (align == "bottom")
                matrix.translate(0.5*size.width(), size.height());
            else if (align == "bottom-right")
                matrix.translate(size.width(), size.height());
        }
        if (styleStack.hasProperty(KoXmlNS::draw, "fill-image-ref-point-x")) {
            QString pointX = styleStack.property(KoXmlNS::draw, "fill-image-ref-point-x");
            matrix.translate(0.01 * pointX.remove('%').toDouble() * imageSize.width(), 0);
        }
        if (styleStack.hasProperty(KoXmlNS::draw, "fill-image-ref-point-y")) {
            QString pointY = styleStack.property(KoXmlNS::draw, "fill-image-ref-point-y");
            matrix.translate(0, 0.01 * pointY.remove('%').toDouble() * imageSize.height());
        }
        resultBrush.setTransform(matrix);
    }

    return resultBrush;
}

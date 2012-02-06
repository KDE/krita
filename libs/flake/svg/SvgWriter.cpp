/* This file is part of the KDE project
   Copyright (C) 2002 Lars Siebold <khandha5@gmx.net>
   Copyright (C) 2002-2003,2005 Rob Buis <buis@kde.org>
   Copyright (C) 2002,2005-2006 David Faure <faure@kde.org>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2004 Nicolas Goutte <nicolasg@snafu.de>
   Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005,2007-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006 Gábor Lehel <illissius@gmail.com>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2006 Christian Mueller <cmueller@gmx.de>
   Copyright (C) 2006 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include "SvgWriter.h"
#include "SvgUtil.h"
#include "SvgSavingContext.h"
#include "SvgShape.h"
#include "SvgStyleWriter.h"

#include <KoShapeLayer.h>
#include <KoShapeGroup.h>
#include <KoPathShape.h>
#include <KoXmlWriter.h>
#include <KoShapePainter.h>

#include <QFile>
#include <QString>
#include <QTextStream>
#include <QBuffer>
#include <QPainter>
#include <QSvgGenerator>

SvgWriter::SvgWriter(const QList<KoShapeLayer*> &layers, const QSizeF &pageSize)
    : m_pageSize(pageSize)
    , m_writeInlineImages(true)
{
    foreach(KoShapeLayer *layer, layers)
        m_toplevelShapes.append(layer);
}

SvgWriter::SvgWriter(const QList<KoShape*> &toplevelShapes, const QSizeF &pageSize)
    : m_toplevelShapes(toplevelShapes)
    , m_pageSize(pageSize)
    , m_writeInlineImages(true)
{
}

SvgWriter::~SvgWriter()
{

}

bool SvgWriter::save(const QString &filename, bool writeInlineImages)
{
    QFile fileOut(filename);
    if (!fileOut.open(QIODevice::WriteOnly))
        return false;

    m_writeInlineImages = writeInlineImages;

    const bool success = save(fileOut);

    m_writeInlineImages = true;

    fileOut.close();

    return success;
}

bool SvgWriter::save(QIODevice &outputDevice)
{
    if (m_toplevelShapes.isEmpty())
        return false;

    QTextStream svgStream(&outputDevice);

    // standard header:
    svgStream << "<?xml version=\"1.0\" standalone=\"no\"?>" << endl;
    svgStream << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" ";
    svgStream << "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" << endl;

    // add some PR.  one line is more than enough.
    svgStream << "<!-- Created using Karbon, part of Calligra: http://www.calligra.org/karbon -->" << endl;

    svgStream << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"";
    svgStream << " width=\"" << m_pageSize.width() << "pt\"";
    svgStream << " height=\"" << m_pageSize.height() << "pt\">" << endl;

    {
        SvgSavingContext savingContext(outputDevice, m_writeInlineImages);

        // top level shapes
        foreach(KoShape *shape, m_toplevelShapes) {
            KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>(shape);
            if(layer) {
                saveLayer(layer, savingContext);
            } else {
                KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(shape);
                if (group)
                    saveGroup(group, savingContext);
                else
                    saveShape(shape, savingContext);
            }
        }
    }

    // end tag:
    svgStream << endl << "</svg>" << endl;

    return true;
}

void SvgWriter::saveLayer(KoShapeLayer *layer, SvgSavingContext &context)
{
    context.shapeWriter().startElement("g");
    context.shapeWriter().addAttribute("id", context.getID(layer));

    QList<KoShape*> sortedShapes = layer->shapes();
    qSort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    foreach(KoShape * shape, sortedShapes) {
        KoShapeGroup * group = dynamic_cast<KoShapeGroup*>(shape);
        if (group)
            saveGroup(group, context);
        else
            saveShape(shape, context);
    }

    context.shapeWriter().endElement();
}

void SvgWriter::saveGroup(KoShapeGroup * group, SvgSavingContext &context)
{
    context.shapeWriter().startElement("g");
    context.shapeWriter().addAttribute("id", context.getID(group));
    context.shapeWriter().addAttribute("transform", SvgUtil::transformToString(group->transformation()));

    SvgStyleWriter::saveSvgStyle(group, context);

    QList<KoShape*> sortedShapes = group->shapes();
    qSort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    foreach(KoShape * shape, sortedShapes) {
        KoShapeGroup * childGroup = dynamic_cast<KoShapeGroup*>(shape);
        if (childGroup)
            saveGroup(childGroup, context);
        else
            saveShape(shape, context);
    }

    context.shapeWriter().endElement();
}

void SvgWriter::saveShape(KoShape *shape, SvgSavingContext &context)
{
    SvgShape *svgShape = dynamic_cast<SvgShape*>(shape);
    if (svgShape && svgShape->saveSvg(context))
        return;

    KoPathShape * path = dynamic_cast<KoPathShape*>(shape);
    if (path) {
        savePath(path, context);
    } else {
        // generic saving of shape via a switch element
        saveGeneric(shape, context);
    }
}

void SvgWriter::savePath(KoPathShape *path, SvgSavingContext &context)
{
    context.shapeWriter().startElement("path");
    context.shapeWriter().addAttribute("id", context.getID(path));
    context.shapeWriter().addAttribute("transform", SvgUtil::transformToString(path->transformation()));

    SvgStyleWriter::saveSvgStyle(path, context);

    context.shapeWriter().addAttribute("d", path->toString(context.userSpaceTransform()));
    context.shapeWriter().endElement();
}

void SvgWriter::saveGeneric(KoShape *shape, SvgSavingContext &context)
{
    const QRectF bbox = shape->boundingRect();

    // paint shape to the image
    KoShapePainter painter;
    painter.setShapes(QList<KoShape*>()<< shape);

    // generate svg from shape
    QBuffer svgBuffer;
    QSvgGenerator svgGenerator;
    svgGenerator.setOutputDevice(&svgBuffer);

    QPainter svgPainter;
    svgPainter.begin(&svgGenerator);
    painter.paint(svgPainter, SvgUtil::toUserSpace(bbox).toRect(), bbox);
    svgPainter.end();

    // remove anything before the start of the svg element from the buffer
    int startOfContent = svgBuffer.buffer().indexOf("<svg");
    if(startOfContent>0) {
        svgBuffer.buffer().remove(0, startOfContent);
    }

    // check if painting to svg produced any output
    if (svgBuffer.buffer().isEmpty()) {
        // prepare a transparent image, make it twice as big as the original size
        QImage image(2*bbox.size().toSize(), QImage::Format_ARGB32);
        image.fill(0);
        painter.paint(image);

        context.shapeWriter().startElement("image");
        context.shapeWriter().addAttribute("id", context.getID(shape));
        context.shapeWriter().addAttributePt("x", bbox.x());
        context.shapeWriter().addAttributePt("y", bbox.y());
        context.shapeWriter().addAttributePt("width", bbox.width());
        context.shapeWriter().addAttributePt("height", bbox.height());
        context.shapeWriter().addAttribute("xlink:href", context.saveImage(image));
        context.shapeWriter().endElement(); // image

    } else {
        context.shapeWriter().addCompleteElement(&svgBuffer);
    }

    // TODO: once we support saving single (flat) odf files
    // we can embed these here to have full support for generic shapes
}

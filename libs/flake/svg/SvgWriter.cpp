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
#include <KoXmlNS.h>

#include <QFile>
#include <QString>
#include <QTextStream>
#include <QBuffer>
#include <QPainter>
#include <QSvgGenerator>

#include <kis_debug.h>

SvgWriter::SvgWriter(const QList<KoShapeLayer*> &layers)
    : m_writeInlineImages(true)
{
    Q_FOREACH (KoShapeLayer *layer, layers)
        m_toplevelShapes.append(layer);
}

SvgWriter::SvgWriter(const QList<KoShape*> &toplevelShapes)
    : m_toplevelShapes(toplevelShapes)
    , m_writeInlineImages(true)
{
}

SvgWriter::~SvgWriter()
{

}

bool SvgWriter::save(const QString &filename, const QSizeF &pageSize, bool writeInlineImages)
{
    QFile fileOut(filename);
    if (!fileOut.open(QIODevice::WriteOnly))
        return false;

    m_writeInlineImages = writeInlineImages;

    const bool success = save(fileOut, pageSize);

    m_writeInlineImages = true;

    fileOut.close();

    return success;
}

bool SvgWriter::save(QIODevice &outputDevice, const QSizeF &pageSize)
{
    if (m_toplevelShapes.isEmpty()) {
        return false;
    }

    QTextStream svgStream(&outputDevice);
    svgStream.setCodec("UTF-8");

    // standard header:
    svgStream << "<?xml version=\"1.0\" standalone=\"no\"?>" << endl;
    svgStream << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" ";
    svgStream << "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" << endl;

    // add some PR.  one line is more than enough.
    svgStream << "<!-- Created using Krita: http://krita.org -->" << endl;

    svgStream << "<svg xmlns=\"http://www.w3.org/2000/svg\" \n";
    svgStream << "    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
    svgStream << QString("    xmlns:krita=\"%1\"\n").arg(KoXmlNS::krita);
    svgStream << "    xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n";
    svgStream << "    width=\"" << pageSize.width() << "pt\"\n";
    svgStream << "    height=\"" << pageSize.height() << "pt\"\n";
    svgStream << "    viewBox=\"0 0 "
              << pageSize.width() << " " << pageSize.height()
              << "\"";
    svgStream << ">" << endl;

    {
        SvgSavingContext savingContext(outputDevice, m_writeInlineImages);
        saveShapes(m_toplevelShapes, savingContext);
    }

    // end tag:
    svgStream << endl << "</svg>" << endl;

    return true;
}

bool SvgWriter::saveDetached(QIODevice &outputDevice)
{
    if (m_toplevelShapes.isEmpty())
        return false;

    SvgSavingContext savingContext(outputDevice, m_writeInlineImages);
    saveShapes(m_toplevelShapes, savingContext);

    return true;
}

bool SvgWriter::saveDetached(SvgSavingContext &savingContext)
{
    if (m_toplevelShapes.isEmpty())
        return false;

    saveShapes(m_toplevelShapes, savingContext);

    return true;
}

void SvgWriter::saveShapes(const QList<KoShape *> shapes, SvgSavingContext &savingContext)
{
    // top level shapes
    Q_FOREACH (KoShape *shape, shapes) {
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

void SvgWriter::saveLayer(KoShapeLayer *layer, SvgSavingContext &context)
{
    context.shapeWriter().startElement("g");
    context.shapeWriter().addAttribute("id", context.getID(layer));

    QList<KoShape*> sortedShapes = layer->shapes();
    std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    Q_FOREACH (KoShape * shape, sortedShapes) {
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

    SvgUtil::writeTransformAttributeLazy("transform", group->transformation(), context.shapeWriter());

    SvgStyleWriter::saveSvgStyle(group, context);

    QList<KoShape*> sortedShapes = group->shapes();
    std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    Q_FOREACH (KoShape * shape, sortedShapes) {
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

    SvgUtil::writeTransformAttributeLazy("transform", path->transformation(), context.shapeWriter());

    SvgStyleWriter::saveSvgStyle(path, context);

    context.shapeWriter().addAttribute("d", path->toString(context.userSpaceTransform()));
    context.shapeWriter().endElement();
}

void SvgWriter::saveGeneric(KoShape *shape, SvgSavingContext &context)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape);

    const QRectF bbox = shape->boundingRect();

    // paint shape to the image
    KoShapePainter painter;
    painter.setShapes(QList<KoShape*>()<< shape);

    // generate svg from shape
    QBuffer svgBuffer;
    QSvgGenerator svgGenerator;
    svgGenerator.setOutputDevice(&svgBuffer);

    /**
     * HACK ALERT: Qt (and Krita 3.x) has a weird bug, it assumes that all font sizes are
     *             defined in 96 ppi resolution, even though your the resolution in QSvgGenerator
     *             is manually set to 72 ppi. So here we do a tricky thing: we set a fake resolution
     *             to (72 * 72 / 96) = 54 ppi, which guarantees that the text, when painted in 96 ppi,
     *             will be actually painted in 72 ppi.
     *
     * BUG: 389802
     */
    if (shape->shapeId() == "TextShapeID") {
        svgGenerator.setResolution(54);
    }

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
        context.shapeWriter().addAttribute("x", bbox.x());
        context.shapeWriter().addAttribute("y", bbox.y());
        context.shapeWriter().addAttribute("width", bbox.width());
        context.shapeWriter().addAttribute("height", bbox.height());
        context.shapeWriter().addAttribute("xlink:href", context.saveImage(image));
        context.shapeWriter().endElement(); // image

    } else {
        context.shapeWriter().addCompleteElement(&svgBuffer);
    }

    // TODO: once we support saving single (flat) odf files
    // we can embed these here to have full support for generic shapes
}

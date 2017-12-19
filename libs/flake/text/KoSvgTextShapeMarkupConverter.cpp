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

#include "KoSvgTextShapeMarkupConverter.h"

#include "klocalizedstring.h"
#include "kis_assert.h"
#include "kis_debug.h"

#include <QBuffer>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <KoSvgTextShape.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoDocumentResourceManager.h>

#include <SvgParser.h>
#include <SvgWriter.h>
#include <SvgSavingContext.h>
#include <SvgGraphicContext.h>

#include <html/HtmlSavingContext.h>
#include <html/HtmlWriter.h>


struct KoSvgTextShapeMarkupConverter::Private {
    Private(KoSvgTextShape *_shape) : shape(_shape) {}

    KoSvgTextShape *shape;

    QStringList errors;
    QStringList warnings;

    void clearErrors() {
        errors.clear();
        warnings.clear();
    }
};

KoSvgTextShapeMarkupConverter::KoSvgTextShapeMarkupConverter(KoSvgTextShape *shape)
    : d(new Private(shape))
{
}

KoSvgTextShapeMarkupConverter::~KoSvgTextShapeMarkupConverter()
{
}

bool KoSvgTextShapeMarkupConverter::convertToSvg(QString *svgText, QString *stylesText)
{
    d->clearErrors();

    QBuffer shapesBuffer;
    QBuffer stylesBuffer;

    shapesBuffer.open(QIODevice::WriteOnly);
    stylesBuffer.open(QIODevice::WriteOnly);

    {
        SvgSavingContext savingContext(shapesBuffer, stylesBuffer);
        savingContext.setStrippedTextMode(true);
        SvgWriter writer({d->shape});
        writer.saveDetached(savingContext);
    }

    shapesBuffer.close();
    stylesBuffer.close();

    *svgText = QString::fromUtf8(shapesBuffer.data());
    *stylesText = QString::fromUtf8(stylesBuffer.data());

    return true;
}

bool KoSvgTextShapeMarkupConverter::convertFromSvg(const QString &svgText, const QString &stylesText,
                                                   const QRectF &boundsInPixels, qreal pixelsPerInch)
{

    qDebug() << "convertFromSvg. text:" << svgText << "styles:" << stylesText << "bounds:" << boundsInPixels << "ppi:" << pixelsPerInch;

    d->clearErrors();

    KoXmlDocument doc;
    QString errorMessage;
    int errorLine = 0;
    int errorColumn = 0;

    const QString fullText = QString("<svg>\n%1\n%2\n</svg>\n").arg(stylesText).arg(svgText);

    if (!doc.setContent(fullText, &errorMessage, &errorLine, &errorColumn)) {
        d->errors << QString("line %1, col %2: %3").arg(errorLine).arg(errorColumn).arg(errorMessage);
        return false;
    }

    d->shape->resetTextShape();

    KoDocumentResourceManager resourceManager;
    SvgParser parser(&resourceManager);
    parser.setResolution(boundsInPixels, pixelsPerInch);

    KoXmlElement root = doc.documentElement();
    KoXmlNode node = root.firstChild();

    bool textNodeFound = false;

    for (; !node.isNull(); node = node.nextSibling()) {
        KoXmlElement el = node.toElement();
        if (el.isNull()) continue;

        if (el.tagName() == "defs") {
            parser.parseDefsElement(el);
        }
        else if (el.tagName() == "text") {
            if (textNodeFound) {
                d->errors << i18n("More than one 'text' node found!");
                return false;
            }

            KoShape *shape = parser.parseTextElement(el, d->shape);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape == d->shape, false);
            textNodeFound = true;
            break;
        } else {
            d->errors << i18n("Unknown node of type \'%1\' found!", el.tagName());
            return false;
        }
    }

    if (!textNodeFound) {
        d->errors << i18n("No \'text\' node found!");
        return false;
    }

    return true;

}

bool KoSvgTextShapeMarkupConverter::convertToHtml(QString *htmlText)
{
    d->clearErrors();

    QBuffer shapesBuffer;
    shapesBuffer.open(QIODevice::WriteOnly);
    {
        HtmlWriter writer({d->shape});
        if (!writer.save(shapesBuffer)) {
            d->errors = writer.errors();
            d->warnings = writer.warnings();
            return false;
        }
    }

    shapesBuffer.close();

    *htmlText = QString(shapesBuffer.data());

    return true;
}

void convertCSSToSvgStyle(KoXmlWriter *svgWriter, const QString &styleDefinition)
{
    Q_FOREACH(const QString def, styleDefinition.split(";", QString::SkipEmptyParts)) {
        QStringList kv = def.split(":", QString::SkipEmptyParts);
        if (kv.size() != 2) continue;
        svgWriter->addAttribute(kv[0].toLatin1(), kv[1]);
    }
}


//bool convertBodyToSvgText(KoXmlWriter *svgWriter, KoXmlElement &e, const QRectF &boundsInPixels, qreal pixelsPerInch)
//{
//    qDebug() << "convertBodyToSvgText()" << e.tagName() << e.text() << e.isText() << e.isNull();

//    if (e.tagName() == "p" || e.tagName() == "span" || e.tagName() == "br") {
//        svgWriter->startElement("tspan");
//        if (e.hasAttribute("style")) {
//            convertCSSToSvgStyle(svgWriter, e.attribute("style"));
//        }
//        svgWriter->addTextNode(e.text());
//    }
//    if (e.hasChildNodes()) {
//        KoXmlNode node = e.firstChild();
//        for(; !node.isNull(); node = node.nextSibling()) {
//            KoXmlElement el = node.toElement();
//            qDebug() << el.tagName();
//            convertBodyToSvgText(svgWriter, el, boundsInPixels, pixelsPerInch);
//        }
//    }
//    if (e.tagName() == "p" || e.tagName() == "span" || e.tagName() == "br") {
//        svgWriter->endElement();
//    }

//    return true;
//}


bool KoSvgTextShapeMarkupConverter::convertFromHtml(const QString &htmlText,
                                                    const QRectF &boundsInPixels, qreal pixelsPerInch)
{

    qDebug() << ">>>>>>>>>>>" << htmlText;

    QBuffer svgBuffer;
    svgBuffer.open(QIODevice::WriteOnly);

    QXmlStreamReader htmlReader(htmlText);
    QXmlStreamWriter svgWriter(&svgBuffer);
    svgWriter.setAutoFormatting(false);

    int lineHeight = 0; // for the dY instruction
    QVector<int> lines;
    QStringRef elementName;
    while (!htmlReader.atEnd()) {
        QXmlStreamReader::TokenType token = htmlReader.readNext();
        switch (token) {
//        case QXmlStreamReader::StartDocument:
//            svgWriter.writeStartDocument();
//            break;
//        case QXmlStreamReader::EndDocument:
//            svgWriter.writeEndDocument();
//            break;
        case QXmlStreamReader::StartElement:
        {
            elementName = htmlReader.name();
            qDebug() << "start Element" << elementName;

            if (elementName == "body") {
                svgWriter.writeStartElement("text");
            }
            else if (elementName == "p" || elementName == "br") {
                // new line
                svgWriter.writeStartElement("tspan");
                svgWriter.writeAttribute("dy", "0");
            }
            else if (elementName == "span") {
                svgWriter.writeStartElement("tspan");
            }
            break;
        }
        case QXmlStreamReader::EndElement:
        {
            qDebug() << "end element" << htmlReader.name();
            if (elementName == "p" || elementName == "br") {
                lines << lineHeight;
                lineHeight = 0;
            }
            svgWriter.writeEndElement();
            break;
        }
        case QXmlStreamReader::Characters:
            qDebug() << htmlReader.text();
            if (elementName == "p" || elementName == "span") {
                svgWriter.writeCharacters(htmlReader.text().toUtf8());
            }
            break;
        default:
            qDebug() << "Default:" << htmlReader.name() << token;
            break;
        }
    }
    if (htmlReader.hasError()) {
        d->errors << htmlReader.errorString();
    }
    if (svgWriter.hasError()) {
        d->errors << i18n("Unknown error writing SVG text element");
    }

//    KoXmlDocument doc;
//    QString errorMessage;
//    int errorLine = 0;
//    int errorColumn = 0;
//    if (!doc.setContent(htmlText, &errorMessage, &errorLine, &errorColumn)) {
//        d->errors << QString("line %1, col %2: %3").arg(errorLine).arg(errorColumn).arg(errorMessage);
//        return false;
//    }

//    qDebug() << htmlText;

//    KoXmlWriter svgWriter(&svgBuffer);

//    // Convert html to svg
//    KoXmlElement root = doc.documentElement();
//    KoXmlNode node = root.firstChild();
//    for(; !node.isNull(); node = node.nextSibling()) {
//        KoXmlElement el = node.toElement();
//        qDebug() << el.tagName();
//        if (el.tagName() == "body") {

//            svgWriter.startElement("text");
//            if (el.hasAttribute("style")) {
//                convertCSSToSvgStyle(&svgWriter, el.attribute("style"));
//            }
//            // Start parsting body
//            if (!convertBodyToSvgText(&svgWriter, el, boundsInPixels, pixelsPerInch)) {
//                   return false;
//            }

//            svgWriter.endElement();
//        }
//    }

//    svgBuffer.close();

    QString styles;

    return convertFromSvg(QString::fromUtf8(svgBuffer.data()), styles, boundsInPixels, pixelsPerInch);
}

QStringList KoSvgTextShapeMarkupConverter::errors() const
{
    return d->errors;
}

QStringList KoSvgTextShapeMarkupConverter::warnings() const
{
    return d->warnings;
}


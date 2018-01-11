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

#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>

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

#include "kis_dom_utils.h"


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

    qDebug() << "\t\t" << *htmlText;

    return true;
}

bool KoSvgTextShapeMarkupConverter::convertFromHtml(const QString &htmlText, QString *svgText, QString *styles)
{

    qDebug() << ">>>>>>>>>>>" << htmlText;

    QBuffer svgBuffer;
    svgBuffer.open(QIODevice::WriteOnly);

    QXmlStreamReader htmlReader(htmlText);
    QXmlStreamWriter svgWriter(&svgBuffer);

    svgWriter.setAutoFormatting(true);

    QStringRef elementName;

    bool newLine = false;
    int lineCount = 0;
    QString bodyEm = "1em";
    QString em;
    QString p("p");
    //previous style string is for keeping formatting proper on linebreaks and appendstyle is for specific tags
    QString previousStyleString;
    QString appendStyle;

    while (!htmlReader.atEnd()) {
        QXmlStreamReader::TokenType token = htmlReader.readNext();
        switch (token) {
        case QXmlStreamReader::StartElement:
        {
            newLine = false;
            if (htmlReader.name() == "br") {
                qDebug() << "\tdoing br";
                svgWriter.writeEndElement();
                elementName = QStringRef(&p);
                em = bodyEm;
                appendStyle = previousStyleString;
            }
            else {
                elementName = htmlReader.name();
                em = "";
            }

            if (elementName == "body") {
                qDebug() << "\tstart Element" << elementName;
                svgWriter.writeStartElement("text");
                appendStyle = QString();
            }
            else if (elementName == "p") {
                // new line
                qDebug() << "\t\tstart Element" << elementName;
                svgWriter.writeStartElement("tspan");
                newLine = true;
                if (em.isEmpty()) {
                    em = bodyEm;
                    appendStyle = QString();
                }
                lineCount++;
            }
            else if (elementName == "span") {
                qDebug() << "\tstart Element" << elementName;
                svgWriter.writeStartElement("tspan");
                appendStyle = QString();
            }
            else if (elementName == "b" || elementName == "strong") {
                qDebug() << "\tstart Element" << elementName;
                svgWriter.writeStartElement("tspan");
                appendStyle = "font-weight:700;";
            }
            else if (elementName == "i" || elementName == "em") {
                qDebug() << "\tstart Element" << elementName;
                svgWriter.writeStartElement("tspan");
                appendStyle = "font-style:italic;";
            }
            else if (elementName == "u") {
                qDebug() << "\tstart Element" << elementName;
                svgWriter.writeStartElement("tspan");
                appendStyle = "text-decoration:underline";
            }

            QXmlStreamAttributes attributes = htmlReader.attributes();

            QString textAlign;
            if (attributes.hasAttribute("align")) {
                textAlign = attributes.value("align").toString();
            }

            if (attributes.hasAttribute("style")) {
                QString filteredStyles;
                QStringList svgStyles = QString("font-family font-size font-weight font-variant word-spacing text-decoration font-style font-size-adjust font-stretch direction").split(" ");
                QStringList styles = attributes.value("style").toString().split(";");
                for(int i=0; i<styles.size(); i++) {
                    QStringList style = QString(styles.at(i)).split(":");
                    qDebug()<<style.at(0);
                    if (svgStyles.contains(QString(style.at(0)).trimmed())) {
                        filteredStyles.append(styles.at(i)+";");
                    }

                    if (QString(style.at(0)).trimmed() == "color") {
                        filteredStyles.append(" fill:"+style.at(1)+";");
                    }

                    if (QString(style.at(0)).trimmed() == "text-align") {
                        textAlign = QString(style.at(1)).trimmed();
                    }

                    if (QString(style.at(0)).trimmed() == "line-height"){
                        if (style.at(1).contains("%")) {
                            double percentage = QString(style.at(1)).remove("%").toDouble();
                            em = QString::number(percentage/100.0)+"em";
                        } else if(style.at(1).contains("em")) {
                            em = style.at(1);
                        } else if(style.at(1).contains("px")) {
                            em = style.at(1);
                        }
                        if (elementName == "body") {
                            bodyEm = em;
                        }
                    }
                }

                if (textAlign == "center") {
                    filteredStyles.append(" text-anchor:middle;");
                } else if (textAlign == "right") {
                    filteredStyles.append(" text-anchor:end;");
                } else if (textAlign == "left"){
                    filteredStyles.append(" text-anchor:start;");
                }

                filteredStyles.append(appendStyle);

                if (!filteredStyles.isEmpty()) {
                    svgWriter.writeAttribute("style", filteredStyles);
                    previousStyleString = filteredStyles;
                }


            }
            if (newLine && lineCount > 1) {
                qDebug() << "\t\tAdvancing to the next line";
                svgWriter.writeAttribute("x", "0");
                svgWriter.writeAttribute("dy", em);
            }
            break;
        }
        case QXmlStreamReader::EndElement:
        {
            if (htmlReader.name() == "br") break;
            if (elementName == "p" || elementName == "span" || elementName == "body") {
                qDebug() << "\tEndElement" <<  htmlReader.name() << "(" << elementName << ")";
                svgWriter.writeEndElement();
            }
            break;
        }
        case QXmlStreamReader::Characters:
        {
            if (elementName == "style") {
                *styles = htmlReader.text().toString();
            }
            else {
                if (!htmlReader.isWhitespace()) {
                    qDebug() << "\tCharacters:" << htmlReader.text();
                    svgWriter.writeCharacters(htmlReader.text().toString());
                }
            }
            break;
        }
        default:
            ;
        }
    }

    if (htmlReader.hasError()) {
        d->errors << htmlReader.errorString();
        return false;
    }
    if (svgWriter.hasError()) {
        d->errors << i18n("Unknown error writing SVG text element");
        return false;
    }

    *svgText = QString::fromUtf8(svgBuffer.data());
    return true;
}

bool KoSvgTextShapeMarkupConverter::convertDocumentToSvg(const QTextDocument *doc, QString *svgText)
{
    QBuffer svgBuffer;
    svgBuffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter svgWriter(&svgBuffer);

    svgWriter.setAutoFormatting(true);

    QTextBlock block = doc->begin();

    /**
     * Find the most commonly used format.
     * This is what we put into the text-style.
     * We cannot just check the first textCharFormat we find
     * because users might be using things like drop-caps.
     */
    QList<int> formatFrequency;
    QList<QTextCharFormat> allFormats;
    allFormats.append(QTextCharFormat());
    formatFrequency.append(1);
    while (block.isValid()) {
        //copied from QTextDocument.cpp
        for (int f=0; f<block.textFormats().size(); f++) {
            for(int i=0; i<allFormats.size(); i++) {
                //props should proly compare itself to the main text format...
                QTextCharFormat diff = formatDifference(block.textFormats().at(f).format, allFormats.at(i)).toCharFormat();

                if (diff.properties().isEmpty()) {
                    formatFrequency[i] = formatFrequency.at(i) + block.textFormats().at(f).length;
                } else {
                    allFormats.append(block.textFormats().at(f).format);
                    formatFrequency.append(block.textFormats().at(f).length);
                }
            }
        }
        block = block.next();
    }
    QTextCharFormat mostCommonCharFormat = QTextCharFormat();
    int top = 0;
    for(int i=0; i<formatFrequency.size(); i++) {
        if (formatFrequency.at(i)>top) {
            top = formatFrequency.at(i);
            mostCommonCharFormat = allFormats.at(i);
        }
    }

    //Okay, now the actual writing.

    block = doc->begin();
    svgWriter.writeStartElement("text");
    svgWriter.writeAttribute("style", style(mostCommonCharFormat, block.blockFormat()));
    //insert the style of the first block and the first block text format into the text style.

    const QTextBlockFormat globalBlockFormat = block.blockFormat(); // the format of the first block is saved as a style

    int prevBlockLineHeight = globalBlockFormat.lineHeight();
    int prevBlockLineType = globalBlockFormat.lineHeightType();

    while (block.isValid()) {
        const QTextBlockFormat blockFormatDiff = formatDifference(block.blockFormat(), globalBlockFormat).toBlockFormat();

        const QTextLayout *layout = block.layout();
        const QTextLine line = layout->lineAt(0);

        svgWriter.writeStartElement("tspan");
        if (block.blockNumber() > 0) {
            const qreal lineHeightPt = line.height() * qreal(prevBlockLineHeight) / 100.0;

            svgWriter.writeAttribute("x", "0");
            svgWriter.writeAttribute("dy", KisDomUtils::toString(lineHeightPt) + "pt");
        }

        prevBlockLineHeight =
            blockFormatDiff.hasProperty(QTextFormat::LineHeight) ?
                blockFormatDiff.lineHeight() :
                globalBlockFormat.lineHeight();

        prevBlockLineType =
            blockFormatDiff.hasProperty(QTextFormat::LineHeightType) ?
                blockFormatDiff.lineHeightType() :
                globalBlockFormat.lineHeightType();

        if (prevBlockLineType == QTextBlockFormat::SingleHeight) {
            //single line will set lineHeight to 100%
            prevBlockLineHeight = 100;
        }

        QString text = block.text();
        const QVector<QTextLayout::FormatRange> formats = block.textFormats();

        if (formats.size()>1) {
            QStringList texts;
            QVector<QTextCharFormat> charFormats;
            for (int f=0; f<formats.size(); f++) {
                QString chunk;
                for (int c = 0; c<formats.at(f).length; c++) {
                    chunk.append(text.at(formats.at(f).start+c));
                }
                texts.append(chunk);
                charFormats.append(formats.at(f).format);
            }

            for (int c = 0; c<texts.size(); c++) {
                QTextCharFormat diff = formatDifference(charFormats.at(c), mostCommonCharFormat).toCharFormat();

                if (!diff.properties().isEmpty()) {
                    svgWriter.writeStartElement("tspan");
                    svgWriter.writeAttribute("style", style(diff, blockFormatDiff, mostCommonCharFormat));
                    svgWriter.writeCharacters(texts.at(c));
                    svgWriter.writeEndElement();
                } else {
                    svgWriter.writeCharacters(texts.at(c));
                }
            }

        } else {
            svgWriter.writeCharacters(text);
            //check format against
        }
        svgWriter.writeEndElement();

        block = block.next();
    }
    svgWriter.writeEndElement();//text root element.

    if (svgWriter.hasError()) {
        d->errors << i18n("Unknown error writing SVG text element");
        return false;
    }
    *svgText = QString::fromUtf8(svgBuffer.data()).trimmed();
    return true;
}

bool KoSvgTextShapeMarkupConverter::convertSvgToDocument(const QString &svgText, QTextDocument *doc)
{
    QXmlStreamReader svgReader(svgText.trimmed());
    doc->clear();
    QTextCursor cursor(doc);
    bool newBlock = false;
    QString dyString;
    QTextBlockFormat mainBlockFormat = cursor.blockFormat();
    QTextCharFormat mainCharFormat = cursor.charFormat();
    QTextCharFormat oldFormat = cursor.charFormat();

    while (!svgReader.atEnd()) {
        QXmlStreamReader::TokenType token = svgReader.readNext();
        switch (token) {
        case QXmlStreamReader::StartElement:
        {
            newBlock = false;
            if (svgReader.name() == "tspan") {
                if (svgReader.attributes().hasAttribute("dy")) {
                    newBlock = true;
                    oldFormat = mainCharFormat;
                } else {
                    oldFormat = cursor.charFormat();
                }

            }

            QString styleString;
            QString attributes;
            for (int a = 0; a<svgReader.attributes().size(); a++) {
                if (svgReader.attributes().at(a).name() != "style")
                    attributes.append(svgReader.attributes().at(a).name().toString())
                            .append(":")
                            .append(svgReader.attributes().at(a).value().toString())
                            .append(";");
            }
            if (attributes.endsWith(";")) {
                attributes.chop(1);
            }
            if (svgReader.attributes().hasAttribute("style")) {
                styleString = svgReader.attributes().value("style").toString();
                if (styleString.endsWith(";")) {
                    styleString.chop(1);
                }
            }
            if (styleString.size()>0 || attributes.size()>0) {
                //add attributes to parse them as part of the style.
                styleString.append(";")
                        .append(attributes);
                QStringList styles = styleString.split(";");
                QVector<QTextFormat> formats = stylesFromString(styles, cursor.charFormat(), cursor.blockFormat());
                cursor.mergeCharFormat(formats.at(0).toCharFormat());
                cursor.mergeBlockFormat(formats.at(1).toBlockFormat());

                if (svgReader.name()=="text") {
                    mainBlockFormat = cursor.blockFormat();
                    mainCharFormat = cursor.charFormat();
                    oldFormat = mainCharFormat;
                }
            }
            if (svgReader.attributes().hasAttribute("dy")) {
                dyString = svgReader.attributes().value("dy").toString();
            } else {
                dyString.clear();
            }

            //hack
            doc->setTextWidth(100);
            doc->setTextWidth(-1);
            QTextLine line = cursor.block().layout()->lineForTextPosition(cursor.positionInBlock());
            //Can't do this until there's a first QLine.
            int lineHeight = 100;
            if (!dyString.isEmpty()) {
                QTextBlockFormat format = cursor.blockFormat();
                if (cursor.block().layout()->lineCount()>0) {
                    //Always interpret no unit as points..
                    if (dyString.contains("pt")) {
                        dyString = dyString.remove("pt").trimmed();
                    }
                    qreal dy = dyString.toDouble();
                    lineHeight = (dy/line.height())*100;
                }
                format.setLineHeight(lineHeight, QTextBlockFormat::ProportionalHeight);
                cursor.setBlockFormat(format);
            }

            if (newBlock) {
                cursor.setBlockCharFormat(cursor.charFormat());
                cursor.insertBlock();
            }
            break;

        }
        case QXmlStreamReader::EndElement:
        {
            if (!newBlock && svgReader.name()!="text" ) {
                cursor.setCharFormat(oldFormat);
            }
            break;
        }
        case QXmlStreamReader::Characters:
        {
            if (!svgReader.isWhitespace()) {
                cursor.insertText(svgReader.text().toString());
            }

            break;
        }
        default:
            ;
        }
    }
    if (svgReader.hasError()) {
        d->errors << svgReader.errorString();
        return false;
    }
    doc->setModified(false);
    return true;
}

QStringList KoSvgTextShapeMarkupConverter::errors() const
{
    return d->errors;
}

QStringList KoSvgTextShapeMarkupConverter::warnings() const
{
    return d->warnings;
}

QString KoSvgTextShapeMarkupConverter::style(QTextCharFormat format, QTextBlockFormat blockFormat, QTextCharFormat mostCommon)
{
    QStringList style;
    for(int i=0; i<format.properties().size(); i++) {
        QString c;
        int propertyId = format.properties().keys().at(i);

        if (propertyId == QTextCharFormat::FontFamily) {
            c.append("font-family").append(":")
                    .append(format.properties()[propertyId].toString());
        }
        if (propertyId == QTextCharFormat::FontPointSize) {
            c.append("font-size").append(":")
                    .append(format.properties()[propertyId].toString()+"pt");
        }
        if (propertyId == QTextCharFormat::FontPixelSize) {
            c.append("font-size").append(":")
                    .append(format.properties()[propertyId].toString()+"px");
        }
        if (propertyId == QTextCharFormat::FontWeight) {
            //8 comes from QTextDocument...
            c.append("font-weight").append(":")
                    .append(QString::number(format.properties()[propertyId].toInt()*8));
        }
        if (propertyId == QTextCharFormat::FontItalic) {
            QString val = "italic";
            if (!format.fontItalic()) {
                val = "normal";
            }
            c.append("font-style").append(":")
                    .append(val);
        }

        if (propertyId == QTextCharFormat::FontCapitalization) {
            QString val;
            if (format.fontCapitalization() == QFont::SmallCaps){
                c.append("font-variant").append(":")
                        .append("small-caps");
            } else if (format.fontCapitalization() == QFont::AllUppercase) {
                c.append("text-transform").append(":")
                        .append("uppercase");
            } else if (format.fontCapitalization() == QFont::AllLowercase) {
                c.append("text-transform").append(":")
                        .append("lowercase");
            } else if (format.fontCapitalization() == QFont::Capitalize) {
                c.append("text-transform").append(":")
                        .append("capitalize");
            }
        }

        if (propertyId == QTextCharFormat::FontStretch) {
            c.append("font-stretch").append(":")
                    .append(format.properties()[propertyId].toString());
        }
        if (propertyId == QTextCharFormat::FontKerning) {
            QString val = "normal";
            if(!format.fontKerning()) {
                val = "none";
            }
            c.append("kerning").append(":")
                    .append(val);
        }
        if (propertyId == QTextCharFormat::FontWordSpacing) {
            c.append("word-spacing").append(":")
                    .append(QString::number(format.fontWordSpacing()));
        }
        if (propertyId == QTextCharFormat::FontLetterSpacing) {
            QString val;
            if (format.fontLetterSpacingType()==QFont::AbsoluteSpacing) {
                val = QString::number(format.fontLetterSpacing());
            } else {
                val = QString::number(((format.fontLetterSpacing()/100)*format.fontPointSize()));
            }
            c.append("letter-spacing").append(":")
                    .append(val);
        }
        if (propertyId == QTextCharFormat::TextOutline) {
            if (format.textOutline().color() != mostCommon.textOutline().color()) {
                c.append("stroke").append(":")
                        .append(format.textOutline().color().name());
                style.append(c);
                c.clear();
            }
            if (format.textOutline().width() != mostCommon.textOutline().width()) {
                c.append("stroke-width").append(":")
                        .append(QString::number(format.textOutline().width()));
            }
        }


        if (propertyId == QTextCharFormat::TextVerticalAlignment) {
            QString val = "baseline";
            if (format.verticalAlignment() == QTextCharFormat::AlignSubScript)
                val = QLatin1String("sub");
            else if (format.verticalAlignment() == QTextCharFormat::AlignSuperScript)
                val = QLatin1String("super");
                c.append("baseline-shift").append(":")
                        .append(val);
        }

        if (!c.isEmpty()) {
            style.append(c);
        }
    }
    //we might need a better check than 'isn't black'
    if (format.foreground().color()!= mostCommon.foreground().color()) {
        QString c;
        c.append("fill").append(":")
                .append(format.foreground().color().name());
        if (!c.isEmpty()) {
            style.append(c);
        }
    }

    if (format.hasProperty(QTextCharFormat::FontUnderline)
            || format.hasProperty(QTextCharFormat::FontOverline)
            || format.hasProperty(QTextCharFormat::FontStrikeOut)) {
        QString c;
        if (format.underlineStyle()!=QTextCharFormat::NoUnderline ||
                format.underlineStyle() != QTextCharFormat::SpellCheckUnderline) {
            QStringList values;

            if (format.hasProperty(QTextCharFormat::FontUnderline)) {

                values.append("underline");

            } else if(format.hasProperty(QTextCharFormat::FontOverline)) {
                values.append("overline");
            } else {
                values.append("strike-through");
            }
            c.append("text-decoration").append(":")
                    .append(values.join(" "));
        }
        if (!c.isEmpty()) {
            style.append(c);
        }
    }

    if (blockFormat.hasProperty(QTextBlockFormat::BlockAlignment)) {
        QString c;
        QString val;
        if (blockFormat.alignment()==Qt::AlignRight) {
            val = "end";
        } else if (blockFormat.alignment()==Qt::AlignCenter) {
            val = "middle";
        } else {
            val = "start";
        }
        c.append("text-anchor").append(":")
                .append(val);
        if (!c.isEmpty()) {
            style.append(c);
        }
    }

    return style.join("; ");
}

QVector<QTextFormat> KoSvgTextShapeMarkupConverter::stylesFromString(QStringList styles, QTextCharFormat currentCharFormat, QTextBlockFormat currentBlockFormat)
{
    QVector<QTextFormat> formats;
    QTextCharFormat charFormat;
    charFormat.setTextOutline(currentCharFormat.textOutline());
    QTextBlockFormat blockFormat;

    for (int i=0; i<styles.size(); i++) {
        if (!styles.at(i).isEmpty()){
            QStringList style = styles.at(i).split(":");
            QString property = style.at(0).trimmed();
            QString value = style.at(1).trimmed();

            if (property == "font-family") {
                charFormat.setFontFamily(value);
            }

            if (property == "font-size") {
                if (value.contains("%")) {
                    value = value.remove("%");
                    int fontSize = currentCharFormat.fontPointSize();
                    if (fontSize<0) {
                        fontSize = 10;
                    }
                    charFormat.setFontPointSize(qreal(value.toInt()/100*fontSize));
                } else if (value.contains("em")) {
                    value = value.remove("em");
                    int fontSize = currentCharFormat.fontPointSize();
                    if (fontSize<0) {
                        fontSize = 10;
                    }
                    charFormat.setFontPointSize(qreal(value.toInt()*fontSize));
                } else if (value.contains("px")) {
                    value = value.remove("px");
                    QFont font = charFormat.font();
                    font.setPixelSize(value.toInt());
                    charFormat.setFont(font);
                } else {
                    value = value.remove("pt");
                    charFormat.setFontPointSize(value.toDouble());
                }
            }

            if (property == "font-variant") {
                if (value=="small-caps") {
                    charFormat.setFontCapitalization(QFont::SmallCaps);
                } else {
                    charFormat.setFontCapitalization(QFont::MixedCase);
                }
            }

            if (property == "font-style") {
                if (value=="italic" || value=="oblique") {
                    charFormat.setFontItalic(true);
                } else {
                    charFormat.setFontItalic(false);
                }
            }

            if (property == "font-stretch") {
                charFormat.setFontStretch(value.toInt());
            }

            if (property == "font-weight") {
                charFormat.setFontWeight(value.toInt()/8);
            }

            if (property == "text-decoration") {
                charFormat.setFontUnderline(false);
                charFormat.setFontOverline(false);
                charFormat.setFontStrikeOut(false);
                if (value == "strike-through") {
                    charFormat.setFontStrikeOut(true);
                } else if (value == "overline") {
                    charFormat.setFontOverline(true);
                } else {
                    charFormat.setFontUnderline(true);
                }
            }

            if (property == "text-transform") {
                if (value == "uppercase") {
                    charFormat.setFontCapitalization(QFont::AllUppercase);
                } else if (value == "lowercase") {
                    charFormat.setFontCapitalization(QFont::AllLowercase);
                } else if (value == "capitalize") {
                    charFormat.setFontCapitalization(QFont::Capitalize);
                } else{
                    charFormat.setFontCapitalization(QFont::MixedCase);
                }
            }

            if (property == "letter-spacing") {
                charFormat.setFontLetterSpacing(value.toDouble());
            }

            if (property == "word-spacing") {
                charFormat.setFontWordSpacing(value.toDouble());
            }

            if (property == "kerning") {
                if (value=="normal") {
                    charFormat.setFontKerning(true);
                } else {
                    charFormat.setFontKerning(false);
                }
            }

            if (property == "stroke") {
                QPen pen = charFormat.textOutline();
                QColor color;
                color.setNamedColor(value);
                pen.setColor(color);
                charFormat.setTextOutline(pen);
            }

            if (property == "stroke-width") {
                QPen pen = charFormat.textOutline();
                pen.setWidth(value.toInt());
                charFormat.setTextOutline(pen);
            }

            if (property == "fill") {
                QBrush brush = currentCharFormat.foreground();
                QColor color;
                color.setNamedColor(value);
                brush.setColor(color);
                charFormat.setForeground(brush);
            }

            if (property == "text-anchor") {
                qDebug()<<"setting alignment"<< value;
                if (value == "end") {
                    blockFormat.setAlignment(Qt::AlignRight);
                } else if (value == "middle") {
                    blockFormat.setAlignment(Qt::AlignCenter);
                } else {
                    blockFormat.setAlignment(Qt::AlignLeft);
                }
            }

            if (property == "baseline-shift") {
                if (value == "super") {
                    charFormat.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                } else if (value == "sub") {
                    charFormat.setVerticalAlignment(QTextCharFormat::AlignSubScript);
                } else {
                    charFormat.setVerticalAlignment(QTextCharFormat::AlignNormal);
                }
            }
        }
    }

    formats.append(charFormat);
    formats.append(blockFormat);
    return formats;
}

QTextFormat KoSvgTextShapeMarkupConverter::formatDifference(QTextFormat test, QTextFormat reference)
{
    //copied from QTextDocument.cpp
    QTextFormat diff = test;
    //props should proly compare itself to the main text format...
    const QMap<int, QVariant> props = reference.properties();
    for (QMap<int, QVariant>::ConstIterator it = props.begin(), end = props.end();
         it != end; ++it)
        if (it.value() == test.property(it.key()))
            diff.clearProperty(it.key());
    return diff;
}


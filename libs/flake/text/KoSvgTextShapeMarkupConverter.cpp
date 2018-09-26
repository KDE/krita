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

#include <QStack>

#include <KoSvgTextShape.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoDocumentResourceManager.h>

#include <SvgParser.h>
#include <SvgWriter.h>
#include <SvgUtil.h>
#include <SvgSavingContext.h>
#include <SvgGraphicContext.h>

#include <html/HtmlSavingContext.h>
#include <html/HtmlWriter.h>

#include "kis_dom_utils.h"
#include <boost/optional.hpp>


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

void postCorrectBlockHeight(QTextDocument *doc,
                            qreal currLineAscent,
                            qreal prevLineAscent,
                            qreal prevLineDescent,
                            int prevBlockCursorPosition,
                            qreal currentBlockAbsoluteLineOffset)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(prevBlockCursorPosition >= 0);

    QTextCursor postCorrectionCursor(doc);
    postCorrectionCursor.setPosition(prevBlockCursorPosition);
    if (!postCorrectionCursor.isNull()) {
        const qreal relativeLineHeight =
                ((currentBlockAbsoluteLineOffset - currLineAscent + prevLineAscent) /
                 (prevLineAscent + prevLineDescent)) * 100.0;

        QTextBlockFormat format = postCorrectionCursor.blockFormat();
        format.setLineHeight(relativeLineHeight, QTextBlockFormat::ProportionalHeight);
        postCorrectionCursor.setBlockFormat(format);
        postCorrectionCursor = QTextCursor();
    }
}

QTextFormat findMostCommonFormat(const QList<QTextFormat> &allFormats)
{
    QTextCharFormat mostCommonFormat;

    QSet<int> propertyIds;

    /**
     * Get all existing property ids
     */
    Q_FOREACH (const QTextFormat &format, allFormats) {
        const QMap<int, QVariant> formatProperties = format.properties();
        Q_FOREACH (int id, formatProperties.keys()) {
            propertyIds.insert(id);
        }
    }

    /**
     * Filter out properties that do not exist in some formats. Otherwise, the
     * global format may override the default value used in these formats
     * (and yes, we do not have access to the default values to use them
     * in difference calculation algorithm
     */
    Q_FOREACH (const QTextFormat &format, allFormats) {
        for (auto it = propertyIds.begin(); it != propertyIds.end();) {
            if (!format.hasProperty(*it)) {
                it = propertyIds.erase(it);
            } else {
                ++it;
            }
        }
        if (propertyIds.isEmpty()) break;
    }

    if (!propertyIds.isEmpty()) {
        QMap<int, QMap<QVariant, int>> propertyFrequency;

        /**
         * Calculate the frequency of values used in *all* the formats
         */
        Q_FOREACH (const QTextFormat &format, allFormats) {
            const QMap<int, QVariant> formatProperties = format.properties();

            Q_FOREACH (int id, propertyIds) {
                KIS_SAFE_ASSERT_RECOVER_BREAK(formatProperties.contains(id));
                propertyFrequency[id][formatProperties.value(id)]++;
            }
        }

        /**
         * Add the most popular property value to the set of most common properties
         */
        for (auto it = propertyFrequency.constBegin(); it != propertyFrequency.constEnd(); ++it) {
            const int id = it.key();
            const QMap<QVariant, int> allValues = it.value();

            int maxCount = 0;
            QVariant maxValue;

            for (auto valIt = allValues.constBegin(); valIt != allValues.constEnd(); ++valIt) {
                if (valIt.value() > maxCount) {
                    maxCount = valIt.value();
                    maxValue = valIt.key();
                }
            }

            KIS_SAFE_ASSERT_RECOVER_BREAK(maxCount > 0);
            mostCommonFormat.setProperty(id, maxValue);
        }

    }

    return mostCommonFormat;
}

qreal calcLineWidth(const QTextBlock &block)
{
    const QString blockText = block.text();

    QTextLayout lineLayout;
    lineLayout.setText(blockText);
    lineLayout.setFont(block.charFormat().font());
    lineLayout.setFormats(block.textFormats());
    lineLayout.setTextOption(block.layout()->textOption());

    lineLayout.beginLayout();
    QTextLine fullLine = lineLayout.createLine();
    if (!fullLine.isValid()) {
        fullLine.setNumColumns(blockText.size());
    }
    lineLayout.endLayout();

    return lineLayout.boundingRect().width();
}

bool KoSvgTextShapeMarkupConverter::convertDocumentToSvg(const QTextDocument *doc, QString *svgText)
{
    QBuffer svgBuffer;
    svgBuffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter svgWriter(&svgBuffer);

    svgWriter.setAutoFormatting(true);


    qreal maxParagraphWidth = 0.0;
    QTextCharFormat mostCommonCharFormat;
    QTextBlockFormat mostCommonBlockFormat;

    struct LineInfo {
        LineInfo() {}
        LineInfo(QTextBlock _block, int _numSkippedLines)
            : block(_block), numSkippedLines(_numSkippedLines)
        {}

        QTextBlock block;
        int numSkippedLines = 0;
    };


    QVector<LineInfo> lineInfoList;

    {
        QTextBlock block = doc->begin();

        QList<QTextFormat> allCharFormats;
        QList<QTextFormat> allBlockFormats;

        int numSequentialEmptyLines = 0;

        while (block.isValid()) {
            if (!block.text().trimmed().isEmpty()) {
                lineInfoList.append(LineInfo(block, numSequentialEmptyLines));
                numSequentialEmptyLines = 0;

                maxParagraphWidth = qMax(maxParagraphWidth, calcLineWidth(block));

                allBlockFormats.append(block.blockFormat());
                Q_FOREACH (const QTextLayout::FormatRange &range, block.textFormats()) {
                    QTextFormat format =  range.format;
                    allCharFormats.append(format);
                }
            } else {
                numSequentialEmptyLines++;
            }

            block = block.next();
        }

        mostCommonCharFormat = findMostCommonFormat(allCharFormats).toCharFormat();
        mostCommonBlockFormat = findMostCommonFormat(allBlockFormats).toBlockFormat();
    }

    //Okay, now the actual writing.

    QTextBlock block = doc->begin();
    Q_UNUSED(block);

    svgWriter.writeStartElement("text");

    {
        const QString commonTextStyle = style(mostCommonCharFormat, mostCommonBlockFormat);
        if (!commonTextStyle.isEmpty()) {
            svgWriter.writeAttribute("style", commonTextStyle);
        }
    }

    int prevBlockRelativeLineSpacing = mostCommonBlockFormat.lineHeight();
    int prevBlockLineType = mostCommonBlockFormat.lineHeightType();
    qreal prevBlockAscent = 0.0;
    qreal prevBlockDescent= 0.0;

    Q_FOREACH (const LineInfo &info, lineInfoList) {
        QTextBlock block = info.block;

        const QTextBlockFormat blockFormatDiff = formatDifference(block.blockFormat(), mostCommonBlockFormat).toBlockFormat();
        QTextCharFormat blockCharFormatDiff = QTextCharFormat();
        const QVector<QTextLayout::FormatRange> formats = block.textFormats();
        if (formats.size()==1) {
            blockCharFormatDiff = formatDifference(formats.at(0).format, mostCommonCharFormat).toCharFormat();
        }

        const QTextLayout *layout = block.layout();
        const QTextLine line = layout->lineAt(0);

        svgWriter.writeStartElement("tspan");

        const QString text = block.text();

        /**
         * Mindblowing part: QTextEdit uses a hi-end algorithm for auto-estimation for the text
         * directionality, so the user expects his text being saved to SVG with the same
         * directionality. Just emulate behavior of direction="auto", which is not supported by
         * SVG 1.1
         *
         * BUG: 392064
         */

        bool isRightToLeft = false;
        for (int i = 0; i < text.size(); i++) {
            const QChar ch = text[i];
            if (ch.direction() == QChar::DirR || ch.direction() == QChar::DirAL) {
                isRightToLeft = true;
                break;
            } else if (ch.direction() == QChar::DirL) {
                break;
            }
        }


        if (isRightToLeft) {
            svgWriter.writeAttribute("direction", "rtl");
            svgWriter.writeAttribute("unicode-bidi", "embed");
        }

        {
            const QString blockStyleString = style(blockCharFormatDiff, blockFormatDiff);
            if (!blockStyleString.isEmpty()) {
                svgWriter.writeAttribute("style", blockStyleString);
            }
        }

        /**
         * The alignment rule will be inverted while rendering the text in the text shape
         * (accordign to the standard the alignment is defined not by "left" or "right",
         * but by "start" and "end", which inverts for rtl text)
         */
        Qt::Alignment blockAlignment = block.blockFormat().alignment();
        if (isRightToLeft) {
            if (blockAlignment & Qt::AlignLeft) {
                blockAlignment &= ~Qt::AlignLeft;
                blockAlignment |= Qt::AlignRight;
            } else if (blockAlignment & Qt::AlignRight) {
                blockAlignment &= ~Qt::AlignRight;
                blockAlignment |= Qt::AlignLeft;
            }
        }

        if (blockAlignment & Qt::AlignHCenter) {
            svgWriter.writeAttribute("x", KisDomUtils::toString(0.5 * maxParagraphWidth) + "pt");
        } else if (blockAlignment & Qt::AlignRight) {
            svgWriter.writeAttribute("x", KisDomUtils::toString(maxParagraphWidth) + "pt");
        } else {
            svgWriter.writeAttribute("x", "0");
        }

        if (block.blockNumber() > 0) {
            const qreal lineHeightPt =
                    line.ascent() - prevBlockAscent +
                    (prevBlockAscent + prevBlockDescent) * qreal(prevBlockRelativeLineSpacing) / 100.0;

            const qreal currentLineSpacing = (info.numSkippedLines + 1) * lineHeightPt;
            svgWriter.writeAttribute("dy", KisDomUtils::toString(currentLineSpacing) + "pt");
        }

        prevBlockRelativeLineSpacing =
                blockFormatDiff.hasProperty(QTextFormat::LineHeight) ?
                    blockFormatDiff.lineHeight() :
                    mostCommonBlockFormat.lineHeight();

        prevBlockLineType =
                blockFormatDiff.hasProperty(QTextFormat::LineHeightType) ?
                    blockFormatDiff.lineHeightType() :
                    mostCommonBlockFormat.lineHeightType();

        if (prevBlockLineType == QTextBlockFormat::SingleHeight) {
            //single line will set lineHeight to 100%
            prevBlockRelativeLineSpacing = 100;
        }

        prevBlockAscent = line.ascent();
        prevBlockDescent = line.descent();


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
                    svgWriter.writeAttribute("style", style(diff, QTextBlockFormat(), mostCommonCharFormat));
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
    }
    svgWriter.writeEndElement();//text root element.

    if (svgWriter.hasError()) {
        d->errors << i18n("Unknown error writing SVG text element");
        return false;
    }
    *svgText = QString::fromUtf8(svgBuffer.data()).trimmed();
    return true;
}

void parseTextAttributes(const QXmlStreamAttributes &elementAttributes,
                         QTextCharFormat &charFormat,
                         QTextBlockFormat &blockFormat)
{
    QString styleString;

    // we convert all the presentation attributes into styles
    QString presentationAttributes;
    for (int a = 0; a < elementAttributes.size(); a++) {
        if (elementAttributes.at(a).name() != "style") {
            presentationAttributes
                    .append(elementAttributes.at(a).name().toString())
                    .append(":")
                    .append(elementAttributes.at(a).value().toString())
                    .append(";");
        }
    }

    if (presentationAttributes.endsWith(";")) {
        presentationAttributes.chop(1);
    }

    if (elementAttributes.hasAttribute("style")) {
        styleString = elementAttributes.value("style").toString();
        if (styleString.endsWith(";")) {
            styleString.chop(1);
        }
    }

    if (!styleString.isEmpty() || !presentationAttributes.isEmpty()) {
        //add attributes to parse them as part of the style.
        styleString.append(";")
                .append(presentationAttributes);
        QStringList styles = styleString.split(";");
        QVector<QTextFormat> formats = KoSvgTextShapeMarkupConverter::stylesFromString(styles, charFormat, blockFormat);

        charFormat = formats.at(0).toCharFormat();
        blockFormat = formats.at(1).toBlockFormat();
    }
}

bool KoSvgTextShapeMarkupConverter::convertSvgToDocument(const QString &svgText, QTextDocument *doc)
{
    QXmlStreamReader svgReader(svgText.trimmed());
    doc->clear();
    QTextCursor cursor(doc);

    struct BlockFormatRecord {
        BlockFormatRecord() {}
        BlockFormatRecord(QTextBlockFormat _blockFormat,
                          QTextCharFormat _charFormat)
            : blockFormat(_blockFormat),
              charFormat(_charFormat)
        {}

        QTextBlockFormat blockFormat;
        QTextCharFormat charFormat;
    };

    QStack<BlockFormatRecord> formatStack;
    formatStack.push(BlockFormatRecord(cursor.blockFormat(), cursor.charFormat()));

    qreal currBlockAbsoluteLineOffset = 0.0;
    int prevBlockCursorPosition = -1;
    qreal prevLineDescent = 0.0;
    qreal prevLineAscent = 0.0;
    boost::optional<qreal> previousBlockAbsoluteXOffset = boost::none;

    while (!svgReader.atEnd()) {
        QXmlStreamReader::TokenType token = svgReader.readNext();
        switch (token) {
        case QXmlStreamReader::StartElement:
        {
            bool newBlock = false;
            QTextBlockFormat newBlockFormat;
            QTextCharFormat newCharFormat;
            qreal absoluteLineOffset = 1.0;

            // fetch format of the parent block and make it default
            if (formatStack.size() >= 2) {
                newBlockFormat = formatStack[formatStack.size() - 2].blockFormat;
                newCharFormat = formatStack[formatStack.size() - 2].charFormat;
            }

            {
                const QXmlStreamAttributes elementAttributes = svgReader.attributes();
                parseTextAttributes(elementAttributes, newCharFormat, newBlockFormat);

                // mnemonic for a newline is (dy != 0 && x == 0)

                boost::optional<qreal> blockAbsoluteXOffset = boost::none;

                if (elementAttributes.hasAttribute("x")) {
                    QString xString = elementAttributes.value("x").toString();
                    if (xString.contains("pt")) {
                        xString = xString.remove("pt").trimmed();
                    }
                    blockAbsoluteXOffset = KisDomUtils::toDouble(xString);
                }


                if (previousBlockAbsoluteXOffset &&
                    blockAbsoluteXOffset &&
                    qFuzzyCompare(*previousBlockAbsoluteXOffset, *blockAbsoluteXOffset) &&
                    svgReader.name() != "text" &&
                    elementAttributes.hasAttribute("dy")) {

                    QString dyString = elementAttributes.value("dy").toString();
                    if (dyString.contains("pt")) {
                        dyString = dyString.remove("pt").trimmed();
                    }

                    KIS_SAFE_ASSERT_RECOVER_NOOP(formatStack.isEmpty() == (svgReader.name() == "text"));

                    absoluteLineOffset = KisDomUtils::toDouble(dyString);
                    newBlock = absoluteLineOffset > 0;
                }

                previousBlockAbsoluteXOffset = blockAbsoluteXOffset;
            }

            //hack
            doc->setTextWidth(100);
            doc->setTextWidth(-1);

            if (newBlock && absoluteLineOffset > 0) {
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!formatStack.isEmpty(), false);
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(cursor.block().layout()->lineCount() == 1, false);

                QTextLine line = cursor.block().layout()->lineAt(0);

                if (prevBlockCursorPosition >= 0) {
                    postCorrectBlockHeight(doc, line.ascent(), prevLineAscent, prevLineDescent,
                                           prevBlockCursorPosition, currBlockAbsoluteLineOffset);
                }

                prevBlockCursorPosition = cursor.position();
                prevLineAscent  = line.ascent();
                prevLineDescent = line.descent();
                currBlockAbsoluteLineOffset = absoluteLineOffset;

                cursor.insertBlock();
                cursor.setCharFormat(formatStack.top().charFormat);
                cursor.setBlockFormat(formatStack.top().blockFormat);
            }



            cursor.mergeCharFormat(newCharFormat);
            cursor.mergeBlockFormat(newBlockFormat);

            formatStack.push(BlockFormatRecord(cursor.blockFormat(), cursor.charFormat()));

            break;
        }
        case QXmlStreamReader::EndElement:
        {
            if (svgReader.name() != "text") {
                formatStack.pop();
                KIS_SAFE_ASSERT_RECOVER(!formatStack.isEmpty()) { break; }

                cursor.setCharFormat(formatStack.top().charFormat);
                cursor.setBlockFormat(formatStack.top().blockFormat);
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
            break;
        }
    }

    if (prevBlockCursorPosition >= 0) {
        QTextLine line = cursor.block().layout()->lineAt(0);
        postCorrectBlockHeight(doc, line.ascent(), prevLineAscent, prevLineDescent,
                               prevBlockCursorPosition, currBlockAbsoluteLineOffset);
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
            style.append(c);
            c.clear();
            QFontMetricsF metrics(format.fontFamily());
            qreal xRatio = metrics.xHeight()/metrics.height();
            c.append("font-size-adjust").append(":").append(QString::number(xRatio));
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
                val = "0";
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
            if (format.verticalAlignment() == QTextCharFormat::AlignSubScript) {
                val = QLatin1String("sub");
            }
            else if (format.verticalAlignment() == QTextCharFormat::AlignSuperScript) {
                val = QLatin1String("super");
            }
            c.append("baseline-shift").append(":").append(val);
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

    if (format.underlineStyle() != QTextCharFormat::NoUnderline &&
        format.underlineStyle() != QTextCharFormat::SpellCheckUnderline) {

        QStringList values;
        QString c;

        if (format.fontUnderline()) {
            values.append("underline");
        }
        if(format.fontOverline()) {
            values.append("overline");
        }
        if(format.fontStrikeOut()) {
            values.append("line-through");
        }
        c.append("text-decoration").append(":")
                .append(values.join(" "));

        if (!values.isEmpty()) {
            style.append(c);
        }
    }

    if (blockFormat.hasProperty(QTextBlockFormat::BlockAlignment)) {
        // TODO: Alignment works incorrectly! The offsets should be calculated
        //       according to the shape width/height!

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
    Q_UNUSED(currentBlockFormat);

    QVector<QTextFormat> formats;
    QTextCharFormat charFormat;
    charFormat.setTextOutline(currentCharFormat.textOutline());
    QTextBlockFormat blockFormat;
    SvgGraphicsContext *context = new SvgGraphicsContext();

    for (int i=0; i<styles.size(); i++) {
        if (!styles.at(i).isEmpty()){
            QStringList style = styles.at(i).split(":");
            QString property = style.at(0).trimmed();
            QString value = style.at(1).trimmed();

            if (property == "font-family") {
                charFormat.setFontFamily(value);
            }

            if (property == "font-size") {
                qreal val = SvgUtil::parseUnitX(context, value);
                charFormat.setFontPointSize(val);
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
                QStringList values = value.split(" ");
                if (values.contains("line-through")) {
                    charFormat.setFontStrikeOut(true);
                }
                if (values.contains("overline")) {
                    charFormat.setFontOverline(true);
                }
                if(values.contains("underline")){
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
                qreal val = SvgUtil::parseUnitX(context, value);
                charFormat.setFontLetterSpacingType(QFont::AbsoluteSpacing);
                charFormat.setFontLetterSpacing(val);
            }

            if (property == "word-spacing") {
                qreal val = SvgUtil::parseUnitX(context, value);
                charFormat.setFontWordSpacing(val);
            }

            if (property == "kerning") {
                if (value=="normal") {
                    charFormat.setFontKerning(true);
                } else {
                    qreal val = SvgUtil::parseUnitX(context, value);
                    charFormat.setFontKerning(false);
                    charFormat.setFontLetterSpacingType(QFont::AbsoluteSpacing);
                    charFormat.setFontLetterSpacing(charFormat.fontLetterSpacing() + val);
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


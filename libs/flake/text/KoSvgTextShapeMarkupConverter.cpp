/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShapeMarkupConverter.h"

#include "klocalizedstring.h"
#include "kis_assert.h"
#include "kis_debug.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <text/KoFontRegistry.h>

#include <QBuffer>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextCodec>

#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>
#include <QFont>

#include <QStack>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QStringRef>
#else
#include <QStringView>
#endif

#include <KoSvgTextShape.h>
#include <KoXmlWriter.h>
#include <KoDocumentResourceManager.h>
#include <KoFontRegistry.h>

#include <SvgParser.h>
#include <SvgWriter.h>
#include <SvgUtil.h>
#include <SvgSavingContext.h>
#include <SvgGraphicContext.h>

#include <html/HtmlSavingContext.h>
#include <html/HtmlWriter.h>

#include "kis_dom_utils.h"
#include <boost/optional.hpp>

#include <FlakeDebug.h>

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

struct KoSvgTextShapeMarkupConverter::ExtraStyles {
    qreal inlineSize{0.0};
    WrappingMode wrappingMode{WrappingMode::QtLegacy};
};

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

    debugFlake << "convertFromSvg. text:" << svgText << "styles:" << stylesText << "bounds:" << boundsInPixels << "ppi:" << pixelsPerInch;

    d->clearErrors();

    QString errorMessage;
    int errorLine = 0;
    int errorColumn = 0;

    const QString fullText = QString("<svg>\n%1\n%2\n</svg>\n").arg(stylesText).arg(svgText);

    QDomDocument doc = SvgParser::createDocumentFromSvg(fullText, &errorMessage, &errorLine, &errorColumn);
    if (doc.isNull()) {
        d->errors << QString("line %1, col %2: %3").arg(errorLine).arg(errorColumn).arg(errorMessage);
        return false;
    }

    KoDocumentResourceManager resourceManager;
    SvgParser parser(&resourceManager);
    parser.setResolution(boundsInPixels, pixelsPerInch);

    QDomElement root = doc.documentElement();
    QDomNode node = root.firstChild();

    bool textNodeFound = false;

    for (; !node.isNull(); node = node.nextSibling()) {
        QDomElement el = node.toElement();
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

    debugFlake << "\t\t" << *htmlText;

    return true;
}

bool KoSvgTextShapeMarkupConverter::convertFromHtml(const QString &htmlText, QString *svgText, QString *styles)
{

    debugFlake << ">>>>>>>>>>>" << htmlText;

    QBuffer svgBuffer;
    svgBuffer.open(QIODevice::WriteOnly);

    QXmlStreamReader htmlReader(htmlText);
    QXmlStreamWriter svgWriter(&svgBuffer);

    svgWriter.setAutoFormatting(false);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QStringRef elementName;
#else
    QStringView elementName;
#endif

    bool newLine = false;
    int lineCount = 0;
    QString bodyEm = "1em";
    QString em;
    QString p("p");
    //previous style string is for keeping formatting proper on linebreaks and appendstyle is for specific tags
    QString previousStyleString;
    QString appendStyle;
    bool firstElement = true;

    while (!htmlReader.atEnd()) {
        QXmlStreamReader::TokenType token = htmlReader.readNext();
        QLatin1String elName = firstElement? QLatin1String("text"): QLatin1String("tspan");
        switch (token) {
        case QXmlStreamReader::StartElement:
        {
            newLine = false;
            if (htmlReader.name() == "br") {
                debugFlake << "\tdoing br";
                svgWriter.writeEndElement();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                elementName = QStringRef(&p);
#else
                elementName = QStringView(p);
#endif
                em = bodyEm;
                appendStyle = previousStyleString;
            }
            else {
                elementName = htmlReader.name();
                em = "";
            }

            if (elementName == "body") {
                debugFlake << "\tstart Element" << elementName;
                svgWriter.writeStartElement(elName);
                firstElement = false;
                appendStyle = QString();
            }
            else if (elementName == "p") {
                // new line
                debugFlake << "\t\tstart Element" << elementName;
                svgWriter.writeStartElement(elName);
                firstElement = false;
                newLine = true;
                if (em.isEmpty()) {
                    em = bodyEm;
                    appendStyle = QString();
                }
                lineCount++;
            }
            else if (elementName == "span") {
                debugFlake << "\tstart Element" << elementName;
                svgWriter.writeStartElement(elName);
                firstElement = false;
                appendStyle = QString();
            }
            else if (elementName == "b" || elementName == "strong") {
                debugFlake << "\tstart Element" << elementName;
                svgWriter.writeStartElement(elName);
                firstElement = false;
                appendStyle = "font-weight:700;";
            }
            else if (elementName == "i" || elementName == "em") {
                debugFlake << "\tstart Element" << elementName;
                svgWriter.writeStartElement(elName);
                firstElement = false;
                appendStyle = "font-style:italic;";
            }
            else if (elementName == "u") {
                debugFlake << "\tstart Element" << elementName;
                svgWriter.writeStartElement(elName);
                firstElement = false;
                appendStyle = "text-decoration:underline";
            }
            else if (elementName == "font") {
                debugFlake << "\tstart Element" << elementName;
                svgWriter.writeStartElement(elName);
                firstElement = false;
                appendStyle = QString();
                if (htmlReader.attributes().hasAttribute("color")) {
                    svgWriter.writeAttribute("fill", htmlReader.attributes().value("color").toString());
                }
            }
            else if (elementName == "pre") {
                debugFlake << "\tstart Element" << elementName;
                svgWriter.writeStartElement(elName);
                firstElement = false;
                appendStyle = "white-space:pre";
            }

            QXmlStreamAttributes attributes = htmlReader.attributes();

            QString textAlign;
            if (attributes.hasAttribute("align")) {
                textAlign = attributes.value("align").toString();
            }

            if (attributes.hasAttribute("style")) {
                QString filteredStyles;
                QStringList svgStyles = QString("font-family font-size font-weight font-variant word-spacing text-decoration font-style font-size-adjust font-stretch direction letter-spacing").split(" ");
                QStringList styles = attributes.value("style").toString().split(";");
                for(int i=0; i<styles.size(); i++) {
                    QStringList style = QString(styles.at(i)).split(":");
                    debugFlake<<style.at(0);
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
                debugFlake << "\t\tAdvancing to the next line";
                svgWriter.writeAttribute("x", "0");
                svgWriter.writeAttribute("dy", em);
            }
            break;
        }
        case QXmlStreamReader::EndElement:
        {
            if (htmlReader.name() == "br") break;
            if (elementName == "p" || elementName == "span" || elementName == "body") {
                debugFlake << "\tEndElement" <<  htmlReader.name() << "(" << elementName << ")";
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
                //TODO: Think up what to do with mix of pretty-print and <BR> (what libreoffice uses).
                //if (!htmlReader.isWhitespace()) {
                    debugFlake << "\tCharacters:" << htmlReader.text();
                    svgWriter.writeCharacters(htmlReader.text().toString());
                //}
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
        QMap<int, QList<std::pair<QVariant, int>>> propertyFrequency;

        /**
         * Calculate the frequency of values used in *all* the formats
         */
        Q_FOREACH (const QTextFormat &format, allFormats) {
            const QMap<int, QVariant> formatProperties = format.properties();

            Q_FOREACH (int id, propertyIds) {
                KIS_SAFE_ASSERT_RECOVER_BREAK(formatProperties.contains(id));
                QList<std::pair<QVariant, int>> &valueFrequencies = propertyFrequency[id];
                const QVariant formatPropValue = formatProperties.value(id);

                // Find the value in frequency table
                auto it = std::find_if(valueFrequencies.begin(), valueFrequencies.end(),
                [formatPropValue](const std::pair<QVariant, int> &element) { return element.first == formatPropValue; });

                if (it != valueFrequencies.end()) {
                    // Increase frequency by 1, if already met
                    it->second += 1;
                } else {
                    // Add with initial frequency of 1 if met for the first time
                    valueFrequencies.push_back({formatPropValue, 1});
                }
            }
        }

        /**
         * Add the most popular property value to the set of most common properties
         */
        for (auto it = propertyFrequency.constBegin(); it != propertyFrequency.constEnd(); ++it) {
            const int id = it.key();
            const QList<std::pair<QVariant, int>>& allValues = it.value();

            int maxCount = 0;
            QVariant maxValue;

            for (const auto& [propValue, valFrequency] : allValues) {
                if (valFrequency > maxCount) {
                    maxCount = valFrequency;
                    maxValue = propValue;
                }
            }

            KIS_SAFE_ASSERT_RECOVER_BREAK(maxCount > 0);
            mostCommonFormat.setProperty(id, maxValue);
        }

    }

    return mostCommonFormat;
}

Q_GUI_EXPORT int qt_defaultDpi();

qreal fixFromQtDpi(qreal value)
{
    // HACK ALERT: see a comment in convertDocumentToSvg!
    return value * 72.0 / qt_defaultDpi();
}

qreal fixToQtDpi(qreal value)
{
    // HACK ALERT: see a comment in convertDocumentToSvg!
    return value * qt_defaultDpi() / 72.0;
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

    return fixFromQtDpi(lineLayout.boundingRect().width());
}

/**
 * Mind blowing part: QTextEdit uses a hi-end algorithm for auto-estimation for the text
 * directionality, so the user expects his text being saved to SVG with the same
 * directionality. Just emulate behavior of direction="auto", which is not supported by
 * SVG 1.1
 *
 * BUG: 392064
 */
static bool guessIsRightToLeft(QStringView text) {
    // Is this just a worse version of QString::isRightToLeft??
    for (int i = 0; i < text.size(); i++) {
        const QChar ch = text[i];
        if (ch.direction() == QChar::DirR || ch.direction() == QChar::DirAL) {
            return true;
        } else if (ch.direction() == QChar::DirL) {
            return false;
        }
    }
    return false;
}

bool KoSvgTextShapeMarkupConverter::convertDocumentToSvg(const QTextDocument *doc, QString *svgText)
{
    QBuffer svgBuffer;
    svgBuffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter svgWriter(&svgBuffer);

    // disable auto-formatting to avoid extra spaces appearing here and there
    svgWriter.setAutoFormatting(false);


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

    const WrappingMode wrappingMode = getWrappingMode(doc->rootFrame()->frameFormat());

    /**
     * DIRTY-DIRTY-DIRTY HACK ALERT!!!
     *
     * All the internals of QTextDocument work with the primary screen's DPI.
     * That is, when we ask Qt about font metrics, it returns values scaled
     * to the pixels in the current screen, that is,
     * ptValue * qt_defaultDpi() / 72.0. We need to convert this value back
     * into points before writing as SVG. Therefore, all the metrics returned
     * by the document are scaled with `fixQtDpi()`.
     *
     * Official Qt's way to workaround this problem is to set logicalDpiX/Y()
     * values on the paint device's associated with the document. But it seems
     * like in our version of Qt (5.12.12, which is rather old) it doesn't work
     * properly.
     */

    QVector<LineInfo> lineInfoList;

    {
        QTextBlock block = doc->begin();

        QList<QTextFormat> allCharFormats;
        QList<QTextFormat> allBlockFormats;

        int numSequentialEmptyLines = 0;

        bool hasExplicitTextWidth = false;
        if (wrappingMode == WrappingMode::WhiteSpacePreWrap) {
            // If the doc is pre-wrap, we expect the inline-size to be set.
            if (std::optional<double> inlineSize = getInlineSize(doc->rootFrame()->frameFormat())) {
                if (*inlineSize > 0.0) {
                    hasExplicitTextWidth = true;
                    maxParagraphWidth = *inlineSize;
                }
            }
        }

        while (block.isValid()) {
            if (wrappingMode != WrappingMode::QtLegacy || !block.text().trimmed().isEmpty()) {
                lineInfoList.append(LineInfo(block, numSequentialEmptyLines));
                numSequentialEmptyLines = 0;

                if (!hasExplicitTextWidth) {
                    maxParagraphWidth = qMax(maxParagraphWidth, calcLineWidth(block));
                }

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

    svgWriter.writeStartElement("text");

    if (wrappingMode == WrappingMode::WhiteSpacePreWrap) {
        // There can only be one text direction for pre-wrap, so take that of
        // the first block.
        if (block.textDirection() == Qt::RightToLeft) {
            svgWriter.writeAttribute("direction", "rtl");
        }
    }

    {
        QString commonTextStyle = style(mostCommonCharFormat,
                                        mostCommonBlockFormat,
                                        {},
                                        /*includeLineHeight=*/wrappingMode != WrappingMode::QtLegacy);
        if (wrappingMode != WrappingMode::QtLegacy) {
            if (!commonTextStyle.isEmpty()) {
                commonTextStyle += "; ";
            }
            commonTextStyle += "white-space: pre";
            if (wrappingMode == WrappingMode::WhiteSpacePreWrap) {
                commonTextStyle += "-wrap;inline-size:";
                commonTextStyle += QString::number(maxParagraphWidth);
            }
        }
        if (!commonTextStyle.isEmpty()) {
            svgWriter.writeAttribute("style", commonTextStyle);
        }
    }

    // TODO: check if we should change into to float
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
            if (wrappingMode == WrappingMode::WhiteSpacePreWrap) {
                // For pre-wrap, be extra sure we are not writing text-anchor
                // to the `tspan`s because they don't do anything.
                blockCharFormatDiff.clearProperty(QTextBlockFormat::BlockAlignment);
            }
        }

        const QTextLayout *layout = block.layout();
        const QTextLine line = layout->lineAt(0);
        if (!line.isValid()) {
            // This layout probably has no lines at all. This can happen when
            // wrappingMode != QtLegacy and the text doc is completely empty.
            // It is safe to just skip the line. Trying to get its metrics will
            // crash.
            continue;
        }

        svgWriter.writeStartElement("tspan");

        const QString text = block.text();

        bool isRightToLeft;
        switch (block.textDirection()) {
        case Qt::LeftToRight:
            isRightToLeft = false;
            break;
        case Qt::RightToLeft:
            isRightToLeft = true;
            break;
        case Qt::LayoutDirectionAuto:
        default:
            // QTextBlock::textDirection() is not supposed to return these,
            // but just in case...
            isRightToLeft = guessIsRightToLeft(text);;
            break;
        }

        if (isRightToLeft && wrappingMode != WrappingMode::WhiteSpacePreWrap) {
            svgWriter.writeAttribute("direction", "rtl");
            svgWriter.writeAttribute("unicode-bidi", "embed");
        }

        {
            const QString blockStyleString = style(blockCharFormatDiff,
                                                   blockFormatDiff,
                                                   {},
                                                   /*includeLineHeight=*/wrappingMode != WrappingMode::QtLegacy);
            if (!blockStyleString.isEmpty()) {
                svgWriter.writeAttribute("style", blockStyleString);
            }
        }

        if (wrappingMode != WrappingMode::WhiteSpacePreWrap) {
            /**
             * The alignment rule will be inverted while rendering the text in the text shape
             * (according to the standard the alignment is defined not by "left" or "right",
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
        }

        if (wrappingMode == WrappingMode::QtLegacy && block.blockNumber() > 0) {
            qreal lineHeightPt =
                    fixFromQtDpi(line.ascent()) - prevBlockAscent +
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

        prevBlockAscent = fixFromQtDpi(line.ascent());
        prevBlockDescent = fixFromQtDpi(line.descent());


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
                const QString subStyle = style(diff, QTextBlockFormat(), mostCommonCharFormat);
                if (!subStyle.isEmpty()) {
                    svgWriter.writeStartElement("tspan");
                    svgWriter.writeAttribute("style", subStyle);
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

        // Add line-breaks for `pre` modes, but not for the final line.
        if (wrappingMode != WrappingMode::QtLegacy && &info != &lineInfoList.constLast()) {
            svgWriter.writeCharacters(QLatin1String("\n"));
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
                         QTextBlockFormat &blockFormat,
                         KoSvgTextShapeMarkupConverter::ExtraStyles &extraStyles)
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
        QVector<QTextFormat> formats =
            KoSvgTextShapeMarkupConverter::stylesFromString(styles, charFormat, blockFormat, extraStyles);

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
    formatStack.push(BlockFormatRecord(QTextBlockFormat(), QTextCharFormat()));
    cursor.setCharFormat(formatStack.top().charFormat);
    cursor.setBlockFormat(formatStack.top().blockFormat);

    qreal currBlockAbsoluteLineOffset = 0.0;
    int prevBlockCursorPosition = -1;
    Qt::Alignment prevBlockAlignment = Qt::AlignLeft;
    bool prevTspanHasTrailingLF = false;
    qreal prevLineDescent = 0.0;
    qreal prevLineAscent = 0.0;
    // work around uninitialized memory warning, therefore, no boost::none
    boost::optional<qreal> previousBlockAbsoluteXOffset =
        boost::optional<qreal>(false, qreal());

    std::optional<ExtraStyles> docExtraStyles;

    while (!svgReader.atEnd()) {
        QXmlStreamReader::TokenType token = svgReader.readNext();
        switch (token) {
        case QXmlStreamReader::StartElement:
        {
            prevTspanHasTrailingLF = false;

            bool newBlock = false;
            QTextBlockFormat newBlockFormat;
            QTextCharFormat newCharFormat;
            qreal absoluteLineOffset = 1.0;

            // fetch format of the parent block and make it default
            if (!formatStack.empty()) {
                newBlockFormat = formatStack.top().blockFormat;
                newCharFormat = formatStack.top().charFormat;
            }

            {
                ExtraStyles extraStyles{};
                const QXmlStreamAttributes elementAttributes = svgReader.attributes();
                parseTextAttributes(elementAttributes, newCharFormat, newBlockFormat, extraStyles);

                if (!docExtraStyles && svgReader.name() == QLatin1String("text")) {
                    if (extraStyles.inlineSize > 0.0) {
                        // There is a valid inline-size, forcing pre-wrap mode.
                        extraStyles.wrappingMode = WrappingMode::WhiteSpacePreWrap;
                    } else if (extraStyles.wrappingMode == WrappingMode::WhiteSpacePreWrap && extraStyles.inlineSize <= 0.0) {
                        // Without a valid inline-size, there is no point using
                        // pre-wrap, so change to pre.
                        extraStyles.wrappingMode = WrappingMode::WhiteSpacePre;
                    }
                    docExtraStyles = extraStyles;
                }

                // For WrappingMode::QtLegacy,
                // mnemonic for a newline is (dy != 0 && (x == prevX || alignmentChanged))

                // work around uninitialized memory warning, therefore, no
                // boost::none
                boost::optional<qreal> blockAbsoluteXOffset =
                    boost::make_optional(false, qreal());

                if (elementAttributes.hasAttribute("x")) {
                    QString xString = elementAttributes.value("x").toString();
                    if (xString.contains("pt")) {
                        xString = xString.remove("pt").trimmed();
                    }
                    blockAbsoluteXOffset = fixToQtDpi(KisDomUtils::toDouble(xString));
                }

                // Get current text alignment: If current block has alignment,
                // use it. Otherwise, try to inherit from parent block.
                Qt::Alignment thisBlockAlignment = Qt::AlignLeft;
                if (newBlockFormat.hasProperty(QTextBlockFormat::BlockAlignment)) {
                    thisBlockAlignment = newBlockFormat.alignment();
                } else if (!formatStack.empty()) {
                    thisBlockAlignment = formatStack.top().blockFormat.alignment();
                }

                const auto isSameXOffset = [&]() {
                    return previousBlockAbsoluteXOffset && blockAbsoluteXOffset
                        && qFuzzyCompare(*previousBlockAbsoluteXOffset, *blockAbsoluteXOffset);
                };
                if ((isSameXOffset() || thisBlockAlignment != prevBlockAlignment) && svgReader.name() != "text"
                    && elementAttributes.hasAttribute("dy")) {

                    QString dyString = elementAttributes.value("dy").toString();
                    if (dyString.contains("pt")) {
                        dyString = dyString.remove("pt").trimmed();
                    }

                    KIS_SAFE_ASSERT_RECOVER_NOOP(formatStack.isEmpty() == (svgReader.name() == "text"));

                    absoluteLineOffset = fixToQtDpi(KisDomUtils::toDouble(dyString));
                    newBlock = absoluteLineOffset > 0;
                }

                if (elementAttributes.hasAttribute("x")) {
                    previousBlockAbsoluteXOffset = blockAbsoluteXOffset;
                }
                prevBlockAlignment = thisBlockAlignment;
            }

            //hack
            doc->setTextWidth(100);
            doc->setTextWidth(-1);

            if (newBlock && absoluteLineOffset > 0) {
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!formatStack.isEmpty(), false);
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(cursor.block().layout()->lineCount() > 0, false);

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
                // For legacy wrapping mode, don't reset block format here
                // because this will break the block formats of the current
                // (last) block. The latest block format will be applied when
                // creating a new block.
                // However, resetting the block format is required for pre/pre-
                // wrap modes because they do not trigger the same code that
                // creates the new block; these modes rely on a trailing `\n`
                // at the end of a tspan to start a new block. At this point, if
                // there was indeed a trailing `\n`, this means we are already
                // in a new block.
                if (docExtraStyles && docExtraStyles->wrappingMode != WrappingMode::QtLegacy
                    && prevTspanHasTrailingLF) {
                    cursor.setBlockFormat(formatStack.top().blockFormat);
                }
                prevTspanHasTrailingLF = false;
            }
            break;
        }
        case QXmlStreamReader::Characters:
        {
            cursor.insertText(svgReader.text().toString());
            prevTspanHasTrailingLF = svgReader.text().endsWith('\n');
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

    {
        if (!docExtraStyles) {
            docExtraStyles = ExtraStyles{};
        }
        QTextFrameFormat f = doc->rootFrame()->frameFormat();
        setWrappingMode(&f, docExtraStyles->wrappingMode);
        setInlineSize(&f, docExtraStyles->inlineSize);
        doc->rootFrame()->setFrameFormat(f);
    }

    if (svgReader.hasError()) {
        d->errors << svgReader.errorString();
        return false;
    }
    doc->setModified(false);
    return true;
}

// PSD engine data parsing
QJsonValue readJsonValue(QIODevice &dev, bool array) {
    QJsonValue val;
    if (!array) {
        QJsonObject object;
        QString key;
        while(!dev.atEnd()) {
            QByteArray line = dev.readLine();
            if (line.isEmpty()) {
                break;
            }
            // hacky
            if (line.endsWith("\r")) {
                line = line.trimmed();
                line.append("\r");
            } else {
                line = line.trimmed();
            }


            if (line.startsWith("/")) {
                QList<QByteArray> keyVal = line.split(' ');
                if (keyVal.size() > 0) {
                    key = keyVal.first();
                    keyVal.removeFirst();
                    key.remove(0, 1);
                    //qDebug() << key;
                }
                if (keyVal.size() > 0) {
                    if (keyVal.first().startsWith("(")) {
                        line = keyVal.join(' ');
                        if (!line.endsWith(")")) {
                            QByteArray curLine = line;
                            while(!curLine.endsWith(")")) {
                                line = dev.readLine();
                                if (line.endsWith("\r")) {
                                    line = line.trimmed();
                                    line.append("\r");
                                } else {
                                    line = line.trimmed();
                                }
                                curLine.append(line);
                            }
                            line = curLine;
                        }
                        line = line.remove(0, 1);
                        line.chop(1);
                        QTextCodec *Utf16Codec = QTextCodec::codecForName("UTF-16BE");
                        object.insert(key, Utf16Codec->toUnicode(line));

                    } else if (keyVal.first().startsWith("[")) {
                            if (keyVal.size() > 1) {
                                QJsonArray array;
                                for (int i = 0; i < keyVal.size(); i++) {
                                    if (keyVal.at(i) != "[" ||
                                            keyVal.at(i) != "]") {
                                        bool res;
                                        float val = keyVal.at(i).toFloat(&res);
                                        if (res) {
                                            array.append(val);
                                        }
                                    }
                                }
                                object.insert(key, array);
                            } else {
                                object.insert(key, readJsonValue(dev, true));
                            }
                    } else {
                        line = keyVal.join(' ');
                        if (line == "true") {
                            object.insert(key, true);
                        } else if (line == "false") {
                            object.insert(key, false);
                        } else {
                            bool res;
                            float val = line.toFloat(&res);
                            if (res) {
                                object.insert(key, val);
                            } else {
                                object.insert(key, QString(line));
                            }
                        }
                    }
                }
            } else if (line == "[") {
                object.insert(key, readJsonValue(dev, true));
            } else if (line == "<<") {
                object.insert(key, readJsonValue(dev, false));
            } else if (line == ">>") {
                break;
            }
        }
        val = object;
    } else {
        QJsonArray array;
        while(!dev.atEnd()) {
            QByteArray line = dev.readLine();
            if (line.isEmpty()) {
                break;
            }
            line = line.trimmed();
            if (line == "[") {
                array.append(readJsonValue(dev, true));
            }

            if (line == "<<") {
                array.append(readJsonValue(dev, false));
            }

            if (line == "]") {
                break;
            }
        }
        val = array;
    }

    return val;
}

QColor colorFromPSDStyleSheet(QJsonObject color) {
    QColor c(Qt::black);
    if (color.value("Type").toInt() == 1) {
        QJsonArray values = color.value("Values").toArray();
        c = QColor::fromRgbF(values.at(1).toDouble(), values.at(2).toDouble(), values.at(3).toDouble(), values.at(0).toDouble());
    }
    return c;
}
QString stylesForPSDParagraphSheet(QJsonObject PSDParagraphSheet) {
    QStringList styles;

    for (int i=0; i < PSDParagraphSheet.keys().size(); i++) {
        const QString key = PSDParagraphSheet.keys().at(i);
        double val = PSDParagraphSheet.value(key).toDouble();
        if (key == "Justification") {
            QString textAlign = "start";
            switch (PSDParagraphSheet.value(key).toInt()) {
            case 0:
                textAlign = "start";
                break;
            case 1:
                textAlign = "end";
                break;
            case 2:
                textAlign = "center";
                break;
            case 3:
                textAlign = "justify start";
                break;
            case 4:
                textAlign = "justify end"; // guess
                break;
            case 5:
                textAlign = "justify center"; // guess
                break;
            case 6:
                textAlign = "justify";
                break;
            default:
                textAlign = "start";
            }

            styles.append("text-align:"+textAlign);
        } else if (key == "FirstLineIndent") {
            styles.append("text-indent:"+QString::number(val));
            continue;
        } else if (key == "StartIndent") {
            // left margin (also for rtl?)
            continue;
        } else if (key == "EndIndent") {
            // right margin (also for rtl?)
            continue;
        } else if (key == "SpaceBefore") {
            // top margin for paragraph
            continue;
        } else if (key == "SpaceAfter") {
            // bottom margin for paragraph
            continue;
        } else if (key == "AutoHyphenate") {
            // hyphenate: auto;
            continue;
        } else if (key == "HyphenatedWordSize") {
            // minimum wordsize at which to start hyphenating.
            continue;
        } else if (key == "PreHyphen") {
            // minimum number of letters before hyphenation is allowed to start in a word.
            // CSS-Text-4 hyphenate-limit-chars value 1.
            continue;
        } else if (key == "PostHyphen") {
            // minimum amount of letters a hyphnated word is allowed to end with.
            // CSS-Text-4 hyphenate-limit-chars value 2.
            continue;
        } else if (key == "ConsecutiveHyphens") {
            // maximum consequetive lines with hyphenation.
            // CSS-Text-4 hyphenate-limit-lines.
            continue;
        } else if (key == "Zone") {
            // Hyphenation zone to control where hyphenation is allowed to start.
            // CSS-Text-4 hyphenation-limit-zone.
            // Note: there's also a hyphenate capitalized words, but no idea which key.
            continue;
        } else if (key == "WordSpacing") {
            // val 0 is minimum allowed spacing, and val 2 is maximum allowed spacing, both for justified text.
            // 0 to 1000%, 100% default.
            val = PSDParagraphSheet.value(key).toArray()[1].toDouble();
            styles.append("word-spacing:"+QString::number(val));
            continue;
        } else if (key == "LetterSpacing") {
            // val 0 is minimum allowed spacing, and val 2 is maximum allowed spacing, both for justified text.
            // -100% to 500%, 0% default.
            val = PSDParagraphSheet.value(key).toArray()[1].toDouble();
            styles.append("letter-spacing:"+QString::number(val));
            continue;
        } else if (key == "GlyphSpacing") {
            // scaling of the glyphs, list of vals, 50% to 200%, default 100%.
            continue;
        } else if (key == "AutoLeading") {
            styles.append("line-height:"+QString::number(val));
            continue;
        } else if (key == "Hanging") {
            // Roman hanging punctuation, bool
            continue;
        } else if (key == "Burasagari") {
            // CJK hanging punctuation, bool
            if (PSDParagraphSheet.value(key).toBool()) {
                styles.append("hanging-punctuation:allow-end");
            }
            continue;
        } else if (key == "KinsokuOrder") {
            // strict vs loose linebreaking... sorta.
            continue;
        } else if (key == "EveryLineComposer") {
            // bool representing which text-wrapping method to use.
            //'single-line' is 'stable/greedy' line breaking,
            //'everyline' uses a penalty based system like Knuth's method.
            continue;
        } else {
            qWarning() << "Unsupported PSD paragraph style key" << key << PSDParagraphSheet.value(key);
        }
    }

    return styles.join("; ");
}

struct font_info_psd {
    font_info_psd() {
        weight = 400;
        width = 100;
    }
    QString familyName;
    int weight;
    int width {100};
    bool italic {false};
};

QString stylesForPSDStyleSheet(QJsonObject PSDStyleSheet, QMap<int, font_info_psd> fontNames) {
    QStringList styles;

    QStringList unsupportedStyles;

    int weight = 400;
    bool italic = false;
    QStringList textDecor;
    QStringList baselineShift;
    QStringList fontVariantLigatures;
    for (int i=0; i < PSDStyleSheet.keys().size(); i++) {
        const QString key = PSDStyleSheet.keys().at(i);
        if (key == "Font") {
            font_info_psd fontInfo = fontNames.value(PSDStyleSheet.value(key).toInt());
            weight = fontInfo.weight;
            italic = fontInfo.italic;
            styles.append("font-family:"+fontInfo.familyName);
            if (fontInfo.width != 100) {
                styles.append("font-width:"+QString::number(fontInfo.width));
            }
            continue;
        } else if (key == "FontSize") {
            // Note: FontSize might be in real points (72.27), but no idea where this is defined.
            styles.append("font-size:"+QString::number(PSDStyleSheet.value(key).toDouble()));
            continue;
        } else if (key == "AutoKerning") {
            if (!PSDStyleSheet.value(key).toBool()) {
                styles.append("font-kerning: none");
            }
            continue;
        } else if (key == "Kerning") {
            // adjusts kerning value, we don't support this.
            unsupportedStyles << key;
            continue;
        } else if (key == "FauxBold") {
            if (PSDStyleSheet.value(key).toBool()) {
                weight = 700;
            }
            continue;
        } else if (key == "FauxItalic") {
            if (PSDStyleSheet.value(key).toBool()) {
                italic = true;
            }
            // synthetic Italic, bool
            continue;
        } else if (key == "Leading") {
            bool autoleading = true;
            if (PSDStyleSheet.keys().contains("AutoLeading")) {
                autoleading = PSDStyleSheet.value("AutoLeading").toBool();
            }
            if (!autoleading) {
                double val = PSDStyleSheet.value(key).toDouble();
                styles.append("line-height:"+QString::number(val));
            }
            // value for line-height
            continue;
        } else if (key == "HorizontalScale") {
            // adjust horizontal scale of glyphs, we don't support this.
            unsupportedStyles << key;
            continue;
        } else if (key == "VerticalScale") {
            // adjusts vertical scale glyphs, we don't support this.
            unsupportedStyles << key;
            continue;
        } else if (key == "Tracking") {
            double fontSize = PSDStyleSheet.value("FontSize").toDouble();
            double letterSpacing = fontSize * (0.01 * PSDStyleSheet.value(key).toDouble());
            styles.append("letter-spacing:"+QString::number(letterSpacing));
            continue;
        } else if (key == "BaselineShift") {
            if (PSDStyleSheet.value(key).toDouble() > 0) {
                baselineShift.append(QString::number(PSDStyleSheet.value(key).toDouble()));
            }
            continue;
        } else if (key == "FontCaps") {
            switch (PSDStyleSheet.value(key).toInt()) {
            case 0:
                break;
            case 1:
                styles.append("text-transform:uppercase");
                break;
            case 2:
                styles.append("font-varriant-caps:all-small-caps");
                break;
            default:
                qDebug() << QString("Unknown value for %1:").arg(key) << PSDStyleSheet.value(key);
            }
            continue;
        } else if (key == "FontBaseline") {
            // NOTE: This might also be better done with font-variant-position, though
            // we don't support synthetic font stuff, including super and sub script.
            switch (PSDStyleSheet.value(key).toInt()) {
            case 0:
                break;
            case 1:
                baselineShift.append("super");
                break;
            case 2:
                baselineShift.append("sub");
                break;
            default:
                qDebug() << QString("Unknown value for %1:").arg(key) << PSDStyleSheet.value(key);
            }
            continue;
        } else if (key == "Underline") {
            if (PSDStyleSheet.value(key).toBool()) {
                textDecor.append("underline");
            }
            continue;
        } else if (key == "Strikethrough") {
            if (PSDStyleSheet.value(key).toBool()) {
                textDecor.append("line-through");
            }
            continue;
        } else if (key == "Ligatures") {
            // font-variant: common-ligatures
            if (!PSDStyleSheet.value(key).toBool()) {
                fontVariantLigatures.append("no-common-ligatures");
            }
            continue;
        } else if (key == "DLigatures") {
            // font-variant: discretionary ligatures
            if (PSDStyleSheet.value(key).toBool()) {
                fontVariantLigatures.append("discretionary-ligatures");
            }
            continue;
        } /* else if (key == "BaselineDirection") {
            // 1: vertical, 2: horizontal... but why is this a character style?
            continue;
        }*/ else if (key == "Tsume") {
            // Reduce spacing around a single character. Might be related to css-4-text text-spacing?
            unsupportedStyles << key;
            continue;
        }/* else if (key == "StyleRunAlignment") {
            // No idea?
            continue;
        } else if (key == "Language") {
            // This is an enum... which terrifies me.
            continue;
        }*/ else if (key == "NoBreak") {
            // Prevents word from breaking... I guess word-break???
            continue;
        }  else if (key == "FillColor") {
            bool fill = true;
            if (PSDStyleSheet.keys().contains("FillFlag")) {
                fill = PSDStyleSheet.value("FillFlag").toBool();
            }
            if (fill) {
                QJsonObject color = PSDStyleSheet.value(key).toObject();
                styles.append("fill:"+colorFromPSDStyleSheet(color).name());
            } else {
                styles.append("fill:none");
            }
        } else if (key == "StrokeColor") {
            bool fill = true;
            if (PSDStyleSheet.keys().contains("StrokeFlag")) {
                fill = PSDStyleSheet.value("StrokeFlag").toBool();
            }
            if (fill) {
                QJsonObject color = PSDStyleSheet.value(key).toObject();
                styles.append("stroke:"+colorFromPSDStyleSheet(color).name());
            } else {
                styles.append("stroke:none");
            }
            continue;
        } else if (key == "OutlineWidth") {
            styles.append("stroke-width:"+QString::number(PSDStyleSheet.value(key).toDouble()));
        } /*else if (key == "FillFirst") {
            // draw fill on top of stroke? paint-order: stroke markers fill, I guess.
            continue;
        } else if (key == "YUnderline") {
            // Option relating to vertical underline left or right
            continue;
        } else if (key == "CharacterDirection") {
            // text-orientation?
            continue;
        } else if (key == "HindiNumbers") {
            // bool, might be opentype feature.
            continue;
        }*/ else if (key == "Kashida") {
            // number, s related to drawing/inserting Kashida/Tatweel into Arabic justified text... We don't support this.
            unsupportedStyles << key;
            continue;
        } /*else if (key == "DiacriticPos") {
            // number, but no clue...
            continue;
        }*/ else {
            if (key != "FillFlag" && key != "StrokeFlag" && key == "AutoLeading") {
                qWarning() << "Unknown PSD character stylesheet style key" << key << PSDStyleSheet.value(key);
            }
        }
    }
    if (weight != 400) {
        styles.append("font-weight:"+QString::number(weight));
    }
    if (italic) {
        styles.append("font-style:italic");
    }
    if (!textDecor.isEmpty()) {
        styles.append("text-decoration:"+textDecor.join(" "));
    }
    if (!baselineShift.isEmpty()) {
        styles.append("baseline-shift:"+baselineShift.join(" "));
    }
    if (!fontVariantLigatures.isEmpty()) {
        styles.append("font-variant-ligatures:"+fontVariantLigatures.join(" "));
    }
    qWarning() << "Unsupported styles" << unsupportedStyles;
    return styles.join("; ");
}

bool KoSvgTextShapeMarkupConverter::convertPSDTextEngineDataToSVG(QByteArray ba, QString *svgText)
{
    debugFlake << "Convert from psd engine data";

    QJsonObject root;
    QBuffer dev(&ba);
    if (dev.open(QIODevice::ReadOnly)) {
        while(!dev.atEnd()) {
            QByteArray buf = dev.readLine();
            if (buf.trimmed() == "<<") {
                root = readJsonValue(dev, false).toObject();
            }
        }
        dev.close();
    } else {
        d->errors << dev.errorString();
        return false;
    }
    qDebug() << "Parsed JSON Object" << root;

    QJsonObject engineDict = root["EngineDict"].toObject();
    if (engineDict.isEmpty()) {
        d->errors << "No engine dict found in PSD engine data";
        return false;
    }

    QMap<int, font_info_psd> fontNames;
    QJsonObject resourceDict = root["ResourceDict"].toObject();
    if (resourceDict.isEmpty()) {
        d->errors << "No engine dict found in PSD engine data";
        return false;
    } else {
        // PSD only stores the postscript name, and we'll need a bit more information than that.
        QJsonArray fonts = resourceDict["FontSet"].toArray();
        for (int i = 0; i < fonts.size(); i++) {
            QJsonObject font = fonts.at(i).toObject();
            font_info_psd fontInfo;
            KoFontRegistry::instance()->getCssDataForPostScriptName(font.value("Name").toString(),
                                                                    &fontInfo.familyName,
                                                                    fontInfo.weight,
                                                                    fontInfo.width,
                                                                    fontInfo.italic);
            fontNames.insert(i, fontInfo);
        }
    }
    QString paragraphStyle;

    QJsonObject rendered = engineDict["Rendered"].toObject();
    // rendering info...
    if (!rendered.isEmpty()) {
        QJsonObject shapeChild = rendered["Shapes"].toObject()["Children"].toArray()[0].toObject();
        int shapeType = shapeChild["ShapeType"].toInt();
        if (shapeType == 1) {
            QJsonArray BoxBounds = shapeChild["Cookie"].toObject()["Photoshop"].toObject()["BoxBounds"].toArray();
            paragraphStyle = "inline-size:"+QString::number(BoxBounds[2].toInt());
        }
    }

    QPainterPath textCurve;
    bool reversed = false;
    QJsonObject curve = root["Curve"].toObject();
    if (!curve.isEmpty()) {
        QJsonArray points = curve["Points"].toArray();
        QJsonArray range = curve["TextOnPathTRange"].toArray();
        reversed = curve["Reversed"].toBool();
        int length = points.size()/8;
        for (int i = 0; i < length; i++) {
            int iAdjust = i*8;
            QPointF p1(points[iAdjust  ].toDouble(), points[iAdjust+1].toDouble());
            QPointF p2(points[iAdjust+2].toDouble(), points[iAdjust+3].toDouble());
            QPointF p3(points[iAdjust+4].toDouble(), points[iAdjust+5].toDouble());
            QPointF p4(points[iAdjust+6].toDouble(), points[iAdjust+7].toDouble());

            if (i == 0 || textCurve.currentPosition() != p1) {
                textCurve.moveTo(p1);
            }
            if (p1==p2 && p3==p4) {
                textCurve.lineTo(p4);
            } else {
                textCurve.cubicTo(p2, p3, p4);
            }
        }
        qDebug() << curve;
    }

    QJsonObject documentResources = root["DocumentResources"].toObject();

    QBuffer svgBuffer;
    svgBuffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter svgWriter(&svgBuffer);

    // disable auto-formatting to avoid axtra spaces appearing here and there
    svgWriter.setAutoFormatting(false);

    svgWriter.writeStartElement("text");

    QJsonObject editor = engineDict["Editor"].toObject();
    QString text = "";
    if (editor.isEmpty()) {
        d->errors << "No editor dict found in PSD engine data";
        return false;
    } else {
        text = editor["Text"].toString();
        text.replace("\r", "\n");
    }

    if (engineDict.contains("AntiAlias")) {
        int antiAliasing = engineDict["AntiAlias"].toInt();
        //0 = None, 1 = Sharp, 2 = Crisp, 3 = Strong, 4 = Smooth
        if (antiAliasing) {
            svgWriter.writeAttribute("text-rendering", "auto");
        } else {
            svgWriter.writeAttribute("text-rendering", "OptimizeSpeed");
        }
    }

    QJsonObject paragraphRun = engineDict["ParagraphRun"].toObject();
    if (!paragraphRun.isEmpty()) {
        //QJsonArray runLengthArray = paragraphRun.value("RunLengthArray").toArray();
        QJsonArray runArray = paragraphRun["RunArray"].toArray();
        QJsonObject styleSheet = runArray[0].toObject()["ParagraphSheet"].toObject()["Properties"].toObject();
        svgWriter.writeAttribute("style", stylesForPSDParagraphSheet(styleSheet));
    }

    if (!textCurve.isEmpty()) {
        svgWriter.writeStartElement("textPath");
        svgWriter.writeAttribute("path", KoPathShape::createShapeFromPainterPath(textCurve)->toString());
        if (reversed) {
            svgWriter.writeAttribute("side", "right");
        }
    }

    QJsonObject styleRun = engineDict.value("StyleRun").toObject();
    if (styleRun.isEmpty()) {
        d->errors << "No styleRun dict found in PSD engine data";
        return false;
    } else {
        QJsonArray runLengthArray = styleRun.value("RunLengthArray").toArray();
        QJsonArray runArray = styleRun.value("RunArray").toArray();
        if (runLengthArray.isEmpty() && runArray.isEmpty()) {
            d->errors << "No styleRun dict found in PSD engine data";
            return false;
        } else {
            QJsonObject styleSheet = runArray.at(0).toObject().value("StyleSheet").toObject().value("StyleSheetData").toObject();
            int length = 0;
            int pos = 0;
            for (int i = 0; i < runArray.size(); i++) {
                QJsonObject newStyle = runArray.at(i).toObject().value("StyleSheet").toObject().value("StyleSheetData").toObject();
                if (newStyle == styleSheet) {
                    length += runLengthArray.at(i).toInt();
                } else {
                    svgWriter.writeStartElement("tspan");
                    svgWriter.writeAttribute("style", stylesForPSDStyleSheet(styleSheet, fontNames));
                    svgWriter.writeCharacters(text.mid(pos, length));
                    svgWriter.writeEndElement();
                    styleSheet = newStyle;
                    pos += length;
                    length = runLengthArray.at(i).toInt();
                }
            }
            svgWriter.writeStartElement("tspan");
            svgWriter.writeAttribute("style", stylesForPSDStyleSheet(styleSheet, fontNames));
            svgWriter.writeCharacters(text.mid(pos));
            svgWriter.writeEndElement();
        }
    }

    if (!textCurve.isEmpty()) {
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

void writeJsonValue(QIODevice &dev, QJsonObject object, int indent) {
    QByteArray indentStringOld(indent, QChar::Tabulation);
    dev.write(indentStringOld);
    dev.write("<<\n");
    indent += 1;
    QByteArray indentString(indent, QChar::Tabulation);
    Q_FOREACH(QString key, object.keys()) {
        QJsonValue val = object[key];
        QString name = QString("/"+key);
        if (val.isObject()) {
            dev.write(indentString);
            dev.write((name+"\n").toLatin1());
            writeJsonValue(dev, val.toObject(), indent);
        } else if (val.isArray()) {
            QJsonArray array = val.toArray();
            if (array.at(0).isDouble()) {
                dev.write(indentString);
                dev.write((name+" [").toLatin1());
                for (int i=0; i<array.size(); i++) {
                    dev.write((" "+QString::number(array.at(i).toDouble())).toLatin1());
                }
                dev.write(" ]\n");
            } else if(array.isEmpty()) {
                dev.write(indentString);
                dev.write((name+" [ ]\n").toLatin1());
            } else {
                dev.write(indentString);
                dev.write((name+" [\n").toLatin1());
                for (int i=0; i<val.toArray().size(); i++) {
                    QJsonValue arrVal = val.toArray().at(i);
                    if (arrVal.isObject()) {
                        writeJsonValue(dev, arrVal.toObject(), indent);
                    }
                }
                dev.write(indentString);
                dev.write("]\n");
            }
        } else if (val.isString()) {
            QString newString = val.toString();
            newString.replace("\n", "\r");
            QTextCodec *Utf16Codec = QTextCodec::codecForName("UTF-16BE");
            dev.write(indentString);
            dev.write((name+" (").toLatin1());
            dev.write(Utf16Codec->fromUnicode(newString));
            dev.write(")\n");
        } else if (val.isBool()) {
            QString boolVal = val.toBool()? "true": "false";
            dev.write(indentString);
            dev.write((name+" "+boolVal+"\n").toLatin1());
        } else {
            dev.write(indentString);
            dev.write((name+" "+QString::number(val.toDouble())+"\n").toLatin1());
        }
    }
    dev.write(indentStringOld);
    dev.write(">>\n");
}

void gatherFonts(const QMap<QString, QString> cssStyles, const QString text, QJsonArray &fontSet,
                 QVector<int> &lengths, QVector<int> &fontIndices) {
    if (cssStyles.contains("font-family")) {
        QStringList families = cssStyles.value("font-family").split(",");
        qDebug() << families;
        int fontSize = cssStyles.value("font-size", "10").toInt();
        int fontWeight = cssStyles.value("font-weight", "400").toInt();
        int fontWidth = cssStyles.value("font-stretch", "400").toInt();

        const std::vector<FT_FaceUP> faces = KoFontRegistry::instance()->facesForCSSValues(families, lengths,
                                                      QMap<QString, qreal>(),
                                                      text, 72, 72, fontSize, 1.0,
                                                      fontWeight, fontWidth, false, 0, "");
        for (int i = 0; i < faces.size(); i++) {
            const FT_FaceUP &face = faces.at(static_cast<size_t>(i));
            QString postScriptName = face->family_name;
            if (FT_Get_Postscript_Name(face.data())) {
                postScriptName = FT_Get_Postscript_Name(face.data());
            }
            qDebug() << postScriptName;

            int fontIndex = -1;
            for(int j=0; j<fontSet.size(); j++) {
                if (fontSet[j].toObject()["Name"] == postScriptName) {
                    fontIndex = j;
                    break;
                }
            }
            if (fontIndex < 0) {
                QJsonObject font;
                font["Name"] = postScriptName;
                font["Script"] = 0;
                font["Synthetic"] = 0;
                font["FontType"] = 1;
                fontSet.append(font);
                fontIndex = fontSet.size()-1;
            }
            fontIndices << fontIndex;
        }
    }
}

QJsonObject styleToPSDStylesheet(const QMap<QString, QString> cssStyles,
                                 QJsonObject parentStyle) {
    QJsonObject styleSheet = parentStyle;

    styleSheet["Leading"] = 0.0;

    Q_FOREACH(QString key, cssStyles.keys()) {
        QString val = cssStyles.value(key);

        if (key == "font-size") {
            styleSheet["FontSize"] = val.toInt();
        } else if (key == "font-kerning") {
            if (val == "none") {
                styleSheet["AutoKerning"] = false;
            }
        } else if (key == "text-decoration") {
            QStringList decor = val.split(" ");
            Q_FOREACH(QString param, decor) {
                if (param == "underline") {
                    styleSheet["Underline"] = true;
                } else if (param == "line-through"){
                    styleSheet["Strikethrough"] = true;
                }
            }
        } else {
            qDebug() << "Unsupported css-style:" << key << val;
        }
    }

    return styleSheet;
}

void gatherFills(QDomElement el, QJsonObject &styleDict) {
    if (el.hasAttribute("fill")) {
        if (el.attribute("fill") != "none") {
            QColor c = QColor(el.attribute("fill"));
            double opacity = el.attribute("fill-opacity").toDouble();
            styleDict["FillFlag"] = true;
            styleDict["FillColor"] = QJsonObject {
            {"Type", 1},
            {"Values", QJsonArray({opacity, c.redF(), c.greenF(), c.blueF()})}
        };
        } else {
            styleDict["FillFlag"] = false;
        }
    }
    if (el.hasAttribute("stroke")) {
        if (el.attribute("stroke") != "none" && el.attribute("stroke-width").toDouble() != 0) {
            QColor c = QColor(el.attribute("stroke"));
            double opacity = el.attribute("stroke-opacity").toDouble();
            styleDict["StrokeFlag"] = true;
            styleDict["StrokeColor"] = QJsonObject {
            {"Type", 1},
            {"Values", QJsonArray({opacity, c.redF(), c.greenF(), c.blueF()})}
        };
        } else {
            styleDict["StrokeFlag"] = false;
        }
    }
    if (el.hasAttribute("stroke-width") && el.attribute("stroke-width").toDouble() != 0) {
        styleDict["OutlineWidth"] = el.attribute("stroke-width").toDouble();
    }
}

void gatherStyles(QDomElement el, QString &text, QJsonObject parentStyle, QMap<QString, QString> parentCssStyles, QJsonArray &styles, QJsonArray &runs, QJsonArray &fontSet) {
    QMap<QString, QString> cssStyles = parentCssStyles;
    if (el.hasAttribute("style")) {
        QString style = el.attribute("style");
        QStringList dummy = style.split(";");

        Q_FOREACH(QString style, dummy) {
            QString key = style.split(":").first().trimmed();
            QString val = style.split(":").last().trimmed();
            cssStyles.insert(key, val);
        }
        Q_FOREACH(QString attribute, KoSvgTextProperties::supportedXmlAttributes()) {
            if (el.hasAttribute(attribute)) {
                cssStyles.insert(attribute, el.attribute(attribute));
            }
        }
    }

    if (el.firstChild().isText()) {
        QDomText textNode = el.firstChild().toText();
        QString currentText = textNode.data();
        text += currentText;

        QJsonObject styleDict = styleToPSDStylesheet(cssStyles, parentStyle);
        gatherFills(el, styleDict);

        QVector<int> lengths;
        QVector<int> fontIndices;
        gatherFonts(cssStyles, currentText, fontSet, lengths, fontIndices);
        for (int i = 0; i< fontIndices.size(); i++) {
            QJsonObject curDict = styleDict;
            curDict["Font"] = fontIndices.at(i);
            styles.append(curDict);
            runs.append(QJsonValue{lengths.at(i)});
        }

    } else if (el.childNodes().size()>0) {
        QJsonObject styleDict = styleToPSDStylesheet(cssStyles, parentStyle);
        gatherFills(el, styleDict);
        QDomElement childEl = el.firstChildElement();
        while(!childEl.isNull()) {
            gatherStyles(childEl, text, styleDict, cssStyles, styles, runs, fontSet);
            childEl = childEl.nextSiblingElement();
        }
    }
}

QJsonObject gatherParagraphStyle(QDomElement el, QJsonObject defaultProperties) {
    QString cssStyle = el.attribute("style");
    QStringList dummy = cssStyle.split(";");
    QMap<QString, QString> cssStyles;
    Q_FOREACH(QString style, dummy) {
        QString key = style.split(":").first().trimmed();
        QString val = style.split(":").last().trimmed();
        cssStyles.insert(key, val);
    }

    QJsonObject paragraphStyleSheet = defaultProperties;
    Q_FOREACH(QString key, cssStyles.keys()) {
        QString val = cssStyles.value(key);

        if (key == "text-align") {
            int alignVal = 0;
            if (val == "start") {alignVal = 0;}
            if (val == "center") {alignVal = 2;}
            if (val == "end") {alignVal = 1;}
            if (val == "justify start") {alignVal = 3;}
            if (val == "justify center") {alignVal = 4;}
            if (val == "justify end") {alignVal = 5;}
            if (val == "justify") {alignVal = 6;}
            paragraphStyleSheet["Justification"] = alignVal;
        }
    }
    return QJsonObject{{"DefaultStyleSheet", 0},{"Properties", paragraphStyleSheet}};
}

bool KoSvgTextShapeMarkupConverter::convertToPSDTextEngineData(const QString &svgText, QByteArray *ba, QString &textTotal)
{
    QJsonObject root;

    QJsonObject engineDict;
    QJsonObject resourceDict;

    QString text;
    QJsonArray styles;
    QJsonArray styleRunArray;
    QJsonArray fontSet;

    // default fonts.
    fontSet.append(QJsonObject{{"Name", "AdobeInvisFont"}, {"FontType", 0}, {"Script", 0}, {"Synthetic", 0}});
    fontSet.append(QJsonObject{{"Name", "MyriadPro-Regular"}, {"FontType", 0}, {"Script", 0}, {"Synthetic", 0}});

    // We're creating the default character and paragraph style sheets here.
    QJsonObject defaultStyle;
    defaultStyle["Font"] = 1;
    defaultStyle["FontSize"] = 12;
    defaultStyle["FauxBold"] = false;
    defaultStyle["FauxItalic"] = false;
    defaultStyle["AutoLeading"] = true;
    defaultStyle["Leading"] = 0;
    defaultStyle["HorizontalScale"] = 1.0;
    defaultStyle["VerticalScale"] = 1.0;
    defaultStyle["Tracking"] = 0;
    defaultStyle["AutoKerning"] = true;
    defaultStyle["Kerning"] = 0;
    defaultStyle["BaselineShift"] = 0;
    defaultStyle["FontCaps"] = 0;
    defaultStyle["FontBaseline"] = 0;
    defaultStyle["Underline"] = false;
    defaultStyle["Strikethrough"] = false;
    defaultStyle["Ligatures"] = true;
    defaultStyle["DLigatures"] = false;
    defaultStyle["BaselineDirection"] = 2;
    defaultStyle["Tsume"] = 0.0;
    defaultStyle["StyleRunAlignment"] = 2;
    defaultStyle["Language"] = 0;
    defaultStyle["NoBreak"] = false;
    defaultStyle["FillColor"] =   QJsonObject{{"Type", 1}, {"Values", QJsonArray({1, 0, 0, 0})}};
    defaultStyle["StrokeColor"] = QJsonObject{{"Type", 1}, {"Values", QJsonArray({1, 0, 0, 0})}};
    defaultStyle["FillFlag"] = true;
    defaultStyle["StrokeFlag"] = false;
    defaultStyle["FillFirst"] = true;
    defaultStyle["YUnderline"] = 1;
    defaultStyle["OutlineWidth"] = 1.0;
    defaultStyle["CharacterDirection"] = 0;
    defaultStyle["HindiNumbers"] = false;
    defaultStyle["Kashida"] = 1;
    defaultStyle["DiacriticPos"] = 2;
    defaultStyle = styleToPSDStylesheet(KoSvgTextProperties::defaultProperties().convertToSvgTextAttributes(),
                                        defaultStyle);
    QVector<int> lengths;
    QVector<int> fontIndices;
    gatherFonts(KoSvgTextProperties::defaultProperties().convertToSvgTextAttributes(), "", fontSet, lengths, fontIndices);
    if (!fontIndices.isEmpty()) {
        defaultStyle["font"] = fontIndices.first();
    }

    QJsonObject defaultParagraph;
    defaultParagraph["Name"] = "Default";
    defaultParagraph["DefaultStyleSheet"] = 0;
    QJsonObject defaultParagraphProps = QJsonObject{
    {"Justification", 0},
    {"FirstLineIndent", 0},
    {"StartIndent", 0},
    {"EndIndent", 0},
    {"SpaceBefore", 0},
    {"AutoHyphenate", false},
    {"HyphenatedWordSize", 6},
    {"PreHyphen", 2},
    {"PostHyphen", 2},
    {"Zone", 36.0},
    {"WordSpacing", QJsonArray({0.8, 1.0, 1.3})},
    {"LetterSpacing", QJsonArray({0, 0, 0})},
    {"GlyphSpacing", QJsonArray({1.0, 1.0, 1.0})},
    {"SpaceAfter", 0},
    {"AutoLeading", 1.0},
    {"LeadingType", 0},
    {"Hanging", false},
    {"Burasagari", false},
    {"KinsokuOrder", 0},
    {"EveryLineComposer", false}};
    defaultParagraph["Properties"] = defaultParagraphProps;

    // go down the document children to get the style.
    QDomDocument doc;
    doc.setContent(svgText);
    gatherStyles(doc.documentElement(), text, QJsonObject(), QMap<QString, QString>(), styles, styleRunArray, fontSet);
    QJsonObject paragraphStyle = gatherParagraphStyle(doc.documentElement(), defaultParagraphProps);

    QJsonObject editor;
    editor["Text"] = text;
    engineDict["Editor"] = editor;

    QJsonObject grid;
    grid["GridIsOn"] = false;
    grid["ShowGrid"] = false;
    grid["GridSize"] = 18.0;
    grid["GridLeading"] = 22.0;
    grid["GridColor"] = QJsonObject{{"Type", 1}, {"Values", QJsonArray({0, 0, 0, 1})}};
    grid["GridLeadingFillColor"] = QJsonObject{{"Type", 1}, {"Values", QJsonArray({0, 0, 0, 1})}};
    grid["AlignLineHeightToGridFlags"] = false;
    engineDict["GridInfo"] = grid;

    QJsonObject paragraphRun;
    paragraphRun["RunLengthArray"] = QJsonArray({QJsonValue(text.length())});
    QJsonObject paragraphAdjustments  = QJsonObject {
    {"Axis", QJsonArray({1, 0, 1})},
    {"XY", QJsonArray({0, 0})}
    };
    paragraphRun["RunArray"] = QJsonArray({ QJsonObject{
                                                {"ParagraphSheet", paragraphStyle},
                                                {"Adjustments", paragraphAdjustments}
                                            }
                                            });
    paragraphRun["IsJoinable"] = 1; //no idea what this means.
    paragraphRun["DefaultRunData"] = QJsonObject{
    {"ParagraphSheet",
            QJsonObject{{"DefaultStyleSheet", 0},
                        {"Properties", QJsonObject()}} },
    {"Adjustments", paragraphAdjustments}};

    engineDict["ParagraphRun"] = paragraphRun;

    QJsonObject styleRun;
    styleRun["RunLengthArray"] = styleRunArray;
    QJsonArray properStyleRun;
    Q_FOREACH(QJsonValue entry, styles) {
        QJsonObject properStyle;
        properStyle["StyleSheetData"] = entry;
        QJsonObject s;
        s["StyleSheet"] = properStyle;
        properStyleRun.append(s);
    }
    styleRun["RunArray"] = properStyleRun;
    styleRun["IsJoinable"] = 2;

    styleRun["DefaultRunData"] = QJsonObject{{"StyleSheet", QJsonObject{{"StyleSheetData", QJsonObject()}} }};

    engineDict["StyleRun"] = styleRun;

    resourceDict["FontSet"] = fontSet;

    QJsonObject rendered;
    int shapeType = 0; // 0 point, 1 paragraph, but what does that make text-on-path?
    int writingDirection = 0;
    QJsonObject photoshop = QJsonObject {{"ShapeType", shapeType},
    {"TransformPoint0", QJsonArray({1.0, 0.0})},
    {"TransformPoint1", QJsonArray({0.0, 1.0})},
    {"TransformPoint2", QJsonArray({0.0, 0.0})}};
    if (shapeType == 0) {
        photoshop["PointBase"] = QJsonArray({0.0, 0.0});
    } else if (shapeType == 1) {
        // this is the bounding box of the paragraph shape.
        photoshop["BoxBounds"] = QJsonArray({0.0, 0.0, 100, 50});
    }
    QJsonObject renderChild = QJsonObject{
    {"ShapeType", shapeType},
    {"Procession", 0},
    {"Lines", QJsonObject{{"WritingDirection", writingDirection}, {"Children", QJsonArray()}}},
    {"Cookie", QJsonObject{{"Photoshop", photoshop}}}};
    rendered["Version"] = 1;
    rendered["Shapes"] = QJsonObject{{"WritingDirection", writingDirection}, {"Children", QJsonArray({renderChild})}};

    engineDict["Rendered"] = rendered;
    engineDict["UseFractionalGlyphWidths"] = true;
    engineDict["AntiAlias"] = 1;
    root["EngineDict"] = engineDict;

    // default resoure dict

    QJsonObject kinsokuHard;
    kinsokuHard["Name"] = "PhotoshopKinsokuHard";
    kinsokuHard["Hanging"] = "、。.,";
    kinsokuHard["Keep"] = "―‥";
    kinsokuHard["NoEnd"] = "‘“（〔［｛〈〰రะက尨[{￥＄£＠§〒＃";
    kinsokuHard["NoStart"] = "、。，．・：；？！ー―’”）〕］｝〉》」』】ヽヾゝゞ々ぁぃぅぇぉっゃゅょゎァィゥェォッャュョヮヵヶ゛゜?!\\⤀崀紀Ⰰ⸀㨀㬡̡ऀꋿԠ";
    QJsonObject kinsokuSoft;
    kinsokuSoft["Name"] = "PhotoshopKinsokuSoft";
    kinsokuSoft["Hanging"] = "、。.,";
    kinsokuSoft["Keep"] = "―‥";
    kinsokuSoft["NoEnd"] = "‘“（〔［｛〈〰రะ";
    kinsokuSoft["NoStart"] = "、。，．・：；？！’”）〕］｝〉》」』】ヽヾゝゞ々";
    resourceDict["KinsokuSet"] = QJsonArray({kinsokuHard, kinsokuSoft});
    resourceDict["MojiKumiSet"] = QJsonArray( {QJsonObject{{"InternalName", "Photoshop6MojiKumiSet1"}},
                                               QJsonObject{{"InternalName", "Photoshop6MojiKumiSet2"}},
                                               QJsonObject{{"InternalName", "Photoshop6MojiKumiSet3"}},
                                               QJsonObject{{"InternalName", "Photoshop6MojiKumiSet4"}}
                                              });
    resourceDict["SubscriptPosition"] = 0.333;
    resourceDict["SubscriptSize"] = 0.583;
    resourceDict["SuperscriptPosition"] = 0.333;
    resourceDict["SuperscriptSize"] = 0.583;
    resourceDict["SmallCapSize"] = 0.7;
    resourceDict["TheNormalParagraphSheet"] = 0;
    resourceDict["TheNormalStyleSheet"] = 0;

    QJsonObject resourceStyleSheet = QJsonObject{{"Name", "Default"}, {"StyleSheetData", defaultStyle}};
    resourceDict["StyleSheetSet"] = QJsonArray({resourceStyleSheet});
    resourceDict["ParagraphSheetSet"] = QJsonArray({defaultParagraph});

    // documentResources and ResourceDict always seem to be the same...?

    root["ResourceDict"] = resourceDict;
    root["DocumentResources"] = resourceDict;

    QBuffer dev(ba);
    if (dev.open(QIODevice::WriteOnly)) {
        int indent = 0;
        dev.write("\n\n");
        writeJsonValue(dev, root, indent);
        dev.close();
    } else {
        d->errors << dev.errorString();
        return false;
    }
    textTotal = text;

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

bool compareFormatUnderlineWithMostCommon(QTextCharFormat format, QTextCharFormat mostCommon)
{
    // color and style is not supported in rich text editor yet
    // TODO: support color and style
    return format.fontUnderline() == mostCommon.fontUnderline()
            && format.fontOverline() == mostCommon.fontOverline()
            && format.fontStrikeOut() == mostCommon.fontStrikeOut();
}

QString convertFormatUnderlineToSvg(QTextCharFormat format)
{
    // color and style is not supported in rich text editor yet
    // and text-decoration-line and -style and -color are not supported in svg render either
    // hence we just use text-decoration
    // TODO: support color and style
    QStringList line;

    if (format.fontUnderline()) {
        line.append("underline");
        if (format.underlineStyle() != QTextCharFormat::SingleUnderline) {
            warnFile << "Krita only supports solid underline style";
        }
    }

    if (format.fontOverline()) {
        line.append("overline");
    }

    if (format.fontStrikeOut()) {
        line.append("line-through");
    }

    if (line.isEmpty())
    {
        line.append("none");
    }

    QString c = QString("text-decoration").append(":")
            .append(line.join(" "));

    return c;
}

QString KoSvgTextShapeMarkupConverter::style(QTextCharFormat format,
                                             QTextBlockFormat blockFormat,
                                             QTextCharFormat mostCommon,
                                             const bool includeLineHeight)
{
    QStringList style;
    for(int i=0; i<format.properties().size(); i++) {
        QString c;
        int propertyId = format.properties().keys().at(i);

        if (propertyId == QTextCharFormat::FontFamily) {
            const QString fontFamily = format.properties()[propertyId].toString();
            c.append("font-family").append(":").append(fontFamily);
        }
        if (propertyId == QTextCharFormat::FontPointSize ||
            propertyId == QTextCharFormat::FontPixelSize) {

            // in Krita we unify point size and pixel size of the font

            c.append("font-size").append(":")
                    .append(format.properties()[propertyId].toString());
        }
        if (propertyId == QTextCharFormat::FontWeight) {
            // Convert from QFont::Weight range to SVG range,
            // as defined in qt's qfont.h
            int convertedWeight = 400; // Defaulting to Weight::Normal in svg scale

            switch (format.properties()[propertyId].toInt()) {
                case QFont::Weight::Thin:
                    convertedWeight = 100;
                    break;
                case QFont::Weight::ExtraLight:
                    convertedWeight = 200;
                    break;
                case QFont::Weight::Light:
                    convertedWeight = 300;
                    break;
                case QFont::Weight::Normal:
                    convertedWeight = 400;
                    break;
                case QFont::Weight::Medium:
                    convertedWeight = 500;
                    break;
                case QFont::Weight::DemiBold:
                    convertedWeight = 600;
                    break;
                case QFont::Weight::Bold:
                    convertedWeight = 700;
                    break;
                case QFont::Weight::ExtraBold:
                    convertedWeight = 800;
                    break;
                case QFont::Weight::Black:
                    convertedWeight = 900;
                    break;
                default:
                    warnFile << "WARNING: Invalid QFont::Weight value supplied to KoSvgTextShapeMarkupConverter::style.";
                    break;
            }

            c.append("font-weight").append(":")
                    .append(QString::number(convertedWeight));
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
            QString valueString = QString::number(format.fontStretch(), 10);
            if (format.fontStretch() == QFont::ExtraCondensed) {
                valueString = "extra-condensed";
            } else if (format.fontStretch() == QFont::SemiCondensed) {
                valueString = "semi-condensed";
            } else if (format.fontStretch() == QFont::Condensed) {
                valueString = "condensed";
            } else if (format.fontStretch() == QFont::AnyStretch) {
                valueString = "normal";
            } else if (format.fontStretch() == QFont::Expanded) {
                valueString = "expanded";
            } else if (format.fontStretch() == QFont::SemiExpanded) {
                valueString = "semi-expanded";
            }  else if (format.fontStretch() == QFont::ExtraExpanded) {
                valueString = "extra-expanded";
            }  else if (format.fontStretch() == QFont::UltraExpanded) {
                valueString = "ultra-expanded";
            }
            c.append("font-stretch").append(":")
                    .append(valueString);
        }
        if (propertyId == QTextCharFormat::FontKerning) {
            QString val;
            if (format.fontKerning()) {
                val = "auto";
            } else {
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

        if (propertyId == QTextCharFormat::ForegroundBrush) {
            QColor::NameFormat colorFormat;

            if (format.foreground().color().alphaF() < 1.0) {
                colorFormat = QColor::HexArgb;
            } else {
                colorFormat = QColor::HexRgb;
            }

            c.append("fill").append(":")
                    .append(format.foreground().color().name(colorFormat));
        }

        if (!c.isEmpty()) {
            style.append(c);
        }
    }

    if (!compareFormatUnderlineWithMostCommon(format, mostCommon)) {

        QString c = convertFormatUnderlineToSvg(format);
        if (!c.isEmpty()) {
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

    if (includeLineHeight && blockFormat.hasProperty(QTextBlockFormat::LineHeight)) {
        double h = 0;
        if (blockFormat.lineHeightType() == QTextBlockFormat::ProportionalHeight) {
            h = blockFormat.lineHeight() / 100.0;
        } else if (blockFormat.lineHeightType() == QTextBlockFormat::SingleHeight) {
            h = -1.0;
        }
        QString c = "line-height:";
        if (h >= 0) {
            c += QString::number(blockFormat.lineHeight() / 100.0);
        } else {
            c += "normal";
        }
        style.append(c);
    }

    return style.join("; ");
}

QVector<QTextFormat> KoSvgTextShapeMarkupConverter::stylesFromString(QStringList styles,
                                                                     QTextCharFormat currentCharFormat,
                                                                     QTextBlockFormat currentBlockFormat,
                                                                     ExtraStyles &extraStyles)
{
    Q_UNUSED(currentBlockFormat);

    QVector<QTextFormat> formats;
    QTextCharFormat charFormat;
    charFormat.setTextOutline(currentCharFormat.textOutline());
    QTextBlockFormat blockFormat;
    QScopedPointer<SvgGraphicsContext> context(new SvgGraphicsContext());
    const KoSvgTextProperties resolved = KoSvgTextProperties::defaultProperties();

    for (int i=0; i<styles.size(); i++) {
        if (!styles.at(i).isEmpty()){
            QStringList style = styles.at(i).split(":");
            // ignore the property instead of crashing,
            // if user forgets to separate property name and value with ':'.
            if (style.size() < 2) {
                continue;
            }

            QString property = style.at(0).trimmed();
            QString value = style.at(1).trimmed();

            if (property == "font-family") {
                charFormat.setFontFamily(value);
            }

            if (property == "font-size") {
                qreal val = SvgUtil::parseUnitX(context.data(), resolved, value);
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
                if (value == "ultra-condensed") {
                    charFormat.setFontStretch(QFont::UltraCondensed);
                } else if (value == "condensed") {
                    charFormat.setFontStretch(QFont::Condensed);
                } else if (value == "semi-condensed") {
                    charFormat.setFontStretch(QFont::SemiCondensed);
                } else if (value == "normal") {
                    charFormat.setFontStretch(100);
                } else if (value == "semi-expanded") {
                    charFormat.setFontStretch(QFont::SemiExpanded);
                } else if (value == "expanded") {
                    charFormat.setFontStretch(QFont::Expanded);
                } else if (value == "extra-expanded") {
                    charFormat.setFontStretch(QFont::ExtraExpanded);
                } else if (value == "ultra-expanded") {
                    charFormat.setFontStretch(QFont::UltraExpanded);
                } else { // "normal"
                    charFormat.setFontStretch(value.toInt());
                }
            }

            if (property == "font-weight") {
                // Convert from SVG range to QFont::Weight range,
                // as defined in qt's qfont.h
                int convertedWeight = QFont::Weight::Normal; // Defaulting to Weight::Normal

                switch (value.toInt()) {
                    case 100:
                        convertedWeight = QFont::Weight::Thin;
                        break;
                    case 200:
                        convertedWeight = QFont::Weight::ExtraLight;
                        break;
                    case 300:
                        convertedWeight = QFont::Weight::Light;
                        break;
                    case 400:
                        convertedWeight = QFont::Weight::Normal;
                        break;
                    case 500:
                        convertedWeight = QFont::Weight::Medium;
                        break;
                    case 600:
                        convertedWeight = QFont::Weight::DemiBold;
                        break;
                    case 700:
                        convertedWeight = QFont::Weight::Bold;
                        break;
                    case 800:
                        convertedWeight = QFont::Weight::ExtraBold;
                        break;
                    case 900:
                        convertedWeight = QFont::Weight::Black;
                        break;
                    default:
                        warnFile << "WARNING: Invalid weight value supplied to KoSvgTextShapeMarkupConverter::stylesFromString.";
                        break;
                }

                charFormat.setFontWeight(convertedWeight);
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
                qreal val = SvgUtil::parseUnitX(context.data(), resolved, value);
                charFormat.setFontLetterSpacingType(QFont::AbsoluteSpacing);
                charFormat.setFontLetterSpacing(val);
            }

            if (property == "word-spacing") {
                qreal val = SvgUtil::parseUnitX(context.data(), resolved, value);
                charFormat.setFontWordSpacing(val);
            }

            if (property == "kerning") {
                if (value == "auto") {
                    charFormat.setFontKerning(true);
                } else {
                    qreal val = SvgUtil::parseUnitX(context.data(), resolved, value);
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
                QColor color;
                color.setNamedColor(value);

                // avoid assertion failure in `KoColor` later
                if (!color.isValid()) {
                    continue;
                }

                // default color is #ff000000, so default alpha will be 1.0
                qreal currentAlpha = charFormat.foreground().color().alphaF();

                // if alpha was already defined by `fill-opacity` prop
                if (currentAlpha < 1.0) {
                    // and `fill` doesn't have alpha component
                    if (color.alphaF() < 1.0) {
                        color.setAlphaF(currentAlpha);
                    }
                }

                charFormat.setForeground(color);
            }

            if (property == "fill-opacity") {
                QColor color = charFormat.foreground().color();
                bool ok = true;
                qreal alpha = qBound(0.0, SvgUtil::fromPercentage(value, &ok), 1.0);

                // if conversion fails due to non-numeric input,
                // it defaults to 0.0, default to current alpha instead
                if (!ok) {
                    alpha = color.alphaF();
                }
                color.setAlphaF(alpha);
                charFormat.setForeground(color);
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

            if (property == "line-height") {
                double lineHeightPercent = -1.0;
                bool ok = false;
                if (value.endsWith('%')) {
                    // Note: Percentage line-height behaves differently than
                    // unitless number in case of nested descendant elements,
                    // but here we pretend they are the same.
                    lineHeightPercent = value.left(value.length() - 1).toDouble(&ok);
                    if (!ok) {
                        lineHeightPercent = -1.0;
                    }
                } else if(const double unitless = value.toDouble(&ok); ok) {
                    lineHeightPercent = unitless * 100.0;
                } else if (value == QLatin1String("normal")) {
                    lineHeightPercent = -1.0;
                    blockFormat.setLineHeight(1, QTextBlockFormat::SingleHeight);
                }
                if (lineHeightPercent >= 0) {
                    blockFormat.setLineHeight(lineHeightPercent, QTextBlockFormat::ProportionalHeight);
                }
            }

            if (property == "inline-size") {
                const qreal val = SvgUtil::parseUnitX(context.data(), resolved, value);
                if (val > 0.0) {
                    extraStyles.inlineSize = val;
                }
            }

            if (property == "white-space") {
                if (value == QLatin1String("pre")) {
                    extraStyles.wrappingMode = WrappingMode::WhiteSpacePre;
                } else if (value == QLatin1String("pre-wrap")) {
                    extraStyles.wrappingMode = WrappingMode::WhiteSpacePreWrap;
                } else {
                    extraStyles.wrappingMode = WrappingMode::QtLegacy;
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
        if (it.value() == test.property(it.key())) {
            // Some props must not be removed as default state gets in the way.
            switch (it.key()) {
            case QTextFormat::TextUnderlineStyle: // 0x2023
            case QTextFormat::FontLetterSpacingType: // 0x2033 in Qt5, but is 0x1FE9 in Qt6
            case QTextFormat::LineHeightType:
                continue;
            }
            diff.clearProperty(it.key());
        }
    return diff;
}

KoSvgTextShapeMarkupConverter::WrappingMode
KoSvgTextShapeMarkupConverter::getWrappingMode(const QTextFrameFormat &frameFormat)
{
    const QVariant wrappingMode = frameFormat.property(WrappingModeProperty);
    if (wrappingMode.userType() != QMetaType::Int) {
        return WrappingMode::QtLegacy;
    }
    return static_cast<WrappingMode>(wrappingMode.toInt());
}

void KoSvgTextShapeMarkupConverter::setWrappingMode(QTextFrameFormat *frameFormat, WrappingMode wrappingMode)
{
    frameFormat->setProperty(WrappingModeProperty, static_cast<int>(wrappingMode));
}

std::optional<double> KoSvgTextShapeMarkupConverter::getInlineSize(const QTextFrameFormat &frameFormat)
{
    const QVariant inlineSize = frameFormat.property(InlineSizeProperty);
    if (inlineSize.userType() != QMetaType::Double) {
        return {};
    }
    const double val = inlineSize.toDouble();
    if (val > 0.0) {
        return {val};
    }
    return {};
}

void KoSvgTextShapeMarkupConverter::setInlineSize(QTextFrameFormat *frameFormat, double inlineSize)
{
    if (inlineSize >= 0.0) {
        frameFormat->setProperty(InlineSizeProperty, inlineSize);
    } else {
        frameFormat->clearProperty(InlineSizeProperty);
    }
}

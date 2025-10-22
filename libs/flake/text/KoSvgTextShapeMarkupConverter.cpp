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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QBuffer>
#include <QTextCodec>
#include <QtMath>

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

#include <KoColor.h>

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

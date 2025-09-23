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
#include <QVariantList>
#include <QVariantHash>
#include <QVariant>
#include <QJsonObject>
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
#include <KoPathSegment.h>
#include <KoPathPoint.h>
#include <KoXmlWriter.h>
#include <KoDocumentResourceManager.h>
#include <KoFontRegistry.h>
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

QColor colorFromPSDStyleSheet(QVariantHash color, const KoColorSpace *imageCs) {
    QColor c(Qt::black);
    if (color.keys().contains("/Color")) {
        color = color["/Color"].toHash();
    }
    QDomDocument doc;
    QDomElement root;
    QVariantList values = color.value("/Values").toList();
    if (color.value("/Type").toInt() == 0) { //graya
        root = doc.createElement("Gray");
        root.setAttribute("g", values.at(1).toDouble());
    } else if (color.value("/Type").toInt() == 2) { // CMYK
        root = doc.createElement("CMYK");
        root.setAttribute("c", values.value(1).toDouble());
        root.setAttribute("m", values.value(2).toDouble());
        root.setAttribute("y", values.value(3).toDouble());
        root.setAttribute("k", values.value(4).toDouble());
    } else if (color.value("/Type").toInt() == 3) { // LAB
        root = doc.createElement("Lab");
        root.setAttribute("L", values.value(1).toDouble());
        root.setAttribute("a", values.value(2).toDouble());
        root.setAttribute("b", values.value(3).toDouble());
    } else if (color.value("/Type").toInt() == 1) {
        root = doc.createElement("RGB");
        root.setAttribute("r", values.value(1).toDouble());
        root.setAttribute("g", values.value(2).toDouble());
        root.setAttribute("b", values.value(3).toDouble());
    }
    KoColor final = KoColor::fromXML(root, "U8");
    if (final.colorSpace()->colorModelId() == imageCs->colorModelId()) {
        final.setProfile(imageCs->profile());
    }
    final.toQColor(&c);
    return c;
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

// language is one of pt, pt-BR, fr, fr-CA, de, de-1901, gsw, nl, en-UK, en-US, fi, it, nb, nn, es, sv
static QHash <int, QString> psdLanguageMap {
    {0, "en-US"},   // US English
    {1, "fi"},      // Finnish
    {2, "fr"},      // French
    {3, "fr-CA"},   // Canadian French
    {4, "de"},      // German
    {5, "de-1901"}, // German before spelling reform
    {6, "gsw"},     // Swiss German
    {7, "it"},      // Italian
    {8, "nb"},      //Norwegian
    {9, "nn"},      // Norsk (nynorsk)
    {10, "pt"},     // Portuguese
    {11, "pt-BR"},  // Brazilian Portuguese
    {12, "es"},     // Spansh
    {13, "sv"},     // Swedish
    {14, "en-UK"},  // British English
    {15, "nl"},     // Dutch
    {16, "da"},     // Danish
    //{17, ""},
    {18, "ru"},     // Russian
    //{19, ""},
    //{20, ""},
    //{21, ""},
    {22, "cs"},     // Czech
    {23, "pl"},     // Polish
    //{24, ""},
    {25, "el"},     // Greek
    {26, "tr"},     // Turkish
    //{27, ""},
    {28, "hu"},     // Hungarian
};

QString stylesForPSDStyleSheet(QString &lang, QVariantHash PSDStyleSheet, QMap<int, font_info_psd> fontNames, QTransform scale, const KoColorSpace *imageCs) {
    QStringList styles;

    QStringList unsupportedStyles;

    int weight = 400;
    bool italic = false;
    QStringList textDecor;
    QStringList baselineShift;
    QStringList fontVariantLigatures;
    QStringList fontVariantNumeric;
    QStringList fontVariantCaps;
    QStringList fontVariantEastAsian;
    QStringList fontFeatureSettings;
    QString underlinePos;
    for (int i=0; i < PSDStyleSheet.keys().size(); i++) {
        const QString key = PSDStyleSheet.keys().at(i);
        if (key == "/Font") {
            font_info_psd fontInfo = fontNames.value(PSDStyleSheet.value(key).toInt());
            weight = fontInfo.weight;
            italic = italic? true: fontInfo.italic;
            styles.append("font-family:"+fontInfo.familyName);
            if (fontInfo.width != 100) {
                styles.append("font-width:"+QString::number(fontInfo.width));
            }
            continue;
        } else if (key == "/FontSize") {
            double val = PSDStyleSheet.value(key).toDouble();
            val = scale.map(QPointF(val, val)).y();
            styles.append("font-size:"+QString::number(val));
            continue;
        } else if (key == "/AutoKerning" || key == "/AutoKern") {
            if (!PSDStyleSheet.value(key).toBool()) {
                styles.append("font-kerning: none");
            }
            continue;
        } else if (key == "/Kerning") {
            // adjusts kerning value, we don't support this.
            unsupportedStyles << key;
            continue;
        } else if (key == "/FauxBold") {
            if (PSDStyleSheet.value(key).toBool()) {
                weight = 700;
            }
            continue;
        } else if (key == "/FauxItalic") {
            if (PSDStyleSheet.value(key).toBool()) {
                italic = true;
            }
            // synthetic Italic, bool
            continue;
        } else if (key == "/Leading") {
            bool autoleading = true;
            if (PSDStyleSheet.keys().contains("AutoLeading")) {
                autoleading = PSDStyleSheet.value("AutoLeading").toBool();
            }
            if (!autoleading) {
                double fontSize = PSDStyleSheet.value("FontSize").toDouble();
                double val = PSDStyleSheet.value(key).toDouble();
                styles.append("line-height:"+QString::number(val/fontSize));
            }
            // value for line-height
            continue;
        } else if (key == "/HorizontalScale" || key == "/VerticalScale") {
            // adjusts scale glyphs, we don't support this.
            unsupportedStyles << key;
            continue;
        } else if (key == "/Tracking") {
            // tracking is in 1/1000 of an EM (as is kerning for that matter...)
            double letterSpacing = (0.001 * PSDStyleSheet.value(key).toDouble());
            styles.append("letter-spacing:"+QString::number(letterSpacing)+"em");
            continue;
        } else if (key == "/BaselineShift") {
            if (PSDStyleSheet.value(key).toDouble() > 0) {
                double val = PSDStyleSheet.value(key).toDouble();
                val = scale.map(QPointF(val, val)).y();
                baselineShift.append(QString::number(val));
            }
            continue;
        } else if (key == "/FontCaps") {
            switch (PSDStyleSheet.value(key).toInt()) {
            case 0:
                break;
            case 1:
                fontVariantCaps.append("all-small-caps");
                break;
            case 2:
                styles.append("text-transform:uppercase");
                break;
            default:
                qDebug() << QString("Unknown value for %1:").arg(key) << PSDStyleSheet.value(key);
            }
            continue;
        } else if (key == "/FontBaseline") {
            // NOTE: This might also be better done with font-variant-position, though
            // we don't support synthetic font stuff, including super and sub script.
            // Actually, seems like this is specifically font-synthesis
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
        } else if (key == "/FontOTPosition") {
            // NOTE: This might also be better done with font-variant-position, though
            // we don't support synthetic font stuff, including super and sub script.
            switch (PSDStyleSheet.value(key).toInt()) {
            case 0:
                break;
            case 1:
                styles.append("font-variant-position:super");
                break;
            case 2:
                styles.append("font-variant-position:sub");
                break;
            case 3:
                fontFeatureSettings.append("'numr' 1");
                break;
            case 4:
                fontFeatureSettings.append("'dnum' 1");
                break;
            default:
                qDebug() << QString("Unknown value for %1:").arg(key) << PSDStyleSheet.value(key);
            }
            continue;
        } else if (key == "/Underline") {
            if (PSDStyleSheet.value(key).toBool()) {
                textDecor.append("underline");
            }
            continue;
        }  else if (key == "/UnderlinePosition") {
            switch (PSDStyleSheet.value(key).toInt()) {
            case 0:
                break;
            case 1:
                textDecor.append("underline");
                underlinePos = "auto left";
                break;
            case 2:
                textDecor.append("underline");
                underlinePos = "auto right";
                break;
            default:
                qDebug() << QString("Unknown value for %1:").arg(key) << PSDStyleSheet.value(key);
            }
            continue;
        } else if (key == "/YUnderline") {
            // Option relating to vertical underline left or right
            if (PSDStyleSheet.value(key).toInt() == 1) {
                underlinePos = "auto left";
            } else if (PSDStyleSheet.value(key).toInt() == 0) {
                underlinePos = "auto right";
            }
            continue;
        } else if (key == "/Strikethrough" || key == "/StrikethroughPosition") {
            if (PSDStyleSheet.value(key).toBool()) {
                textDecor.append("line-through");
            }
            continue;
        } else if (key == "/Ligatures") {
            if (!PSDStyleSheet.value(key).toBool()) {
                fontVariantLigatures.append("no-common-ligatures");
            }
            continue;
        } else if (key == "/DLigatures" || key == "/DiscretionaryLigatures" || key == "/AlternateLigatures") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontVariantLigatures.append("discretionary-ligatures");
            }
            continue;
        } else if (key == "/ContextualLigatures") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontVariantLigatures.append("contextual");
            }
            continue;
        } else if (key == "/Fractions") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontVariantNumeric.append("diagonal-fractions");
            }
            continue;
        } else if (key == "/Ordinals") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontVariantNumeric.append("ordinal");
            }
            continue;
        } else if (key == "/Swash") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontFeatureSettings.append("'swsh' 1");
            }
            continue;
        } else if (key == "/Titling") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontVariantCaps.append("titling-caps");
            }
            continue;
        } else if (key == "/StylisticAlternates") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontFeatureSettings.append("'salt' 1");
            }
            continue;
        } else if (key == "/Ornaments") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontFeatureSettings.append("'ornm' 1");
            }
            continue;
        }  else if (key == "/OldStyle") {
            if (PSDStyleSheet.value(key).toBool() && !fontVariantNumeric.contains("oldstyle-nums")) {
                fontVariantNumeric.append("oldstyle-nums");
            }
            continue;
        } else if (key == "/FigureStyle") {
            switch (PSDStyleSheet.value(key).toInt()) {
            case 0:
                break;
            case 1:
                fontVariantNumeric.append("tabular-nums");
                fontVariantNumeric.append("lining-nums");
                break;
            case 2:
                fontVariantNumeric.append("proportional-nums");
                fontVariantNumeric.append("oldstyle-nums");
                break;
            case 3:
                fontVariantNumeric.append("proportional-nums");
                fontVariantNumeric.append("lining-nums");
                break;
            case 4:
                fontVariantNumeric.append("tabular-nums");
                fontVariantNumeric.append("oldstyle-nums");
                break;
            default:
                qDebug() << QString("Unknown value for %1:").arg(key) << PSDStyleSheet.value(key);
            }
            continue;
        } else if (key == "/Italics") {
            // This is an educated guess: other italic happens via postscript name.
            if (PSDStyleSheet.value(key).toBool()) {
                fontFeatureSettings.append("'ital' 1");
            }
            continue;
        } else if (key == "/BaselineDirection") {
            int val = PSDStyleSheet.value(key).toInt();
            if (val == 1) {
                styles.append("text-orientation: upright");
            } else if (val == 2) {
                styles.append("text-orientation: mixed");
            } else if (val == 3) { //TCY or tate-chu-yoko
                styles.append("text-combine-upright: all");
            } else {
                qDebug() << key << PSDStyleSheet.value(key);
            }
            continue;
        } else if (key == "/Tsume" || key == "/LeftAki" || key == "/RightAki" || key == "/JiDori") {
            // Reduce spacing around a single character. Partially related to text-spacing,
            // Tsume is reduction, Aki expansion, and both can be used as part of Mojikumi
            // However, in this particular case, the property seems to just reduce the space 
            // of a single character, and may not be possible to support (as in CSS that'd
            // just be padding/margin-reduction, but SVG cannot do that).
            unsupportedStyles << key;
            continue;
        } else if (key == "/StyleRunAlignment") {
            // 3 = roman
            // 5 = em-box top/right, 2 = em-box center, 0 = em-box bottom/left
            // 4 = icf-top/right, 1 icf-bottom/left?
            QString dominantBaseline;
            switch(PSDStyleSheet.value(key).toInt()) {
            case 3:
                dominantBaseline = "alphabetic";
                break;
            case 2:
                dominantBaseline = "center";
                break;
            case 0:
                dominantBaseline = "ideographic";
                break;
            case 4:
                dominantBaseline = "text-top";
                break;
            case 1:
                dominantBaseline = "text-bottom";
                break;
            default:
                qDebug() << QString("Unknown value for %1:").arg(key) << PSDStyleSheet.value(key);
                dominantBaseline = QString();
            }
            if (!dominantBaseline.isEmpty()) {
                styles.append("alignment-baseline: "+dominantBaseline);
            }
            continue;
        } else if (key == "/Language") {
            int val = PSDStyleSheet.value(key).toInt();
            if (psdLanguageMap.keys().contains(val)) {
                lang = psdLanguageMap.value(val);
            } else {
                qDebug() << QString("Unknown value for %1:").arg(key) << PSDStyleSheet.value(key);
            }
            continue;
        }  else if (key == "/ProportionalMetrics") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontFeatureSettings.append("'palt' 1");
            }
            continue;
        } else if (key == "/Kana") {
            if (PSDStyleSheet.value(key).toBool()) {
                fontFeatureSettings.append("'hkna' 1");
            }
            continue;
        } else if (key == "/Ruby") {
            fontVariantEastAsian.append("ruby");
        } else if (key == "/JapaneseAlternateFeature") {
            // hojo kanji - 'hojo'
            // nlc kanji - 'nlck'
            // alternate notation - nalt
            // proportional kana - 'pkna'
            // vertical kana - 'vkna'
            // vert alt+rot - vrt2, or vert + vrtr
            int val = PSDStyleSheet.value(key).toInt();
            if (val == 0) {
                continue;
            } else if (val == 1) { // japanese traditional - 'tnam'/'trad'
                fontVariantEastAsian.append("traditional");
            } else if (val == 2) {  // japanese expert - 'expt'
                fontFeatureSettings.append("'expt' 1");
            } else if (val == 3) { // Japanese 78 - jis78
                fontVariantEastAsian.append("jis78");
            } else {
                qDebug() << QString("Unknown value for %1:").arg(key) << PSDStyleSheet.value(key);
            }
            continue;
        } else if (key == "/NoBreak") {
            // Prevents word from breaking... I guess word-break???
            if (PSDStyleSheet.value(key).toBool()) {
                styles.append("word-break: keep-all");
            }
            continue;
        } else if (key == "/DirOverride") {
            QString dir = PSDStyleSheet.value(key).toBool()? "rtl": "ltr";
            if (PSDStyleSheet.value(key).toBool()) {
                styles.append("direction: "+dir);
                styles.append("unicode-bidi: isolate");
            }
            continue;
        }  else if (key == "/FillColor") {
            bool fill = true;
            if (PSDStyleSheet.keys().contains("/FillFlag")) {
                fill = PSDStyleSheet.value("/FillFlag").toBool();
            }
            if (fill) {
                QVariantHash color = PSDStyleSheet.value(key).toHash();
                styles.append("fill:"+colorFromPSDStyleSheet(color, imageCs).name());
            } else {
                styles.append("fill:none");
            }
        } else if (key == "/StrokeColor") {
            bool fill = true;
            if (PSDStyleSheet.keys().contains("/StrokeFlag")) {
                fill = PSDStyleSheet.value("/StrokeFlag").toBool();
            }
            if (fill) {
                QVariantHash color = PSDStyleSheet.value(key).toHash();
                styles.append("stroke:"+colorFromPSDStyleSheet(color, imageCs).name());
            } else {
                styles.append("stroke:none");
            }
            continue;
        } else if (key == "/OutlineWidth" || key == "/LineWidth") {
            double val = PSDStyleSheet.value(key).toDouble();
            val = scale.map(QPointF(val, val)).y();
            styles.append("stroke-width:"+QString::number(val));
        } else if (key == "/FillFirst") {
            // draw fill on top of stroke? paint-order: stroke markers fill, I guess.
            if (PSDStyleSheet.value(key).toBool()) {
                styles.append("paint-order: fill");
            }
            continue;
        } else if (key == "/HindiNumbers") {
            // bool. Looks like this automatically selects hindi numbers for arabic. There also
            // seems to be a more complex option to automatically have arabic numbers for hebrew, and an option for farsi numbers, but this might be a different bool alltogether.
            unsupportedStyles << key;
            continue;
        } else if (key == "/Kashida") {
            // number, s related to drawing/inserting Kashida/Tatweel into Arabic justified text... We don't support this.
            // options are none, short, medium, long, stylistic, indesign apparantly has a 'naskh' option, which is what toggles jalt usage.
            unsupportedStyles << key;
            continue;
        } else if (key == "/DiacriticPos") {
            // number, which is odd, because it looks like it should be a point.
            // this controls how high or low the diacritic is on arabic text.
            unsupportedStyles << key;
            continue;
        }  else if (key == "/SlashedZero") {
            // font-variant: common-ligatures
            if (PSDStyleSheet.value(key).toBool()) {
                fontVariantNumeric.append("slashed-zero");
            }
            continue;
        } else if (key == "/StylisticSets") {
            int flags = PSDStyleSheet.value(key).toInt();
            if (flags & 1) {
                fontFeatureSettings.append("'ss01' 1");
            }
            if (flags & 2) {
                fontFeatureSettings.append("'ss02' 1");
            }
            if (flags & 4) {
                fontFeatureSettings.append("'ss03' 1");
            }
            if (flags & 8) {
                fontFeatureSettings.append("'ss04' 1");
            }
            // TODO: extend till ss20.

            continue;
        } else if (key == "/LineCap") {
            switch (PSDStyleSheet.value(key).toInt()) {
            case 0:
                styles.append("stroke-linecap: butt");
                break;
            case 1:
                styles.append("stroke-linecap: round");
                break;
            case 2:
                styles.append("stroke-linecap: square");
                break;
            default:
                styles.append("stroke-linecap: butt");
            }
        } else if (key == "/LineJoin") {
            switch (PSDStyleSheet.value(key).toInt()) {
            case 0:
                styles.append("stroke-linejoin: miter");
                break;
            case 1:
                styles.append("stroke-linejoin: round");
                break;
            case 2:
                styles.append("stroke-linejoin: bevel");
                break;
            default:
                styles.append("stroke-linejoin: miter");
            }
        } else if (key == "/MiterLimit") {
            styles.append("stroke-miterlimit: "+PSDStyleSheet.value(key).toString());
        //} else if (key == "/LineDashArray") {
            //"stroke-dasharray"
        } else if (key == "/LineDashOffset") {
            styles.append("stroke-dashoffset: "+PSDStyleSheet.value(key).toString());
        } else if (key == "/EnableWariChu" || key == "/WariChuWidowAmount" || key == "/WariChuLineGap" || key == "/WariChuJustification"
                   || key == "/WariChuOrphanAmount" || key == "/WariChuLineCount" || key == "/WariChuSubLineAmount") {
            // Inline cutting note features.
            unsupportedStyles << key;
            continue;
        } else if (key == "/TCYUpDownAdjustment" || key == "/TCYLeftRightAdjustment") {
            // Extra text-combine-upright stuff we don't support.
            unsupportedStyles << key;
            continue;
        }  else if (key == "/Type1EncodingNames" || key == "/ConnectionForms") {
            // no clue what these are
            unsupportedStyles << key;
            continue;
        } else if (key == "/FillOverPrint" || key == "/StrokeOverPrint" || key == "/Blend") {
            // Fill stuff we don't support.
            unsupportedStyles << key;
            continue;
        } else if (key == "/UnderlineOffset") {
            // Needs css text-decor-4 features
            unsupportedStyles << key;
            continue;
        } else {
            if (key != "/FillFlag" && key != "/StrokeFlag" && key != "/AutoLeading") {
                debugFlake << "Unknown PSD character stylesheet style key" << key << PSDStyleSheet.value(key);
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
    if (!fontVariantNumeric.isEmpty()) {
        styles.append("font-variant-numeric:"+fontVariantNumeric.join(" "));
    }
    if (!fontVariantCaps.isEmpty()) {
        styles.append("font-variant-caps:"+fontVariantCaps.join(" "));
    }
    if (!fontVariantEastAsian.isEmpty()) {
        styles.append("font-variant-east-asian:"+fontVariantEastAsian.join(" "));
    }
    if (!fontFeatureSettings.isEmpty()) {
        styles.append("font-feature-settings:"+fontFeatureSettings.join(", "));
    }
    if (!underlinePos.isEmpty()) {
        styles.append("text-decoration-position:"+underlinePos);
    }
    debugFlake << "Unsupported styles" << unsupportedStyles;
    return styles.join("; ");
}

QString stylesForPSDParagraphSheet(QVariantHash PSDParagraphSheet, QString &lang, QMap<int, font_info_psd> fontNames, QTransform scaleToPt, const KoColorSpace *imageCs) {
    QStringList styles;
    QStringList unsupportedStyles;

    for (int i=0; i < PSDParagraphSheet.keys().size(); i++) {
        const QString key = PSDParagraphSheet.keys().at(i);
        double val = PSDParagraphSheet.value(key).toDouble();
        if (key == "/Justification") {
            QString textAlign = "start";
            QString textAnchor = "start";
            switch (PSDParagraphSheet.value(key).toInt()) {
            case 0:
                textAlign = "start";
                textAnchor = "start";
                break;
            case 1:
                textAlign = "end";
                textAnchor = "end";
                break;
            case 2:
                textAlign = "center";
                textAnchor = "middle";
                break;
            case 3:
                textAlign = "justify start";
                textAnchor = "start";
                break;
            case 4:
                textAlign = "justify end"; // guess
                textAnchor = "end";
                break;
            case 5:
                textAlign = "justify center"; // guess
                textAnchor = "middle";
                break;
            case 6:
                textAlign = "justify";
                textAnchor = "middle";
                break;
            default:
                textAlign = "start";
            }

            styles.append("text-align:"+textAlign);
            styles.append("text-anchor:"+textAnchor);
        } else if (key == "/FirstLineIndent") { //-1296..1296
            val = scaleToPt.map(QPointF(val, val)).x();
            styles.append("text-indent:"+QString::number(val));
            continue;
        } else if (key == "/StartIndent") {
            // left margin (also for rtl?), pixels -1296..1296
            unsupportedStyles << key;
            continue;
        } else if (key == "/EndIndent") {
            // right margin (also for rtl?), pixels -1296..1296
            unsupportedStyles << key;
            continue;
        } else if (key == "/SpaceBefore") {
            // top margin for paragraph, pixels -1296..1296
            unsupportedStyles << key;
            continue;
        } else if (key == "/SpaceAfter") {
            // bottom margin for paragraph, pixels -1296..1296
            unsupportedStyles << key;
            continue;
        } else if (key == "/AutoHyphenate") {
            // hyphenate: auto;
            unsupportedStyles << key;
            continue;
        } else if (key == "/HyphenatedWordSize") {
            // minimum wordsize at which to start hyphenating. 2-25
            unsupportedStyles << key;
            continue;
        } else if (key == "/PreHyphen") {
            // minimum number of letters before hyphenation is allowed to start in a word. 1-15
            // CSS-Text-4 hyphenate-limit-chars value 1.
            unsupportedStyles << key;
            continue;
        } else if (key == "/PostHyphen") {
            // minimum amount of letters a hyphnated word is allowed to end with. 1-15
            // CSS-Text-4 hyphenate-limit-chars value 2.
            unsupportedStyles << key;
            continue;
        } else if (key == "/ConsecutiveHyphens") {
            // maximum consequetive lines with hyphenation. 2-25
            // CSS-Text-4 hyphenate-limit-lines.
            unsupportedStyles << key;
            continue;
        } else if (key == "/HyphenateCapitalized") {
            unsupportedStyles << key;
            continue;
        } else if (key == "/HyphenationPreference") {
            unsupportedStyles << key;
            continue;
        } else if (key == "/SingleWordJustification") {
            unsupportedStyles << key;
            continue;
        } else if (key == "/Zone") {
            // Hyphenation zone to control where hyphenation is allowed to start, pixels. 0..8640 for 72ppi
            // CSS-Text-4 hyphenation-limit-zone.
            unsupportedStyles << key;
            continue;
        } else if (key == "/WordSpacing") {
            // val 0 is minimum allowed spacing, and val 2 is maximum allowed spacing, both for justified text.
            // 0 to 1000%, 100% default.
            unsupportedStyles << key;
            continue;
        } else if (key == "/LetterSpacing") {
            // val 0 is minimum allowed spacing, and val 2 is maximum allowed spacing, both for justified text.
            // -100% to 500%, 0% default.
            unsupportedStyles << key;
            continue;
        } else if (key == "/GlyphSpacing") {
            // scaling of the glyphs, list of vals, 50% to 200%, default 100%.
            unsupportedStyles << key;
            continue;
        } else if (key == "/AutoLeading") {
            styles.append("line-height:"+QString::number(val));
            continue;
        } else if (key == "/LeadingType") {
            // Probably how leading is measured for asian glyphs.
            // 0 = top-to-top, 1 = bottom-to-bottom. CSS can only do the second.
            unsupportedStyles << key;
            continue;
        } else if (key == "/Hanging") {
            // Roman hanging punctuation (?), bool
            continue;
        } else if (key == "/Burasagari" || key == "/BurasagariType") {
            // CJK hanging punctuation, bool
            // options are none, regular (allow-end) and force (force-end).
            if (PSDParagraphSheet.value(key).toBool()) {
                styles.append("hanging-punctuation:allow-end");
            }
            continue;
        } else if (key == "/Kinsoku") {
            // line breaking strictness.
            unsupportedStyles << key;
            continue;
        }  else if (key == "/KinsokuOrder") {
            // might be 0 = pushInFirst, 1 = pushOutFirst, 2 = pushOutOnly, if so, Krita only supports 2.
            unsupportedStyles << key;
            continue;
        } else if (key == "/EveryLineComposer") {
            // bool representing which text-wrapping method to use.
            //'single-line' is 'stable/greedy' line breaking,
            //'everyline' uses a penalty based system like Knuth's method.
            unsupportedStyles << key;
            continue;
        } else if (key == "/ComposerEngine") {
            unsupportedStyles << key;
            continue;
        } else if (key == "/KurikaeshiMojiShori") {
            unsupportedStyles << key;
            continue;
        } else if (key == "/MojiKumiTable") {
            unsupportedStyles << key;
            continue;
        }  else if (key == "/DropCaps") {
            unsupportedStyles << key;
            continue;
        } else if (key == "/TabStops" || key == "/AutoTCY" || key == "/KeepTogether" ) {
            unsupportedStyles << key;
            continue;
        } else if (key == "/ParagraphDirection") {
            switch (PSDParagraphSheet.value(key).toInt()) {
            case 1:
                styles.append("direction:rtl");
                break;
            case 0:
                styles.append("direction:ltr");
                break;
            default:
                break;
            }
        } else if (key == "/DefaultTabWidth") {
            unsupportedStyles << key;
            continue;
        } else if (key == "/DefaultStyle") {
            styles.append(stylesForPSDStyleSheet(lang, PSDParagraphSheet.value(key).toHash(), fontNames, scaleToPt, imageCs));
        } else {
            debugFlake << "Unknown PSD paragraph style key" << key << PSDParagraphSheet.value(key);
        }
    }
    debugFlake << "Unsupported paragraph styles" << unsupportedStyles;

    return styles.join("; ");
}

bool KoSvgTextShapeMarkupConverter::convertPSDTextEngineDataToSVG(const QVariantHash tySh,
                                                                  const QVariantHash txt2,
                                                                  const KoColorSpace *imageCs,
                                                                  const int textIndex,
                                                                  QString *svgText,
                                                                  QString *svgStyles,
                                                                  QPointF &offset,
                                                                  bool &offsetByAscent,
                                                                  bool &isHorizontal,
                                                                  QTransform scaleToPt)
{
    debugFlake << "Convert from psd engine data";
    

    QVariantHash root = tySh;
    //qDebug() << textIndex << "Parsed JSON Object" << QJsonObject::fromVariantHash(txt2);
    bool loadFallback = txt2.isEmpty();
    const QVariantHash docObjects = txt2.value("/DocumentObjects").toHash();

    QVariantHash textObject = docObjects.value("/TextObjects").toList().value(textIndex).toHash();
    if (textObject.isEmpty() || loadFallback) {
        textObject = root["/EngineDict"].toHash();
        loadFallback = true;
        qDebug() << "loading from tySh data";
    }
    if (textObject.isEmpty()) {
        d->errors << "No engine dict found in PSD engine data";
        return false;
    }

    QMap<int, font_info_psd> fontNames;
    QVariantHash resourceDict = loadFallback? root.value("/DocumentResources").toHash(): txt2.value("/DocumentResources").toHash();
    if (resourceDict.isEmpty()) {
        d->errors << "No engine dict found in PSD engine data";
        return false;
    } else {
        // PSD only stores the postscript name, and we'll need a bit more information than that.
        QVariantList fonts = loadFallback? resourceDict.value("/FontSet").toList()
                                         : resourceDict.value("/FontSet").toHash().value("/Resources").toList();
        for (int i = 0; i < fonts.size(); i++) {
            QVariantHash font = loadFallback? fonts.value(i).toHash()
                                            : fonts.value(i).toHash().value("/Resource").toHash().value("/Identifier").toHash();
            //qDebug() << font;
            font_info_psd fontInfo;
            QString postScriptName = font.value("/Name").toString();
            QString foundPostScriptName;
            KoFontRegistry::instance()->getCssDataForPostScriptName(postScriptName,
                                                                    &foundPostScriptName,
                                                                    &fontInfo.familyName,
                                                                    fontInfo.weight,
                                                                    fontInfo.width,
                                                                    fontInfo.italic);

            if (postScriptName != foundPostScriptName) {
                fontInfo.familyName = "sans-serif";
                d->errors << QString("Font %1 not found, substituting %2").arg(postScriptName).arg(fontInfo.familyName);
            }
            fontNames.insert(i, fontInfo);
        }
    }

    QString inlineSizeString;
    QRectF bounds;

    // load text shape
    KoPathShape *textShape = nullptr;
    double textPathStartOffset = -3;
    double shapePadding = 0.0;
    int textType = 0; ///< 0 = point text, 1 = paragraph text (including text in shape), 2 = text on path.
    bool reversed = false;
    if (loadFallback) {
        QVariantHash rendered = textObject.value("/Rendered").toHash();
        // rendering info...
        if (!rendered.isEmpty()) {
            QVariantHash shapeChild = rendered.value("/Shapes").toHash().value("/Children").toList()[0].toHash();
            textType = shapeChild.value("/ShapeType").toInt();
            if (textType == 1) {
                QVariantList BoxBounds = shapeChild.value("/Cookie").toHash().value("/Photoshop").toHash().value("/BoxBounds").toList();
                if (BoxBounds.size() == 4) {
                    bounds = QRectF(BoxBounds[0].toDouble(), BoxBounds[1].toDouble(), BoxBounds[2].toDouble(), BoxBounds[3].toDouble());
                    bounds = scaleToPt.mapRect(bounds);
                    if (isHorizontal) {
                        inlineSizeString = " inline-size:"+QString::number(bounds.width())+";";
                    } else {
                        inlineSizeString = " inline-size:"+QString::number(bounds.height())+";";
                    }
                }
            }
            //qDebug() << bounds;
        }
    } else {
        QVariantHash view = textObject.value("/View").toHash();
        // todo: if multiple frames in frames array, there's multiple shapes in shape-inside.
        QVariantList frames = view.value("/Frames").toList();
        if (!frames.isEmpty()) {
            int textFrameIndex = view.value("/Frames").toList().value(0).toHash().value("/Resource").toInt();
            QVariantList textFrameSet = resourceDict.value("/TextFrameSet").toHash().value("/Resources").toList();
            QVariantHash textFrame = textFrameSet.at(textFrameIndex).toHash().value("/Resource").toHash();


            if (!textFrame.isEmpty()) {
                textType = textFrame["/Data"].toHash()["/Type"].toInt();
                //qDebug() << QJsonObject::fromVariantHash(textFrame);

                if (textType > 0) {
                    KoPathShape *textCurve = new KoPathShape();
                    QVariantHash data = textFrame.value("/Data").toHash();
                    QVariantList points = textFrame.value("/Bezier").toHash().value("/Points").toList();
                    QVariantList range = data.value("/TextOnPathTRange").toList();
                    QVariantList fm = data.value("/FrameMatrix").toList();
                    shapePadding = data.value("/Spacing").toDouble();
                    QVariantHash pathData = data.value("/PathData").toHash();
                    reversed = pathData.value("/Flip").toBool();

                    QVariant lineOrientation = data.value("/LineOrientation");
                    if (!lineOrientation.isNull()) {
                        if (lineOrientation.toInt() == 2) {
                            isHorizontal = false;
                        }
                    }
                    QTransform frameMatrix = scaleToPt;
                    if (fm.size() == 6) {
                        frameMatrix = QTransform(fm[0].toDouble(), fm[1].toDouble(), fm[2].toDouble(), fm[3].toDouble(), fm[4].toDouble(), fm[5].toDouble());
                        frameMatrix = frameMatrix * scaleToPt;
                    }

                    int length = points.size()/8;

                    QPointF startPoint;
                    QPointF endPoint;
                    for (int i = 0; i < length; i++) {
                        int iAdjust = i*8;
                        QPointF p1(points[iAdjust  ].toDouble(), points[iAdjust+1].toDouble());
                        QPointF p2(points[iAdjust+2].toDouble(), points[iAdjust+3].toDouble());
                        QPointF p3(points[iAdjust+4].toDouble(), points[iAdjust+5].toDouble());
                        QPointF p4(points[iAdjust+6].toDouble(), points[iAdjust+7].toDouble());

                        if (i == 0 || endPoint != frameMatrix.map(p1)) {
                            if (endPoint == startPoint && i > 0) {
                                textCurve->closeMerge();
                            }
                            textCurve->moveTo(frameMatrix.map(p1));
                            startPoint = frameMatrix.map(p1);
                        }
                        if (p1==p2 && p3==p4) {
                            textCurve->lineTo(frameMatrix.map(p4));
                        } else {
                            textCurve->curveTo(frameMatrix.map(p2), frameMatrix.map(p3), frameMatrix.map(p4));
                        }
                        endPoint = frameMatrix.map(p4);
                    }
                    if (points.size() > 8) {
                        if (endPoint == startPoint) {
                            textCurve->closeMerge();
                        }
                        textShape = textCurve;
                    }
                    if (!range.isEmpty()) {
                        textPathStartOffset = range[0].toDouble();
                        int segment = qFloor(textPathStartOffset);
                        double t = textPathStartOffset - segment;
                        double length = 0;
                        double totalLength = 0;
                        for (int i=0; i<textShape->subpathPointCount(0); i++) {
                            double l = textShape->segmentByIndex(KoPathPointIndex(0, i)).length();
                            totalLength += l;
                            if (i < segment) {
                                length += l;
                            } else if (i == segment) {
                                length += textShape->segmentByIndex(KoPathPointIndex(0, i)).lengthAt(t);
                            }
                        }
                        textPathStartOffset = (length/totalLength) * 100.0;
                    }
                }
            }
        }
    }
    QString paragraphStyle = isHorizontal? "writing-mode: horizontal-tb;": "writing-mode: vertical-rl;";
    paragraphStyle += " white-space: pre-wrap;";

    QBuffer svgBuffer;
    QBuffer styleBuffer;
    svgBuffer.open(QIODevice::WriteOnly);
    styleBuffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter svgWriter(&svgBuffer);
    QXmlStreamWriter stylesWriter(&styleBuffer);
    stylesWriter.writeStartElement("defs");
    if (bounds.isValid()) {
        stylesWriter.writeStartElement("rect");
        stylesWriter.writeAttribute("id", "bounds");
        stylesWriter.writeAttribute("x", QString::number(bounds.x()));
        stylesWriter.writeAttribute("y", QString::number(bounds.y()));
        stylesWriter.writeAttribute("width", QString::number(bounds.width()));
        stylesWriter.writeAttribute("height", QString::number(bounds.height()));
        stylesWriter.writeEndElement();
    }
    if (textShape) {
        stylesWriter.writeStartElement("path");
        stylesWriter.writeAttribute("id", "textShape");
        stylesWriter.writeAttribute("d", textShape->toString());
        stylesWriter.writeAttribute("sodipodi:nodetypes", textShape->nodeTypes());
        stylesWriter.writeEndElement();
    }


    // disable auto-formatting to avoid axtra spaces appearing here and there
    svgWriter.setAutoFormatting(false);

    svgWriter.writeStartElement("text");

    QVariantHash editor = loadFallback? textObject.value("/Editor").toHash() : textObject.value("/Model").toHash();
    QString text = "";
    if (editor.isEmpty()) {
        d->errors << "No editor dict found in PSD engine data";
        return false;
    } else {
        text = editor.value("/Text").toString();
        text.replace("\r", "\n"); // return, used for paragraph hard breaks.
        text.replace(0x03, "\n"); // end of text character, used for non-paragraph hard breaks.
    }

    int antiAliasing = 0;
        antiAliasing = loadFallback? textObject.value("/AntiAlias").toInt()
                                   : textObject.value("/StorySheet").toHash().value("/AntiAlias").toInt();
    //0 = None, 4 = Sharp, 1 = Crisp, 2 = Strong, 3 = Smooth
    if (antiAliasing == 3) {
        svgWriter.writeAttribute("text-rendering", "auto");
    } else if (antiAliasing == 0) {
        svgWriter.writeAttribute("text-rendering", "OptimizeSpeed");
    }

    QVariantHash paragraphRun = loadFallback? textObject.value("/ParagraphRun").toHash() : editor.value("/ParagraphRun").toHash();
    if (!paragraphRun.isEmpty()) {
        //QVariantList runLengthArray = paragraphRun.value("RunLengthArray").toList();
        QVariantList runArray = paragraphRun.value("/RunArray").toList();
        QString features = loadFallback? "/Properties": "/Features";
        QVariantHash style = loadFallback? runArray.value(0).toHash() : runArray.value(0).toHash().value("/RunData").toHash();
        QVariantHash parasheet = loadFallback? runArray.value(0).toHash()["/ParagraphSheet"].toHash():
                runArray.at(0).toHash()["/RunData"].toHash()["/ParagraphSheet"].toHash();
        QVariantHash styleSheet = parasheet[features].toHash();

        QString lang;
        QString styleString = stylesForPSDParagraphSheet(styleSheet, lang, fontNames, scaleToPt, imageCs);
        if (!lang.isEmpty()) {
            svgWriter.writeAttribute("xml:lang", lang);
        }
        if (textType < 2) {
            if (textShape) {
                offsetByAscent = false;
                paragraphStyle += " shape-inside:url(#textShape);";
                if (shapePadding > 0) {
                    QPointF sPadding = scaleToPt.map(QPointF(shapePadding, shapePadding));
                    paragraphStyle += " shape-padding:"+QString::number(sPadding.x())+";";
                }
            } else if (styleString.contains("text-align:justify") && bounds.isValid()) {
                offsetByAscent = false;
                paragraphStyle += " shape-inside:url(#bounds);";
            } else if (bounds.isValid()){
                offsetByAscent = true;
                offset = isHorizontal? bounds.topLeft(): bounds.topRight();
                if (styleString.contains("text-anchor:middle")) {
                    offset = isHorizontal? QPointF(bounds.center().x(), offset.y()):
                                           QPointF(offset.x(), bounds.center().y());
                } else if (styleString.contains("text-anchor:end")) {
                    offset = isHorizontal? QPointF(bounds.right(), offset.y()):
                                           QPointF(offset.x(), bounds.bottom());
                }
                paragraphStyle += inlineSizeString;
                svgWriter.writeAttribute("transform", QString("translate(%1, %2)").arg(offset.x()).arg(offset.y()));
            }
        }
        paragraphStyle += styleString;
        svgWriter.writeAttribute("style", paragraphStyle);

    }

    bool textPathCreated = false;
    if (textShape && textType == 2) {
        svgWriter.writeStartElement("textPath");
        textPathCreated = true;
        svgWriter.writeAttribute("path", textShape->toString());
        if (reversed) {
            svgWriter.writeAttribute("side", "right");
        }
        svgWriter.writeAttribute("startOffset", QString::number(textPathStartOffset)+"%");
    }

    QVariantHash styleRun = loadFallback? textObject.value("/StyleRun").toHash(): editor.value("/StyleRun").toHash();
    if (styleRun.isEmpty()) {
        d->errors << "No styleRun dict found in PSD engine data";
        return false;
    } else {
        QString features = loadFallback? "/StyleSheetData": "/Features";
        QVariantList runLengthArray = styleRun.value("/RunLengthArray").toList();
        QVariantList runArray = styleRun.value("/RunArray").toList();
        if (runArray.isEmpty()) {
            d->errors << "No styleRun dict found in PSD engine data";
            return false;
        } else {
            QVariantHash style = loadFallback? runArray.at(0).toHash() : runArray.at(0).toHash()["/RunData"].toHash();
            QVariantHash styleSheet = style.value("/StyleSheet").toHash().value(features).toHash();
            int length = 0;
            int pos = 0;
            for (int i = 0; i < runArray.size(); i++) {
                style = loadFallback? runArray.at(i).toHash() : runArray.at(i).toHash()["/RunData"].toHash();
                int l = loadFallback? runLengthArray.at(i).toInt(): runArray.at(i).toHash().value("/Length").toInt();

                QVariantHash newStyleSheet = style.value("/StyleSheet").toHash().value(features).toHash();
                if (newStyleSheet == styleSheet) {
                    length += l;
                } else {
                    svgWriter.writeStartElement("tspan");
                    QString lang;
                    svgWriter.writeAttribute("style", stylesForPSDStyleSheet(lang, styleSheet, fontNames, scaleToPt, imageCs));
                    if (!lang.isEmpty()) {
                        svgWriter.writeAttribute("xml:lang", lang);
                    }
                    svgWriter.writeCharacters(text.mid(pos, length));
                    svgWriter.writeEndElement();
                    styleSheet = newStyleSheet;
                    pos += length;
                    length = l;
                }
            }
            svgWriter.writeStartElement("tspan");
            QString lang;
            svgWriter.writeAttribute("style", stylesForPSDStyleSheet(lang, styleSheet, fontNames, scaleToPt, imageCs));
            if (!lang.isEmpty()) {
                svgWriter.writeAttribute("xml:lang", lang);
            }
            svgWriter.writeCharacters(text.mid(pos));
            svgWriter.writeEndElement();
        }
    }

    if (textPathCreated) {
        svgWriter.writeEndElement();
    }

    svgWriter.writeEndElement();//text root element.
    stylesWriter.writeEndElement();

    if (svgWriter.hasError() || stylesWriter.hasError()) {
        d->errors << i18n("Unknown error writing SVG text element");
        return false;
    }
    *svgText = QString::fromUtf8(svgBuffer.data()).trimmed();
    *svgStyles = QString::fromUtf8(styleBuffer.data()).trimmed();
    //qDebug() << *svgText;

    return true;
}


#include <KoCSSFontInfo.h>
void gatherFonts(const QMap<QString, QString> cssStyles, const QString text, QVariantList &fontSet,
                 QVector<int> &lengths, QVector<int> &fontIndices) {
    if (cssStyles.contains("font-family")) {
        QStringList families = cssStyles.value("font-family").split(",");
        int fontSize = cssStyles.value("font-size", "10").toInt();
        int fontWeight = cssStyles.value("font-weight", "400").toInt();
        int fontWidth = cssStyles.value("font-stretch", "100").toInt();

        KoCSSFontInfo fontInfo;
        fontInfo.families = families;
        fontInfo.size = fontSize;
        fontInfo.weight = fontWeight;
        fontInfo.width = fontWidth;
        const std::vector<FT_FaceSP> faces = KoFontRegistry::instance()->facesForCSSValues(lengths, fontInfo,
                                                      text, 72, 72);

        for (uint i = 0; i < faces.size(); i++) {
            const FT_FaceSP &face = faces.at(static_cast<size_t>(i));
            QString postScriptName = face->family_name;
            if (FT_Get_Postscript_Name(face.data())) {
                postScriptName = FT_Get_Postscript_Name(face.data());
            }

            int fontIndex = -1;
            for(int j=0; j<fontSet.size(); j++) {
                if (fontSet[j].toHash()["/Name"] == postScriptName) {
                    fontIndex = j;
                    break;
                }
            }
            if (fontIndex < 0) {
                QVariantHash font;
                font["/Name"] = postScriptName;
                font["/Type"] = 1;
                fontSet.append(font);
                fontIndex = fontSet.size()-1;
            }
            fontIndices << fontIndex;
        }
    }
}

QVariantHash styleToPSDStylesheet(const QMap<QString, QString> cssStyles,
                                 QVariantHash parentStyle, QTransform scaleToPx) {
    QVariantHash styleSheet = parentStyle;

    Q_FOREACH(QString key, cssStyles.keys()) {
        QString val = cssStyles.value(key);

        if (key == "font-size") {
            double size = val.toDouble();
            size = scaleToPx.map(QPointF(size, size)).x();
            styleSheet["/FontSize"] = size;
        } else if (key == "letter-spacing") {
            double space = val.toDouble();
            space = scaleToPx.map(QPointF(space, space)).x();
            double size = styleSheet["/FontSize"].toDouble();
            styleSheet["/Tracking"] = (space/size) * 1000.0;
        } else if (key == "line-height") {
            double space = val.toDouble();
            double size = styleSheet["/FontSize"].toDouble();
            styleSheet["/Leading"] = (space*size);
            styleSheet["/AutoLeading"] = false;
        } else if (key == "font-kerning") {
            if (val == "none") {
                styleSheet["/AutoKern"] = 0;
            }
        } else if (key == "baseline-shift") {
            if (val == "super") {
                styleSheet["/FontBaseline"] = 1;
            } else if (val == "super") {
                styleSheet["/FontBaseline"] = 2;
            } else {
                double offset = val.toDouble();
                offset = scaleToPx.map(QPointF(offset, offset)).y();
                styleSheet["/BaselineShift"] = offset;
            }
        } else if (key == "text-decoration") {
            QStringList decor = val.split(" ");
            Q_FOREACH(QString param, decor) {
                if (param == "underline") {
                    styleSheet["/UnderlinePosition"] = 1;
                    if (cssStyles.value("text-decoration-position").contains("right")) {
                        styleSheet["/UnderlinePosition"] = 2;
                    }
                } else if (param == "line-through"){
                    styleSheet["/StrikethroughPosition"] = 1;
                }
            }
        } else if (key == "font-variant") {
            QStringList params = val.split(" ");
            bool tab = params.contains("tabular-nums");
            bool old = params.contains("oldstyle-nums");
            Q_FOREACH(QString param, params) {
                if (param == "small-caps" || param == "all-small-caps") {
                    styleSheet["/FontCaps"] = 1;
                } else if (param == "titling-caps") {
                    styleSheet["/Titling"] = true;
                } else if (param == "no-common-ligatures"){
                    styleSheet["/Ligatures"] = false;
                } else if (param == "discretionary-ligatures"){
                    styleSheet["/DiscretionaryLigatures"] = true;
                } else if (param == "contextual"){
                    styleSheet["/ContextualLigatures"] = true;
                } else if (param == "diagonal-fractions"){
                    styleSheet["/Fractions"] = true;
                } else if (param == "ordinal"){
                    styleSheet["/Ordinals"] = true;
                } else if (param == "slashed-zero"){
                    styleSheet["/SlashedZero"] = true;
                } else if (param == "super") {
                    styleSheet["/FontOTPosition"] = 1;
                } else if (param == "sub") {
                    styleSheet["/FontOTPosition"] = 2;
                } else if (param == "ruby") {
                    styleSheet["/Ruby"] = true;
                } else if (param == "traditional") {
                    styleSheet["/JapaneseAlternateFeature"] = 1;
                } else if (param == "jis78") {
                    styleSheet["/JapaneseAlternateFeature"] = 3;
                }
            }
            styleSheet["/OldStyle"] = old;
            if (tab && old) {
                styleSheet["/FigureStyle"] = 4;
            } else if (tab) {
                styleSheet["/FigureStyle"] = 1;
            } else if (old) {
                styleSheet["/FigureStyle"] = 2;
            }
        } else if (key == "font-feature-settings") {
            QStringList params = val.split(",");
            Q_FOREACH(QString param, params) {
                if (param.trimmed() == "'swsh' 1") {
                    styleSheet["/Swash"] = true;
                } else if (param.trimmed() == "'titl' 1") {
                    styleSheet["/Titling"] = true;
                } else if (param.trimmed() == "'salt' 1") {
                    styleSheet["/StylisticAlternates"] = true;
                } else if (param.trimmed() == "'ornm' 1") {
                    styleSheet["/Ornaments"] = true;
                } else if (param.trimmed() == "'ital' 1") {
                    styleSheet["/Italics"] = true;
                } else if (param.trimmed() == "'numr' 1") {
                    styleSheet["/FontOTPosition"] = 3;
                } else if (param.trimmed() == "'dnum' 1") {
                    styleSheet["/FontOTPosition"] = 4;
                } else if (param.trimmed() == "'expt' 1") {
                    styleSheet["/JapaneseAlternateFeature"] = 2;
                } else if (param.trimmed() == "'hkna' 1") {
                    styleSheet["/Kana"] = true;
                } else if (param.trimmed() == "'palt' 1") {
                    styleSheet["/ProportionalMetrics"] = true;
                }
            }
        } else if (key == "text-orientation") {
            if (val == "upright") {
                styleSheet["/BaselineDirection"] = 1;
            } else if (val == "mixed") {
                styleSheet["/BaselineDirection"] = 2;
            }
        } else if (key == "text-combine-upright") {
            if (val == "all") {
                 styleSheet["/BaselineDirection"] = 3;
            }
        } else if (key == "word-break") {
            styleSheet["/NoBreak"] = val == "keep-all";
        } else if (key == "direction") {
            styleSheet["/DirOverride"] = val == "ltr"? 0 :1;
        } else if (key == "xml:lang") {
            if (psdLanguageMap.values().contains(val)) {
                styleSheet["/Language"] = psdLanguageMap.key(val);
            }
        } else if (key == "paint-order") {
            QStringList decor = val.split(" ");
            styleSheet["/FillFirst"] = decor.first() == "fill";
        } else {
            qDebug() << "Unsupported css-style:" << key << val;
        }
    }

    return styleSheet;
}

void gatherFills(QDomElement el, QVariantHash &styleDict) {
    if (el.hasAttribute("fill")) {
        if (el.attribute("fill") != "none") {
            QColor c = QColor(el.attribute("fill"));
            //double opacity = el.attribute("fill-opacity", "1.0").toDouble();
            styleDict["/FillFlag"] = true;
            styleDict["/FillColor"] = QVariantHash({ {"/StreamTag", "/SimplePaint"},
                                                     { "/Color", QVariantHash({
                                                           {"/Type", 1},
                                                           {"/Values", QVariantList({1.0, c.redF(), c.greenF(), c.blueF()})
                                                           }})}
                                                   });
        } else {
            styleDict["/FillFlag"] = false;
        }
    }
    if (el.hasAttribute("stroke")) {
        if (el.attribute("stroke") != "none" && el.attribute("stroke-width").toDouble() != 0) {
            QColor c = QColor(el.attribute("stroke"));
            //double opacity = el.attribute("stroke-opacity").toDouble();
            styleDict["/StrokeFlag"] = true;
            styleDict["/StrokeColor"] = QVariantHash({ {"/StreamTag", "/SimplePaint"},
                                                       { "/Color", QVariantHash({
                                                             {"/Type", 1},
                                                             {"/Values", QVariantList({1.0, c.redF(), c.greenF(), c.blueF()})
                                                             }})}
                                                     });
        } else {
            styleDict["/StrokeFlag"] = false;
        }
    }
    if (el.hasAttribute("stroke-linejoin")) {
        QString val = el.attribute("stroke-linejoin");
        if (val == "miter") {
            styleDict["/LineJoin"] = 0;
        } else if (val == "round") {
            styleDict["/LineJoin"] = 1;
        } else if (val == "bevel") {
            styleDict["/LineJoin"] = 2;
        }
    }
    if (el.hasAttribute("stroke-linecap")) {
        QString val = el.attribute("stroke-linecap");
        if (val == "butt") {
            styleDict["/LineCap"] = 0;
        } else if (val == "round") {
            styleDict["/LineCap"] = 1;
        } else if (val == "square") {
            styleDict["/LineCap"] = 2;
        }
    }
    if (el.hasAttribute("stroke-width") && el.attribute("stroke-width").toDouble() != 0) {
        styleDict["/LineWidth"] = el.attribute("stroke-width").toDouble();
    }
}

void gatherStyles(QDomElement el, QString &text,
                  QVariantHash parentStyle,
                  QMap<QString, QString> parentCssStyles,
                  QVariantList &styles,
                  QVariantList &fontSet, QTransform scaleToPx) {
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

        QVariantHash styleDict = styleToPSDStylesheet(cssStyles, parentStyle, scaleToPx);
        gatherFills(el, styleDict);

        QVector<int> lengths;
        QVector<int> fontIndices;
        gatherFonts(cssStyles, currentText, fontSet, lengths, fontIndices);
        for (int i = 0; i< fontIndices.size(); i++) {
            QVariantHash curDict = styleDict;
            curDict["/Font"] = fontIndices.at(i);
            QVariantHash fDict = {
                {"/StyleSheet", QVariantHash({{"/Name", ""}, {"/Parent", 0}, {"/Features", curDict}})}
            };
            styles.append(QVariantHash({
                                           {"/Length", lengths.at(i)},
                                           {"/RunData", fDict},
                                       }));
        }

    } else if (el.childNodes().size()>0) {
        QVariantHash styleDict = styleToPSDStylesheet(cssStyles, parentStyle, scaleToPx);
        gatherFills(el, styleDict);
        QDomElement childEl = el.firstChildElement();
        while(!childEl.isNull()) {
            gatherStyles(childEl, text, styleDict, cssStyles, styles, fontSet, scaleToPx);
            childEl = childEl.nextSiblingElement();
        }
    }
}

QVariantHash gatherParagraphStyle(QDomElement el,
                                 QVariantHash defaultProperties,
                                 bool &isHorizontal,
                                 QString *inlineSize,
                                 QTransform scaleToPx) {
    QString cssStyle = el.attribute("style");
    QStringList dummy = cssStyle.split(";");
    QMap<QString, QString> cssStyles;
    Q_FOREACH(QString style, dummy) {
        QString key = style.split(":").first().trimmed();
        QString val = style.split(":").last().trimmed();
        cssStyles.insert(key, val);
    }
    for (int i = 0; i < el.attributes().length(); i++) {
        const QDomAttr attr = el.attributes().item(i).toAttr();
        cssStyles.insert(attr.name(), attr.value());
    }
    int alignVal = 0;
    int anchorVal = 0;

    QVariantHash paragraphStyleSheet = defaultProperties;
    Q_FOREACH(QString key, cssStyles.keys()) {
        QString val = cssStyles.value(key);

        if (key == "text-align") {
            if (val == "start") {alignVal = 0;}
            if (val == "center") {alignVal = 2;}
            if (val == "end") {alignVal = 1;}
            if (val == "justify start") {alignVal = 3;}
            if (val == "justify center") {alignVal = 4;}
            if (val == "justify end") {alignVal = 5;}
            if (val == "justify") {alignVal = 6;}
        } else if (key == "text-anchor") {
            if (val == "start") {anchorVal = 0;}
            if (val == "middle") {anchorVal = 2;}
            if (val == "end") {anchorVal = 1;}
        } else if (key == "writing-mode") {
            if (val == "horizontal-tb") {
                isHorizontal = true;
            } else {
                isHorizontal = false;
            }
        } else if (key == "direction") {
            paragraphStyleSheet["/ParagraphDirection"] = val == "ltr"? 0 :1;
        } else if (key == "line-height") {
            paragraphStyleSheet["/AutoLeading"] = val.toDouble();
        } else if (key == "inline-size") {
            *inlineSize = val;
        }
    }
    if (cssStyles.keys().contains("shape-inside")) {
        paragraphStyleSheet["/Justification"] = alignVal;
    } else {
        paragraphStyleSheet["/Justification"] = anchorVal;
    }
    return QVariantHash{{"/Name", ""}, {"/Parent", 0}, {"/Features", paragraphStyleSheet}};
}

bool KoSvgTextShapeMarkupConverter::convertToPSDTextEngineData(const QString &svgText, QRectF &boundingBox,
                                                               const QList<KoShape *> &shapesInside,
                                                               QVariantHash &txt2,
                                                               int &textIndex,
                                                               QString &textTotal,
                                                               bool &isHorizontal,
                                                               QTransform scaleToPx)
{
    QVariantHash root;

    QVariantHash model;
    QVariantHash view;

    QString text;
    QVariantList styles;
    QVariantList fontSet;

    QVariantList textObjects = txt2.value("/DocumentObjects").toHash().value("/TextObjects").toList();
    QVariantHash defaultParagraphProps = txt2.value("/DocumentObjects").toHash().value("/OriginalNormalParagraphFeatures").toHash();

    const int tIndex = textObjects.size();
    QVariantHash docResources = txt2.value("/DocumentResources").toHash();
    QVariantList textFrames = docResources.value("/TextFrameSet").toHash().value("/Resources").toList();
    const QVariantList resFontSet = docResources.value("/FontSet").toHash().value("/Resources").toList();

    Q_FOREACH(const QVariant entry, resFontSet) {
        const QVariantHash docFont = entry.toHash().value("/Resource").toHash();
        QVariantHash font = docFont.value("/Identifier").toHash();
        fontSet.append(font);
    }

    QVector<int> lengths;
    QVector<int> fontIndices;
    gatherFonts(KoSvgTextProperties::defaultProperties().convertToSvgTextAttributes(), "", fontSet, lengths, fontIndices);

    // go down the document children to get the style.
    QDomDocument doc;
    doc.setContent(svgText);
    gatherStyles(doc.documentElement(), text, QVariantHash(), QMap<QString, QString>(), styles, fontSet, scaleToPx);

    QString inlineSize;
    QVariantHash paragraphStyle = gatherParagraphStyle(doc.documentElement(),
                                                      defaultParagraphProps,
                                                      isHorizontal, &inlineSize,
                                                      scaleToPx);

    text += '\n';
    model.insert("/Text", text);

    QVariantHash paragraphSet;
    paragraphSet.insert("/Length", QVariant(text.length()));
    paragraphSet.insert("/RunData", QVariantHash{{"/ParagraphSheet", paragraphStyle}});

    model.insert("/ParagraphRun", QVariantHash{{"/RunArray", QVariantList({paragraphSet})}});

    QVariantHash styleRun;
    QVariantList properStyleRun;
    Q_FOREACH(QVariant entry, styles) {
        properStyleRun.append(entry);
    }
    styleRun.insert("/RunArray", properStyleRun);

    model.insert("/StyleRun", styleRun);

    QVariantHash storySheet;
    storySheet.insert("/UseFractionalGlyphWidths", true);
    storySheet.insert("/AntiAlias", 1);
    model.insert("/StorySheet", storySheet);

    QRectF bounds;
    if (!(inlineSize.isEmpty() || inlineSize == "auto")) {
        bounds = boundingBox;
        bool ok;
        double inlineSizeVal = inlineSize.toDouble(&ok);
        if (ok) {
            if (isHorizontal) {
                bounds.setWidth(inlineSizeVal);
            } else {
                bounds.setHeight(inlineSizeVal);
            }
        }
    } else {
        bounds = QRectF();
    }

    int shapeType = bounds.isEmpty()? 0: 1; ///< 0 point, 1 paragraph, 2 text-on-path.
    int writingDirection = isHorizontal? 0: 2;


    const int textFrameIndex = textFrames.size();
    QVariantHash newTextFrame;
    QVariantHash newTextFrameData;
    newTextFrameData.insert("/LineOrientation", writingDirection);


    QList<QPointF> points;

    KoPathShape *textShape = nullptr;
    Q_FOREACH(KoShape *shape, shapesInside) {
        KoPathShape *p = dynamic_cast<KoPathShape*>(shape);
        if (p) {
            textShape = p;
            break;
        }
    }
    if (textShape) {
        for (int i = 0; i<textShape->subpathPointCount(0); i++) {
            KoPathSegment s = textShape->segmentByIndex(KoPathPointIndex(0, i));
            points.append(s.first()->point());
            points.append(s.first()->controlPoint2());
            points.append(s.second()->controlPoint1());
            points.append(s.second()->point());
        }
    } else if (!bounds.isEmpty()) {
        points.append(bounds.topLeft());
        points.append(bounds.topLeft());
        points.append(bounds.topRight());
        points.append(bounds.topRight());
        points.append(bounds.topRight());
        points.append(bounds.topRight());
        points.append(bounds.bottomRight());
        points.append(bounds.bottomRight());
        points.append(bounds.bottomRight());
        points.append(bounds.bottomRight());
        points.append(bounds.bottomLeft());
        points.append(bounds.bottomLeft());
        points.append(bounds.bottomLeft());
        points.append(bounds.bottomLeft());
        points.append(bounds.topLeft());
        points.append(bounds.topLeft());
    }
    if (!points.isEmpty()) {
        QVariantList p;
        for(int i = 0; i < points.size(); i++) {
            QPointF p2 = scaleToPx.map(points.at(i));
            p.append(p2.x());
            p.append(p2.y());
        }
        newTextFrame.insert("/Bezier", QVariantHash({{"/Points", p}}));
        shapeType = 1;
    }
    newTextFrameData.insert("/Type", shapeType);
    newTextFrame.insert("/Data", newTextFrameData);

    view.insert("/Frames", QVariantList({QVariantHash({{"/Resource", textFrameIndex}})}));

    QVariantList bbox = {0.0, 0.0, bounds.width(), bounds.height()};
    QVariantList bbox2 = {bounds.left(), bounds.top(), bounds.right(), bounds.bottom()};

    /*
    QVariantHash glyphStrike {
        {"/Bounds", bbox},
        {"/GlyphAdjustments", QVariantHash({{"/Data", QVariantList()}, {"/RunLengths", QVariantList()}})},
        {"/Glyphs", QVariantList()},
        {"/Invalidation", bbox},
        {"/RenderedBounds", bbox},
        {"/VisualBounds", bbox},
        {"/SelectionAscent", 10.0},
        {"/SelectionDescent", -10.0},
        {"/ShadowStylesRun", QVariantHash({{"/Data", QVariantList()}, {"/RunLengths", QVariantList()}})},
        {"/StreamTag", "/GlyphStrike"},
        {"/Transform", QVariantHash({{"/Origin", QVariantList({0.0, 0.0})}})}
    };
    QVariantHash frameStrike {
        {"/Bounds", QVariantList({0.0, 0.0, 0.0, 0.0})},
        {"/ChildProcession", 2},
        {"/Children", QVariantList({glyphStrike})},
        {"/StreamTag", "/FrameStrike"},
        {"/Frame", textFrameIndex},
        {"/Transform", QVariantHash({{"/Origin", QVariantList({0.0, 0.0})}})}
    };
    QVariantHash pathStrike {
        {"/Bounds", QVariantList({0.0, 0.0, 0.0, 0.0})},
        {"/ChildProcession", 0},
        {"/Children", QVariantList({frameStrike})},
        {"/StreamTag", "/PathSelectGroupCharacter"},
        {"/Transform", QVariantHash({{"/Origin", QVariantList({0.0, 0.0})}})}
    };
    view.insert("/Strikes", QVariantList({pathStrike}));*/
    QVariantHash rendered {
        {"/RunData", QVariantHash({{"/LineCount", 1}})},
        {"/Length", textTotal.length()}
    };
    view.insert("/RenderedData", QVariantHash({{"/RunArray", QVariantList({rendered})}}));


    textFrames.append(QVariantHash({{"/Resource", newTextFrame}}));
    textObjects.append(QVariantHash({{"/Model", model}, {"/View", view}}));

    // default resoure dict

    textTotal = text;

    QVariantList newFontSet;

    Q_FOREACH(const QVariant entry, fontSet) {
        newFontSet.append(QVariantHash({{"/Resource", QVariantHash({{"/StreamTag", "/CoolTypeFont"}, {"/Identifier", entry}})}}));
    }

    QVariantHash docObjects = txt2.value("/DocumentObjects").toHash();
    docObjects.insert("/TextObjects", textObjects);
    txt2.insert("/DocumentObjects", docObjects);
    docResources.insert("/TextFrameSet", QVariantHash({{"/Resources", textFrames}}));
    docResources.insert("/FontSet", QVariantHash({{"/Resources", newFontSet}}));
    txt2.insert("/DocumentResources", docResources);
    textIndex = tIndex;

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

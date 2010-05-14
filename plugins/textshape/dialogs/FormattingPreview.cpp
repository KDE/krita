/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2009-2010 Thomas Zander <zander@kde.org>
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

#include "FormattingPreview.h"

#include <KoPostscriptPaintDevice.h>
#include <KoZoomHandler.h>

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QTextLayout>
#include <QTextLine>
#include <QTextOption>

#include <math.h>

#include <klocale.h>
#include <kdebug.h>

FormattingPreview::FormattingPreview(QWidget* parent)
        : QFrame(parent),
        m_previewType(FontPreview),
        m_sampleText(i18n("Font")),
        //Character properties
        m_backgroundColor(QColor(Qt::transparent)),
        m_font(QFont("Sans Serif", 12)),
        m_fontCapitalisation(QFont::MixedCase),
        m_strikethroughType(KoCharacterStyle::NoLineType),
        m_strikethroughStyle(KoCharacterStyle::NoLineStyle),
        m_strikethroughColor(QColor(Qt::black)),
        m_textColor(QColor(Qt::black)),
        m_underlineType(KoCharacterStyle::NoLineType),
        m_underlineStyle(KoCharacterStyle::NoLineStyle),
        m_underlineColor(QColor(Qt::black)),
        //Paragraph properties
        m_paragBackgroundColor(QColor(Qt::white)),
        m_firstLineMargin(0),
        m_fixedLineHeight(0),
        m_horizAlign(Qt::AlignHCenter),
        m_leftMargin(0),
        m_lineSpacing(0),
        m_minimumLineHeight(0),
        m_percentLineHeight(0),
        m_rightMargin(0),
        m_useFontProperties(false),
        m_vertAlign(Qt::AlignVCenter)
{
    m_align = m_vertAlign | m_horizAlign;
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setMinimumSize(500, 150);
}

FormattingPreview::~FormattingPreview()
{
}

void FormattingPreview::setPreviewType(FormattingPreview::PreviewType type)
{
    m_previewType = type;
}

void FormattingPreview::setText(const QString &sampleText)
{
    m_sampleText = sampleText;
    update();
}

//Character properties
void FormattingPreview::setCharacterStyle(const KoCharacterStyle* style)
{
    if (style->hasProperty(QTextFormat::ForegroundBrush) && style->foreground().color().isValid())
        m_textColor = style->foreground().color();
    else
        m_textColor = QColor(Qt::black);
    if (style->hasProperty(QTextFormat::BackgroundBrush) && style->background().color().isValid())
        m_backgroundColor = style->background().color();
    else
        m_backgroundColor = QColor(Qt::transparent);
    m_font = style->font();
    m_fontCapitalisation = style->fontCapitalization();
    m_strikethroughType = style->strikeOutType();
    m_strikethroughStyle = style->strikeOutStyle();
    m_strikethroughColor = style->strikeOutColor();
    m_underlineType = style->underlineType();
    m_underlineStyle = style->underlineStyle();
    m_underlineColor = style->underlineColor();
    update();
}

void FormattingPreview::setBackgroundColor(QColor color)
{
    m_backgroundColor = color;
    update();
}

void FormattingPreview::setFont(const QFont &font)
{
    m_font = font;
    update();
}

void FormattingPreview::setFontCapitalisation(QFont::Capitalization capitalisation)
{
    m_fontCapitalisation = capitalisation;
    update();
}

void FormattingPreview::setStrikethrough(KoCharacterStyle::LineType strikethroughType, KoCharacterStyle::LineStyle strikethroughStyle, const QColor &strikethroughColor)
{
    m_strikethroughType = strikethroughType;
    m_strikethroughStyle = strikethroughStyle;
    m_strikethroughColor = strikethroughColor;
    update();
}

void FormattingPreview::setTextColor(QColor color)
{
    m_textColor = color;
    update();
}

void FormattingPreview::setUnderline(KoCharacterStyle::LineType underlineType, KoCharacterStyle::LineStyle underlineStyle, const QColor &underlineColor)
{
    m_underlineType = underlineType;
    m_underlineStyle = underlineStyle;
    m_underlineColor = underlineColor;
    update();
}

//Paragraph properties
void FormattingPreview::setParagraphBackgroundColor(const QColor &color)
{
    m_paragBackgroundColor = color;
    update();
}

void FormattingPreview::setFirstLineMargin(qreal margin)
{
    m_firstLineMargin = margin;
    update();
}

void FormattingPreview::setHorizontalAlign(Qt::Alignment align)
{
    m_horizAlign = align;
    m_align = m_horizAlign | m_vertAlign;
    update();
}

void FormattingPreview::setLeftMargin(qreal margin)
{
    m_leftMargin = margin;
    update();
}

void FormattingPreview::setListItemText(const QString &text)
{
    m_listItemText = text;
    update();
}

void FormattingPreview::setLineSpacing(qreal fixedLineHeight, qreal lineSpacing, qreal minimumLineHeight, int percentLineSpacing, bool useFontProperties)
{
    m_fixedLineHeight = fixedLineHeight;
    m_lineSpacing = lineSpacing;
    m_minimumLineHeight = minimumLineHeight;
    m_percentLineHeight = percentLineSpacing;
    m_useFontProperties = useFontProperties;
    update();
}

void FormattingPreview::setRightMargin(qreal margin)
{
    m_rightMargin = margin;
    update();
}

//Painting related methods

void FormattingPreview::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    KoPostscriptPaintDevice *paintDevice = new KoPostscriptPaintDevice;
    KoZoomHandler *zoomHandler = new KoZoomHandler;
    zoomHandler->setResolutionToStandard();
    zoomHandler->setZoom(1.0);
    qreal zoomX, zoomY;
    zoomHandler->zoom(&zoomX, &zoomY);
    QPainter painter(this);
    painter.scale(zoomX, zoomY);

    painter.fillRect(contentsRect(), QBrush(QColor(Qt::white)));
    if (m_paragBackgroundColor.isValid() && m_paragBackgroundColor.alpha() > 1)
        painter.fillRect(contentsRect(), QBrush(m_paragBackgroundColor));

//set up the Font properties
    QFont displayFont = QFont(m_font, paintDevice);
    displayFont.setCapitalization(m_fontCapitalisation);
    painter.setPen(m_textColor);
    painter.setFont(displayFont);

//first quickly check if we can draw at least one word
    int index = m_sampleText.indexOf(' ');
    QString word = m_sampleText;
    if (index > 0)
        word = word.left(index-1);
    QRectF boundingRect = painter.boundingRect(contentsRect(), Qt::AlignCenter, word);
    while (true) {
        if (boundingRect.width() * zoomX > contentsRect().width()
                || boundingRect.height() * zoomY > contentsRect().height()) {
            painter.scale(0.8, 0.8);
            zoomX *= 0.8;
            zoomY *= 0.8;
        } else {
            break;
        }
    }

// list item
    qreal xmargin = 5; //leave a tiny space on borders
    qreal ymargin = 5;
    QTextLayout listItemLayout(m_listItemText, displayFont);
    QFontMetricsF fm = QFontMetrics(displayFont, paintDevice);
    listItemLayout.beginLayout();
    QTextLine listItem = listItemLayout.createLine();
    listItem.setPosition(QPointF(xmargin, ymargin));
    listItemLayout.endLayout();

//now start the actual layout
    QTextLayout layout(m_sampleText, displayFont);
    layout.setTextOption(QTextOption(m_align));

    xmargin += listItem.naturalTextWidth();
    qreal height = 0;
    qreal lineWidth = (contentsRect().width() - 2*xmargin)/zoomX;
    bool firstLine = true;

    qreal lineSpacing = 0;
    if (m_fixedLineHeight != 0) { // this one is easy.
        lineSpacing = m_fixedLineHeight;
    } else {
        if (m_useFontProperties)
            lineSpacing = fm.height();
        else if (m_lineSpacing > 0)
            lineSpacing = m_lineSpacing;
        else
            lineSpacing = 12;
        if (m_percentLineHeight > 0)
            lineSpacing = lineSpacing * m_percentLineHeight / 100.;
        lineSpacing = qMax(lineSpacing, m_minimumLineHeight);
    }

    layout.beginLayout();
    while (true) {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;
        qreal leftIndent = zoomHandler->documentToViewX(m_leftMargin);
        if (firstLine)
            leftIndent += zoomHandler->documentToViewX(m_firstLineMargin);
        qreal rightIndent = zoomHandler->documentToViewX(m_rightMargin);
        line.setLineWidth(lineWidth - leftIndent - rightIndent);
        line.setPosition(QPointF(leftIndent, height*zoomY));
        height += lineSpacing;

        firstLine = false;

        if (height + lineSpacing > ((contentsRect().height() - 2* ymargin))/zoomY)
            break;
    }
    layout.endLayout();

    boundingRect = layout.boundingRect();
    QTextCharFormat charFormat;
    charFormat.setBackground(QBrush(m_backgroundColor));
    QVector<QTextLayout::FormatRange> selections;
    QTextLayout::FormatRange fr;
    fr.start = 0;
    fr.length = m_sampleText.length();
    fr.format = charFormat;
    selections.append(fr);

    listItemLayout.draw(&painter, QPointF());
    layout.draw(&painter, QPointF(xmargin, ymargin), selections);

//draw decorations
    qreal width;
    switch (m_font.weight()) {
    case QFont::Light:
        width = fm.lineWidth() /2;
        break;
    case QFont::Normal: //Fallthrough
    case QFont::DemiBold:
        width = fm.lineWidth();
        break;
    case QFont::Bold: //Fallthrough
    case QFont::Black:
        width = fm.lineWidth() * 2;
        break;
    default:
        width = fm.lineWidth();
    }

    for (int i = 0; i <= layout.lineCount() - 1; i++) {
        QTextLine line = layout.lineAt(i);

        qreal xstart = line.naturalTextRect().x() + xmargin;
        qreal xend = line.naturalTextRect().x() + xmargin + line.naturalTextWidth();
        qreal y = line.naturalTextRect().y() + ymargin + line.ascent();

        if ((m_underlineType != KoCharacterStyle::NoLineType) && (m_underlineStyle != KoCharacterStyle::NoLineStyle))
            drawLine(painter, xstart, xend, y + fm.underlinePos() + 1, width, fm.underlinePos(), m_underlineType, m_underlineStyle, (m_underlineColor.isValid())?m_underlineColor:m_textColor);
        if ((m_strikethroughType != KoCharacterStyle::NoLineType) && (m_strikethroughStyle != KoCharacterStyle::NoLineStyle))
            drawLine(painter, xstart, xend, y - fm.strikeOutPos(), width, fm.underlinePos(), m_strikethroughType, m_strikethroughStyle, (m_strikethroughColor.isValid())?m_strikethroughColor:m_textColor);
    }

    delete paintDevice;
    delete zoomHandler;
}

void FormattingPreview::drawLine(QPainter &painter, qreal xstart, qreal xend, qreal y, qreal width, int distToBaseLine, KoCharacterStyle::LineType lineType, KoCharacterStyle::LineStyle lineStyle, QColor lineColor)
{
//Following code derived from plugins/textShape/Layout::drawDecorationLine
    painter.save();
    QPen pen = painter.pen();
    pen.setColor(lineColor);
    pen.setWidthF(width);

    if (lineStyle == KoCharacterStyle::WaveLine) {
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const qreal halfWaveWidth = 2 * distToBaseLine;
        const qreal halfWaveLength = 4 * distToBaseLine;
        const qreal startAngle = 45.; //in °
        const qreal middleAngle = 225.; //in °
        const qreal spanAngle = 90.; //in °
        qreal xbound = xstart - (1 - cos(startAngle))*halfWaveLength / 2;
        qreal ybound1 = y - (1 - sin(startAngle))*halfWaveWidth / 2;
        qreal ybound2 = y - (1 + sin(startAngle))*halfWaveWidth / 2;
        while (xbound < xend) {
            QRectF rectangle1(xbound, ybound1, halfWaveLength, halfWaveWidth);
            if (lineType == KoCharacterStyle::DoubleLine) {
                painter.translate(0, -pen.width());
                painter.drawArc(rectangle1, startAngle * 16, spanAngle * 16);
                painter.translate(0, 2*pen.width());
                painter.drawArc(rectangle1, startAngle * 16, spanAngle * 16);
                painter.translate(0, -pen.width());
            } else {
                painter.drawArc(rectangle1, startAngle * 16, spanAngle * 16);
            }
            if (xbound + (1 - cos(startAngle)) * halfWaveLength > xend)
                break;
            xbound = xbound + (1 - cos(startAngle)) * halfWaveLength;
            QRectF rectangle2(xbound, ybound2, halfWaveLength, halfWaveWidth);
            if (lineType == KoCharacterStyle::DoubleLine) {
                painter.translate(0, -pen.width());
                painter.drawArc(rectangle2, middleAngle * 16, spanAngle * 16);
                painter.translate(0, 2*pen.width());
                painter.drawArc(rectangle2, middleAngle * 16, spanAngle * 16);
                painter.translate(0, -pen.width());
            } else {
                painter.drawArc(rectangle2, middleAngle * 16, spanAngle * 16);
            }
            xbound = xbound + (1 - cos(startAngle)) * halfWaveLength;
        }
    }
    else {
        if (lineStyle == KoCharacterStyle::SolidLine)
            pen.setStyle(Qt::SolidLine);
        else if (lineStyle == KoCharacterStyle::DashLine)
            pen.setStyle(Qt::DashLine);
        else if (lineStyle == KoCharacterStyle::DottedLine)
            pen.setStyle(Qt::DotLine);
        else if (lineStyle == KoCharacterStyle::DotDashLine)
            pen.setStyle(Qt::DashDotLine);
        else if (lineStyle == KoCharacterStyle::DotDotDashLine)
            pen.setStyle(Qt::DashDotDotLine);
        painter.setPen(pen);
        if (lineType == KoCharacterStyle::DoubleLine) {
            painter.translate(0, -pen.width());
            painter.drawLine(QPointF(xstart, y), QPointF(xend, y));
            painter.translate(0, 2*pen.width());
            painter.drawLine(QPointF(xstart, y), QPointF(xend, y));
            painter.translate(0, -pen.width());
        } else {
            painter.drawLine(QPointF(xstart, y), QPointF(xend, y));
        }
    }
    painter.restore();
}

#include <FormattingPreview.moc>

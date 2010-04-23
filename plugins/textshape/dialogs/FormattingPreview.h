/* This file is part of the KDE project
   Copyright (C)  2008 Pierre Stirnweiss <pstirnweiss@googlemail.com>

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

#ifndef FORMATTINGPREVIEW_H
#define FORMATTINGPREVIEW_H

#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>

#include <QFont>
#include <QFrame>
#include <QWidget>

class QString;

class FormattingPreview : public QFrame
{
    Q_OBJECT

public:
    enum PreviewType {
        FontPreview,
        ParagPreview
    };

    explicit FormattingPreview(QWidget* parent = 0);
    ~FormattingPreview();

    void setPreviewType(PreviewType type);

public slots:
    ///Character properties
    void setCharacterStyle(const KoCharacterStyle *style);

    void setBackgroundColor(QColor color);
    void setFont(const QFont &font);
    void setFontCapitalisation(QFont::Capitalization capitalisation);
    void setStrikethrough(KoCharacterStyle::LineType strikethroughType, KoCharacterStyle::LineStyle striketrhoughStyle, const QColor &strikethroughColor);
    void setText(const QString &sampleText);
    void setTextColor(QColor color);
    void setUnderline(KoCharacterStyle::LineType underlineType, KoCharacterStyle::LineStyle underlineStyle, const QColor &underlineColor);

    ///Paragraph properties
    void setParagraphBackgroundColor(const QColor &color);
    void setFirstLineMargin(qreal margin);
    void setHorizontalAlign(Qt::Alignment);
    void setLeftMargin(qreal margin);
    void setLineSpacing(qreal fixedLineHeight, qreal lineSpacing, qreal minimumLineHeight, int percentLineSpacing, bool useFontProperties);
    void setRightMargin(qreal margin);
    void setListItemText(const QString &text);

protected:
    void paintEvent(QPaintEvent *event);
    void drawLine(QPainter &painter, qreal xstart, qreal xend, qreal y, qreal width, int underlineDist, KoCharacterStyle::LineType lineType, KoCharacterStyle::LineStyle lineStyle, QColor lineColor);

private:
    PreviewType m_previewType;

    QString m_sampleText;
    QString m_listItemText;

    ///Character properties
    QColor m_backgroundColor;

    QFont m_font;

    QFont::Capitalization m_fontCapitalisation;

    KoCharacterStyle::LineType m_strikethroughType;
    KoCharacterStyle::LineStyle m_strikethroughStyle;
    QColor m_strikethroughColor;

    QColor m_textColor;

    KoCharacterStyle::LineType m_underlineType;
    KoCharacterStyle::LineStyle m_underlineStyle;
    QColor m_underlineColor;

    ///Paragraph properties
    Qt::Alignment m_align;
    QColor m_paragBackgroundColor;
    qreal m_firstLineMargin;
    qreal m_fixedLineHeight;
    Qt::Alignment m_horizAlign;
    qreal m_leftMargin;
    qreal m_lineSpacing;
    qreal m_minimumLineHeight;
    int m_percentLineHeight;
    qreal m_rightMargin;
    bool m_useFontProperties;
    Qt::Alignment m_vertAlign;
};

#endif //FORMATTINGPREVIEW_H

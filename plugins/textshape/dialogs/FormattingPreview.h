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

#include <QFont>
#include <QFrame>
#include <QWidget>

class QString;

class FormattingPreview : public QFrame
{
    Q_OBJECT

public:
    explicit FormattingPreview(QWidget* parent = 0);
    ~FormattingPreview();

public slots:
    void setBackgroundColor(QColor color);
    void setFont(const QFont &font);
    void setFontCapitalisation(QFont::Capitalization capitalisation);
    void setStrikethrough(KoCharacterStyle::LineType strikethroughType, KoCharacterStyle::LineStyle striketrhoughStyle, const QColor &strikethroughColor);
    void setText(const QString &sampleText);
    void setTextColor(QColor color);
    void setUnderline(KoCharacterStyle::LineType underlineType, KoCharacterStyle::LineStyle underlineStyle, const QColor &underlineColor);

protected:
    void paintEvent(QPaintEvent *event);
    void drawLine(QPainter &painter, qreal xstart, qreal xend, qreal y, qreal width, int underlineDist, KoCharacterStyle::LineType lineType, KoCharacterStyle::LineStyle lineStyle, QColor lineColor);
    
private:
    QColor m_backgroundColor;

    QFont m_font;
    
    QFont::Capitalization m_fontCapitalisation;
    
    QString m_sampleText;
    
    KoCharacterStyle::LineType m_strikethroughType;
    KoCharacterStyle::LineStyle m_strikethroughStyle;
    QColor m_strikethroughColor;
    
    QColor m_textColor;

    KoCharacterStyle::LineType m_underlineType;
    KoCharacterStyle::LineStyle m_underlineStyle;
    QColor m_underlineColor;
};

#endif //FORMATTINGPREVIEW_H

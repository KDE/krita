/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __kofontdiapreview_h__
#define __kofontdiapreview_h__

#include <q3frame.h>
#include <QString>
#include <QColor>
#include <QFont>
#include <qpainter.h>

/**
 * The font dialog Preview
 */
class KoFontDiaPreview : public Q3Frame
{
    Q_OBJECT
public:
    /**
    * Constructor
    */
    KoFontDiaPreview( QWidget* parent =0, const char* name = 0, Qt::WFlags fl = 0 );
    ~KoFontDiaPreview();

    void setText( const QString &text );
    void setFont( const QFont &font );
    void setFontColor( const QColor &textColor );
    void setBackgroundColor( const QColor &backgroundColor );
    void setShadow( double sdx, double sdy, QColor shadowColor );
    void setUnderlining( int underlining, int underliningStyle, const QColor underliningColor, bool wordByWord );
    void setWordByWord( bool wordByWord );
    void setStrikethrough( int strikethrough, int strikethroughStylestrikethrough, bool wordByWord );
    void setCapitalisation( int capitalisation );
    void setSubSuperscript( int subSuper, int offset, double relativeSize );


private:
    void drawContents( QPainter* );

    QString m_text;
    QString displayText;
    QFont m_font;
    QFont displayFont;
    int m_fontSize;
    QColor m_textColor;
    QColor m_backgroundColor;
    double m_shadowDistanceX;
    double m_shadowDistanceY;
    QColor m_shadowColor;
    int m_underlining;
    int m_underliningStyle;
    QColor m_underliningColor;
    bool m_wordByWord;
    int m_strikethrough;
    int m_strikethroughStyle;
    int m_capitalisation;
    int m_subSuper;
    int m_offset;
    double m_relativeSize;

    QString formatCapitalisation( const QString &string );
    void drawUnderline( int x, int y, int width, int thickness, QColor & color, QPainter *p );
    void drawUnderlineWave( int x, int y, int width, int thickness, QColor & color, QPainter *p );
    void drawStrikethrough( int x, int y, int width, int thickness, QPainter *p );
};

#endif

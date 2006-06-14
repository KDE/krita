/* This file is part of the KDE project
   Copyright (C) 2002, Laurent MONTEL <lmontel@mandrakesoft.com>

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

#ifndef KOTEXTVIEW_IFACE_H
#define KOTEXTVIEW_IFACE_H

#include <KoDocumentIface.h>
// #include <dcopref.h>

#include <QString>
#include <QColor>
#include <KoRichText.h>
#include <koffice_export.h>
class KoTextView;

class KOTEXT_EXPORT KoTextViewIface /*:  public DCOPObject*/
{
//     K_DCOP
public:
    KoTextViewIface( KoTextView *_textview );

// k_dcop:
    void insertSoftHyphen();
    void insertNonbreakingSpace();
    void insertNonbreakingHyphen();
    void insertText( const QString &text );
    void newParagraph();
    void setBold(bool b);
    void setItalic(bool on);
    void setUnderline(bool on);
    void setDoubleUnderline(bool on);
    void setStrikeOut(bool on);
    void setPointSize( int s );
    void setTextSubScript(bool on);
    void setTextSuperScript(bool on);
    void setUnderlineColor( const QColor & color );
    void setDefaultFormat();
    void setRelativeTextSize( double _size);

    QColor textColor() const;
    QString textFontFamily()const;
    QColor textBackgroundColor()const;
    QColor textUnderlineColor() const;
    bool textDoubleUnderline() const;

    bool textItalic() const;
    bool textBold() const;
    bool textUnderline()const;
    bool textStrikeOut()const;
    bool textSubScript() const;
    bool textSuperScript() const;
    // TODO text shadow
    double relativeTextSize() const;
    bool wordByWord() const;

    void setWordByWord( bool _b );
    void setTextColor(const QColor &color);
    void setTextBackgroundColor(const QColor &);
    void setAlign(int align);
    void setAlign(const QString &);

    bool isReadWrite() const ;
    void setReadWrite( bool b );

    void hideCursor();
    void showCursor();

    int cursorParagraph() const;
    int cursorIndex() const;

    void moveCursorLeft(bool select);
    void moveCursorRight(bool select);
    void moveCursorUp(bool select);
    void moveCursorDown(bool select);
    void moveCursorHome(bool select);
    void moveCursorEnd(bool select);
    void moveCursorWordRight(bool select);
    void moveCursorWordLeft(bool select);
    void moveCursorLineEnd(bool select);
    void moveCursorLineStart(bool select);

    bool paragraphHasBorder() const;

    //return line spacing in pt
    double lineSpacing() const;

    double rightMargin() const;
    double leftMargin() const;
    double marginFirstLine() const;
    double spaceAfterParag() const;
    double spaceBeforeParag() const;

    void setMarginFirstLine(double pt);
    void setLineSpacing(double pt);
    void setLeftMargin(double pt);
    void setRightMargin(double pt);
    void setSpaceBeforeParag(double pt);
    void setSpaceAfterParag(double pt);

    // apply border, for the futur add border style
    void setLeftBorder( const QColor & c,double width );
    void setRightBorder( const QColor & c,double width );

    void setTopBorder( const QColor & c,double width );
    void setBottomBorder(const QColor & c,double width );

    void setLeftBorderColor( const QColor & c );
    void setRightBorderColor( const QColor & c );
    void setTopBorderColor( const QColor & c);
    void setBottomBorderColor(const QColor & c );
    void setLeftBorderWidth( double _witdh );
    void setRightBorderWidth( double _witdh );
    void setTopBorderWidth( double _witdh );
    void setBottomBorderWidth( double _witdh );


    //return border width in pt
    double leftBorderWidth() const ;
    double rightBorderWidth() const;
    double topBorderWidth() const;
    double bottomBorderWidth() const;

    QColor leftBorderColor() const ;
    QColor rightBorderColor() const;
    QColor topBorderColor() const;
    QColor bottomBorderColor() const;

    void changeCaseOfText( const QString & caseType);
    bool isALinkVariable() const;
    //return false if there is not a link
    bool changeLinkVariableUrl( const QString & _url) const;
    //return false if there is not a link
    bool changeLinkVariableName( const QString & _name) const;

    //be carefull these functions return QString::null when there is not
    //a variable
    QString linkVariableUrl( ) const;
    QString linkVariableName( ) const;

    bool isANoteVariable() const ;
    QString noteVariableText() const;
    //return false if there is not a note variable
    bool setNoteVariableText(const QString & note) const;

    void removeComment();
    QString underlineStyle() const;
    QString strikeOutStyle()const;

    void setLanguage(const QString & _lang);
    QString language() const;
    void addBookmarks(const QString &url);
    void copyLink();
    void removeLink();
    void copyTextOfComment();
    QString fontAttibute()const;
private:
    KoTextView *m_textView;
protected:
    //QString lineStyleToString( KoTextFormat::LineStyle _style ) const;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2002 Laurent MONTEL <lmontel@mandrakesoft.com>

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

#include "KoTextViewIface.h"
#include "KoTextView.h"
#include "KoTextParag.h"
#include "KoBorder.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <KoVariable.h>
#include <kcommand.h>


KoTextViewIface::KoTextViewIface( KoTextView *_textview )
    : DCOPObject( /*TODO name*/ )
{
   m_textView = _textview;
}

void KoTextViewIface::insertSoftHyphen()
{
   m_textView->insertSoftHyphen();
}

void KoTextViewIface::insertText( const QString &text )
{
    m_textView->insertText(text);
}

void KoTextViewIface::newParagraph()
{
    m_textView->newParagraph();
}


void KoTextViewIface::setBold(bool b)
{
    KCommand *cmd=m_textView->setBoldCommand(b);
    delete cmd;
}

void KoTextViewIface::setItalic(bool on)
{
    KCommand *cmd=m_textView->setItalicCommand(on);
    delete cmd;
}

void KoTextViewIface::setUnderline(bool on)
{
    KCommand *cmd=m_textView->setUnderlineCommand(on);
    delete cmd;
}

void KoTextViewIface::setDoubleUnderline(bool on)
{
    KCommand *cmd=m_textView->setDoubleUnderlineCommand(on);
    delete cmd;
}


void KoTextViewIface::setStrikeOut(bool on)
{
    KCommand *cmd=m_textView->setStrikeOutCommand(on);
    delete cmd;
}

void KoTextViewIface::setPointSize( int s )
{
    KCommand *cmd=m_textView->setPointSizeCommand(s);
    delete cmd;
}

void KoTextViewIface::setTextSubScript(bool on)
{
    KCommand *cmd=m_textView->setTextSubScriptCommand(on);
    delete cmd;
}

void KoTextViewIface::setTextSuperScript(bool on)
{
    KCommand *cmd=m_textView->setTextSuperScriptCommand(on);
    delete cmd;
}

void KoTextViewIface::setLanguage(const QString & _lang)
{
    KCommand *cmd=m_textView->setLanguageCommand(_lang);
    delete cmd;
}

QString KoTextViewIface::language() const
{
    return m_textView->language();
}

void KoTextViewIface::setDefaultFormat()
{
    KCommand *cmd=m_textView->setDefaultFormatCommand();
    delete cmd;
}

QColor KoTextViewIface::textColor() const
{
    return m_textView->textColor();
}

QString KoTextViewIface::textFontFamily()const
{
    return m_textView->textFontFamily();
}

QColor KoTextViewIface::textBackgroundColor()const
{
    return m_textView->textBackgroundColor();
}

QColor KoTextViewIface::textUnderlineColor() const
{
    return m_textView->textUnderlineColor();
}

double KoTextViewIface::relativeTextSize() const
{
    return m_textView->relativeTextSize();
}

void KoTextViewIface::setRelativeTextSize( double _size)
{
    KCommand *cmd=m_textView->setRelativeTextSizeCommand(_size);
    delete cmd;
}

void KoTextViewIface::setUnderlineColor( const QColor & color )
{
    KCommand *cmd=m_textView->setUnderlineColorCommand(color);
    delete cmd;
}

void KoTextViewIface::setTextColor(const QColor &color)
{
    KCommand *cmd=m_textView->setTextColorCommand(color);
    delete cmd;
}

void KoTextViewIface::setTextBackgroundColor(const QColor &color)
{
    KCommand *cmd=m_textView->setTextBackgroundColorCommand(color);
    delete cmd;
}

void KoTextViewIface::setAlign(int align)
{
    KCommand *cmd=m_textView->setAlignCommand(align);
    delete cmd;
}

void KoTextViewIface::setAlign(const QString &align)
{
    KCommand *cmd=0L;
    if( align=="AlignLeft")
        cmd=m_textView->setAlignCommand(Qt::AlignLeft);
    else if (align=="AlignRight")
        cmd=m_textView->setAlignCommand(Qt::AlignRight);
    else if (align=="AlignCenter" || align=="AlignHCenter")
        cmd=m_textView->setAlignCommand(Qt::AlignHCenter);
    else if (align=="AlignJustify")
        cmd=m_textView->setAlignCommand(Qt::AlignJustify);
    else
    {
        kDebug(32500)<<"Align value not recognized...\n";
        cmd=m_textView->setAlignCommand(Qt::AlignLeft);
    }
    delete cmd;
}

bool KoTextViewIface::textDoubleUnderline() const
{
    return m_textView->textDoubleUnderline();
}

bool KoTextViewIface::textItalic() const
{
    return m_textView->textItalic();
}

bool KoTextViewIface::textBold() const
{
    return m_textView->textBold();
}

bool KoTextViewIface::textUnderline()const
{
    return m_textView->textUnderline();
}

bool KoTextViewIface::textStrikeOut()const
{
    return m_textView->textStrikeOut();
}

bool KoTextViewIface::textSubScript() const
{
    return m_textView->textSubScript();
}

bool KoTextViewIface::textSuperScript() const
{
    return m_textView->textSuperScript();
}

bool KoTextViewIface::isReadWrite() const
{
    return m_textView->isReadWrite();
}

void KoTextViewIface::setReadWrite( bool b )
{
    m_textView->setReadWrite(b);
}

void KoTextViewIface::hideCursor()
{
    m_textView->hideCursor();
}

void KoTextViewIface::showCursor()
{
    m_textView->showCursor();
}

int KoTextViewIface::cursorParagraph() const
{
    return m_textView->cursor()->parag()->paragId();
}

int KoTextViewIface::cursorIndex() const
{
    return m_textView->cursor()->index();
}

void KoTextViewIface::moveCursorLeft(bool select)
{
  m_textView->moveCursor(KoTextView::MoveBackward,select);
}

void KoTextViewIface::moveCursorRight(bool select)
{
  m_textView->moveCursor(KoTextView::MoveForward,select);
}

void KoTextViewIface::moveCursorUp(bool select)
{
  m_textView->moveCursor(KoTextView::MoveUp,select);
}

void KoTextViewIface::moveCursorDown(bool select)
{
  m_textView->moveCursor(KoTextView::MoveDown,select);
}

void KoTextViewIface::moveCursorHome(bool select)
{
  m_textView->moveCursor(KoTextView::MoveHome,select);
}

void KoTextViewIface::moveCursorEnd(bool select)
{
  m_textView->moveCursor(KoTextView::MoveEnd,select);
}

void KoTextViewIface::moveCursorWordLeft(bool select)
{
  m_textView->moveCursor(KoTextView::MoveWordBackward,select);
}

void KoTextViewIface::moveCursorWordRight(bool select)
{
  m_textView->moveCursor(KoTextView::MoveWordForward,select);
}

void KoTextViewIface::moveCursorLineEnd(bool select)
{
  m_textView->moveCursor(KoTextView::MoveLineEnd,select);
}

void KoTextViewIface::moveCursorLineStart(bool select)
{
  m_textView->moveCursor(KoTextView::MoveLineStart,select);
}

bool KoTextViewIface::paragraphHasBorder() const
{
    return m_textView->cursor()->parag()->hasBorder();
}

double KoTextViewIface::lineSpacing() const
{
    return m_textView->cursor()->parag()->kwLineSpacing();
}

double KoTextViewIface::leftMargin() const
{
    return m_textView->cursor()->parag()->margin( Q3StyleSheetItem::MarginLeft);
}

double KoTextViewIface::rightMargin() const
{
    return m_textView->cursor()->parag()->margin( Q3StyleSheetItem::MarginRight);
}

double KoTextViewIface::spaceBeforeParag() const
{
    return m_textView->cursor()->parag()->margin( Q3StyleSheetItem::MarginTop);
}

double KoTextViewIface::spaceAfterParag() const
{
    return m_textView->cursor()->parag()->margin( Q3StyleSheetItem::MarginBottom);
}

double KoTextViewIface::marginFirstLine() const
{
    return m_textView->cursor()->parag()->margin( Q3StyleSheetItem::MarginFirstLine);
}

void KoTextViewIface::setMarginFirstLine(double pt)
{
    m_textView->cursor()->parag()->setMargin( Q3StyleSheetItem::MarginFirstLine,pt);
}

void KoTextViewIface::setLineSpacing(double pt)
{
    m_textView->cursor()->parag()->setLineSpacing(pt);
}

void KoTextViewIface::setLeftMargin(double pt)
{
    m_textView->cursor()->parag()->setMargin( Q3StyleSheetItem::MarginLeft,pt);
}

void KoTextViewIface::setRightMargin(double pt)
{
    m_textView->cursor()->parag()->setMargin( Q3StyleSheetItem::MarginRight,pt);
}

void KoTextViewIface::setSpaceBeforeParag(double pt)
{
    m_textView->cursor()->parag()->setMargin( Q3StyleSheetItem::MarginTop,pt);
}

void KoTextViewIface::setSpaceAfterParag(double pt)
{
    m_textView->cursor()->parag()->setMargin( Q3StyleSheetItem::MarginBottom,pt);
}


void KoTextViewIface::setLeftBorder( const QColor & c,double width )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->setLeftBorder(KoBorder( c, KoBorder::SOLID, width ));

}

void KoTextViewIface::setRightBorder( const QColor & c,double width )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->setRightBorder(KoBorder( c, KoBorder::SOLID, width ));
}

void KoTextViewIface::setTopBorder( const QColor & c,double width )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->setTopBorder(KoBorder( c, KoBorder::SOLID, width ));
}

void KoTextViewIface::setBottomBorder(const QColor & c,double width )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->setBottomBorder(KoBorder( c, KoBorder::SOLID, width ));
}

double KoTextViewIface::leftBorderWidth() const
{
    KoTextParag *parag= m_textView->cursor()->parag();
    return  parag->leftBorder().width();
}

double KoTextViewIface::rightBorderWidth() const
{
    KoTextParag *parag= m_textView->cursor()->parag();
    return parag->rightBorder().width();

}
double KoTextViewIface::topBorderWidth() const
{
    KoTextParag *parag= m_textView->cursor()->parag();
    return parag->topBorder().width();

}

double KoTextViewIface::bottomBorderWidth() const
{
    KoTextParag *parag= m_textView->cursor()->parag();
    return parag->bottomBorder().width();

}

QColor KoTextViewIface::leftBorderColor() const
{
    KoTextParag *parag= m_textView->cursor()->parag();
    return parag->leftBorder().color;
}

QColor KoTextViewIface::rightBorderColor() const
{
    KoTextParag *parag= m_textView->cursor()->parag();
    return parag->rightBorder().color;
}

QColor KoTextViewIface::topBorderColor() const
{
    KoTextParag *parag= m_textView->cursor()->parag();
    return parag->topBorder().color;
}

QColor KoTextViewIface::bottomBorderColor() const
{
    KoTextParag *parag= m_textView->cursor()->parag();
    return parag->bottomBorder().color;
}

void KoTextViewIface::setLeftBorderColor( const QColor & c )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->leftBorder().color = c ;
}

void KoTextViewIface::setRightBorderColor( const QColor & c )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->rightBorder().color = c ;
}

void KoTextViewIface::setTopBorderColor( const QColor & c )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->topBorder().color = c ;
}

void KoTextViewIface::setBottomBorderColor(const QColor & c )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->bottomBorder().color = c ;
}

void KoTextViewIface::setLeftBorderWidth( double _witdh )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->leftBorder().setPenWidth(_witdh) ;
}

void KoTextViewIface::setRightBorderWidth( double _witdh )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->rightBorder().setPenWidth(_witdh) ;
}

void KoTextViewIface::setTopBorderWidth( double _witdh )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->topBorder().setPenWidth(_witdh) ;
}

void KoTextViewIface::setBottomBorderWidth( double _witdh )
{
    KoTextParag *parag= m_textView->cursor()->parag();
    parag->bottomBorder().setPenWidth(_witdh) ;
}


void KoTextViewIface::changeCaseOfText( const QString & caseType)
{
    KCommand *cmd=0L;
    if( caseType.toLower() == "uppercase" )
    {
        cmd=m_textView->setChangeCaseOfTextCommand( KoChangeCaseDia::UpperCase );
    }
    else if( caseType.toLower() =="lowercase" )
    {
        cmd=m_textView->setChangeCaseOfTextCommand( KoChangeCaseDia::LowerCase );
    }
    else if( caseType.toLower() =="titlecase" )
    {
        cmd=m_textView->setChangeCaseOfTextCommand( KoChangeCaseDia::TitleCase );
    }
    else if( caseType.toLower() =="togglecase" )
    {
        cmd=m_textView->setChangeCaseOfTextCommand( KoChangeCaseDia::ToggleCase );
    }
    else if ( caseType.toLower() =="sentencecase" )
    {
        cmd=m_textView->setChangeCaseOfTextCommand( KoChangeCaseDia::SentenceCase );

    }
    else
        kDebug(32500)<<"Error in void KoTextViewIface::changeCaseOfText( const QString & caseType) parameter\n";
    delete cmd;
}

bool KoTextViewIface::isALinkVariable() const
{
    return (m_textView->linkVariable()!=0);
}

QString KoTextViewIface::linkVariableUrl( ) const
{
    KoLinkVariable *var=m_textView->linkVariable();
    if ( !var)
        return QString::null;
    else
    {
        return var->url();
    }
}

QString KoTextViewIface::linkVariableName( ) const
{
    KoLinkVariable *var=m_textView->linkVariable();
    if ( !var)
        return QString::null;
    else
    {
        return var->value();
    }
}


bool KoTextViewIface::changeLinkVariableUrl( const QString & _url) const
{
    KoLinkVariable *var=m_textView->linkVariable();
    if ( !var)
        return false;
    else
    {
        var->setLink(var->value(), _url);
        var->recalc();
    }
    return true;
}

bool KoTextViewIface::changeLinkVariableName( const QString & _name) const
{
    KoLinkVariable *var=m_textView->linkVariable();
    if ( !var)
        return false;
    else
    {
        var->setLink(_name, var->url());
        var->recalc();
    }
    return true;
}

bool KoTextViewIface::isANoteVariable() const
{
    KoNoteVariable *var = dynamic_cast<KoNoteVariable *>(m_textView->variable());
    return (var!=0);
}

QString KoTextViewIface::noteVariableText() const
{
    KoNoteVariable *var = dynamic_cast<KoNoteVariable *>(m_textView->variable());
    if( var )
        return var->note();
    else
        return QString::null;
}

bool KoTextViewIface::setNoteVariableText(const QString & note) const
{
    KoNoteVariable *var = dynamic_cast<KoNoteVariable *>(m_textView->variable());
    if( var )
    {
        var->setNote( note);
        return true;
    }
    else
        return false;
}

void KoTextViewIface::removeComment()
{
    removeComment();
}

QString KoTextViewIface::underlineStyle() const
{
    return KoTextFormat::underlineStyleToString( m_textView->underlineStyle() );
}

QString KoTextViewIface::strikeOutStyle()const
{
    return KoTextFormat::strikeOutStyleToString( m_textView->strikeOutStyle() );
}

void KoTextViewIface::addBookmarks(const QString &url)
{
    m_textView->addBookmarks(url);
}

void KoTextViewIface::copyLink()
{
    m_textView->copyLink();
}

void KoTextViewIface::removeLink()
{
    m_textView->removeLink();
}

bool KoTextViewIface::wordByWord() const
{
    return m_textView->wordByWord();
}

void KoTextViewIface::setWordByWord( bool _b )
{
    KCommand *cmd=m_textView->setWordByWordCommand(_b );
    delete cmd;
}

void KoTextViewIface::copyTextOfComment()
{
    m_textView->copyTextOfComment();
}

QString KoTextViewIface::fontAttibute()const
{
    return KoTextFormat::attributeFontToString( m_textView->fontAttribute() );
}

void KoTextViewIface::insertNonbreakingSpace()
{
    m_textView->insertNonbreakingSpace();
}

void KoTextViewIface::insertNonbreakingHyphen()
{
    m_textView->insertNonbreakingHyphen();
}

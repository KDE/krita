/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
                 2009 Jeremias Epperlein <jeeree@web.de>

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
   Boston, MA 02110-1301, USA.
*/

#include "FormulaEditor.h"
#include "BasicElement.h"
#include "RowElement.h"
#include "FixedElement.h"
#include "NumberElement.h"
#include "TableElement.h"
#include "TableDataElement.h"
#include "TableRowElement.h"
#include "ElementFactory.h"
#include "OperatorElement.h"
#include "IdentifierElement.h"
#include "ElementFactory.h"
#include "FormulaCommand.h"
#include <QPainter>
#include <QPen>
#include <algorithm>
#include <QObject>

#include <kdebug.h>
#include <klocale.h>
#include <kundo2command.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>

FormulaEditor::FormulaEditor( FormulaCursor cursor, FormulaData* data )
{
    m_cursor=cursor;
    m_data=data;
}

FormulaEditor::FormulaEditor ( FormulaData* data )
{
    m_cursor=FormulaCursor(data->formulaElement(),0);
    m_data=data;
}



void FormulaEditor::paint( QPainter& painter ) const
{
    m_cursor.paint(painter);
}

FormulaCommand* FormulaEditor::insertText( const QString& text )
{
    FormulaCommand *undo = 0;
    m_inputBuffer = text;
    if (m_cursor.insideToken()) {
        TokenElement* token=static_cast<TokenElement*>(m_cursor.currentElement());
        if (m_cursor.hasSelection()) {
            undo=new FormulaCommandReplaceText(token,m_cursor.selection().first,m_cursor.selection().second-m_cursor.selection().first,text);
        } else {
            undo=new FormulaCommandReplaceText(token,m_cursor.position(),0,text);
        }
    } else {
        TokenElement* token = static_cast<TokenElement*>
            (ElementFactory::createElement(tokenType(text[0]),0));
        token->insertText(0,text);
        undo=insertElement(token);
        if (undo) {
            undo->setRedoCursorPosition(FormulaCursor(token,text.length()));
        }
    }
    if (undo) {
        undo->setText(i18nc("(qtundo-format)", "Add text"));
    }
    return undo;
}

FormulaCommand* FormulaEditor::insertMathML( const QString& data )
{
    // setup a DOM structure and start the actual loading process
    KoXmlDocument tmpDocument;
    tmpDocument.setContent( QString(data), false, 0, 0, 0 );
    BasicElement* element=ElementFactory::createElement(tmpDocument.documentElement().tagName(),0);
    element->readMathML( tmpDocument.documentElement() );     // and load the new formula
    FormulaCommand* command=insertElement( element );
    kDebug()<<"Inserting "<< tmpDocument.documentElement().tagName();
    if (command==0) {
        delete element;
    }
    return command;
}

FormulaCommand* FormulaEditor::changeTable ( bool insert, bool rows )
{
    FormulaCommand* undo;
    TableDataElement* data=m_cursor.currentElement()->parentTableData();
    if (data) {
        TableElement* table=static_cast<TableElement*>(data->parentElement()->parentElement());
        int rowNumber=table->childElements().indexOf(data->parentElement());
        int columnNumber=data->parentElement()->childElements().indexOf(data);
        if (rows) {
            //Changing rows
            if (insert) {
                undo=new FormulaCommandReplaceRow(formulaData(),cursor(),table,rowNumber,0,1);
                if (undo) {
                    undo->setText(i18nc("(qtundo-format)", "Insert row"));
                }
            } else {
                undo=new FormulaCommandReplaceRow(formulaData(),cursor(),table,rowNumber,1,0);
                if (undo) {
                    undo->setText(i18nc("(qtundo-format)", "Remove row"));
                }
            }
        } else {
            //Changing columns
            if (insert) {
                undo=new FormulaCommandReplaceColumn(formulaData(),cursor(),table,columnNumber,0,1);
                if (undo) {
                    undo->setText(i18nc("(qtundo-format)", "Insert column"));
                }
            } else {
                undo=new FormulaCommandReplaceColumn(formulaData(),cursor(),table,columnNumber,1,0);
                if (undo) {
                    undo->setText(i18nc("(qtundo-format)", "Remove column"));
                }
            }
        }
    } else {
        return 0;
    }
    return undo;
}

FormulaCommand* FormulaEditor::insertElement( BasicElement* element )
{
    FormulaCommand *undo = 0;
    if (m_cursor.insideInferredRow()) {
        RowElement* tmprow=static_cast<RowElement*>(m_cursor.currentElement());
        QList<BasicElement*> list;
        list<<element;
        if (m_cursor.hasSelection()) {
            undo=new FormulaCommandReplaceElements(tmprow,m_cursor.selection().first,m_cursor.selection().second-m_cursor.selection().first,list,true);
        } else {
            undo=new FormulaCommandReplaceElements(tmprow,m_cursor.position(),0,list,false);
        }
    } else if (m_cursor.insideToken() && element->elementType()==Glyph) {
        //TODO: implement the insertion of glyphs
    }
    if (undo) {
        undo->setText(i18nc("(qtundo-format)", "Insert formula elements."));
        undo->setUndoCursorPosition(cursor());
    }
    return undo;
}

FormulaCommand* FormulaEditor::remove( bool elementBeforePosition )
{
    FormulaCommand *undo=0;
    if (m_cursor.insideInferredRow()) {
        RowElement* tmprow=static_cast<RowElement*>(m_cursor.currentElement());
        if (m_cursor.isSelecting()) {
            undo=new FormulaCommandReplaceElements(tmprow,m_cursor.selection().first,m_cursor.selection().second-m_cursor.selection().first,QList<BasicElement*>());
        } else {
            if (elementBeforePosition && !m_cursor.isHome()) {
                undo=new FormulaCommandReplaceElements(tmprow,m_cursor.position()-1,1,QList<BasicElement*>());
            } else if (!elementBeforePosition && !m_cursor.isEnd()) {
                undo=new FormulaCommandReplaceElements(tmprow,m_cursor.position(),1,QList<BasicElement*>());
            }
        }
    } else if (m_cursor.insideToken()) {
        TokenElement* tmptoken=static_cast<TokenElement*>(m_cursor.currentElement());
        if (m_cursor.hasSelection()) {
            undo=new FormulaCommandReplaceText(tmptoken,m_cursor.selection().first,m_cursor.selection().second-m_cursor.selection().first,"");
        } else {
            if (elementBeforePosition && !m_cursor.isHome()) {
                undo=new FormulaCommandReplaceText(tmptoken,m_cursor.position()-1,1,"");
            } else if (!elementBeforePosition && !m_cursor.isEnd()) {
                undo=new FormulaCommandReplaceText(tmptoken,m_cursor.position(),1,"");
            }
        }
    }
    if (undo) {
        undo->setText(i18nc("(qtundo-format)", "Remove formula elements"));
        undo->setUndoCursorPosition(cursor());
    }
    return undo;
}

void FormulaEditor::setData ( FormulaData* data )
{
    m_data=data;
}


FormulaData* FormulaEditor::formulaData() const
{
    return m_data;
}

QString FormulaEditor::inputBuffer() const
{
    return m_inputBuffer;
}

QString FormulaEditor::tokenType ( const QChar& character ) const
{
    QChar::Category chat=character.category();
    if (character.isNumber()) {
        return "mn";
    }
    else if (chat==QChar::Punctuation_Connector ||
             chat==QChar::Punctuation_Dash ||
             chat==QChar::Punctuation_Open ||
             chat==QChar::Punctuation_Close ||
             chat==QChar::Punctuation_InitialQuote ||
             chat==QChar::Punctuation_FinalQuote ||
             chat==QChar::Symbol_Math) {
        return "mo";
    }
    else if (character.isLetter()) {
        return "mi";
    }
    return "mi";
}


FormulaCursor& FormulaEditor::cursor() 
{
    return m_cursor;
}

void FormulaEditor::setCursor ( FormulaCursor& cursor )
{
    m_cursor=cursor;
}


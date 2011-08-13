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

#include "FormulaCommand.h"
#include "FormulaCursor.h"
#include <klocale.h> 
#include "TokenElement.h"
#include "BasicElement.h"
#include "TableElement.h"
#include "TableRowElement.h"
#include "TableDataElement.h"
#include <kdebug.h>

FormulaCommand::FormulaCommand(KUndo2Command* parent)
              : KUndo2Command(parent)
{
    m_done=false;
}

void FormulaCommand::changeCursor ( FormulaCursor& cursor, bool undo ) const
{
    if (undo) {
        cursor.moveTo(m_undoCursorPosition);
    } else {
        cursor.moveTo(m_redoCursorPosition);
    }
    if (!cursor.isAccepted()) {
        cursor.move(MoveRight);
    }
}

void FormulaCommand::setUndoCursorPosition ( const FormulaCursor& position )
{
    m_undoCursorPosition=position;
}

void FormulaCommand::setRedoCursorPosition ( const FormulaCursor& position )
{
    m_redoCursorPosition=position;
}

FormulaCommandReplaceText::FormulaCommandReplaceText( TokenElement* owner, int position, int length, const QString& added , KUndo2Command* parent)
                  : FormulaCommand(parent)
{
    m_ownerElement = owner;
    m_position = position;
    m_added = added;
    m_length = length;
    m_removedGlyphs=m_ownerElement->glyphList(position,length);
    m_removed=m_ownerElement->text().mid(position,length);
    setText( i18nc( "(qtundo-format)", "Add text to formula" ) );
    setUndoCursorPosition(FormulaCursor(m_ownerElement, m_position+m_removed.length()));
    setRedoCursorPosition(FormulaCursor(m_ownerElement, m_position+m_added.length()));
}

FormulaCommandReplaceText::~FormulaCommandReplaceText()
{
}

void FormulaCommandReplaceText::redo()
{
    m_done=true;
    if (m_length>0) {
        m_glyphpos=m_ownerElement->removeText(m_position,m_length);
    }
    m_ownerElement->insertText(m_position, m_added);
}

void FormulaCommandReplaceText::undo()
{
    m_done=false;
    m_ownerElement->removeText(m_position,m_added.length());
    m_ownerElement->insertText(m_position, m_removed);
    m_ownerElement->insertGlyphs(m_glyphpos,m_removedGlyphs);
}

FormulaCommandReplaceElements::FormulaCommandReplaceElements ( RowElement* owner, int position, int length, QList< BasicElement* > elements, bool wrap,KUndo2Command* parent )
                            : FormulaCommand(parent)
{
    m_ownerElement=owner;
    m_position=position;
    m_added=elements;
    m_length=length;
    m_wrap=wrap;
    m_removed=m_ownerElement->childElements().mid(m_position,m_length);
    m_placeholderPosition=0;
    
    //we have to remember to which descendant of m_added the elements got moved
    BasicElement* placeholder=0;
    foreach (BasicElement* tmp, m_added) {
        if ( (placeholder=tmp->emptyDescendant()) ) {
            break;
        }
    }
    if (placeholder) { //we are actually wrapping stuff
        //empty descandant only returns a element hows parent is an inferred mrow
        m_placeholderParent=static_cast<RowElement*>(placeholder->parentElement());
        m_placeholderPosition=m_placeholderParent->positionOfChild(placeholder);
        m_placeholderParent->removeChild(placeholder);
        delete placeholder;
        if (m_wrap) {
            setRedoCursorPosition(FormulaCursor(m_placeholderParent,m_placeholderPosition+m_removed.count()));
        } else {
            setRedoCursorPosition(FormulaCursor(m_placeholderParent,m_placeholderPosition));
        }
    } else {
        m_placeholderParent=0;
        setRedoCursorPosition(FormulaCursor(m_ownerElement,m_position+m_added.length()));
    }
    setUndoCursorPosition(FormulaCursor(m_ownerElement,m_position+m_removed.length()));
}

FormulaCommandReplaceElements::~FormulaCommandReplaceElements()
{
    if (m_done) {
        if (!(m_wrap && m_placeholderParent)) {
            foreach (BasicElement* tmp, m_removed) {
                delete tmp;
            }
        }
    } else {
        foreach (BasicElement* tmp, m_added) {
            delete tmp;
        }
    }
}

void FormulaCommandReplaceElements::redo()
{
    m_done=true;
    for (int i=0; i<m_length; ++i) {
        m_ownerElement->removeChild(m_removed[i]);
    }
    if (m_wrap &&  m_placeholderParent!=0) {
        int counter=0;
        foreach (BasicElement *tmp, m_removed) {
            m_placeholderParent->insertChild(m_placeholderPosition+counter,tmp);
            counter++;
        }
    }
    for (int i=0; i<m_added.length(); ++i) {
        m_ownerElement->insertChild(m_position+i,m_added[i]);
    }
}

void FormulaCommandReplaceElements::undo()
{
    m_done=false;
    for (int i=0; i<m_added.length(); ++i) {
        m_ownerElement->removeChild(m_added[i]);
    }
    if (m_wrap &&  m_placeholderParent!=0) {
        foreach (BasicElement *tmp, m_removed) {
            m_placeholderParent->removeChild(tmp);
        }
    }
    for (int i=0; i<m_length; ++i) {
        m_ownerElement->insertChild(m_position+i,m_removed[i]);
    }
}

FormulaCommandLoad::FormulaCommandLoad ( FormulaData* data, FormulaElement* newelement, KUndo2Command* parent )
                   : FormulaCommand ( parent)
{
    m_data=data;
    m_newel=newelement;
    m_oldel=data->formulaElement();
    setUndoCursorPosition(FormulaCursor(m_oldel,0));
    setRedoCursorPosition(FormulaCursor(m_newel,0));
}

FormulaCommandLoad::~FormulaCommandLoad()
{
    if (m_done) {
    } else {
    }
}

void FormulaCommandLoad::redo()
{
    m_done=true;
    m_data->setFormulaElement(m_newel);
}

void FormulaCommandLoad::undo()
{
    m_done=false;
    m_data->setFormulaElement(m_oldel);
}

FormulaCommandReplaceRow::FormulaCommandReplaceRow ( FormulaData* data, FormulaCursor oldposition, TableElement* table, int number, int oldlength, int newlength)
{
    m_data=data;
    m_table=table;
    m_number=number;
    m_empty=0;
    int columnnumber=m_table->childElements()[0]->childElements().count();
    TableRowElement* tmpRow;
    for (int i=0; i<newlength;i++) {
        tmpRow = new TableRowElement();
        for (int j=0; j<columnnumber; j++) {
            tmpRow->insertChild(i,new TableDataElement());
        }
        m_newRows<<tmpRow;
    }
    m_oldRows=table->childElements().mid(number, oldlength);
    setText( i18nc( "(qtundo-format)", "Change rows" ) );
    if (newlength==0 && oldlength>=table->childElements().count()) {
        m_empty=new TableRowElement();
        m_empty->insertChild(0, new TableDataElement());
    }
    setUndoCursorPosition(oldposition);

    if (newlength>0) {
        setRedoCursorPosition(FormulaCursor(m_newRows[0]->childElements()[0],0));
    } else {
        if (m_empty) {
            setRedoCursorPosition(FormulaCursor(m_empty->childElements()[0],0));
        } else {
            int rowcount=m_table->childElements().count();
            if (number+oldlength < rowcount) {
                //we can place the cursor after the removed elements
                setRedoCursorPosition(FormulaCursor(table->childElements()[number+oldlength]->childElements()[0],0));
            } else {
                //we have to place the cursor before the removed rows
                setRedoCursorPosition(FormulaCursor(table->childElements()[number==0 ? 0: number-1]->childElements()[0],0));
            }
        }
    }
}

FormulaCommandReplaceRow::~FormulaCommandReplaceRow()
{
    if (m_done) {
        qDeleteAll(m_oldRows);
    } else {
        if (m_empty) {
            delete m_empty;
        } else {
            qDeleteAll(m_newRows);
        }
    }
}

void FormulaCommandReplaceRow::redo()
{
    for (int i=0; i<m_oldRows.count(); i++) {
        m_table->removeChild(m_oldRows[i]);
    }
    if (m_empty) {
        m_table->insertChild(0,m_empty);
    } else {
        for (int i=0; i<m_newRows.count(); i++) {
            m_table->insertChild(i+m_number,m_newRows[i]);
        }
    }
}

void FormulaCommandReplaceRow::undo()
{
    if (m_empty) {
        m_table->removeChild(m_empty);
    } else {
        for (int i=0; i<m_newRows.count(); i++) {
            m_table->removeChild(m_newRows[i]);
        }
    }
    for (int i=0; i<m_oldRows.count(); i++) {
        m_table->insertChild(i+m_number,m_oldRows[i]);
    }
}


FormulaCommandReplaceColumn::FormulaCommandReplaceColumn ( FormulaData* data, FormulaCursor oldcursor, TableElement* table, int position, int oldlength, int newlength)
{
    m_data=data;
    m_table=table;
    m_position=position;
    m_empty=0;
    int rownumber=m_table->childElements().count();
    QList<BasicElement*> tmp;

    if (newlength==0 && oldlength>=table->childElements().count()) {
        //we remove the whole table
        m_empty=new TableRowElement();
        m_empty->insertChild(0, new TableDataElement());
        m_oldRows=table->childElements();
    } else {
        for (int i=0; i<newlength;i++) {
            for (int j=0; j<rownumber;j++) {
                tmp<<new TableDataElement();
            }
            m_newColumns<<tmp;
            tmp.clear();
        }
        for (int i=0; i<oldlength;i++) {
            for (int j=0; j<rownumber;j++) {
                tmp<<table->childElements()[j]->childElements()[m_position+i];
            }
            m_oldColumns<<tmp;
            tmp.clear();
        }
    }
    setUndoCursorPosition(oldcursor);

    if (newlength>0) {
        setRedoCursorPosition(FormulaCursor(m_newColumns[0][0],0));
    } else {
        if (m_empty) {
            setRedoCursorPosition(FormulaCursor(m_empty->childElements()[0],0));
        } else {
            int columncount=m_table->childElements()[0]->childElements().count();
            if (position+oldlength < columncount) {
                //we can place the cursor after the removed elements
                setRedoCursorPosition(FormulaCursor(table->childElements()[0]->childElements()[position+oldlength],0));
            } else {
                //we have to place the cursor before the removed rows
                setRedoCursorPosition(FormulaCursor(table->childElements()[0]->childElements()[position==0 ? 0: position-1],0));
            }
        }
    }
}

FormulaCommandReplaceColumn::~FormulaCommandReplaceColumn()
{
    if (m_done) {
        if (m_empty) {
            qDeleteAll(m_oldRows);
        } else {
            foreach (QList<BasicElement*> column, m_oldColumns) {
                foreach( BasicElement* element, column) {
                    delete element;
                }
            }
        }
    } else {
        if (m_empty) {
            delete m_empty;
        } else {
            foreach (QList<BasicElement*> column, m_newColumns) {
                foreach( BasicElement* element, column) {
                    delete element;
                }
            }
        }
    }
}

void FormulaCommandReplaceColumn::redo()
{
    if (m_empty) {
        for (int i=0; i<m_oldRows.count();i++) {
            m_table->removeChild(m_oldRows[i]);
        }
        m_table->insertChild(0,m_empty);
    } else {
        for (int i=0; i<m_table->childElements().count(); i++) {
            TableRowElement* row=static_cast<TableRowElement*>(m_table->childElements()[i]);
            for (int j=0; j<m_oldColumns.count(); j++) {
                row->removeChild(m_oldColumns[j][i]);
            }
            for (int j=0; j<m_newColumns.count(); j++) {
                row->insertChild(m_position+j,m_newColumns[j][i]);
            }
        }
    }
}

void FormulaCommandReplaceColumn::undo()
{
    if (m_empty) {
        m_table->removeChild(m_empty);
        for (int i=0; i<m_oldRows.count(); ++i) {
            m_table->insertChild(i,m_oldRows[i]);
        }
    } else {
        for (int i=0; i<m_table->childElements().count(); i++) {
            TableRowElement* row=static_cast<TableRowElement*>(m_table->childElements()[i]);
            for (int j=0; j<m_newColumns.count(); j++) {
                row->removeChild(m_newColumns[j][i]);
            }
            for (int j=0; j<m_oldColumns.count(); j++) {
                row->insertChild(m_position+j,m_oldColumns[j][i]);
            }
        }
    }
}


// FormulaCommandAttribute::FormulaCommandAttribute( FormulaCursor* cursor,
//                                                   QHash<QString,QString> attributes )
//                        : KUndo2Command()
// {
//     m_ownerElement = cursor.ownerElement();
//     m_attributes = attributes;
//     m_oldAttributes = m_ownerElement->attributes();
//     QHashIterator<QString, QString> i( m_oldAttributes );
//     while( i.hasNext() )
//     {
//         i.next();
// 	if( !m_attributes.contains( i.key() ) )
//             m_attributes.insert( i.key(), i.value() );
//     }
// 
//     setText( i18nc( "(qtundo-format)", "Attribute Changed" ) );
// }
// 
// void FormulaCommandAttribute::redo()
// {
//     m_ownerElement->setAttributes( m_attributes );
// }
// 
// void FormulaCommandAttribute::undo()
// {
//     m_ownerElement->setAttributes( m_oldAttributes );
// }

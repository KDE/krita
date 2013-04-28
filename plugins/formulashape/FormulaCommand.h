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

#ifndef FORMULACOMMAND_H
#define FORMULACOMMAND_H

#include <kundo2command.h>
#include <QList>
#include <QHash>
#include <QMetaType>
#include "FormulaCursor.h"
class BasicElement;
class TokenElement;
class FormulaData;
class GlyphElement;
class TableElement;
class TableRowElement;

/**
 *
 * All FormulaCommands are used to manipulate the formula in various ways.
 * They all provide a redo and undo method as well as changeCursor method
 * which sets the cursor after the current action. A extra method for this is necessary,
 * as there might be no cursor when a undo/redo is done, because the tool was deactivated
 *
 **/

class FormulaCommand :  public KUndo2Command {
public:
    explicit FormulaCommand(KUndo2Command *parent=0);
    
    virtual void changeCursor(FormulaCursor& cursor, bool undo) const;

    void setUndoCursorPosition(const FormulaCursor& position);
    void setRedoCursorPosition(const FormulaCursor& position);

protected:
    bool m_done;

private:
    FormulaCursor m_undoCursorPosition;
    FormulaCursor m_redoCursorPosition;
};

Q_DECLARE_METATYPE(FormulaCommand*)


class FormulaCommandReplaceText : public FormulaCommand {
public:
    FormulaCommandReplaceText( TokenElement* owner, int position,int length, const QString& added , KUndo2Command *parent=0);

    ~FormulaCommandReplaceText();

    /// Execute the command
    void redo();

    /// Revert the actions done in redo()
    void undo();

private:
    /// The BasicElement that owns the newly added Text
    TokenElement* m_ownerElement;

    /// The position inside m_ownerElement
    int m_position;

    int m_length;

    int m_glyphpos;
    
    /// The list of added elements
    QString m_added;

    QString m_removed;

    QList<GlyphElement*> m_removedGlyphs;
};

class FormulaCommandReplaceElements : public FormulaCommand {
public:
    FormulaCommandReplaceElements( RowElement* owner, int position, int length, QList<BasicElement*> elements , bool wrap=false, KUndo2Command *parent=0);

    ~FormulaCommandReplaceElements();

    /// Execute the command
    void redo();

    /// Revert the actions done in redo()
    void undo();

private:
    /// The BasicElement that owns the newly added Text
    RowElement* m_ownerElement;

    /// The position inside m_ownerElement
    int m_position;

    int m_placeholderPosition;
    
    int m_length;

    bool m_wrap;

    RowElement* m_placeholderParent;

//     BasicElement* m_placeholder;

    /// The list of added elements
    QList<BasicElement*> m_added;

    /// The list of removed elements
    QList<BasicElement*> m_removed;
};

class FormulaCommandLoad : public FormulaCommand {
public:
    FormulaCommandLoad( FormulaData* data, FormulaElement* newelement, KUndo2Command *parent=0);

    ~FormulaCommandLoad ();
    
    /// Execute the command
    void redo();

    /// Revert the actions done in redo()
    void undo();

private:
    FormulaData* m_data;
    FormulaElement* m_oldel;
    FormulaElement* m_newel;
};

class FormulaCommandReplaceRow : public FormulaCommand {
public:
    FormulaCommandReplaceRow ( FormulaData* data, FormulaCursor oldPosition, TableElement* table, int number, int oldlength, int newlength );

    ~FormulaCommandReplaceRow ();

    /// Execute the command
    void redo();

    /// Revert the actions done in redo()
    void undo();

private:
    FormulaData* m_data;
    TableElement* m_table;
    TableRowElement* m_empty;
    int m_number;
    QList<BasicElement*> m_newRows;
    QList<BasicElement*> m_oldRows;
};

class FormulaCommandReplaceColumn : public FormulaCommand {
public:
    FormulaCommandReplaceColumn ( FormulaData* data, FormulaCursor oldPosition, TableElement* table, int number, int oldlength, int newlength );

    ~FormulaCommandReplaceColumn ();

    /// Execute the command
    void redo();

    /// Revert the actions done in redo()
    void undo();

private:
    FormulaData* m_data;

    ///the table we are manipulating
    TableElement* m_table;

    ///used to hold the new empty row, if we remove the whole table
    TableRowElement* m_empty;

    ///used to store the old rows, if we remove the whole table
    QList<BasicElement*> m_oldRows;

    ///the position where we start to insert / remove rows
    int m_position;

    ///used to store the old columns
    QList< QList<BasicElement*> > m_newColumns;

    ///used to store the new columns
    QList< QList<BasicElement*> > m_oldColumns;
};



// /**
//  * @short The command for changes of an element's attributes
//  * 
//  * Whenever the user changes the attributes assigned to an element an instance of this
//  * class is created to make it possible to revert the changes. The former attributes
//  * are stored in m_oldAttributes.
//  */
// class FormulaCommandAttribute : public KUndo2Command {
// public:
//     /**
//      * The constructor
//      * @param cursor The FormulaCursor where the elements will be replaced 
//      * @param attributes The list of the old attributes
//      */
//     FormulaCommandAttribute( FormulaCursor* cursor, QHash<QString,QString> attributes );
// 
//     /// Execute the command
//     void redo();
// 
//     /// Revert the actions done in redo()
//     void undo();
//     
// private:
//     /// The BasicElement whose attributes have been changed
//     BasicElement* m_ownerElement;
//     
//     /// All attributes that are set newly
//     QHash<QString,QString> m_attributes;
//     
//     /// All attributes the element had before
//     QHash<QString,QString> m_oldAttributes;
// };

#endif // FORMULACOMMAND_H

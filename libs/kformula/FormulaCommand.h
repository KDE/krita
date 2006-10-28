/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include <kcommand.h>
#include <QList>
#include <QHash>

namespace KFormula {
class BasicElement;

/**
 * @short The command for addition of elements
 * 
 * Whenever the user adds new elements to his formula an instance of this class is
 * created to make it possible to revert the changes. The added elements may have
 * child elements but m_addedElements contains only the top elements that need to
 * be removed to revert the changes again.
 * 
 * @since 2.0
 */
class FormulaCommandAdd : public KCommand {
public:
    /**
     * The constructor
     * @param cursor The FormulaCursor where the elements will be added
     * @param added The list of elements that has been added
     */
    FormulaCommandAdd( FormulaCursor* cursor, QList<BasicElement*> added );

    /// Execute the command
    void execute();

    /// Revert the actions done in execute()
    void unexecute();

    /// @return The name of this command 
    QString name() const;

private:
    /// The BasicElement that owns the newly added elements
    BasicElement* m_ownerElement;

    /// The position inside m_ownerElement
    int m_positionInElement;
    
    /// The list of added elements
    QList<BasicElement*> m_addedElements;
};


/**
 * @short The command for removal of elements
 * 
 * Whenever the user removes elements from his formula an instance of this class is
 * created to make it possible to revert the changes. The removed elements may have
 * child elements but m_removedElements contains only the top elements that need to
 * be added to revert the changes again.
 * 
 * @since 2.0
 */
class FormulaCommandRemove : public KCommand {
public:
    /**
     * The constructor
     * @param cursor The FormulaCursor where the elements will be removed
     * @param elements The list of removed elements
     */
    FormulaCommandRemove( FormulaCursor* cursor, QList<BasicElement*> elements );

    /// Execute the command
    void execute();

    /// Revert the actions done in execute()
    void unexecute();

    /// @return The name of this command 
    QString name() const;

private:
    /// The BasicElement that owned the removed elements
    BasicElement* m_ownerElement;
 
    /// The position inside m_ownerElement
    int m_positionInElement;

    /// The list of removed elements
    QList<BasicElement*> m_removedElements;
};


/**
 * @short The command for replacing of elements
 * 
 * Whenever the user replaces elements in his formula an instance of this class is
 * created to make it possible to revert the changes. The replaced elements are
 * stored in m_replacedElements and the elements that have replaced the old are
 * stored in m_replacingElements.
 *
 * @since 2.0
 */
class FormulaCommandReplace : public KCommand {
public:
    /**
     * The constructor
     * @param cursor The FormulaCursor where the elements will be replaced 
     * @param replaced The list of elements that have been replaced
     * @param replacing The list of elements that has replaced the old elements
     */
    FormulaCommandReplace( FormulaCursor* cursor, QList<BasicElement*> replaced,
                                                  QList<BasicElement*> replacing );

    /// Execute the command
    void execute();

    /// Revert the actions done in execute()
    void unexecute();

    /// @return The name of this command 
    QString name() const;

private:
    /// The BasicElement that owned the replaced elements
    BasicElement* m_ownerElement;

    /// The position inside m_ownerElement
    int m_positionInElement;

    /// The list of the new elements
    QList<BasicElement*> m_replacingElements;
    
    /// The list of replaced elements
    QList<BasicElement*> m_replacedElements;
};


/**
 * @short The command for changes of an element's attributes
 * 
 * Whenever the user changes the attributes assigned to an element an instance of this
 * class is created to make it possible to revert the changes. The former attributes
 * are stored in m_oldAttributes.
 * 
 * @since 2.0
 */
class FormulaCommandAttribute : public KCommand {
public:
    /**
     * The constructor
     * @param owner The BasicElement which owns the changed attributes
     * @param attributes The list of the old attributes
     */
    FormulaCommandAttribute( FormulaCursor* cursor, QHash<QString,QString> attributes );

    /// Execute the command
    void execute();

    /// Revert the actions done in execute()
    void unexecute();

    /// @return The name of this command 
    QString name() const;
    
private:
    /// The BasicElement whose attributes have been changed
    BasicElement* m_ownerElement;
    
    /// All attributes that are set newly
    QHash<QString,QString> m_attributes;
    
    /// All attributes the element had before
    QHash<QString,QString> m_oldAttributes;
};

} //namespace KFormula

#endif // FORMULACOMMAND_H

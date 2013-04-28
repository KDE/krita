/* This file is part of the KDE project
   Copyright (C) 2009 Jeremias Epperlein <jeeree@web.de>

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

#ifndef FORMULADATA_H
#define FORMULADATA_H

#include "FormulaElement.h"
#include "kformula_export.h"
#include <QObject>
#include <kundo2command.h>
#include "KoFormulaShape.h"
#include "KoShapeSavingContext.h"

class FormulaCommand;

/**
 * This is a QObject wrapper around a formulaElement, which allows to communicate 
 * between tool, cursor and shape
 */
class KOFORMULA_EXPORT FormulaData : public QObject {
Q_OBJECT
public:
    explicit FormulaData(FormulaElement *element);
    
    ~FormulaData();

    /// @return formulaElement that represents the data
    FormulaElement* formulaElement() const;
    
    ///emit a dataChanged signal
    void notifyDataChange(FormulaCommand* command, bool undo);
    void setFormulaElement ( FormulaElement* element);
    
signals:
    void dataChanged(FormulaCommand* element, bool undo);
    
public slots:
    ///only for debugging
    void writeElementTree();

    void saveMathML( KoShapeSavingContext& context);
    
private:
    FormulaElement* m_element;
};

#endif // FORMULADATA_H

/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef KOFORMULASHAPEFACTORY_H
#define KOFORMULASHAPEFACTORY_H

#include <KoShapeFactory.h>

class KoShape;

/**
 * @short Factory for the formula shape
 *
 * This class is a part of the FormulaShape plugin and provides a generic
 * way to obtain instances of the KoFormulaShape class.
 * It follows the factory design pattern and implements the two virtual methods 
 * createDefaultShape() and createShape() of KoShapeFactory.
 */
class KoFormulaShapeFactory : public KoShapeFactory {
    Q_OBJECT

public:
    /// The constructor - reimplemented from KoShapeFactory
    explicit KoFormulaShapeFactory( QObject *parent );

    /// The destructor - reimplemented from KoShapeFactory
    ~KoFormulaShapeFactory();

    /// reimplemented
    KoShape* createDefaultShape() const;

    /// reimplemented
    KoShape* createShape( const KoProperties* params ) const;
};

#endif // KOFORMULASHAPEFACTORY_H

/* This file is part of the KDE project

   Copyright (C) 2008 Johannes Simon <Johannes.simon@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#ifndef SIMPLETEXT_H
#define SIMPLETEXT_H


#include <KoShape.h>


/**
 * Interface for the SimpleTextShape plugin, originally written for Karbon
 * 
 * Use this pure virtual class instead of using SimpleTextShape directly
 * to avoid unnecessary dependencies of your code, as all plugins are optional.
 */
class SimpleTextShapeInterface : public KoShape
{
public:

    /// Sets the text to display
    virtual void setText( const QString & text ) = 0;

    /// Returns the text content
    virtual QString text() const = 0;
    
    virtual ~SimpleTextShapeInterface() {};

    /**
     * Sets the font used for drawing
     * Note that it is expected that the font has its point size set
     * in postscript points.
     */
    virtual void setFont( const QFont & font ) = 0;

    /// Returns the font
    virtual QFont font() const = 0;
};

#endif


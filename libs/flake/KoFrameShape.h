/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOFRAMESHAPE_H
#define KOFRAMESHAPE_H

#include "flake_export.h"

class KoXmlElement;
class KoShapeLoadingContext;

/**
 * @brief Base class for shapes that are saved as a part of a draw:frame.
 */
class FLAKE_EXPORT KoFrameShape
{
public:
    KoFrameShape( const char * ns, const char * tag );
    virtual ~KoFrameShape();

    virtual bool loadOdfFrame( const KoXmlElement & element, KoShapeLoadingContext &context );

protected:
    virtual bool loadOdfFrameElement( const KoXmlElement & element, KoShapeLoadingContext & context ) = 0;

private:
    struct Private;
    Private * const d;
};

#endif /* KOFRAMESHAPE_H */

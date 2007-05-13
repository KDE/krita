/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOSHAPELOADINGCONTEXT_H
#define KOSHAPELOADINGCONTEXT_H

#include <QMap>
#include <QString>

#include <flake_export.h>

class KoOasisLoadingContext;
class KoShapeLayer;
class KoShape;

/**
 * Context passed to shapes during loading
 */
class FLAKE_EXPORT KoShapeLoadingContext
{
public:
	KoShapeLoadingContext( KoOasisLoadingContext & context );

    KoOasisLoadingContext & koLoadingContext();

    KoShapeLayer * layer( const QString & layerName );

    void addShapeId( KoShape * shape, const QString & id );
    KoShape * shapeById( const QString & id );

private:    
    KoOasisLoadingContext &m_context;
    QMap<QString, KoShapeLayer*> m_layers;
    QMap<QString, KoShape*> m_drawIds; 
};

#endif /* KOSHAPELOADINGCONTEXT_H */

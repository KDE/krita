/* This file is part of the KDE project
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOPASHAPEADDREMOVEDATA_H
#define KOPASHAPEADDREMOVEDATA_H

#include <KoShapeAddRemoveData.h>

class KoPAPage;
class KoShapeLayer;

class KoPAShapeAddRemoveData : public KoShapeAddRemoveData
{
public:    
    KoPAShapeAddRemoveData( KoPAPage * activePage, KoShapeLayer * activeLayer );
    virtual ~KoPAShapeAddRemoveData() {}

    virtual KoShapeAddRemoveData * clone() const;

    void setPage( KoPAPage * activePage ) { m_activePage = activePage; } 
    KoPAPage * page() const { return m_activePage; }

    void setLayer( KoShapeLayer * activeLayer ) { m_activeLayer = activeLayer; }
    KoShapeLayer * layer() const { return m_activeLayer; }

private:    
    KoPAPage * m_activePage;
    KoShapeLayer * m_activeLayer;
};

#endif /* KOPASHAPEADDREMOVEDATA_H */

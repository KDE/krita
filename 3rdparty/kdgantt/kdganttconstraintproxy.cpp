/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/
#include "kdganttconstraintproxy.h"
#include "kdganttconstraintmodel.h"

#include <QAbstractProxyModel>

using namespace KDGantt;

/*!\class KDGantt::ConstraintProxy
 * \internal
 */

ConstraintProxy::ConstraintProxy( QObject* parent )
    : QObject( parent )
{
}

ConstraintProxy::~ConstraintProxy()
{
}

void ConstraintProxy::setSourceModel( ConstraintModel* src )
{
    if ( m_source ) disconnect( m_source );
    m_source = src;

    copyFromSource();

    connect( m_source, SIGNAL( constraintAdded( const Constraint& ) ),
             this, SLOT( slotSourceConstraintAdded( const Constraint& ) ) );
    connect( m_source, SIGNAL( constraintRemoved( const Constraint& ) ),
             this, SLOT( slotSourceConstraintRemoved( const Constraint& ) ) );
}

void ConstraintProxy::setDestinationModel( ConstraintModel* dest )
{
    if ( m_destination ) disconnect( m_destination );
    m_destination = dest;

    copyFromSource();

    connect( m_destination, SIGNAL( constraintAdded( const Constraint& ) ),
             this, SLOT( slotDestinationConstraintAdded( const Constraint& ) ) );
    connect( m_destination, SIGNAL( constraintRemoved( const Constraint& ) ),
             this, SLOT( slotDestinationConstraintRemoved( const Constraint& ) ) );
}

void ConstraintProxy::setProxyModel( QAbstractProxyModel* proxy )
{
    m_proxy = proxy;
}

ConstraintModel* ConstraintProxy::sourceModel() const { return m_source; }
ConstraintModel* ConstraintProxy::destinationModel() const { return m_destination; }
QAbstractProxyModel* ConstraintProxy::proxyModel() const { return m_proxy; }


void ConstraintProxy::copyFromSource()
{
    if ( m_destination ) {
        m_destination->clear();
        if ( !m_source ) return;
        QList<Constraint> lst = m_source->constraints();
        Q_FOREACH( const Constraint& c, lst ) {
            m_destination->addConstraint( Constraint( m_proxy->mapFromSource( c.startIndex() ),
                                                      m_proxy->mapFromSource( c.endIndex() ),
                                                      c.type(), c.relationType() ) );
        }
    }
}

void ConstraintProxy::slotSourceConstraintAdded( const Constraint& c )
{
    //qDebug() << "ConstraintProxy::slotSourceConstraintAdded("<<c<<")";
    if ( m_destination ) m_destination->addConstraint( Constraint( m_proxy->mapFromSource( c.startIndex() ),
                                                                   m_proxy->mapFromSource( c.endIndex() ),
                                                                   c.type(), c.relationType() ) );
}

void ConstraintProxy::slotSourceConstraintRemoved( const Constraint& c )
{
    //qDebug() << "ConstraintProxy::slotSourceConstraintRemoved("<<c<<")";
    if ( m_destination ) m_destination->removeConstraint( Constraint( m_proxy->mapFromSource( c.startIndex() ),
                                                                      m_proxy->mapFromSource( c.endIndex() ),
                                                                      c.type(), c.relationType() ) );
}

void ConstraintProxy::slotDestinationConstraintAdded( const Constraint& c )
{
    //qDebug() << "ConstraintProxy::slotDestinationConstraintAdded("<<c<<")";
    if ( m_source ) m_source->addConstraint( Constraint( m_proxy->mapToSource( c.startIndex() ),
                                                         m_proxy->mapToSource( c.endIndex() ),
                                                         c.type(), c.relationType() ) );
}

void ConstraintProxy::slotDestinationConstraintRemoved( const Constraint& c )
{
    //qDebug() << "ConstraintProxy::slotDestinationConstraintRemoved("<<c<<")";
    if ( m_source ) m_source->removeConstraint( Constraint( m_proxy->mapToSource( c.startIndex() ),
                                                            m_proxy->mapToSource( c.endIndex() ),
                                                            c.type(), c.relationType() ) );
}

#include "moc_kdganttconstraintproxy.cpp"

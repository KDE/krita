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
#include "kdganttconstraintmodel.h"
#include "kdganttconstraintmodel_p.h"

#include <QDebug>

#include <cassert>

using namespace KDGantt;

/*!\class KDGantt::ConstraintModel
 *\ingroup KDGantt
 * The ConstraintModel keeps track of the
 * interdependencies between gantt items in
 * a View.
 *
 */

ConstraintModel::Private::Private()
{
}

void ConstraintModel::Private::addConstraintToIndex( const QModelIndex& idx, const Constraint& c )
{
    IndexType::iterator it = indexMap.find(idx);
    while (it != indexMap.end() && it.key() == idx) {
        // Check if we already have this
        if ( *it == c ) return;
        ++it;
    }

    indexMap.insert( idx, c );
}

void ConstraintModel::Private::removeConstraintFromIndex( const QModelIndex& idx,  const Constraint& c )
{
    IndexType::iterator it = indexMap.find(idx);
    while (it != indexMap.end() && it.key() == idx) {
        if ( *it == c ) {
            it =indexMap.erase( it );
        } else {
            ++it;
        }
    }
}

/*! Constructor. Creates an empty ConstraintModel with parent \a parent
 */
ConstraintModel::ConstraintModel( QObject* parent )
    : QObject( parent ), _d( new Private )
{
    init();
}

/*!\internal*/
ConstraintModel::ConstraintModel( Private* d_ptr, QObject* parent )
    : QObject( parent ), _d( d_ptr )
{
    init();
}

/*! Destroys this ConstraintModel */
ConstraintModel::~ConstraintModel()
{
    delete _d;
}

#define d d_func()

void ConstraintModel::init()
{
}

/*! Adds the constraint \a c to this ConstraintModel
 *  If the Constraint \a c is already in this ConstraintModel,
 *  nothing happens.
 */
void ConstraintModel::addConstraint( const Constraint& c )
{
    //int size = d->constraints.size();
    bool hasConstraint = d->constraints.contains( c );
    //d->constraints.insert( c );
    //if ( size != d->constraints.size() ) {
    if ( !hasConstraint ) {
        d->constraints.push_back( c );
        d->addConstraintToIndex( c.startIndex(), c );
        d->addConstraintToIndex( c.endIndex(), c );
        emit constraintAdded( c );
    }
}

/*! Removes the Constraint \a c from this
 * ConstraintModel. If \a c was found and removed,
 * the signal constraintRemoved(const Constraint&) is emitted.
 *
 * \returns If \a c was found and removed, it returns true,
 * otherwise it returns false.
 */
bool ConstraintModel::removeConstraint( const Constraint& c )
{
    //qDebug() << "ConstraintModel::removeConstraint("<<c<<") from "<< d->constraints;
    bool rc = d->constraints.removeAll( c );
    //bool rc = d->constraints.remove( c );
    if ( rc ) {
        d->removeConstraintFromIndex( c.startIndex(), c );
        d->removeConstraintFromIndex( c.endIndex(), c );
        emit constraintRemoved( c );
    }
    return rc;
}

/*! Removes all Constraints from this model
 * The signal constraintRemoved(const Constraint&) is emitted
 * for every Constraint that is removed.
 */
void ConstraintModel::clear()
{
    QList<Constraint> lst = constraints();
    Q_FOREACH( const Constraint& c, lst ) {
        removeConstraint( c );
    }
}

/*! Not used */
void ConstraintModel::cleanup()
{
#if 0
    QSet<Constraint> orphans;
    Q_FOREACH( const Constraint& c, d->constraints ) {
        if ( !c.startIndex().isValid() || !c.endIndex().isValid() ) orphans.insert( c );
    }
    //qDebug() << "Constraint::cleanup() found" << orphans << "orphans";
    d->constraints.subtract( orphans );
#endif
}

/*! \returns A list of all Constraints in this
 * ConstraintModel.
 */
QList<Constraint> ConstraintModel::constraints() const
{
    //return d->constraints.toList();
    return d->constraints;
}

/*! \returns A list of all Constraints in this ConstraintModel
 * that have an endpoint at \a idx.
 */
QList<Constraint> ConstraintModel::constraintsForIndex( const QModelIndex& idx ) const
{
    assert( idx.isValid() || !d->indexMap.isEmpty() || d->indexMap.keys().front().model() || idx.model() == d->indexMap.keys().front().model() );
    if ( !idx.isValid() ) { //Why the assert idx.isValid() above? (dag)
        // Because of a Qt bug we need to treat this as a special case
        QSet<Constraint> result;
        Q_FOREACH( Constraint c, d->constraints ) {
            if ( !c.startIndex().isValid() || !c.endIndex().isValid() ) result.insert( c );
        }
        return result.toList();
    } else {
        QList<Constraint> result;
        Q_FOREACH( Constraint c, d->constraints ) {
            if ( c.startIndex() == idx || c.endIndex() == idx ) result.push_back( c );
        }
        return result;
    }

    //return d->indexMap.values( idx );
}

/*! Returns true if a Constraint with start \a s and end \a e
 * exists, otherwise false.
 */
bool ConstraintModel::hasConstraint( const Constraint& c ) const
{
    /*
    // Because of a Qt bug we have to search like this
    Q_FOREACH( Constraint c2, d->constraints ) {
        if ( c==c2 ) return true;
    }
    return false;
    */
    return d->constraints.contains( c );
}

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<( QDebug dbg, const KDGantt::ConstraintModel& model )
{
    dbg << "KDGantt::ConstraintModel[ " << static_cast<const QObject*>( &model ) << ":"
        << model.constraints() << "]";
    return dbg;
}

#endif /* QT_NO_DEBUG_STREAM */

#undef d

#ifndef KDAB_NO_UNIT_TESTS

#include <QStandardItemModel>

#include "unittest/test.h"

std::ostream& operator<<( std::ostream& os, const QModelIndex& idx )
{
    QString str;
    QDebug( &str )<<idx;
    os<<str.toStdString();
    return os;
}

KDAB_SCOPED_UNITTEST_SIMPLE( KDGantt, ConstraintModel, "test" )
{
    QStandardItemModel dummyModel( 100, 100 );
    ConstraintModel model;

    QModelIndex invalidIndex;
    assertEqual( invalidIndex, invalidIndex );

    assertEqual( model.constraints().count(), 0 );

    model.addConstraint( Constraint( QModelIndex(), QModelIndex() ) );
    assertEqual( model.constraints().count(), 1 );

    model.addConstraint( Constraint( QModelIndex(), QModelIndex() ) );
    assertEqual( model.constraints().count(), 1 );

    QPersistentModelIndex idx1 = dummyModel.index( 7, 17, QModelIndex() );
    QPersistentModelIndex idx2 = dummyModel.index( 42, 17, QModelIndex() );

    model.addConstraint( Constraint( idx1, idx2 ) );
    assertEqual( model.constraints().count(), 2 );
    assertTrue( model.hasConstraint( Constraint( idx1, idx2 ) ) );

    assertEqual( model.constraintsForIndex( QModelIndex() ).count(), 1 );

    assertEqual( model.constraints().count(), 2 );
    model.removeConstraint( Constraint( QModelIndex(), QModelIndex() ) );
    assertEqual( model.constraints().count(), 1 );
    assertFalse( model.hasConstraint( Constraint( QModelIndex(), QModelIndex() ) ) );

    model.removeConstraint( Constraint( QModelIndex(), QModelIndex() ) );
    assertEqual( model.constraints().count(), 1 );

    model.removeConstraint( Constraint( idx1, idx2 ) );
    assertEqual( model.constraints().count(), 0 );
    assertFalse( model.hasConstraint( Constraint( idx1, idx2 ) ) );

    model.addConstraint( Constraint( idx1, idx2 ) );
    assertTrue( model.hasConstraint( Constraint( idx1, idx2 ) ) );
    dummyModel.removeRow( 8 );
    assertTrue( model.hasConstraint( Constraint( idx1, idx2 ) ) );
    dummyModel.removeRow( 7 );
    assertTrue( model.hasConstraint( Constraint( idx1, idx2 ) ) );
}

#endif /* KDAB_NO_UNIT_TESTS */

#include "moc_kdganttconstraintmodel.cpp"

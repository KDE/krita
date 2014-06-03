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
#include "kdganttsummaryhandlingproxymodel.h"
#include "kdganttsummaryhandlingproxymodel_p.h"

#include <QDebug>

#include <cassert>

using namespace KDGantt;

/*!\class KDGantt::SummaryHandlingProxyModel
 * \brief Proxy model that supports summary gantt items.
 *
 * This proxy model provides the functionality of summary items.
 * A summary item is an item with type KDGantt::TypeSummary and
 * zero or more children in the model that it summarizes.
 * GraphicsView itself does not dictate any policy for summary items,
 * instead the logic for making the summary items start and end points
 * span it's children is provided by this proxy.
 *
 * The start and end times of a summary is the min/max of the
 * start/end times of it's children.
 *
 * \see GraphicsView::setModel
 */

typedef ForwardingProxyModel BASE;

bool SummaryHandlingProxyModel::Private::cacheLookup( const QModelIndex& idx,
                                                      QPair<QDateTime,QDateTime>* result ) const
{
    //qDebug() << "cacheLookup("<<idx<<"), cache has " << cached_summary_items.count() << "items";
    QHash<QModelIndex,QPair<QDateTime,QDateTime> >::const_iterator it =
        cached_summary_items.constFind( idx );
    if ( it != cached_summary_items.constEnd() ) {
        *result = *it;
        return true;
    } else {
        return false;
    }
}

void SummaryHandlingProxyModel::Private::insertInCache( const SummaryHandlingProxyModel* model,
                                                        const QModelIndex& sourceIdx ) const
{
    QAbstractItemModel* sourceModel = model->sourceModel();
    const QModelIndex& mainIdx = sourceIdx;
    QDateTime st;
    QDateTime et;

    for ( int r = 0; r < sourceModel->rowCount( mainIdx ); ++r ) {
        QModelIndex pdIdx = model->mapFromSource( sourceModel->index( r, 0, mainIdx ) );
        /* The probably results in recursive calls here */
	QVariant tmpsv = model->data( pdIdx, StartTimeRole );
	QVariant tmpev = model->data( pdIdx, EndTimeRole );
	if( !qVariantCanConvert<QDateTime>(tmpsv) ||
	    !qVariantCanConvert<QDateTime>(tmpev) ) {
            qDebug() << "Skipping item " << sourceIdx << " because it doesn't contain QDateTime";
            continue;
        }

    // check for valid datetimes
    if( tmpsv.type() == QVariant::DateTime && !qVariantValue<QDateTime>(tmpsv).isValid()) continue;
    if( tmpev.type() == QVariant::DateTime && !qVariantValue<QDateTime>(tmpev).isValid()) continue;

	// We need to test for empty strings to
	// avoid a stupid Qt warning
	if( tmpsv.type() == QVariant::String && qVariantValue<QString>(tmpsv).isEmpty()) continue;
	if( tmpev.type() == QVariant::String && qVariantValue<QString>(tmpev).isEmpty()) continue;
        QDateTime tmpst = tmpsv.toDateTime();
        QDateTime tmpet = tmpev.toDateTime();
        if ( st.isNull() || st > tmpst ) st = tmpst;
        if ( et.isNull() || et < tmpet ) et = tmpet;
    }
    QVariant tmpssv = sourceModel->data( mainIdx, StartTimeRole );
    QVariant tmpsev = sourceModel->data( mainIdx, EndTimeRole );
    if ( qVariantCanConvert<QDateTime>( tmpssv )
         && !( qVariantCanConvert<QString>( tmpssv ) && qVariantValue<QString>( tmpssv ).isEmpty() )
         && qVariantValue<QDateTime>( tmpssv ) != st )
        sourceModel->setData( mainIdx, st, StartTimeRole );
    if ( qVariantCanConvert<QDateTime>( tmpsev )
         && !( qVariantCanConvert<QString>( tmpsev ) && qVariantValue<QString>( tmpsev ).isEmpty() )
         && qVariantValue<QDateTime>( tmpsev ) != et )
        sourceModel->setData( mainIdx, et, EndTimeRole );
    cached_summary_items[sourceIdx]=qMakePair( st, et );
}

void SummaryHandlingProxyModel::Private::removeFromCache( const QModelIndex& idx ) const
{
    cached_summary_items.remove( idx );
}

void SummaryHandlingProxyModel::Private::clearCache() const
{
    cached_summary_items.clear();
}

/*! Constructor. Creates a new SummaryHandlingProxyModel with
 * parent \a parent
 */
SummaryHandlingProxyModel::SummaryHandlingProxyModel( QObject* parent )
    : BASE( parent ), _d( new Private )
{
    init();
}

#define d d_func()
SummaryHandlingProxyModel::~SummaryHandlingProxyModel()
{
    delete d;
}

void SummaryHandlingProxyModel::init()
{
}

namespace {

    // Think this is ugly? Well, it's not from me, it comes from QProxyModel
    struct KDPrivateModelIndex {
        int r, c;
        void *p;
        const QAbstractItemModel *m;
    };
}

/*! Sets the model to be used as the source model for this proxy.
 * The proxy does not take ownership of the model.
 * \see QAbstractProxyModel::setSourceModel
 */
void SummaryHandlingProxyModel::setSourceModel( QAbstractItemModel* model )
{
    BASE::setSourceModel( model );
    d->clearCache();
}

void SummaryHandlingProxyModel::sourceModelReset()
{
    d->clearCache();
    BASE::sourceModelReset();
}

void SummaryHandlingProxyModel::sourceLayoutChanged()
{
    d->clearCache();
    BASE::sourceLayoutChanged();
}

void SummaryHandlingProxyModel::sourceDataChanged( const QModelIndex& from, const QModelIndex& to )
{
    QAbstractItemModel* model = sourceModel();
    QModelIndex parentIdx = from;
    do {
        const QModelIndex& dataIdx = parentIdx;
        if ( model->data( dataIdx, ItemTypeRole )==TypeSummary ) {
            //qDebug() << "removing " << parentIdx << "from cache";
            d->removeFromCache( dataIdx );
            QModelIndex proxyDataIdx = mapFromSource( dataIdx );
            emit dataChanged( proxyDataIdx, proxyDataIdx );
        }
    } while ( ( parentIdx=model->parent( parentIdx ) ) != QModelIndex() );

    BASE::sourceDataChanged( from, to );
}

void SummaryHandlingProxyModel::sourceColumnsAboutToBeInserted( const QModelIndex& parentIdx,
                                                                    int start,
                                                                    int end )
{
    BASE::sourceColumnsAboutToBeInserted( parentIdx, start, end );
    d->clearCache();
}

void SummaryHandlingProxyModel::sourceColumnsAboutToBeRemoved( const QModelIndex& parentIdx,
                                                                    int start,
                                                                    int end )
{
    BASE::sourceColumnsAboutToBeRemoved( parentIdx, start, end );
    d->clearCache();
}

void SummaryHandlingProxyModel::sourceRowsAboutToBeInserted( const QModelIndex & parentIdx, int start, int end )
{
    BASE::sourceRowsAboutToBeInserted( parentIdx, start, end );
    d->clearCache();
}

void SummaryHandlingProxyModel::sourceRowsAboutToBeRemoved( const QModelIndex & parentIdx, int start, int end )
{
    BASE::sourceRowsAboutToBeRemoved( parentIdx, start, end );
    d->clearCache();
}

/*! \see QAbstractItemModel::flags */
Qt::ItemFlags SummaryHandlingProxyModel::flags( const QModelIndex& idx ) const
{
    const QModelIndex sidx = mapToSource( idx );
    const QAbstractItemModel* model = sourceModel();
    Qt::ItemFlags f = model->flags( sidx );
    if ( d->isSummary(sidx) ) {
        f &= !Qt::ItemIsEditable;
    }
    return f;
}

/*! \see QAbstractItemModel::data */
QVariant SummaryHandlingProxyModel::data( const QModelIndex& proxyIndex, int role) const
{
    //qDebug() << "SummaryHandlingProxyModel::data("<<proxyIndex<<role<<")";
    const QModelIndex sidx = mapToSource( proxyIndex );
    const QAbstractItemModel* model = sourceModel();
    if ( d->isSummary(sidx) && ( role==StartTimeRole || role==EndTimeRole )) {
      //qDebug() << "requested summary";
        QPair<QDateTime,QDateTime> result;
        if ( d->cacheLookup( sidx, &result ) ) {
	  //qDebug() << "SummaryHandlingProxyModel::data(): Looking up summary for " << proxyIndex << role;
            switch( role ) {
            case StartTimeRole: return result.first;
            case EndTimeRole: return result.second;
            default: /* fall thru */;
            }
        } else {
            d->insertInCache( this, sidx );
            return data( proxyIndex, role ); /* TODO: Optimise */
        }
    }
    return model->data( sidx, role );
}

/*! \see QAbstractItemModel::setData */
bool SummaryHandlingProxyModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    QAbstractItemModel* model = sourceModel();
    if ( role==StartTimeRole || role==EndTimeRole ) {
        QModelIndex parentIdx = mapToSource( index );
        do {
            if ( d->isSummary(parentIdx) ) {
	      //qDebug() << "removing " << parentIdx << "from cache";
                d->removeFromCache( parentIdx );
                QModelIndex proxyParentIdx = mapFromSource( parentIdx );
                emit dataChanged( proxyParentIdx, proxyParentIdx );
            }
        } while ( ( parentIdx=model->parent( parentIdx ) ) != QModelIndex() );
    }
    return BASE::setData( index, value, role );
}

#undef d

#ifndef KDAB_NO_UNIT_TESTS

#include "unittest/test.h"

#include <QStandardItemModel>

namespace {
    std::ostream& operator<<( std::ostream& os, const QDateTime& dt )
    {
        os << dt.toString().toStdString();
        return os;
    }
}

KDAB_SCOPED_UNITTEST_SIMPLE( KDGantt, SummaryHandlingProxyModel, "test" ) {
    SummaryHandlingProxyModel model;
    QStandardItemModel sourceModel;

    model.setSourceModel( &sourceModel );

    QStandardItem* topitem = new QStandardItem( QString::fromLatin1( "Summary" ) );
    topitem->setData( KDGantt::TypeSummary,  KDGantt::ItemTypeRole );
    sourceModel.appendRow( topitem );

    QStandardItem* task1 = new QStandardItem( QString::fromLatin1( "Task1" ) );
    task1->setData( KDGantt::TypeTask, KDGantt::ItemTypeRole );
    QStandardItem* task2 = new QStandardItem( QString::fromLatin1( "Task2" ) );
    task2->setData( KDGantt::TypeTask, KDGantt::ItemTypeRole );
    topitem->appendRow( task1 );
    topitem->appendRow( task2 );


    QDateTime startdt = QDateTime::currentDateTime();
    QDateTime enddt = startdt.addDays( 1 );


    task1->setData( startdt, KDGantt::StartTimeRole );
    task1->setData( enddt, KDGantt::EndTimeRole );
    task2->setData( startdt, KDGantt::StartTimeRole );
    task2->setData( enddt, KDGantt::EndTimeRole );

    const QModelIndex topidx = model.index( 0, 0, QModelIndex() );

    assertEqual( model.data( topidx, KDGantt::ItemTypeRole ).toInt(), KDGantt::TypeSummary );
    assertEqual( model.data( model.index( 0, 0, topidx ), KDGantt::ItemTypeRole ).toInt(), KDGantt::TypeTask );

    QDateTime task1startdt = model.data( model.index( 0, 0, topidx ), KDGantt::StartTimeRole ).toDateTime();
    assertEqual( task1startdt, startdt );

    QDateTime summarystartdt = model.data( topidx, KDGantt::StartTimeRole ).toDateTime();
    assertEqual( summarystartdt, startdt );
    assertTrue( model.flags( model.index( 0, 0, topidx ) ) & Qt::ItemIsEditable );
    assertFalse( model.flags( topidx ) & Qt::ItemIsEditable );
}

#endif /* KDAB_NO_UNIT_TESTS */

#include "moc_kdganttsummaryhandlingproxymodel.cpp"

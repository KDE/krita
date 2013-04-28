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
#include "kdganttgraphicsscene.h"
#include "kdganttgraphicsscene_p.h"
#include "kdganttgraphicsitem.h"
#include "kdganttconstraint.h"
#include "kdganttconstraintgraphicsitem.h"
#include "kdganttitemdelegate.h"
#include "kdganttabstractrowcontroller.h"
#include "kdganttdatetimegrid.h"
#include "kdganttsummaryhandlingproxymodel.h"
#include "kdganttgraphicsview.h"

#include <QApplication>
#include <QToolTip>
#include <QGraphicsSceneHelpEvent>
#include <QTextDocument>
#include <QDebug>

#include <functional>
#include <algorithm>
#include <cassert>

/*!\class KDGantt::GraphicsScene
 * \internal
 */

using namespace KDGantt;

GraphicsScene::Private::Private( GraphicsScene* _q )
    : q( _q ),
      dragSource( 0 ),
      itemDelegate( new ItemDelegate( _q ) ),
      rowController( 0 ),
      grid( &default_grid ),
      readOnly( false ),
      summaryHandlingModel( new SummaryHandlingProxyModel( _q ) ),
      selectionModel( 0 )
{
    default_grid.setStartDateTime( QDateTime::currentDateTime().addDays( -1 ) );
}

void GraphicsScene::Private::resetConstraintItems()
{
    q->clearConstraintItems();
    if ( constraintModel.isNull() ) return;
    QList<Constraint> clst = constraintModel->constraints();
    Q_FOREACH( Constraint c, clst ) {
        createConstraintItem( c );
    }
    q->updateItems();
}

void GraphicsScene::Private::createConstraintItem( const Constraint& c )
{
    GraphicsItem* sitem = q->findItem( summaryHandlingModel->mapFromSource( c.startIndex() ) );
    GraphicsItem* eitem = q->findItem( summaryHandlingModel->mapFromSource( c.endIndex() ) );

    if ( sitem && eitem ) {
        ConstraintGraphicsItem* citem = new ConstraintGraphicsItem( c );
        sitem->addStartConstraint( citem );
        eitem->addEndConstraint( citem );
        q->addItem( citem );
    }



    //q->insertConstraintItem( c, citem );
}

// Delete the constraint item, and clean up pointers in the start- and end item
void GraphicsScene::Private::deleteConstraintItem( ConstraintGraphicsItem *citem )
{
    //qDebug()<<"GraphicsScene::Private::deleteConstraintItem citem="<<(void*)citem;
    if ( citem == 0 ) {
        return;
    }
    Constraint c = citem->constraint();
    GraphicsItem* item = items.value( summaryHandlingModel->mapFromSource( c.startIndex() ), 0 );
    if ( item ) {
        //qDebug()<<"GraphicsScene::Private::deleteConstraintItem startConstraints"<<item<<(void*)citem;
        item->removeStartConstraint( citem );
    } //else qDebug()<<"GraphicsScene::Private::deleteConstraintItem"<<c.startIndex()<<"start item not found";
    item = items.value( summaryHandlingModel->mapFromSource( c.endIndex() ), 0 );
    if ( item ) {
        //qDebug()<<"GraphicsScene::Private::deleteConstraintItem endConstraints"<<item<<(void*)citem;
        item->removeEndConstraint( citem );
    } //else qDebug()<<"GraphicsScene::Private::deleteConstraintItem"<<c.endIndex()<<"end item not found";
    //qDebug()<<"GraphicsScene::Private::deleteConstraintItem"<<citem<<"deleted";
    delete citem;
}

void GraphicsScene::Private::deleteConstraintItem( const Constraint& c )
{
    //qDebug()<<"GraphicsScene::Private::deleteConstraintItem c="<<c;
    deleteConstraintItem( findConstraintItem( c ) );
}

namespace {
    /* Very useful functional to compose functions.
     * Unfortunately an SGI extension and not standard
     */
    template<typename Operation1,typename Operation2>
    struct unary_compose : public std::unary_function<typename Operation1::result_type,typename Operation2::argument_type> {
        unary_compose( const Operation1& f, const Operation2& g ) : _f( f ), _g( g ) {}

        inline typename Operation1::result_type operator()( const typename Operation2::argument_type& arg ) {
            return _f( _g( arg ) );
        }

        Operation1 _f;
        Operation2 _g;
    };

    template<typename Operation1,typename Operation2>
    inline unary_compose<Operation1,Operation2> compose1( const Operation1& f, const Operation2& g )
    {
        return unary_compose<Operation1,Operation2>( f, g );
    }
}

ConstraintGraphicsItem* GraphicsScene::Private::findConstraintItem( const Constraint& c ) const
{
    GraphicsItem* item = items.value( summaryHandlingModel->mapFromSource( c.startIndex() ), 0 );
    if ( item ) {
        QList<ConstraintGraphicsItem*> clst = item->startConstraints();
        QList<ConstraintGraphicsItem*>::iterator it = clst.begin();
        //qDebug()<<"GraphicsScene::Private::findConstraintItem start:"<<c<<item<<clst;
        for( ; it != clst.end() ; ++it )
            if ((*it)->constraint() == c )
                break;
        if (  it != clst.end() ) {
            return *it;
        }
    }
    item = items.value( summaryHandlingModel->mapFromSource( c.endIndex() ), 0 );
    if ( item ) {
        QList<ConstraintGraphicsItem*> clst = item->endConstraints();
        QList<ConstraintGraphicsItem*>::iterator it = clst.begin();
        //qDebug()<<"GraphicsScene::Private::findConstraintItem end:"<<c<<item<<clst;
        for( ; it != clst.end() ; ++it )
            if ((*it)->constraint() == c )
                break;
        if (  it != clst.end() ) {
            return *it;
        }
    }
    //qDebug()<<"GraphicsScene::Private::findConstraintItem No item or constraintitem"<<c;
    return 0;
}

GraphicsScene::GraphicsScene( QObject* parent )
    : QGraphicsScene( parent ), _d( new Private( this ) )
{
    init();
}

#define d d_func()
GraphicsScene::~GraphicsScene()
{
    clearConstraintItems();
    clearItems();
    delete d;
}

void GraphicsScene::init()
{
    setConstraintModel( new ConstraintModel( this ) );
    connect( d->grid, SIGNAL( gridChanged() ), this, SLOT( slotGridChanged() ) );
}

/* NOTE: The delegate should really be a property
 * of the view, but that doesn't really fit at
 * this time
 */
void GraphicsScene::setItemDelegate( ItemDelegate* delegate )
{
    if ( !d->itemDelegate.isNull() && d->itemDelegate->parent()==this ) delete d->itemDelegate;
    d->itemDelegate = delegate;
    update();
}

ItemDelegate* GraphicsScene::itemDelegate() const
{
    return d->itemDelegate;
}

QAbstractItemModel* GraphicsScene::model() const
{
    assert(!d->summaryHandlingModel.isNull());
    return d->summaryHandlingModel->sourceModel();
}

void GraphicsScene::setModel( QAbstractItemModel* model )
{
    assert(!d->summaryHandlingModel.isNull());
    d->summaryHandlingModel->setSourceModel(model);
    d->grid->setModel( d->summaryHandlingModel );
    setSelectionModel( new QItemSelectionModel( model, this ) );
}

QAbstractProxyModel* GraphicsScene::summaryHandlingModel() const
{
    return d->summaryHandlingModel;
}

void GraphicsScene::setSummaryHandlingModel( QAbstractProxyModel* proxyModel )
{
    proxyModel->setSourceModel( model() );
    d->summaryHandlingModel = proxyModel;
}

void GraphicsScene::setRootIndex( const QModelIndex& idx )
{
    d->grid->setRootIndex( idx );
}

QModelIndex GraphicsScene::rootIndex() const
{
    return d->grid->rootIndex();
}

ConstraintModel* GraphicsScene::constraintModel() const
{
    return d->constraintModel;
}

void GraphicsScene::setConstraintModel( ConstraintModel* cm )
{
    if ( !d->constraintModel.isNull() ) {
        disconnect( d->constraintModel );
    }
    d->constraintModel = cm;

    connect( cm, SIGNAL( constraintAdded( const Constraint& ) ), this, SLOT( slotConstraintAdded( const Constraint& ) ) );
    connect( cm, SIGNAL( constraintRemoved( const Constraint& ) ), this, SLOT( slotConstraintRemoved( const Constraint& ) ) );
    d->resetConstraintItems();
}

void GraphicsScene::setSelectionModel( QItemSelectionModel* smodel )
{
    d->selectionModel = smodel;
    // TODO: update selection from model and connect signals
}

QItemSelectionModel* GraphicsScene::selectionModel() const
{
    return d->selectionModel;
}

void GraphicsScene::setRowController( AbstractRowController* rc )
{
    d->rowController = rc;
}

AbstractRowController* GraphicsScene::rowController() const
{
    return d->rowController;
}

void GraphicsScene::setGrid( AbstractGrid* grid )
{
    QAbstractItemModel* model = d->grid->model();
    if ( grid == 0 ) grid = &d->default_grid;
    if ( d->grid ) disconnect( d->grid );
    d->grid = grid;
    connect( d->grid, SIGNAL( gridChanged() ), this, SLOT( slotGridChanged() ) );
    d->grid->setModel( model );
    slotGridChanged();
}

AbstractGrid* GraphicsScene::grid() const
{
    return d->grid;
}

void GraphicsScene::setReadOnly( bool ro )
{
    d->readOnly = ro;
}

bool GraphicsScene::isReadOnly() const
{
    return d->readOnly;
}

/* Returns the index with column=0 fromt the
 * same row as idx and with the same parent.
 * This is used to traverse the tree-structure
 * of the model
 */
QModelIndex GraphicsScene::mainIndex( const QModelIndex& idx )
{
#if 0
    if ( idx.isValid() ) {
        return idx.model()->index( idx.row(), 0,idx.parent() );
    } else {
        return QModelIndex();
    }
#else
    return idx;
#endif
}

/*! Returns the index pointing to the last column
 * in the same row as idx. This can be thought of
 * as in "inverse" of mainIndex()
 */
QModelIndex GraphicsScene::dataIndex( const QModelIndex& idx )
{
#if 0
    if ( idx.isValid() ) {
        const QAbstractItemModel* model = idx.model();
        return model->index( idx.row(), model->columnCount( idx.parent() )-1,idx.parent() );
    } else {
        return QModelIndex();
    }
#else
    return idx;
#endif
}

/*! Creates a new item of type type.
 * TODO: If the user should be allowed to override
 * this in any way, it needs to be in View!
 */
GraphicsItem* GraphicsScene::createItem( ItemType type ) const
{
#if 0
    switch( type ) {
    case TypeEvent:   return 0;
    case TypeTask:    return new TaskItem;
    case TypeSummary: return new SummaryItem;
    default:          return 0;
    }
#endif
    //qDebug() << "GraphicsScene::createItem("<<type<<")";
    Q_UNUSED( type );
    return new GraphicsItem;
}

void GraphicsScene::updateRow( const QModelIndex& rowidx )
{
    //qDebug() << "GraphicsScene::updateRow("<<rowidx<<")";
    if ( !rowidx.isValid() ) return;
    const QAbstractItemModel* model = rowidx.model(); // why const?
    assert( model );
    assert( rowController() );
    assert( model == summaryHandlingModel() );

    const QModelIndex sidx = summaryHandlingModel()->mapToSource( rowidx );
    const Span rg=rowController()->rowGeometry( sidx );
    for ( int col = 0; col < summaryHandlingModel()->columnCount( rowidx.parent() ); ++col ) {
        const QModelIndex idx = summaryHandlingModel()->index( rowidx.row(), col, rowidx.parent() );
        const int itemtype = summaryHandlingModel()->data( idx, ItemTypeRole ).toInt();
        if ( itemtype == TypeNone ) {
            removeItem( idx );
            continue;
        }
        if ( itemtype == TypeMulti ) {
            int cr=0;
            QModelIndex child;
            while ( ( child = idx.child( cr, 0 ) ).isValid() ) {
                GraphicsItem* item = findItem( child );
                if (!item) {
                    item = createItem( static_cast<ItemType>( itemtype ) );
                    item->setIndex( child );
                    insertItem( child, item);
                }
                item->updateItem( rg, child );
                setSceneRect( sceneRect().united( item->boundingRect() ) );
                ++cr;
            }
        }
	{
            if ( summaryHandlingModel()->data( rowidx.parent(), ItemTypeRole ).toInt() == TypeMulti ) continue;

            GraphicsItem* item = findItem( idx );
            if (!item) {
                item = createItem( static_cast<ItemType>( itemtype ) );
                item->setIndex( idx );
                insertItem(idx, item);
            }
            item->updateItem( rg, idx );
            setSceneRect( sceneRect().united( item->boundingRect() ) );
        }
    }
}

void GraphicsScene::insertItem( const QPersistentModelIndex& idx, GraphicsItem* item )
{
    if ( !d->constraintModel.isNull() ) {
        // Create items for constraints
        const QModelIndex sidx = summaryHandlingModel()->mapToSource( idx );
        const QList<Constraint> clst = d->constraintModel->constraintsForIndex( sidx );
        Q_FOREACH( Constraint c,  clst ) {
            QModelIndex other_idx;
            if ( c.startIndex() == sidx ) {
                other_idx = c.endIndex();
                GraphicsItem* other_item = d->items.value(summaryHandlingModel()->mapFromSource( other_idx ),0);
                if ( !other_item ) continue;
                ConstraintGraphicsItem* citem = new ConstraintGraphicsItem( c );
                item->addStartConstraint( citem );
                other_item->addEndConstraint( citem );
                addItem( citem );
            } else if ( c.endIndex() == sidx ) {
                other_idx = c.startIndex();
                GraphicsItem* other_item = d->items.value(summaryHandlingModel()->mapFromSource( other_idx ),0);
                if ( !other_item ) continue;
                ConstraintGraphicsItem* citem = new ConstraintGraphicsItem( c );
                other_item->addStartConstraint( citem );
                item->addEndConstraint( citem );
                addItem( citem );
            } else {
                assert( 0 ); // Impossible
            }
        }
    }
    d->items.insert( idx, item );
    addItem( item );
}

void GraphicsScene::removeItem( const QModelIndex& idx )
{
    //qDebug() << "GraphicsScene::removeItem("<<idx<<")";
    QHash<QPersistentModelIndex,GraphicsItem*>::iterator it = d->items.find( idx );
    if ( it != d->items.end() ) {
        GraphicsItem* item = *it;
        // We have to remove the item from the list first because
        // there is a good chance there will be reentrant calls
        d->items.erase( it );
        //qDebug() << "GraphicsScene::removeItem item="<<item;
        {
            // Remove any constraintitems starting here
            const QList<ConstraintGraphicsItem*> clst = item->startConstraints();
            //qDebug()<<"GraphicsScene::removeItem start:"<<clst;
            Q_FOREACH( ConstraintGraphicsItem* citem, clst ) {
                //qDebug()<<"GraphicsScene::removeItem start citem="<<citem;
                d->deleteConstraintItem( citem );
            }
        }
        {// Remove any constraintitems ending here
            const QList<ConstraintGraphicsItem*> clst = item->endConstraints();
            //qDebug()<<"GraphicsScene::removeItem end:"<<clst;
            Q_FOREACH( ConstraintGraphicsItem* citem, clst ) {
                //qDebug()<<"GraphicsScene::removeItem end citem="<<citem;
                d->deleteConstraintItem( citem );
            }
        }
        // Get rid of the item
        delete item;
    }
}

GraphicsItem* GraphicsScene::findItem( const QModelIndex& idx ) const
{
    if ( !idx.isValid() ) return 0;
    assert( idx.model() == summaryHandlingModel() );
    QHash<QPersistentModelIndex,GraphicsItem*>::const_iterator it = d->items.find( idx );
    return ( it != d->items.end() )?*it:0;
}

GraphicsItem* GraphicsScene::findItem( const QPersistentModelIndex& idx ) const
{
    if ( !idx.isValid() ) return 0;
    assert( idx.model() == summaryHandlingModel() );
    QHash<QPersistentModelIndex,GraphicsItem*>::const_iterator it = d->items.find( idx );
    return ( it != d->items.end() )?*it:0;
}

void GraphicsScene::clearItems()
{
    // TODO constraints
    qDeleteAll( items() );
    d->items.clear();
}

void GraphicsScene::updateItems()
{
    for ( QHash<QPersistentModelIndex,GraphicsItem*>::iterator it = d->items.begin();
          it != d->items.end(); ++it ) {
        GraphicsItem* const item = it.value();
        const QPersistentModelIndex& idx = it.key();
        item->updateItem( Span( item->pos().y(), item->rect().height() ), idx );
    }
}

void GraphicsScene::deleteSubtree( const QModelIndex& _idx )
{
    QModelIndex idx = dataIndex( _idx );
    removeItem( idx );
    for ( int i = 0; i < summaryHandlingModel()->rowCount( _idx ); ++i ) {
        deleteSubtree( summaryHandlingModel()->index( i, summaryHandlingModel()->columnCount(_idx)-1, _idx ) );
    }
}


ConstraintGraphicsItem* GraphicsScene::findConstraintItem( const Constraint& c ) const
{
    return d->findConstraintItem( c );
}

void GraphicsScene::clearConstraintItems()
{
    // TODO
    // d->constraintItems.clearConstraintItems();
}

void GraphicsScene::slotConstraintAdded( const Constraint& c )
{
    d->createConstraintItem( c );
}

void GraphicsScene::slotConstraintRemoved( const Constraint& c )
{
    d->deleteConstraintItem( c );
}

void GraphicsScene::slotGridChanged()
{
    updateItems();
    update();
    emit gridChanged();
}

void GraphicsScene::helpEvent( QGraphicsSceneHelpEvent *helpEvent )
{
#ifndef QT_NO_TOOLTIP
    QGraphicsItem *item = itemAt( helpEvent->scenePos() );
    if ( GraphicsItem* gitem = qgraphicsitem_cast<GraphicsItem*>( item ) ) {
        QToolTip::showText(helpEvent->screenPos(), gitem->ganttToolTip());
    } else if ( ConstraintGraphicsItem* citem = qgraphicsitem_cast<ConstraintGraphicsItem*>( item ) ) {
        QToolTip::showText(helpEvent->screenPos(), citem->ganttToolTip());
    } else {
        QGraphicsScene::helpEvent( helpEvent );
    }
#endif /* QT_NO_TOOLTIP */
}

void GraphicsScene::drawBackground( QPainter* painter, const QRectF& rect )
{
    d->grid->paintGrid( painter, sceneRect(), rect, d->rowController );
}

void GraphicsScene::itemEntered( const QModelIndex& idx )
{
    emit entered( idx );
}

void GraphicsScene::itemPressed( const QModelIndex& idx )
{
    emit pressed( idx );
}

void GraphicsScene::itemClicked( const QModelIndex& idx )
{
    emit clicked( idx );
}

void GraphicsScene::itemDoubleClicked( const QModelIndex& idx )
{
    emit doubleClicked( idx );
}

void GraphicsScene::setDragSource( GraphicsItem* item )
{
    d->dragSource = item;
}

GraphicsItem* GraphicsScene::dragSource() const
{
    return d->dragSource;
}

#define KDGANTT_LIST_CHART_GAP 0.0
QRectF GraphicsScene::printRect( bool drawRowLabels, GraphicsView *view )
{
    assert(rowController());

    int indentation = 20;
    bool indentRoot = true;

    qreal leftEdge = sceneRect().left();
    QVector<QGraphicsTextItem*> labelItems;
    if ( drawRowLabels ) {
        labelItems.reserve(d->items.size());
        qreal textWidth = 0.;
        qreal rowHeight = 0.;
        {Q_FOREACH( GraphicsItem* item, d->items ) {
            QModelIndex sidx = summaryHandlingModel()->mapToSource( item->index() );
            if ( sidx.parent().isValid() && sidx.parent().data( ItemTypeRole ).toInt() == TypeMulti ) {
                continue;
            }
            const Span rg = rowController()->rowGeometry( sidx );
            const QString txt = item->index().data( Qt::DisplayRole ).toString();
            QGraphicsTextItem* ti = new QGraphicsTextItem( txt, 0, this );
            ti->setPos( 0, rg.start() );
            ti->document()->size().setWidth( ti->document()->size().width() + KDGANTT_LIST_CHART_GAP );
            int indent = indentation * ( level( item->index() ) + ( indentRoot ? 1 : 0 ) );
            if ( ti->document()->size().width() + indentation > textWidth ) {
                textWidth = ti->document()->size().width() + indent;
            }
            if ( rg.length() > rowHeight ) {
                rowHeight = rg.length();
            }
            labelItems << ti;
        }}
        {Q_FOREACH( QGraphicsTextItem* item, labelItems ) {
            item->setPos( leftEdge-textWidth-rowHeight, item->pos().y() );
            item->show();
        }}
    }
    QRectF res = itemsBoundingRect();
    if ( view ) {
        res.setHeight( res.height() + view->headerHeight() );
    }
    qDeleteAll( labelItems );
    //qDebug()<<"printRect()"<<res;
    return res;
}

void GraphicsScene::drawTreeIndication( QPainter *painter, const QModelIndex &idx, const QRect &rect, int indent, bool drawRoot )
{
//    qDebug()<<"drawTreeIndication:"<<idx.data().toString()<<rect;

    QStyleOptionViewItemV4 option;
    option.rect.setRect( rect.left(), rect.top(), rect.width(), rect.height() );
    option.state = option.state | QStyle::State_Enabled | QStyle::State_Active | QStyle::State_Item;
    if ( idx.sibling(idx.row()+1, 0).isValid() ) {
        option.state |= QStyle::State_Sibling;
    }
    if ( idx.model()->hasChildren( idx ) ) {
        option.state |= QStyle::State_Children;
    }

    QPalette::ColorGroup cg;
    cg = QPalette::Active;
    option.palette.setCurrentColorGroup(cg);

    QModelIndex parent = idx.parent();
    QModelIndex currentParent = parent;
    QModelIndex ancestor = currentParent.parent();

    QRect area( option.rect.right() + 1, option.rect.top(), indent, option.rect.height() ); // moved left before use

    if ( currentParent.isValid() || drawRoot ) {
        // draw for idx
        area.moveLeft(area.left() - indent);
        option.rect = area;
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter, 0);
    }
    option.state &= ~QStyle::State_Item;
    option.state &= ~QStyle::State_Children;
    while ( currentParent.isValid() ) {
        // then draw ancestors
        if ( ! ancestor.isValid() && ! drawRoot ) {
            break;
        }
        area.moveLeft(area.left() - indent);
        option.rect = area;
        if ( currentParent.sibling( currentParent.row() + 1, 0 ).isValid() ) {
            option.state |= QStyle::State_Sibling;
        } else {
            option.state &= ~QStyle::State_Sibling;
        }
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter);
        currentParent = ancestor;
        ancestor = currentParent.parent();
    }
}

int GraphicsScene::level( const QModelIndex &idx ) const
{
    int l = 0;
    for ( QModelIndex p = idx.parent(); p.isValid(); p = p.parent() ) {
        ++l;
    }
    return l;
}

void GraphicsScene::print( QPainter* painter, const QRectF& target, const QRectF& source, bool drawRowLabels, GraphicsView *view )
{
    QRectF targetRect(target);

    assert(rowController());

    int indentation = 20;
    bool indentRoot = true;

    qreal scale = 1.0;
    if ( source.width() > target.width() ) {
        scale = target.width() / source.width();
    }
    if ( source.height() > target.height() ) {
        scale = qMin( scale, target.height() / source.height() );
    }
    painter->scale( scale, scale );
    targetRect.setWidth( targetRect.width() / scale );
    targetRect.setHeight( targetRect.height() / scale );

    qreal textWidth = 0.;
    if (drawRowLabels) {
        painter->save();

        qreal top = view ? view->headerHeight() : 0.0;
        {Q_FOREACH( GraphicsItem* item, d->items ) {
            QModelIndex sidx = summaryHandlingModel()->mapToSource( item->index() );
            if ( sidx.parent().isValid() && sidx.parent().data( ItemTypeRole ).toInt() == TypeMulti ) {
                continue;
            }
            const Span rg = rowController()->rowGeometry( sidx );
            const QString txt = item->index().data( Qt::DisplayRole ).toString();
            int indent = indentation * (level( item->index() ) + ( indentRoot ? 1 : 0 ) );
            QRectF r( indent, rg.start() + top, 0.0, rg.length() );
            r = painter->boundingRect( r, Qt::AlignLeft | Qt::AlignVCenter, txt );
            painter->drawText( r, Qt::AlignLeft | Qt::AlignVCenter, txt );
            if ( r.width() > textWidth ) {
                textWidth = r.width() + indent;
            }
            QRect rect = r.toRect();
            rect.setLeft( 1 );
            rect.setRight( r.left() );
            drawTreeIndication( painter, item->index(), rect, indentation, indentRoot );
        }}
        painter->restore();
        if ( view ) {
            QStyle* style = QApplication::style();
            QRectF r( 0.0, 0.0, textWidth, view->headerHeight() );
            QStyleOptionHeader opt;
            opt.init( view );
            opt.rect = r.toRect();
            opt.text = summaryHandlingModel()->headerData( 0, Qt::Horizontal, Qt::DisplayRole ).toString();
            opt.textAlignment = Qt::AlignCenter;
            // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
            style->drawControl( QStyle::CE_HeaderSection, &opt, painter, view );
            QStyleOptionHeader subopt = opt;
            subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, view );
            if ( subopt.rect.isValid() ) {
                style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, view );
            }
        }
        if ( textWidth > 0 ) {
            textWidth += KDGANTT_LIST_CHART_GAP;
        }
    }
    QRectF oldSceneRect( sceneRect() );
    setSceneRect( itemsBoundingRect() );
    QRectF sourceRect = source.adjusted( textWidth, (qreal)0.0, (qreal)0.0, (qreal)0.0 );
    targetRect.adjust( textWidth, (qreal)0.0, (qreal)0.0, (qreal)0.0 );
    //qDebug()<<"GraphicsScene::print() 1"<<"scene"<<sceneRect()<<"target"<<targetRect<<"source"<<source<<"sourceRect"<<sourceRect<<"text"<<textWidth<<"scale"<<scale;
    if ( view ) {
        qreal width = sourceRect.width();
        qreal height = sourceRect.height();
        qreal scale = 1.0;
        if ( width > targetRect.width() ) {
            scale = targetRect.width() / width;
        }
        if ( height > targetRect.height() ) {
            scale = qMin( scale, targetRect.height() / height );
        }
        QRectF t = targetRect;
        QRectF s = sourceRect;
        s.setHeight( view->headerHeight() );
        t.setWidth( s.width() * scale );
        t.setHeight( s.height() * scale );
        view->renderHeader( painter, t, s, Qt::KeepAspectRatio );
        targetRect.translate( (qreal)0.0, t.height() );
        targetRect.setHeight( targetRect.height() - t.height() );
    }
    //qDebug()<<"GraphicsScene::print() 3"<<sceneRect()<<targetRect<<sourceRect;

    if ( targetRect.width() > sourceRect.width() && targetRect.height() > sourceRect.height() ) {
        // do not scale up
        sourceRect.setSize( targetRect.size() );
        render( painter, targetRect, sourceRect, Qt::IgnoreAspectRatio );
    } else {
        render( painter, targetRect, sourceRect );
    }

    setSceneRect( oldSceneRect );
}


#include "moc_kdganttgraphicsscene.cpp"


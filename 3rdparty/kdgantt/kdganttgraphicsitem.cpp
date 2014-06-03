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
#include "kdganttgraphicsitem.h"
#include "kdganttgraphicsscene.h"
#include "kdganttgraphicsview.h"
#include "kdganttitemdelegate.h"
#include "kdganttconstraintgraphicsitem.h"
#include "kdganttconstraintmodel.h"
#include "kdganttconstraint.h"
#include "kdganttabstractgrid.h"
#include "kdganttabstractrowcontroller.h"

#include <cassert>
#include <cmath>
#include <algorithm>
#include <iterator>

#include <QPainter>
#include <QAbstractItemModel>
#include <QAbstractProxyModel>
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>

#include <QDebug>

/*!\class KDGantt::GraphicsItem
 * \internal
 */

using namespace KDGantt;

typedef QGraphicsItem BASE;

namespace {
    class Updater {
        bool *u_ptr;
        bool oldval;
    public:
        Updater( bool* u ) : u_ptr( u ), oldval( *u ) {
            *u=true;
        }
        ~Updater() {
            *u_ptr = oldval;
        }
    };
}

GraphicsItem::GraphicsItem( QGraphicsItem* parent, GraphicsScene* scene )
    : BASE( parent, scene ),  m_isupdating( false )
{
  init();
}

GraphicsItem::GraphicsItem( const QModelIndex& idx, QGraphicsItem* parent,
                                            GraphicsScene* scene )
    : BASE( parent, scene ),  m_index( idx ), m_isupdating( false )
{
  init();
}

GraphicsItem::~GraphicsItem()
{
}

void GraphicsItem::init()
{
    setFlags( ItemIsMovable|ItemIsSelectable|ItemIsFocusable );
    setAcceptsHoverEvents( true );
    setHandlesChildEvents( true );
    setZValue( 100. );
    m_dragline = 0;
}

int GraphicsItem::type() const
{
    return Type;
}

StyleOptionGanttItem GraphicsItem::getStyleOption() const
{
    StyleOptionGanttItem opt;
    opt.itemRect = rect();
    opt.boundingRect = boundingRect();
    opt.displayPosition = StyleOptionGanttItem::Right;
    opt.displayAlignment = static_cast< Qt::Alignment >( m_index.model()->data( m_index, Qt::TextAlignmentRole ).toInt() );
    opt.grid = scene()->grid();
    opt.text = m_index.model()->data( m_index, Qt::DisplayRole ).toString(); 
    if ( isEnabled() ) opt.state  |= QStyle::State_Enabled;
    if ( isSelected() ) opt.state |= QStyle::State_Selected;
    if ( hasFocus() ) opt.state   |= QStyle::State_HasFocus;
    return opt;
}

GraphicsScene* GraphicsItem::scene() const
{
    return qobject_cast<GraphicsScene*>( QGraphicsItem::scene() );
}

void GraphicsItem::setRect( const QRectF& r )
{
    prepareGeometryChange();
    m_rect = r;
    updateConstraintItems();
    update();
}

void GraphicsItem::setBoundingRect( const QRectF& r )
{
    prepareGeometryChange();
    m_boundingrect = r;
    update();
}

bool GraphicsItem::isEditable() const
{
    return !scene()->isReadOnly() && m_index.model()->flags( m_index ) & Qt::ItemIsEditable;
}

void GraphicsItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option,
                                  QWidget* widget )
{
    Q_UNUSED( widget );
    if ( boundingRect().isValid() && scene() ) {
        StyleOptionGanttItem opt = getStyleOption();
        *static_cast<QStyleOption*>(&opt) = *static_cast<const QStyleOption*>( option );
        scene()->itemDelegate()->paintGanttItem( painter, opt, index() );
    }
}

void GraphicsItem::setIndex( const QPersistentModelIndex& idx )
{
    m_index=idx;
    update();
}

QString GraphicsItem::ganttToolTip() const
{
    return scene()->itemDelegate()->toolTip( index() );
}

QRectF GraphicsItem::boundingRect() const
{
    return m_boundingrect;
}

QPointF GraphicsItem::startConnector( int relationType ) const
{
    switch ( relationType ) {
        case Constraint::StartStart:
        case Constraint::StartFinish:
            return mapToScene( m_rect.left(), m_rect.top()+m_rect.height()/2. );
        default:
            break;
    }
    return mapToScene( m_rect.right(), m_rect.top()+m_rect.height()/2. );
}

QPointF GraphicsItem::endConnector( int relationType ) const
{
    switch ( relationType ) {
        case Constraint::FinishFinish:
        case Constraint::StartFinish:
            return mapToScene( m_rect.right(), m_rect.top()+m_rect.height()/2. );
        default:
            break;
    }
    return mapToScene( m_rect.left(), m_rect.top()+m_rect.height()/2. );
}


void GraphicsItem::addStartConstraint( ConstraintGraphicsItem* item )
{
    assert( item );
    m_startConstraints << item;
    item->setStart( startConnector( item->constraint().relationType() ) );
}

void GraphicsItem::addEndConstraint( ConstraintGraphicsItem* item )
{
    assert( item );
    m_endConstraints << item;
    item->setEnd( endConnector( item->constraint().relationType() ) );
}

void GraphicsItem::removeStartConstraint( ConstraintGraphicsItem* item )
{
    assert( item );
    m_startConstraints.removeAll( item );
}

void GraphicsItem::removeEndConstraint( ConstraintGraphicsItem* item )
{
    assert( item );
    m_endConstraints.removeAll( item );
}

void GraphicsItem::updateConstraintItems()
{
    { // Workaround for multiple definition error with MSVC6
    Q_FOREACH( ConstraintGraphicsItem* item, m_startConstraints ) {
        QPointF s = startConnector( item->constraint().relationType() );
        item->setStart( s );
    }}
    {// Workaround for multiple definition error with MSVC6
    Q_FOREACH( ConstraintGraphicsItem* item, m_endConstraints ) {
        QPointF e = endConnector( item->constraint().relationType() );
        item->setEnd( e );
    }}
}

void GraphicsItem::updateItem( const Span& rowGeometry, const QPersistentModelIndex& idx )
{
    Updater updater( &m_isupdating );
    //qDebug() << "GraphicsItem::updateItem("<<rowGeometry<<idx<<")";
    if ( !idx.isValid() || idx.data( ItemTypeRole )==TypeMulti ) {
        setRect( QRectF() );
	hide();
        return;
    }

    Span s = scene()->grid()->mapToChart( idx );
    setPos( QPointF( s.start(), rowGeometry.start() ) );
    setRect( QRectF( 0., 0., s.length(), rowGeometry.length() ) );
    setIndex( idx );
    Span bs = scene()->itemDelegate()->itemBoundingSpan( getStyleOption(), index() );
    setBoundingRect( QRectF( bs.start(), 0., bs.length(), rowGeometry.length() ) );
    int maxh = scene()->rowController()->maximumItemHeight();
    if ( maxh < rowGeometry.length() ) {
        QRectF r = rect();
        Qt::Alignment align = getStyleOption().displayAlignment;
        if ( align & Qt::AlignTop ) {
            // Do nothing
        } else if ( align & Qt::AlignBottom ) {
            r.setY( rowGeometry.length()-maxh );
        } else {
            // Center
            r.setY( ( rowGeometry.length()-maxh ) / 2. );
        }
        r.setHeight( maxh );
        setRect( r );
    }

    scene()->setSceneRect( scene()->sceneRect().united( mapToScene( boundingRect() ).boundingRect() ) );
    updateConstraintItems();
}

QVariant GraphicsItem::itemChange( GraphicsItemChange change, const QVariant& value )
{
    if ( !isUpdating() && change==ItemPositionChange && scene() ) {
        QPointF newPos=value.toPointF();
        if ( isEditable() ) {
            newPos.setY( pos().y() );
            return newPos;
        } else {
            return pos();
        }
    } else if ( change==QGraphicsItem::ItemSelectedChange ) {
        if ( value.toBool() ) {
            scene()->selectionModel()->select( index(), QItemSelectionModel::Select );
        } else {
            scene()->selectionModel()->select( index(), QItemSelectionModel::Deselect );
        }
    }

    return QGraphicsItem::itemChange( change, value );
}

void GraphicsItem::focusInEvent( QFocusEvent* event )
{
    Q_UNUSED( event );
    scene()->selectionModel()->select( index(), QItemSelectionModel::SelectCurrent );
}

void GraphicsItem::updateModel()
{
    //qDebug() << "GraphicsItem::updateModel("<<p<<")";
    if ( isEditable() ) {
        QAbstractItemModel* model = const_cast<QAbstractItemModel*>( index().model() );
        ConstraintModel* cmodel = scene()->constraintModel();
        assert( model );
        assert( cmodel );
        if ( model ) {
            //ItemType typ = static_cast<ItemType>( model->data( index(),
            //                                                   ItemTypeRole ).toInt() );
            const QModelIndex sourceIdx = scene()->summaryHandlingModel()->mapToSource( index() );
            QList<Constraint> constraints;
            for( QList<ConstraintGraphicsItem*>::iterator it1 = m_startConstraints.begin() ; it1 != m_startConstraints.end() ; ++it1 )
				constraints.push_back((*it1)->proxyConstraint());
            for( QList<ConstraintGraphicsItem*>::iterator it2 = m_endConstraints.begin() ; it2 != m_endConstraints.end() ; ++it2 )
				constraints.push_back((*it2)->proxyConstraint());
            if ( scene()->grid()->mapFromChart( Span( pos().x(), rect().width() ),
                                                index(),
                                                constraints ) ) {
                scene()->updateRow( index().parent() );
            }
        }
    }
}

void GraphicsItem::hoverMoveEvent( QGraphicsSceneHoverEvent* event )
{
    if (  !isEditable() ) return;
    StyleOptionGanttItem opt = getStyleOption();
    ItemDelegate::InteractionState istate = scene()->itemDelegate()->interactionStateFor( event->pos(), opt, index() );
    switch( istate ) {
    case ItemDelegate::State_ExtendLeft:
        setCursor( Qt::SizeHorCursor );
        scene()->itemEntered( index() );
        break;
    case ItemDelegate::State_ExtendRight:
        setCursor( Qt::SizeHorCursor );
        scene()->itemEntered( index() );
        break;
    case ItemDelegate::State_Move:
        setCursor( Qt::SplitHCursor );
        scene()->itemEntered( index() );
        break;
    default:
        unsetCursor();
    };
}

void GraphicsItem::hoverLeaveEvent( QGraphicsSceneHoverEvent* )
{
    unsetCursor();
}

void GraphicsItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    StyleOptionGanttItem opt = getStyleOption();
    m_istate = scene()->itemDelegate()->interactionStateFor( event->pos(), opt, index() );
    m_presspos = event->pos();
    m_pressscenepos = event->scenePos();
    scene()->itemPressed( index() );

    switch( m_istate ) {
    case ItemDelegate::State_ExtendLeft:
    case ItemDelegate::State_ExtendRight:
    default: /* None and Move */
        BASE::mousePressEvent( event );
        break;
    }
}

void GraphicsItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    if ( !m_presspos.isNull() ) {
        scene()->itemClicked( index() );
    }
    delete m_dragline; m_dragline = 0;
    if ( scene()->dragSource() ) {
        // Create a new constraint
        GraphicsItem* other = qgraphicsitem_cast<GraphicsItem*>( scene()->itemAt( event->scenePos() ) );
        if ( other && scene()->dragSource()!=other &&
             other->mapToScene( other->rect() ).boundingRect().contains( event->scenePos() )) {
            GraphicsView* view = qobject_cast<GraphicsView*>( event->widget()->parentWidget() );
            if ( view ) {
                view->addConstraint( scene()->summaryHandlingModel()->mapToSource( scene()->dragSource()->index() ),
                                     scene()->summaryHandlingModel()->mapToSource( other->index() ), event->modifiers() );
            }
        }
        scene()->setDragSource( 0 );
    } else {
        updateItemFromMouse(event->scenePos());
        updateModel();
    }

    m_presspos = QPointF();
    BASE::mouseReleaseEvent( event );
}

void GraphicsItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event )
{
    StyleOptionGanttItem opt = getStyleOption();
    ItemDelegate::InteractionState istate = scene()->itemDelegate()->interactionStateFor( event->pos(), opt, index() );
    if ( istate != ItemDelegate::State_None ) {
        scene()->itemDoubleClicked( index() );
    }
    BASE::mouseDoubleClickEvent( event );
}

void GraphicsItem::updateItemFromMouse( const QPointF& scenepos )
{
    const QPointF p = scenepos - m_presspos;
    QRectF r = rect();
    QRectF br = boundingRect();
    switch( m_istate ) {
    case ItemDelegate::State_Move:
        setPos( p.x(), pos().y() );
        break;
    case ItemDelegate::State_ExtendLeft: {
        const qreal brr = br.right();
        const qreal rr = r.right();
        const qreal delta = pos().x()-p.x();
        setPos( p.x(), QGraphicsItem::pos().y() );
        br.setRight( brr+delta );
        r.setRight( rr+delta );
        break;
    }
    case ItemDelegate::State_ExtendRight: {
        const qreal rr = r.right();
        r.setRight( scenepos.x()-pos().x() );
        br.setWidth( br.width() + r.right()-rr );
        break;
    }
    default: return;
    }
    setRect( r );
    setBoundingRect( br );
}

void GraphicsItem::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    if (  !isEditable() ) return;
    //qDebug() << "GraphicsItem::mouseMoveEvent("<<event<<"), m_istate="<< static_cast<ItemDelegate::InteractionState>( m_istate );
    switch( m_istate ) {
    case ItemDelegate::State_ExtendLeft:
    case ItemDelegate::State_ExtendRight:
    case ItemDelegate::State_Move:
        // Check for constraint drag
      if ( qAbs( m_pressscenepos.x()-event->scenePos().x() ) < 10.
	   && qAbs( m_pressscenepos.y()-event->scenePos().y() ) > 5. ) {
            m_istate = ItemDelegate::State_DragConstraint;
            m_dragline = new QGraphicsLineItem( this );
            m_dragline->setPen( QPen( Qt::DashLine ) );
            m_dragline->setLine(QLineF( rect().center(), event->pos() ));
            scene()->addItem( m_dragline );
            scene()->setDragSource( this );
            break;
        }

        scene()->selectionModel()->setCurrentIndex( index(), QItemSelectionModel::Current );
        updateItemFromMouse(event->scenePos());
        //BASE::mouseMoveEvent(event);
        break;
    case ItemDelegate::State_DragConstraint: {
        QLineF line = m_dragline->line();
        m_dragline->setLine( QLineF( line.p1(), event->pos() ) );
        //QGraphicsItem* item = scene()->itemAt( event->scenePos() );
        break;
    }
    }
}

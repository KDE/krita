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
#include "kdganttlegend.h"
#include "kdganttlegend_p.h"

#include "kdganttitemdelegate.h"

#include <QApplication>
#include <QPainter>

#include <cassert>

using namespace KDGantt;

/*!\class KDGantt::Legend kdganttlegend.h KDGanttLegend
 * \ingroup KDGantt
 * \brief Legend showing an image and a description for Gantt items
 *
 * This is an item view class showing a small Gantt item and it's
 * text defined by LegendRole.
 */

/*! Constructor. Creates a Legend with parent \a parent.
 * The QObject parent is not used for anything internally. */
Legend::Legend( QWidget* parent )
    : QAbstractItemView( parent ),
      _d( new Private )
{
    setItemDelegate( new ItemDelegate( this ) );
    setFrameStyle( QFrame::NoFrame );
}

/*! Destructor. Does nothing */
Legend::~Legend()
{
    delete _d;
}

#define d d_func()

QModelIndex Legend::indexAt( const QPoint& /*point */) const
{
    return QModelIndex();
}

QRect Legend::visualRect( const QModelIndex& /*index */) const
{
    return QRect();
}

QSize Legend::sizeHint() const
{
    return measureItem( rootIndex() );
}

QSize Legend::minimumSizeHint() const
{
    return measureItem( rootIndex() );
}

void Legend::setModel( QAbstractItemModel* model )
{
    if( this->model() != 0 )
    {
        disconnect( this->model(), SIGNAL( dataChanged( QModelIndex, QModelIndex ) ), this, SLOT( modelDataChanged() ) );
        disconnect( this->model(), SIGNAL( rowsRemoved( QModelIndex, int, int ) ), this, SLOT( modelDataChanged() ) );
        disconnect( this->model(), SIGNAL( columnsRemoved( QModelIndex, int, int ) ), this, SLOT( modelDataChanged() ) );
    }

    QAbstractItemView::setModel( model );
    d->proxyModel.setSourceModel( model );

    if( this->model() != 0 )
    {
        connect( this->model(), SIGNAL( dataChanged( QModelIndex, QModelIndex ) ), this, SLOT( modelDataChanged() ) );
        connect( this->model(), SIGNAL( rowsRemoved( QModelIndex, int, int ) ), this, SLOT( modelDataChanged() ) );
        connect( this->model(), SIGNAL( columnsRemoved( QModelIndex, int, int ) ), this, SLOT( modelDataChanged() ) );
    }

}

/*! Triggers repainting of the legend.
 */
void Legend::modelDataChanged()
{
    updateGeometry();
    viewport()->update();
}

void Legend::paintEvent( QPaintEvent* event )
{
    Q_UNUSED( event );
    // no model, no legend...
    if( model() == 0 )
        return;

    QPainter p( viewport() );
    p.fillRect( viewport()->rect(), palette().color( QPalette::Window ) );
    drawItem( &p, rootIndex() );
}

/*! Creates a StyleOptionGanttItem with all style options filled in
 *  except the target rectangles.
 */
StyleOptionGanttItem Legend::getStyleOption( const QModelIndex& index ) const
{
    StyleOptionGanttItem opt;
    opt.displayPosition = StyleOptionGanttItem::Right;
    opt.displayAlignment = Qt::Alignment( d->proxyModel.data( index, Qt::TextAlignmentRole ).toInt() );
    opt.text = index.model()->data( index, LegendRole ).toString();
    opt.font = qVariantValue< QFont >( index.model()->data( index, Qt::FontRole ) );
    return opt;
}

/*! Draws the legend item at \a index and all of it's children recursively
 *  at \a pos onto \a painter.
 *  Reimplement this if you want to draw items in an user defined way.
 *  \returns the rectangle drawn.
 */
QRect Legend::drawItem( QPainter* painter, const QModelIndex& index, const QPoint& pos ) const
{
    int xPos = pos.x();
    int yPos = pos.y();

    if( index.isValid() && index.model() == &d->proxyModel )
    {
        ItemDelegate* const delegate = qobject_cast< ItemDelegate* >( itemDelegate( index ) );
        assert( delegate != 0 );
        const QRect r( pos, measureItem( index, false ) );
        StyleOptionGanttItem opt = getStyleOption( index );
        opt.rect = r;
        opt.rect.setWidth( r.height() );
        opt.itemRect = opt.rect;
        opt.boundingRect = r;
        opt.boundingRect.setWidth( r.width() + r.height() );
        if( !opt.text.isNull() )
            delegate->paintGanttItem( painter, opt, index );

        xPos = r.right();
        yPos = r.bottom();
    }


    const int rowCount = d->proxyModel.rowCount( index );
    for( int row = 0; row < rowCount; ++row )
    {
        const QRect r = drawItem( painter, d->proxyModel.index( row, 0, index ), QPoint( pos.x(), yPos ) );
        xPos = qMax( xPos, r.right() );
        yPos = qMax( yPos, r.bottom() );
    }

    return QRect( pos, QPoint( xPos, yPos ) );
}

/*! Calculates the needed space for the legend item at \a index and, if \a recursive is true,
 *  all child items.
 */
QSize Legend::measureItem( const QModelIndex& index, bool recursive ) const
{
    if( model() == 0 )
        return QSize();

    QSize baseSize;
    if( index.model() != 0 )
    {
        QFontMetrics fm( qVariantValue< QFont >( index.model()->data( index, Qt::FontRole ) ) );
        const QString text = index.model()->data( index, LegendRole ).toString();
        if( !text.isNull() )
            baseSize += QSize( fm.width( text ) + fm.height() + 2, fm.height() + 2 );
    }

    if( !recursive )
        return baseSize;

    QSize childrenSize;

    const int rowCount = d->proxyModel.rowCount( index );
    for( int row = 0; row < rowCount; ++row )
    {
        const QSize childSize = measureItem( d->proxyModel.index( row, 0, index ) );
        childrenSize.setWidth( qMax( childrenSize.width(), childSize.width() ) );
        childrenSize.rheight() += childSize.height();
    }
    return baseSize + childrenSize;
}

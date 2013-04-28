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
#include "kdganttview.h"
#include "kdganttview_p.h"

#include "kdganttitemdelegate.h"
#include "kdganttgraphicsitem.h"

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QScrollBar>
#include <QPaintEvent>

#include <QDebug>

#include <cassert>

#if defined KDAB_EVAL
#include "../evaldialog/evaldialog.h"
#endif

using namespace KDGantt;

namespace {
    class HeaderView : public QHeaderView {
    public:
        explicit HeaderView( QWidget* parent=0 ) : QHeaderView( Qt::Horizontal, parent ) {
        }

        QSize sizeHint() const { QSize s = QHeaderView::sizeHint(); s.rheight() *= 2; return s; }
    };
}

KDGanttTreeView::KDGanttTreeView( QAbstractProxyModel* proxy, QWidget* parent )
    : QTreeView( parent ),
      m_controller( this, proxy )
{
    setHeader( new HeaderView );
}

KDGanttTreeView::~KDGanttTreeView()
{
}

View::Private::Private(View* v)
    : q(v),
      splitter(v),
      rowController(0),
      gfxview(&splitter),
      model(0)
{
    //init();
}

View::Private::~Private()
{
}

void View::Private::init()
{
    KDGanttTreeView* tw = new KDGanttTreeView( &ganttProxyModel, &splitter );
    tw->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tw->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

    q->setLeftView( tw );
    q->setRowController( tw->rowController() );

    gfxview.setAlignment(Qt::AlignTop|Qt::AlignLeft);
    //gfxview.setRenderHints( QPainter::Antialiasing );
    
    tw->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    gfxview.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QVBoxLayout* layout = new QVBoxLayout(q);
    layout->setMargin(0);
    layout->addWidget(&splitter);
    q->setLayout(layout);

    constraintProxy.setProxyModel( &ganttProxyModel );
    constraintProxy.setDestinationModel( &mappedConstraintModel );
    gfxview.setSelectionModel( leftWidget->selectionModel() );
    gfxview.setConstraintModel( &mappedConstraintModel );
}

void View::Private::updateScene()
{
    gfxview.clearItems();
    if( !model) return;

    if( QTreeView* tw = qobject_cast<QTreeView*>(leftWidget)) {
      QModelIndex idx = ganttProxyModel.mapFromSource( model->index( 0, 0, leftWidget->rootIndex() ) );
      do {
        gfxview.updateRow( idx );
      } while ( ( idx = tw->indexBelow( idx ) ) != QModelIndex() &&
		gfxview.rowController()->isRowVisible(idx) );
      gfxview.updateSceneRect();
    } else {
      const QModelIndex rootidx = ganttProxyModel.mapFromSource( leftWidget->rootIndex() );
      for( int r = 0; r < ganttProxyModel.rowCount(rootidx); ++r ) {
	gfxview.updateRow( ganttProxyModel.index( r, 0, rootidx ) );
      }
    }
}

void View::Private::slotCollapsed(const QModelIndex& _idx)
{
    QTreeView* tw = qobject_cast<QTreeView*>(leftWidget);
    if(!tw) return;
    QModelIndex idx( _idx );
    const QAbstractItemModel* model = leftWidget->model();
    const QModelIndex pidx = ganttProxyModel.mapFromSource(idx);
    if ( pidx.data( ItemTypeRole ).toInt() != TypeMulti ) {
        for ( int i = 0; i < model->rowCount( idx ); ++i ) {
            gfxview.deleteSubtree( ganttProxyModel.index( i, 0, pidx ) );
        }
    }
    //qDebug() << "Looking to update from " << idx;
    while ( ( idx=tw->indexBelow( idx ) ) != QModelIndex() &&
            gfxview.rowController()->isRowVisible( ganttProxyModel.mapFromSource(idx) ) ) {
        const QModelIndex proxyidx( ganttProxyModel.mapFromSource( idx ) );
        gfxview.updateRow(proxyidx);
    }
    gfxview.updateSceneRect();
}

void View::Private::slotExpanded(const QModelIndex& _idx)
{
    QModelIndex idx( ganttProxyModel.mapFromSource( _idx ) );
    do {
        gfxview.updateRow(idx);
    } while( ( idx=gfxview.rowController()->indexBelow( idx ) ) != QModelIndex()
             && gfxview.rowController()->isRowVisible( idx ) );
    gfxview.updateSceneRect();
}

void View::Private::slotVerticalScrollValueChanged( int val )
{
    leftWidget->verticalScrollBar()->setValue( val/gfxview.verticalScrollBar()->singleStep() );
}

void View::Private::slotLeftWidgetVerticalRangeChanged(int min, int max )
{
    gfxview.verticalScrollBar()->setRange( min, max );
}

void View::Private::slotGfxViewVerticalRangeChanged( int min, int max )
{
    int leftMin = leftWidget->verticalScrollBar()->minimum();
    int leftMax = leftWidget->verticalScrollBar()->maximum();
    bool blocked = gfxview.verticalScrollBar()->blockSignals( true );
    gfxview.verticalScrollBar()->setRange( qMax( min, leftMin ), qMax( max, leftMax ) );
    gfxview.verticalScrollBar()->blockSignals( blocked );
}

/*!\class KDGantt::View kdganttview.h KDGanttView
 * \ingroup KDGantt
 * \brief This widget that consists of a QTreeView and a GraphicsView
 *
 * This is the easy to use, complete gantt chart widget. It
 * consists of a QTreeView on the left and a KDGantt::GraphicsView
 * on the right separated by a QSplitter. The two views share the same
 * model.
 */

/*! Constructor. Creates a View with parent \a parent,
 * a DateTimeGrid as default grid implementaion and no model etc.
 */
View::View(QWidget* parent)
    : QWidget(parent),
      _d(new Private(this))
{
#if defined KDAB_EVAL
   EvalDialog::checkEvalLicense( "KD Gantt" );
#endif
   _d->init();
}

View::~View()
{
    delete _d;
}

#define d d_func()

/*! \param aiv The view to be used to the left, instead of the default tree view
 * \sa setRowController()
 */
void View::setLeftView( QAbstractItemView* aiv )
{
    assert( aiv );
    if ( aiv==d->leftWidget ) return;
    if ( !d->leftWidget.isNull() ) {
        d->leftWidget->disconnect( this );
        d->leftWidget->hide();
        d->leftWidget->verticalScrollBar()->disconnect( d->gfxview.verticalScrollBar() );
        d->gfxview.verticalScrollBar()->disconnect( d->leftWidget->verticalScrollBar() );
    }

    d->leftWidget = aiv;
    d->splitter.insertWidget( 0, d->leftWidget );

    if( qobject_cast<QTreeView*>(d->leftWidget) ) {
      connect( d->leftWidget,  SIGNAL( collapsed( const QModelIndex& ) ),
	       this, SLOT( slotCollapsed( const QModelIndex& ) ) );
      connect( d->leftWidget,  SIGNAL( expanded( const QModelIndex& ) ),
	       this, SLOT( slotExpanded( const QModelIndex& ) ) );
    }

    connect( d->gfxview.verticalScrollBar(), SIGNAL( valueChanged( int ) ),
             d->leftWidget->verticalScrollBar(), SLOT( setValue( int ) ) );
    connect( d->leftWidget->verticalScrollBar(), SIGNAL( valueChanged( int ) ),
             d->gfxview.verticalScrollBar(), SLOT( setValue( int ) ) );
    connect( d->leftWidget->verticalScrollBar(), SIGNAL( rangeChanged( int, int ) ),
             this, SLOT( slotLeftWidgetVerticalRangeChanged( int, int ) ) );
    connect( d->gfxview.verticalScrollBar(), SIGNAL( rangeChanged( int, int ) ),
             this, SLOT( slotGfxViewVerticalRangeChanged( int, int ) ) );

    setTabOrder( &(d->splitter), d->leftWidget );
    setTabOrder( d->leftWidget, graphicsView() );
}

/*! Sets \a ctrl to be the rowcontroller used by this View.
 * The default rowcontroller is owned by KDGantt::View and is
 * suitable for the default treeview in the left part of the view.
 * You probably only want to change this if you replace the treeview.
 */
void View::setRowController( AbstractRowController* ctrl )
{
    if ( ctrl == d->rowController ) return;
    d->rowController = ctrl;
    d->gfxview.setRowController( d->rowController );
}

/*! \returns a pointer to the current rowcontroller.
 * \see AbstractRowController
 */
AbstractRowController* View::rowController()
{
    return d->rowController;
}

/*! \overload AbstractRowController* KDGantt::View::rowController()
 */
const AbstractRowController* View::rowController() const
{
    return d->rowController;
}

/*!
 * \returns a pointer to the QAbstractItemView in the left
 * part of the widget.
 * */
const QAbstractItemView* View::leftView() const
{
    return d->leftWidget;
}

/*!
 * \overload const QAbstractItemView* KDGantt::View::leftView() const
 */
QAbstractItemView* View::leftView()
{
    return d->leftWidget;
}

/*!
 * \returns a pointer to the GraphicsView
 */
const GraphicsView* View::graphicsView() const
{
    return &d->gfxview;
}

/*!
 * \overload const GraphicsView* KDGantt::View::graphicsView() const
 */
GraphicsView* View::graphicsView()
{
    return &d->gfxview;
}

/*! \returns the current model displayed by this view
 */
QAbstractItemModel* View::model() const
{
    return leftView()->model();
}

/*! Sets the QAbstractItemModel to be displayed in this view
 * to \a model.
 *
 * \see GraphicsView::setModel
 */
void View::setModel( QAbstractItemModel* model )
{
    leftView()->setModel( model );
    d->ganttProxyModel.setSourceModel( model );
    d->gfxview.setModel( &d->ganttProxyModel );
}

/*! \returns the QItemSelectionModel used by this view
 */
QItemSelectionModel* View::selectionModel() const
{
    return leftView()->selectionModel();
}

/*! Sets the QItemSelectionModel used by this view to manage
 * selections. Similar to QAbstractItemView::setSelectionModel
 */
void View::setSelectionModel( QItemSelectionModel* smodel )
{
    leftView()->setSelectionModel( smodel );
    d->gfxview.setSelectionModel( new QItemSelectionModel( &( d->ganttProxyModel ),this ) );
}

/*! Sets the AbstractGrid for this view. The grid is an
 * object that controls how QModelIndexes are mapped
 * to and from the view and how the background and header
 * is rendered. \see AbstractGrid and DateTimeGrid.
 */
void View::setGrid( AbstractGrid* grid )
{
    d->gfxview.setGrid( grid );
}

/*! \returns the AbstractGrid used by this view.
 */
AbstractGrid* View::grid() const
{
    return d->gfxview.grid();
}

/*! \returns the rootindex for this view.
 */
QModelIndex View::rootIndex() const
{
    return leftView()->rootIndex();
}

/*! Sets the root index of the model displayed by this view.
 * Similar to QAbstractItemView::setRootIndex, default is QModelIndex().
 */
void View::setRootIndex( const QModelIndex& idx )
{
    leftView()->setRootIndex( idx );
    d->gfxview.setRootIndex( idx );
}

/*! \returns the ItemDelegate used by this view to render items
*/
ItemDelegate* View::itemDelegate() const
{
    return d->gfxview.itemDelegate();
}

/*! Sets the KDGantt::ItemDelegate used for rendering items on this
 * view. \see ItemDelegate and QAbstractItemDelegate.
 */
void View::setItemDelegate( ItemDelegate* delegate )
{
    leftView()->setItemDelegate( delegate );
    d->gfxview.setItemDelegate( delegate );
}

/*! Sets the constraintmodel displayed by this view.
 * \see KDGantt::ConstraintModel.
 */
void View::setConstraintModel( ConstraintModel* cm )
{
    d->constraintProxy.setSourceModel( cm );
    d->gfxview.setConstraintModel( &d->mappedConstraintModel );
}

/*! \returns the KDGantt::ConstraintModel displayed by this view.
 */
ConstraintModel* View::constraintModel() const
{
    return d->constraintProxy.sourceModel();
}

const QAbstractProxyModel* View::ganttProxyModel() const
{
    return &( d->ganttProxyModel );
}

QAbstractProxyModel* View::ganttProxyModel()
{
    return &( d->ganttProxyModel );
}

void View::resizeEvent(QResizeEvent*ev)
{
    QWidget::resizeEvent(ev);
}

/*!\returns The QModelIndex for the item located at
 * position \a pos in the view or an invalid index
 * if no item was present at that position.
 *
 * \see GraphicsView::indexAt
 */
QModelIndex View::indexAt( const QPoint& pos ) const
{
    return d->gfxview.indexAt( pos );
}

/*! Render the GanttView inside the rectangle \a target using the painter \a painter.
 * \see KDGantt::GraphicsView::print(QPainter* painter, const QRectF& target,bool drawRowLabels)
 */
void View::print( QPainter* painter, const QRectF& target, const QRectF& source, bool drawRowLabels, bool drawHeader)
{
    
    QRectF targetRect = target;
    if( targetRect.isNull() ) {
        targetRect.setRect(0, 0, painter->device()->width(), painter->device()->height());
    }
    d->gfxview.print( painter, targetRect, source, drawRowLabels, drawHeader);
}

/*! Return the bounding rect needed to paint the GanttView.
 */
QRectF View::printRect(bool drawRowLabels, bool drawHeader)
{
    return d->gfxview.printRect( drawRowLabels, drawHeader );
}

//#include "moc_kdganttview.cpp"
#include "kdganttview.moc"

#ifndef KDAB_NO_UNIT_TESTS
#include "unittest/test.h"

#include "kdganttlistviewrowcontroller.h"
#include <QApplication>
#include <QTimer>
#include <QPixmap>
#include <QListView>

namespace {
    std::ostream& operator<<( std::ostream& os, const QImage& img )
    {
        os << "QImage[ size=("<<img.width()<<", "<<img.height()<<")]";
        return os;
    }
}

KDAB_SCOPED_UNITTEST_SIMPLE( KDGantt, View, "test" ) {
    View view( 0 );
#if 0 // GUI tests do not work well on the server
    QTimer::singleShot( 1000, qApp, SLOT( quit() ) );
    view.show();

    qApp->exec();
    QPixmap screenshot1 = QPixmap::grabWidget( &view );

    QTreeView* tv = new QTreeView;
    view.setLeftView( tv );
    view.setRowController( new TreeViewRowController(tv,view.ganttProxyModel()) );

    QTimer::singleShot( 1000, qApp, SLOT( quit() ) );

    qApp->exec();
    QPixmap screenshot2 = QPixmap::grabWidget( &view );

    assertEqual( screenshot1.toImage(),  screenshot2.toImage() );

    QListView* lv = new QListView;
    view.setLeftView(lv);
    view.setRowController( new ListViewRowController(lv,view.ganttProxyModel()));
    view.show();
    QTimer::singleShot( 1000, qApp, SLOT( quit() ) );
    qApp->exec();
#endif
}
#endif /* KDAB_NO_UNIT_TESTS */

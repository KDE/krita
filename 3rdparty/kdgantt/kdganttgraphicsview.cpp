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
#include "kdganttgraphicsview.h"
#include "kdganttgraphicsview_p.h"
#include "kdganttabstractrowcontroller.h"
#include "kdganttgraphicsitem.h"
#include "kdganttconstraintmodel.h"
#include "kdgantttimescalezoomdialog.h"

#include <QMenu>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QHelpEvent>
#include <QScrollBar>
#include <QAbstractProxyModel>
#include <QAbstractItemView>
#include <QToolButton>
#include <qmath.h>

#include <cassert>

#if defined KDAB_EVAL
#include "../evaldialog/evaldialog.h"
#endif

using namespace KDGantt;

/*!\class KDGantt::HeaderWidget
 * \internal
 */

HeaderWidget::HeaderWidget( GraphicsView* parent )
    : QWidget( parent ), m_offset( 0. )
{
    assert( parent ); // Parent must be set

    m_zoomwidget = new Slider( this );
    m_zoomwidget->setEnableHideOnLeave( true );
    m_zoomwidget->hide();
    m_zoomwidget->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    m_zoomwidget->setGeometry( 0, 0, 200, m_zoomwidget->minimumSizeHint().height()  );

    setMouseTracking( true );
}

HeaderWidget::~HeaderWidget()
{
}

void HeaderWidget::scrollTo( int v )
{
    m_offset = v;
    // QWidget::scroll() wont work properly for me on Mac
    //scroll( static_cast<int>( old-v ), 0 );
    update();
}

void HeaderWidget::paintEvent( QPaintEvent* ev )
{
    QPainter p( this );
    view()->grid()->paintHeader( &p, rect(), ev->rect(), m_offset, this );
}

void HeaderWidget::render( QPainter* painter, const QRectF &targetRect, const QRectF &sourceRect, Qt::AspectRatioMode aspectRatioMode)
{
    view()->grid()->render( painter, targetRect, rect(), sourceRect, this, aspectRatioMode );
}

bool HeaderWidget::event( QEvent* event )
{
    if ( event->type() == QEvent::ToolTip ) {
        DateTimeGrid* const grid = qobject_cast< DateTimeGrid* >( view()->grid() );
        if ( grid ) {
            QHelpEvent *e = static_cast<QHelpEvent*>( event );
            QDateTime dt = grid->mapFromChart( view()->mapToScene( QPoint( e->x(), 0 ) ).x() ).toDateTime();
            setToolTip( dt.toString() );
        }
    }
    return QWidget::event( event );
}

void HeaderWidget::contextMenuEvent( QContextMenuEvent* event )
{
    if ( m_zoomwidget && m_zoomwidget->isVisible() ) {
        event->ignore();
        return;
    }
    QMenu contextMenu;

    DateTimeGrid* const grid = qobject_cast< DateTimeGrid* >( view()->grid() );
    QAction* actionScaleAuto = 0;
    QAction* actionScaleYear = 0;
    QAction* actionScaleMonth = 0;
    QAction* actionScaleWeek = 0;
    QAction* actionScaleDay = 0;
    QAction* actionScaleHour = 0;
    QAction* actionZoom = 0;
    QAction* actionZoomIn = 0;
    QAction* actionZoomOut = 0;
    if( grid != 0 )
    {
        QMenu* menuScale = new QMenu( tr( "Scale" ), &contextMenu );
        QActionGroup* scaleGroup = new QActionGroup( &contextMenu );
        scaleGroup->setExclusive( true );

        actionScaleAuto = new QAction( tr( "Auto" ), menuScale );
        actionScaleAuto->setCheckable( true );
        actionScaleAuto->setChecked( grid->scale() == DateTimeGrid::ScaleAuto );
        actionScaleYear = new QAction( tr( "Year" ), menuScale );
        actionScaleYear->setCheckable( true );
        actionScaleYear->setChecked( grid->scale() == DateTimeGrid::ScaleYear );
        actionScaleMonth = new QAction( tr( "Month" ), menuScale );
        actionScaleMonth->setCheckable( true );
        actionScaleMonth->setChecked( grid->scale() == DateTimeGrid::ScaleMonth );
        actionScaleWeek = new QAction( tr( "Week" ), menuScale );
        actionScaleWeek->setCheckable( true );
        actionScaleWeek->setChecked( grid->scale() == DateTimeGrid::ScaleWeek );
        actionScaleDay = new QAction( tr( "Day" ), menuScale );
        actionScaleDay->setCheckable( true );
        actionScaleDay->setChecked( grid->scale() == DateTimeGrid::ScaleDay );
        actionScaleHour = new QAction( tr( "Hour" ), menuScale );
        actionScaleHour->setCheckable( true );
        actionScaleHour->setChecked( grid->scale() == DateTimeGrid::ScaleHour );

        scaleGroup->addAction( actionScaleAuto );
        menuScale->addAction( actionScaleAuto );
    
        scaleGroup->addAction( actionScaleYear );
        menuScale->addAction( actionScaleYear );
        
        scaleGroup->addAction( actionScaleMonth );
        menuScale->addAction( actionScaleMonth );
        
        scaleGroup->addAction( actionScaleWeek );
        menuScale->addAction( actionScaleWeek );
    
        scaleGroup->addAction( actionScaleDay );
        menuScale->addAction( actionScaleDay );
    
        scaleGroup->addAction( actionScaleHour );
        menuScale->addAction( actionScaleHour );
    
        contextMenu.addMenu( menuScale );

        contextMenu.addSeparator();

        actionZoom = new QAction( tr( "Zoom..." ), &contextMenu );
        contextMenu.addAction( actionZoom );
        actionZoomIn = new QAction( tr( "Zoom In" ), &contextMenu );
        contextMenu.addAction( actionZoomIn );
        actionZoomOut = new QAction( tr( "Zoom Out" ), &contextMenu );
        contextMenu.addAction( actionZoomOut );
    }

    if( contextMenu.isEmpty() )
    {
        event->ignore();
        return;
    }

    const QAction* const action = contextMenu.exec( event->globalPos() );
    if( action == 0 ) {}
    else if( action == actionScaleAuto )
    {
        assert( grid != 0 );
        grid->setScale( DateTimeGrid::ScaleAuto );
    }
    else if( action == actionScaleYear )
    {
        assert( grid != 0 );
        grid->setScale( DateTimeGrid::ScaleYear );
        grid->setDayWidth(1.);
    }
    else if( action == actionScaleMonth )
    {
        assert( grid != 0 );
        grid->setScale( DateTimeGrid::ScaleMonth );
        grid->setDayWidth(12.);
    }
    else if( action == actionScaleWeek )
    {
        assert( grid != 0 );
        grid->setScale( DateTimeGrid::ScaleWeek );
        grid->setDayWidth(52.);
    }
    else if( action == actionScaleDay )
    {
        assert( grid != 0 );
        grid->setScale( DateTimeGrid::ScaleDay );
        grid->setDayWidth(300.);
    }
    else if( action == actionScaleHour )
    {
        assert( grid != 0 );
        grid->setScale( DateTimeGrid::ScaleHour );
        grid->setDayWidth(8000.);
    }
    else if( action == actionZoom )
    {
        assert( grid != 0 );
        TimeScaleZoomDialog dlg;
        dlg.zoom->setGrid( grid );
        dlg.exec();
    }
    else if( action == actionZoomIn )
    {
        assert( grid != 0 );
        grid->zoomIn();
    }
    else if( action == actionZoomOut )
    {
        assert( grid != 0 );
        grid->zoomOut();
    }

    event->accept();
}

void HeaderWidget::mouseMoveEvent( QMouseEvent *event )
{
    if ( event->pos().y() < height() / 2 && event->pos().x() < 200 ) {
        DateTimeGrid* const grid = qobject_cast< DateTimeGrid* >( view()->grid() );
        m_zoomwidget->setGrid( grid );
        if ( grid ) {
            m_zoomwidget->show();
            m_zoomwidget->setFocus();
        }
    }
}

GraphicsView::Private::Private( GraphicsView* _q )
  : q( _q ), rowcontroller(0), headerwidget( _q )
{
}

void GraphicsView::Private::updateHeaderGeometry()
{
    q->setViewportMargins(0,rowcontroller->headerHeight(),0,0);
    headerwidget.setGeometry( q->frameWidth(),
                              q->frameWidth(),
                              q->width()-2*q->frameWidth(),
                              rowcontroller->headerHeight() );
}

void GraphicsView::Private::slotGridChanged()
{
    updateHeaderGeometry();
    headerwidget.update();
    q->updateSceneRect();
    q->update();
}

void GraphicsView::Private::slotHorizontalScrollValueChanged( int val )
{
#if QT_VERSION >= 0x040300
    const QRectF viewRect = q->transform().mapRect( q->sceneRect() );
#else
    const QRectF viewRect = q->sceneRect();
#endif
    headerwidget.scrollTo( val-q->horizontalScrollBar()->minimum()+static_cast<int>( viewRect.left() ) );
}

void GraphicsView::Private::slotColumnsInserted( const QModelIndex& parent,  int start, int end )
{
    Q_UNUSED( start );
    Q_UNUSED( end );
    QModelIndex idx = scene.model()->index( 0, 0, scene.summaryHandlingModel()->mapToSource( parent ) );
    do {
        scene.updateRow( scene.summaryHandlingModel()->mapFromSource( idx ) );
    } while ( ( idx = rowcontroller->indexBelow( idx ) ) != QModelIndex() && rowcontroller->isRowVisible( idx ) );
        //} while ( ( idx = d->treeview.indexBelow( idx ) ) != QModelIndex() && d->treeview.visualRect(idx).isValid() );
     q->updateSceneRect();
}

void GraphicsView::Private::slotColumnsRemoved( const QModelIndex& parent,  int start, int end )
{
    // TODO
    Q_UNUSED( start );
    Q_UNUSED( end );
    Q_UNUSED( parent );
    q->updateScene();
}

void GraphicsView::Private::slotDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    //qDebug() << "GraphicsView::slotDataChanged("<<topLeft<<bottomRight<<")";
    const QModelIndex parent = topLeft.parent();
    for ( int row = topLeft.row(); row <= bottomRight.row(); ++row ) {
        scene.updateRow( scene.summaryHandlingModel()->index( row, 0, parent ) );
    }
}

void GraphicsView::Private::slotLayoutChanged()
{
    //qDebug() << "slotLayoutChanged()";
    q->updateScene();
}

void GraphicsView::Private::slotModelReset()
{
    //qDebug() << "slotModelReset()";
    q->updateScene();
}

void GraphicsView::Private::slotRowsInserted( const QModelIndex& parent,  int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );
    q->updateScene(); // TODO: This might be optimised
}

void GraphicsView::Private::slotRowsAboutToBeRemoved( const QModelIndex& parent,  int start, int end )
{
    //qDebug() << "GraphicsView::Private::slotRowsAboutToBeRemoved("<<parent<<start<<end<<")";
    for ( int row = start; row <= end; ++row ) {
        for ( int col = 0; col < scene.summaryHandlingModel()->columnCount( parent ); ++col ) {
            //qDebug() << "removing "<<scene.summaryHandlingModel()->index( row, col, parent );
            const QModelIndex idx = scene.summaryHandlingModel()->index( row, col, parent );
            QList<Constraint> clst = scene.constraintModel()->constraintsForIndex( idx );
            Q_FOREACH( Constraint c, clst ) {
                scene.constraintModel()->removeConstraint( c );
            }
            scene.removeItem( idx );
        }
    }
}

void GraphicsView::Private::slotRowsRemoved( const QModelIndex& parent,  int start, int end )
{
    //qDebug() << "GraphicsView::Private::slotRowsRemoved("<<parent<<start<<end<<")";
    // TODO
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );

    q->updateScene();
}

void GraphicsView::Private::slotItemClicked( const QModelIndex& idx )
{
    QModelIndex sidx = idx;//scene.summaryHandlingModel()->mapToSource( idx );
    emit q->clicked( sidx );
    if (q->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, 0, q))
        emit q->activated( sidx );
}

void GraphicsView::Private::slotItemDoubleClicked( const QModelIndex& idx )
{
    QModelIndex sidx = idx;//scene.summaryHandlingModel()->mapToSource( idx );
    emit q->doubleClicked( sidx );
    if (!q->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, 0, q))
        emit q->activated( sidx );
}

/*!\class KDGantt::GraphicsView kdganttgraphicsview.h KDGanttGraphicsView
 * \ingroup KDGantt
 * \brief The GraphicsView class provides a model/view implementation of a gantt chart.
 *
 *
 */

/*! \signal void GraphicsView::activated( const QModelIndex & index ) */
/*! \signal void GraphicsView::clicked( const QModelIndex & index ); */
/*! \signal void GraphicsView::doubleClicked( const QModelIndex & index ); */
/*! \signal void GraphicsView::entered( const QModelIndex & index ); */
/*! \signal void GraphicsView::pressed( const QModelIndex & index ); */

typedef QGraphicsView BASE;

/*! Constructor. Creates a new KDGantt::GraphicsView with parent
 * \a parent.
 */
GraphicsView::GraphicsView( QWidget* parent )
    : BASE( parent ), _d( new Private( this ) )
{

#if defined KDAB_EVAL
  EvalDialog::checkEvalLicense( "KD Gantt" );
#endif
    connect( horizontalScrollBar(), SIGNAL( valueChanged( int ) ),
             this, SLOT( slotHorizontalScrollValueChanged( int ) ) );
    connect( &_d->scene, SIGNAL( gridChanged() ),
             this, SLOT( slotGridChanged() ) );
    connect( &_d->scene, SIGNAL( entered( const QModelIndex& ) ),
             this, SIGNAL( entered( const QModelIndex& ) ) );
    connect( &_d->scene, SIGNAL( pressed( const QModelIndex& ) ),
             this, SIGNAL( pressed( const QModelIndex& ) ) );
    connect( &_d->scene, SIGNAL( clicked( const QModelIndex& ) ),
             this, SLOT( slotItemClicked( const QModelIndex& ) ) );
    connect( &_d->scene, SIGNAL( doubleClicked( const QModelIndex& ) ),
             this, SLOT( slotItemDoubleClicked( const QModelIndex& ) ) );
    setScene( &_d->scene );

    // HACK!
    setSummaryHandlingModel( _d->scene.summaryHandlingModel() );
}

/*! Destroys this view. */
GraphicsView::~GraphicsView()
{
    delete _d;
}

#define d d_func()

/*! Sets the model to be displayed in this view to
 * \a model. The view does not take ownership of the model.
 *
 * To make a model work well with GraphicsView it must
 * have a certain layout. Whether the model is flat or has a
 * treestrucure is not important, as long as an
 * AbstractRowController is provided that can navigate the
 * model.
 *
 * GraphicsView operates per row in the model. The data is always
 * taken from the _last_ item in the row. The ItemRoles used are
 * Qt::DisplayRole and the roles defined in KDGantt::ItemDataRole.
 */
void GraphicsView::setModel( QAbstractItemModel* model )
{
    d->scene.setModel( model );
    updateScene();
}

/*! \returns the current model displayed by this view
 */
QAbstractItemModel* GraphicsView::model() const
{
    return d->scene.model();
}

void GraphicsView::setSummaryHandlingModel( QAbstractProxyModel* proxyModel )
{
    disconnect( d->scene.summaryHandlingModel() );
    d->scene.setSummaryHandlingModel( proxyModel );

    /* Connections. We have to rely on the treeview
     * to receive the signals before we do(!)
     */
    connect( proxyModel, SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
             this,  SLOT( slotColumnsInserted( const QModelIndex&,  int, int ) ) );
    connect( proxyModel, SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
             this,  SLOT( slotColumnsRemoved( const QModelIndex&,  int, int ) ) );
    connect( proxyModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
             this,  SLOT( slotDataChanged( const QModelIndex&, const QModelIndex& ) ) );
    connect( proxyModel, SIGNAL( layoutChanged() ),
             this,  SLOT( slotLayoutChanged() ) );
    connect( proxyModel, SIGNAL( modelReset() ),
             this,  SLOT( slotModelReset() ) );
    connect( proxyModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
             this,  SLOT( slotRowsInserted( const QModelIndex&,  int, int ) ) );
    connect( proxyModel, SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
             this,  SLOT( slotRowsAboutToBeRemoved( const QModelIndex&,  int, int ) ) );
    connect( proxyModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
             this,  SLOT( slotRowsRemoved( const QModelIndex&,  int, int ) ) );

    updateScene();
}

/*! Sets the constraintmodel displayed by this view.
 * \see KDGantt::ConstraintModel.
 */
void GraphicsView::setConstraintModel( ConstraintModel* cmodel )
{
    d->scene.setConstraintModel( cmodel );
}

/*! \returns the KDGantt::ConstraintModel displayed by this view.
 */
ConstraintModel* GraphicsView::constraintModel() const
{
    return d->scene.constraintModel();
}

/*! Sets the root index of the model displayed by this view.
 * Similar to QAbstractItemView::setRootIndex, default is QModelIndex().
 */
void GraphicsView::setRootIndex( const QModelIndex& idx )
{
    d->scene.setRootIndex( idx );
}

/*! \returns the rootindex for this view.
 */
QModelIndex GraphicsView::rootIndex() const
{
    return d->scene.rootIndex();
}

/*! Sets the QItemSelectionModel used by this view to manage
 * selections. Similar to QAbstractItemView::setSelectionModel
 */
void GraphicsView::setSelectionModel( QItemSelectionModel* model )
{
    d->scene.setSelectionModel( model );
}

/*! \returns the QItemSelectionModel used by this view
 */
QItemSelectionModel* GraphicsView::selectionModel() const
{
    return d->scene.selectionModel();
}

/*! Sets the KDGantt::ItemDelegate used for rendering items on this
 * view. \see ItemDelegate and QAbstractItemDelegate.
 */
void GraphicsView::setItemDelegate( ItemDelegate* delegate )
{
    d->scene.setItemDelegate( delegate );
}

/*! \returns the ItemDelegate used by this view to render items
*/
ItemDelegate* GraphicsView::itemDelegate() const
{
    return d->scene.itemDelegate();
}

/*! Sets the AbstractRowController used by this view. The
 * AbstractRowController deals with the height and position
 * of each row and with which parts of the model are
 * displayed. \see AbstractRowController
 */
void GraphicsView::setRowController( AbstractRowController* rowcontroller )
{
    d->rowcontroller = rowcontroller;
    d->scene.setRowController( rowcontroller );
    updateScene();
}

/*! \returns the AbstractRowController
 * for this view. \see setRowController
 */
AbstractRowController* GraphicsView::rowController() const
{
    return d->rowcontroller;
}

/*! Sets the AbstractGrid for this view. The grid is an
 * object that controls how QModelIndexes are mapped
 * to and from the view and how the background and header
 * is rendered. \see AbstractGrid and DateTimeGrid.
 */
void GraphicsView::setGrid( AbstractGrid* grid )
{
    d->scene.setGrid( grid );
    d->slotGridChanged();
}

/*! \returns the AbstractGrid used by this view.
 */
AbstractGrid* GraphicsView::grid() const
{
    return d->scene.grid();
}

/*! Sets the view to read-only mode if \a to is true. The default is
 * read/write if the model permits it.
 */
void GraphicsView::setReadOnly( bool ro )
{
    d->scene.setReadOnly( ro );
}

/*!\returns true iff the view is in read-only mode
 */
bool GraphicsView::isReadOnly() const
{
    return d->scene.isReadOnly();
}

/*! Adds a constraint from \from to \a to. \a modifiers are the
 * keyboard modifiers pressed by the user when the action is invoked.
 *
 * Override this to control how contraints are added. The default
 * implementation adds a soft constraint unless the Shift key is pressed,
 * in that case it adds a hard constraint. If a constraint is already
 * present, it is removed and nothing is added.
 */
void GraphicsView::addConstraint( const QModelIndex& from,
                                  const QModelIndex& to,
                                  Qt::KeyboardModifiers modifiers )
{
    if ( isReadOnly() ) return;
    ConstraintModel* cmodel = constraintModel();
    assert( cmodel );
    Constraint c( from, to, ( modifiers&Qt::ShiftModifier )?Constraint::TypeHard:Constraint::TypeSoft );
    if ( cmodel->hasConstraint( c ) ) cmodel->removeConstraint( c );
    else cmodel->addConstraint( c );
}

void GraphicsView::resizeEvent( QResizeEvent* ev )
{
    d->updateHeaderGeometry();
    QRectF r = scene()->itemsBoundingRect();
    // To scroll more to the left than the actual item start, bug #4516
    r.setLeft( qMin( qreal(0.0), r.left() ) );
    // TODO: take scrollbars into account (if not always on)
    // The scene should be at least the size of the viewport
    // NOTE: scrollbar calculus uses maximumViewportSize() - frameWidth()*2 (not the same as viewport()->size())
    QSizeF size = maximumViewportSize() - QSizeF( frameWidth()*2, frameWidth()*2 );
    if ( size.width() > r.width() ) {
        r.setWidth( size.width() );
    }
    if ( size.height() > r.height() ) {
        r.setHeight( size.height() );
    }
    scene()->setSceneRect( r );
    BASE::resizeEvent( ev );
}

/*!\returns The QModelIndex for the item located at
 * position \a pos in the view or an invalid index
 * if no item was present at that position.
 *
 * This is useful for for example contextmenus.
 */
QModelIndex GraphicsView::indexAt( const QPoint& pos ) const
{
    QGraphicsItem* item = itemAt( pos );
    if ( GraphicsItem* gitem = qgraphicsitem_cast<GraphicsItem*>( item ) ) {
        return d->scene.summaryHandlingModel()->mapToSource( gitem->index() );
    } else {
        return QModelIndex();
    }
}

/*! \internal */
void GraphicsView::clearItems()
{
    d->scene.clearItems();
}

/*! \internal */
void GraphicsView::updateRow( const QModelIndex& idx )
{
    d->scene.updateRow( d->scene.summaryHandlingModel()->mapFromSource( idx ) );
}

/*! \internal
 * Adjusts the bounding rectangle of the scene.
 */
void GraphicsView::updateSceneRect()
{
    /* What to do with this? We need to shrink the view to
     * make collapsing items work
     */
    QRectF r = d->scene.itemsBoundingRect();
    // To scroll more to the left than the actual item start, bug #4516
    r.setLeft( qMin( qreal(0.0), r.left() ) );
    r.setSize( r.size().expandedTo( maximumViewportSize() - QSizeF( frameWidth()*2, frameWidth()*2 ) ) );

    d->scene.setSceneRect( r );
}

/*! \internal
 * Resets the state of the view.
 */
void GraphicsView::updateScene()
{
    clearItems();
    if( !model()) return;
    if( !rowController()) return;
    QModelIndex idx = model()->index( 0, 0, rootIndex() );
    do {
        updateRow( idx );
    } while ( ( idx = rowController()->indexBelow( idx ) ) != QModelIndex() && rowController()->isRowVisible(idx) );
    //constraintModel()->cleanup();
    //qDebug() << constraintModel();
    updateSceneRect();
}

/*! \internal */
GraphicsItem* GraphicsView::createItem( ItemType type ) const
{
    return d->scene.createItem( type );
}

/*! \internal */
void GraphicsView::deleteSubtree( const QModelIndex& idx )
{
    d->scene.deleteSubtree( d->scene.summaryHandlingModel()->mapFromSource( idx ) );
}

/*! Render the GanttView inside the rectangle \a target using the painter \a painter.
 * This is useful for printing. If \a target is a null rect, the dimensions of
 * painter's paint device will be used.
 * If \a drawRowLabels is true, an additional per-row label
 * is drawn, mimicing the look of a listview.
 */
void GraphicsView::print( QPainter* painter, const QRectF& target, const QRectF& source, bool drawRowLabels, bool drawHeader )
{
    d->scene.print( painter, target, source, drawRowLabels, ( drawHeader ? this : 0 ) );
}
QRectF GraphicsView::printRect(bool drawRowLabels, bool drawHeader )
{
    return d->scene.printRect( drawRowLabels,(  drawHeader ? this : 0 ) );
}

void GraphicsView::renderHeader( QPainter* painter, const QRectF& target, const QRectF& source, Qt::AspectRatioMode aspectRatioMode )
{
    d->headerwidget.render(painter,target,source,aspectRatioMode);
}

qreal GraphicsView::headerHeight() const
{
    return d->headerwidget.rect().height();
}

#include "moc_kdganttgraphicsview.cpp"
#include "moc_kdganttgraphicsview_p.cpp"

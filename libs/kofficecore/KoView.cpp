/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include <KoView.h>
#include <KoDocument.h>
#include <KoMainWindow.h>
#include <KoFrame.h>
#include <KoViewIface.h>
#include <KoDocumentChild.h>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kparts/partmanager.h>
#include <kparts/event.h>
#include <kcursor.h>
#include <assert.h>
#include <kstatusbar.h>
#include <kapplication.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3PtrList>
#include <QEvent>
#include <Q3ValueList>
#include <QMouseEvent>
#include <QCustomEvent>

class KoViewPrivate
{
public:
  KoViewPrivate()
  {
    m_inOperation = false;
    m_zoom = 1.0;
    m_children.setAutoDelete( true );
    m_manager = 0L;
    m_tempActiveWidget = 0L;
    m_dcopObject = 0;
    m_registered=false;
    m_documentDeleted=false;
  }
  ~KoViewPrivate()
  {
  }

  QPointer<KoDocument> m_doc; // our KoDocument
  QPointer<KParts::PartManager> m_manager;
  double m_zoom;
  Q3PtrList<KoViewChild> m_children;
  QWidget *m_tempActiveWidget;
  KoViewIface *m_dcopObject;
  bool m_registered;  // are we registered at the part manager?
  bool m_documentDeleted; // true when m_doc gets deleted [can't use m_doc==0
                          // since this only happens in ~QObject, and views
                          // get deleted by ~KoDocument].
  QTimer *m_scrollTimer;

  // Hmm sorry for polluting the private class with such a big inner class.
  // At the beginning it was a little struct :)
  class StatusBarItem {
  public:
      StatusBarItem() // for QValueList
          : m_widget(0), m_visible(false)
      {}
      StatusBarItem( QWidget * widget, int stretch, bool permanent )
          : m_widget(widget), m_stretch(stretch), m_permanent(permanent), m_visible(false)
      {}

      QWidget * widget() const { return m_widget; }

      void ensureItemShown( KStatusBar * sb )
      {
            if ( !m_visible )
            {
                sb->addWidget( m_widget, m_stretch, m_permanent );
                m_visible = true;
                m_widget->show();
            }
      }
      void ensureItemHidden( KStatusBar * sb )
      {
            if ( m_visible )
            {
                sb->removeWidget( m_widget );
                m_visible = false;
                m_widget->hide();
            }
      }
  private:
      QWidget * m_widget;
      int m_stretch;
      bool m_permanent;
      bool m_visible;  // true when the item has been added to the statusbar
  };
  Q3ValueList<StatusBarItem> m_statusBarItems; // Our statusbar items
  bool m_inOperation; //in the middle of an operation (no screen refreshing)?
};

KoView::KoView( KoDocument *document, QWidget *parent, const char *name )
 : QWidget( parent, name )
{
  Q_ASSERT( document );

  //kDebug(30003) << "KoView::KoView " << this << endl;
  d = new KoViewPrivate;
  d->m_doc = document;
  KParts::PartBase::setPartObject( this );

  setFocusPolicy( Qt::StrongFocus );

  setMouseTracking( true );

  connect( d->m_doc, SIGNAL( childChanged( KoDocumentChild * ) ),
           this, SLOT( slotChildChanged( KoDocumentChild * ) ) );

  connect( d->m_doc, SIGNAL( sigBeginOperation() ),
           this, SLOT( beginOperation() ) );

  connect( d->m_doc, SIGNAL( sigEndOperation() ),
           this, SLOT( endOperation() ) );


  actionCollection()->setAssociatedWidget(this);
  setupGlobalActions();
  KActionCollection *coll = actionCollection();
  /**** not needed anymore, according to David (Werner)
  QValueList<KAction*> docActions = document->actionCollection()->actions();
  QValueList<KAction*>::ConstIterator it = docActions.begin();
  QValueList<KAction*>::ConstIterator end = docActions.end();
  for (; it != end; ++it )
      coll->insert( *it );
  */

  KStatusBar * sb = statusBar();
  if ( sb ) // No statusbar in e.g. konqueror
  {
      //coll->setHighlightingEnabled( true );
      connect( coll, SIGNAL( actionStatusText( const QString & ) ),
               this, SLOT( slotActionStatusText( const QString & ) ) );
      connect( coll, SIGNAL( clearStatusText() ),
               this, SLOT( slotClearStatusText() ) );

      connect( d->m_doc, SIGNAL( sigStatusBarMessage( const QString& ) ),
               this, SLOT( slotActionStatusText( const QString& ) ) );
      connect( d->m_doc, SIGNAL( sigClearStatusBarMessage() ),
               this, SLOT( slotClearStatusText() ) );
  }
  d->m_doc->setCurrent();

  d->m_scrollTimer = new QTimer( this );
  connect (d->m_scrollTimer, SIGNAL( timeout() ), this, SLOT( slotAutoScroll() ) );
}

KoView::~KoView()
{
  kDebug(30003) << "KoView::~KoView " << this << endl;
  delete d->m_scrollTimer;
  delete d->m_dcopObject;
  if (!d->m_documentDeleted)
  {
    if ( koDocument() && !koDocument()->isSingleViewMode() )
    {
      if ( d->m_manager && d->m_registered ) // if we aren't registered we mustn't unregister :)
        d->m_manager->removePart( koDocument() );
      d->m_doc->removeView(this);
      d->m_doc->setCurrent( false );
    }
  }
  delete d;
}

KoDocument *KoView::koDocument() const
{
  return d->m_doc;
}

void KoView::setDocumentDeleted()
{
    d->m_documentDeleted = true;
}

bool KoView::documentDeleted() const
{
    return d->m_documentDeleted;
}

bool KoView::hasDocumentInWindow( KoDocument *doc )
{
  return child( doc ) != 0L;
}

void KoView::setPartManager( KParts::PartManager *manager )
{
  d->m_manager = manager;
  if ( !koDocument()->isSingleViewMode() &&
       !manager->parts().contains( koDocument() ) ) // is there another view registered?
  {
    d->m_registered = true; // no, so we have to register now and ungregister again in the DTOR
    manager->addPart( koDocument(), false );
  }
  else
    d->m_registered = false;  // There is already another view registered for that part...
}

KParts::PartManager *KoView::partManager() const
{
  return d->m_manager;
}

KAction *KoView::action( const QDomElement &element ) const
{
  static const QString &attrName = KGlobal::staticQString( "name" );
  QString name = element.attribute( attrName );

  KAction *act = KXMLGUIClient::action( name.utf8() );

  if ( !act )
    act = d->m_doc->KXMLGUIClient::action( name.utf8() );

  return act;
}

KoDocument *KoView::hitTest( const QPoint &viewPos )
{
  KoViewChild *viewChild;

  QPoint pos = reverseViewTransformations( viewPos );

  KoDocumentChild *docChild = selectedChild();
  if ( docChild )
  {
    if ( ( viewChild = child( docChild->document() ) ) )
    {
      if ( viewChild->frameRegion().contains( pos ) )
        return 0;
    }
    else
      if ( docChild->frameRegion().contains( pos ) )
        return 0;
  }

  docChild = activeChild();
  if ( docChild )
  {
    if ( ( viewChild = child( docChild->document() ) ) )
    {
      if ( viewChild->frameRegion().contains( pos ) )
        return 0;
    }
    else
      if ( docChild->frameRegion().contains( pos ) )
        return 0;
  }

  return koDocument()->hitTest( pos, this );
}

int KoView::leftBorder() const
{
  return 0;
}

int KoView::rightBorder() const
{
  return 0;
}

int KoView::topBorder() const
{
  return 0;
}

int KoView::bottomBorder() const
{
  return 0;
}

void KoView::setZoom( double zoom )
{
  d->m_zoom = zoom;
  update();
}

double KoView::zoom() const
{
  return d->m_zoom;
}

QWidget *KoView::canvas() const
{
    //dfaure: since the view plays two roles in this method (the const means "you can modify the canvas
    // but not the view", it's just coincidence that the view is the canvas by default ;)
   return const_cast<KoView *>(this);
}

int KoView::canvasXOffset() const
{
  return 0;
}

int KoView::canvasYOffset() const
{
  return 0;
}

void KoView::canvasAddChild( KoViewChild * )
{
}

void KoView::customEvent( QCustomEvent *ev )
{
  if ( KParts::PartActivateEvent::test( ev ) )
    partActivateEvent( (KParts::PartActivateEvent *)ev );
  else if ( KParts::PartSelectEvent::test( ev ) )
    partSelectEvent( (KParts::PartSelectEvent *)ev );
  else if( KParts::GUIActivateEvent::test( ev ) )
    guiActivateEvent( (KParts::GUIActivateEvent*)ev );
}

void KoView::partActivateEvent( KParts::PartActivateEvent *event )
{
  if ( event->part() != (KParts::Part *)koDocument() )
  {
    assert( event->part()->inherits( "KoDocument" ) );

    KoDocumentChild *child = koDocument()->child( (KoDocument *)event->part() );
    if ( child && event->activated() )
    {
      if ( child->isRectangle() && !child->isTransparent() )
      {
        KoViewChild *viewChild = new KoViewChild( child, this );
        d->m_children.append( viewChild );

        QApplication::setOverrideCursor(Qt::WaitCursor);
        // This is the long operation (due to toolbar layout stuff)
        d->m_manager->setActivePart( child->document(), viewChild->frame()->view() );
        QApplication::restoreOverrideCursor();

        // Now we can move the frame to the right place
        viewChild->setInitialFrameGeometry();
      }
      else
      {
        emit regionInvalidated( child->frameRegion( matrix() ), true );
      }
      emit childActivated( child );
    }
    else if ( child )
    {
      emit regionInvalidated( child->frameRegion( matrix() ), true );
      emit childDeactivated( child );
    }
    else
      emit invalidated();
  }
  else
    emit activated( event->activated() );
}

void KoView::partSelectEvent( KParts::PartSelectEvent *event )
{
  if ( event->part() != (KParts::Part *)koDocument() )
  {
    assert( event->part()->inherits( "KoDocument" ) );

    KoDocumentChild *child = koDocument()->child( (KoDocument *)event->part() );

    if ( child && event->selected() )
    {
      QRegion r = child->frameRegion( matrix() );
      r.translate( - canvasXOffset(), - canvasYOffset() );
      emit regionInvalidated( r, true );
      emit childSelected( child );
    }
    else if ( child )
    {
      QRegion r = child->frameRegion( matrix() );
      r.translate( - canvasXOffset(), - canvasYOffset() );
      emit regionInvalidated( r, true );
      emit childUnselected( child );
    }
    else
      emit invalidated();
  }
  else
    emit selected( event->selected() );
}

void KoView::guiActivateEvent( KParts::GUIActivateEvent * ev )
{
    showAllStatusBarItems( ev->activated() );
}

void KoView::showAllStatusBarItems( bool show )
{
    KStatusBar * sb = statusBar();
    if ( !sb )
        return;
    Q3ValueListIterator<KoViewPrivate::StatusBarItem> it = d->m_statusBarItems.begin();
    for ( ; it != d->m_statusBarItems.end() ; ++it )
        if ( show )
            (*it).ensureItemShown( sb );
        else
            (*it).ensureItemHidden( sb );
}

void KoView::addStatusBarItem( QWidget * widget, int stretch, bool permanent )
{
    KoViewPrivate::StatusBarItem item( widget, stretch, permanent );
    d->m_statusBarItems.append(item);
    Q3ValueListIterator<KoViewPrivate::StatusBarItem> it = d->m_statusBarItems.fromLast();
    KStatusBar * sb = statusBar();
    Q_ASSERT(sb);
    if (sb)
        (*it).ensureItemShown( sb );
}

void KoView::removeStatusBarItem( QWidget * widget )
{
    KStatusBar * sb = statusBar();
    Q3ValueListIterator<KoViewPrivate::StatusBarItem> it = d->m_statusBarItems.begin();
    for ( ; it != d->m_statusBarItems.end() ; ++it )
        if ( (*it).widget() == widget )
        {
            if ( sb )
                (*it).ensureItemHidden( sb );
            d->m_statusBarItems.remove( it );
            break;
        }
    if ( it == d->m_statusBarItems.end() )
        kWarning() << "KoView::removeStatusBarItem. Widget not found : " << widget << endl;
}

KoDocumentChild *KoView::selectedChild()
{
  if ( !d->m_manager )
    return 0L;

  KParts::Part *selectedPart = d->m_manager->selectedPart();

  if ( !selectedPart || !selectedPart->inherits( "KoDocument" ) )
    return 0L;

  return koDocument()->child( (KoDocument *)selectedPart );
}

KoDocumentChild *KoView::activeChild()
{
  if ( !d->m_manager )
    return 0L;

  KParts::Part *activePart = d->m_manager->activePart();

  if ( !activePart || !activePart->inherits( "KoDocument" ) )
    return 0L;

  return koDocument()->child( (KoDocument *)activePart );
}

void KoView::enableAutoScroll( )
{
    d->m_scrollTimer->start( 50 );
}

void KoView::disableAutoScroll( )
{
    d->m_scrollTimer->stop();
}

void KoView::paintEverything( QPainter &painter, const QRect &rect, bool transparent )
{
  koDocument()->paintEverything( painter, rect, transparent, this );
}

KoViewChild *KoView::child( KoView *view )
{
  Q3PtrListIterator<KoViewChild> it( d->m_children );
  for (; it.current(); ++it )
    if ( it.current()->frame()->view() == view )
      return it.current();

  return 0L;
}

KoViewChild *KoView::child( KoDocument *doc )
{
  Q3PtrListIterator<KoViewChild> it( d->m_children );
  for (; it.current(); ++it )
    if ( it.current()->documentChild()->document() == doc )
      return it.current();

  return 0L;
}

QMatrix KoView::matrix() const
{
  QMatrix m;
  m.scale( zoom(), zoom() );
  //m.translate(  canvasXOffset() ,  canvasYOffset() );
  return m;
}

void KoView::slotChildActivated( bool a )
{
  // Only interested in deactivate events
  if ( a )
    return;

  KoViewChild* ch = child( (KoView*)sender() );
  if ( !ch )
    return;

  KoView* view = ch->frame()->view();

  QWidget *activeWidget = view->d->m_tempActiveWidget;

  if ( d->m_manager->activeWidget() )
    activeWidget = d->m_manager->activeWidget();

  if ( !activeWidget || !activeWidget->inherits( "KoView" ) )
    return;

  // Is the new active view a child of this one ?
  // In this case we may not delete!
  //  QObject *n = d->m_manager->activeWidget();
  QObject *n = activeWidget;
  while( n )
    if ( n == (QObject *)view )
      return;
    else
     n = n->parent();


  d->m_tempActiveWidget = activeWidget;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  d->m_manager->setActivePart( 0L );

  QPointer<KoDocumentChild> docChild = ch->documentChild();
  QPointer<KoFrame> chFrame = ch->frame();
  if ( docChild && chFrame && chFrame->view() )
  {
    docChild->setContentsPos( chFrame->view()->canvasXOffset(),
                              chFrame->view()->canvasYOffset() );
    docChild->document()->setViewBuildDocument( chFrame->view(), chFrame->view()->xmlguiBuildDocument() );
  }

  d->m_children.remove( ch );

  d->m_manager->addPart( docChild->document(), false ); // the destruction of the view removed the part from the partmanager. re-add it :)

  QApplication::restoreOverrideCursor();

  // #### HACK
  // We want to delete as many views as possible and this
  // trick is used to go upwards in the view-tree.
  emit activated( FALSE );
}

void KoView::slotChildChanged( KoDocumentChild *child )
{
  QRegion region( child->oldPointArray( matrix() ) );
  emit regionInvalidated( child->frameRegion( matrix(), true ).unite( region ), true );
}

int KoView::autoScrollAcceleration( int offset ) const
{
    if(offset < 40)
        return offset;
    else
        return offset*offset/40;
}

void KoView::slotAutoScroll(  )
{
    QPoint scrollDistance;
    bool actuallyDoScroll = false;
    QPoint pos( mapFromGlobal( QCursor::pos() ) );

    //Provide progressive scrolling depending on the mouse position
    if ( pos.y() < topBorder() )
    {
        scrollDistance.setY ((int) - autoScrollAcceleration( - pos.y() + topBorder() ));
        actuallyDoScroll = true;
    }
    else if ( pos.y() > height() - bottomBorder() )
    {
        scrollDistance.setY ((int) autoScrollAcceleration(pos.y() - height() + bottomBorder() ));
        actuallyDoScroll = true;
    }

    if ( pos.x() < leftBorder() )
    {
        scrollDistance.setX ((int) - autoScrollAcceleration( - pos.x() + leftBorder() ));
        actuallyDoScroll = true;
    }
    else if ( pos.x() > width() - rightBorder() )
    {
        scrollDistance.setX ((int) autoScrollAcceleration( pos.x() - width() + rightBorder() ));
        actuallyDoScroll = true;
    }

    if ( actuallyDoScroll )
    {
        int state=0;
        state = QApplication::keyboardModifiers();

        pos = canvas()->mapFrom(this, pos);
        QMouseEvent * event = new QMouseEvent(QEvent::MouseMove, pos, 0, state);

        QApplication::postEvent( canvas(), event );
        emit autoScroll( scrollDistance );
    }
}


void KoView::setupGlobalActions() {
    actionNewView = new KAction( i18n( "&New View" ), "window_new", 0,
        this, SLOT( newView() ),
        actionCollection(), "view_newview" );
}

void KoView::setupPrinter( KPrinter & )
{
    kDebug() << "KoView::setupPrinter not implemented by the application!" << endl;
}

void KoView::print( KPrinter & )
{
    kDebug() << "KoView::print not implemented by the application!" << endl;
}

void KoView::newView() {
    assert( ( d!=0L && d->m_doc ) );

    KoDocument *thisDocument = d->m_doc;
    KoMainWindow *shell = new KoMainWindow( thisDocument->instance() );
    shell->setRootDocument(thisDocument);
    shell->show();
}

bool KoView::isInOperation() const
{
   return d->m_inOperation;
}

void KoView::beginOperation()
{
   d->m_inOperation = true;
   canvas()->setUpdatesEnabled(FALSE);
}

void KoView::endOperation()
{
   canvas()->setUpdatesEnabled(TRUE);
   d->m_inOperation = false;

//   canvas()->update();
}

KoMainWindow * KoView::shell() const
{
    return dynamic_cast<KoMainWindow *>( topLevelWidget() );
}

KMainWindow * KoView::mainWindow() const
{
    return dynamic_cast<KMainWindow *>( topLevelWidget() );
}

KStatusBar * KoView::statusBar() const
{
    KoMainWindow *mw = shell();
    return mw ? mw->statusBar() : 0L;
}

void KoView::slotActionStatusText( const QString &text )
{
  KStatusBar *sb = statusBar();
  if ( sb )
      sb->message( text );
}

void KoView::slotClearStatusText()
{
  KStatusBar *sb = statusBar();
  if ( sb )
      sb->clear();
}

DCOPObject *KoView::dcopObject()
{
    if ( !d->m_dcopObject )
        d->m_dcopObject = new KoViewIface( this );
    return d->m_dcopObject;
}

class KoViewChild::KoViewChildPrivate
{
public:
  KoViewChildPrivate()
  {
  }
  ~KoViewChildPrivate()
  {
  }
};

KoViewChild::KoViewChild( KoDocumentChild *child, KoView *_parentView )
{
  d = new KoViewChildPrivate;
  m_parentView = _parentView;
  m_child = child;

  m_frame = new KoFrame( parentView()->canvas() );
  KoView *view = child->document()->createView( m_frame );
  view->setXMLGUIBuildDocument( child->document()->viewBuildDocument( view ) );

  view->setPartManager( parentView()->partManager() );

  // hack? (Werner)
  view->setZoom( parentView()->zoom() * qMax(child->xScaling(), child->yScaling()) );

  m_frame->setView( view );
  m_frame->show();
  m_frame->raise();

  parentView()->canvasAddChild( this );


  /*
   KoViewChild has basically three geometries to keep in sync.
   - The KoDocumentChild geometry (i.e. the embedded object's geometry, unzoomed)
   - Its own geometry (used for hit-test etc.)
   - The KoFrame geometry (the graphical widget for moving the object when active)

   So we need to subtract the scrollview's offset for the frame geometry, since it's a widget.

   The rules are
   (R1) frameGeometry = viewGeometry(childGeometry) "+" m_frame->{left|right|top|bottom}Border() - scrollview offset,
   (R2) frameGeometry = myGeometry "+" active_frame_border - scrollview offset.

   So: (R3, unused) myGeometry = viewGeometry(childGeometry) "+" m_frame->{left|right|top|bottom}Border() "-" active_frame_border

   Notes: active_frame_border is m_frame->border() (0 when inactive, 5 when active).
          {left|right|top|bottom}Border are the borders used in kspread (0 when inactive, big when active).
          "+" border means we add a border, so it's a subtraction on x, y and an addition on width, height.

          viewGeometry() applies the zoom as well as any other translation the app might want to do
   */

  // Setting the frameGeometry is done in setInitialFrameGeometry, which is
  // also called right after activation.

  connect( view, SIGNAL( activated( bool ) ),
           parentView(), SLOT( slotChildActivated( bool ) ) );
}

KoViewChild::~KoViewChild()
{
  if ( m_frame )
  {
    slotFrameGeometryChanged();
    delete static_cast<KoFrame *>( m_frame );
  }
  delete d;
}

void KoViewChild::slotFrameGeometryChanged()
{
  // Set our geometry from the frame geometry (R2 reversed)
  QRect geom = m_frame->geometry();
  int b = m_frame->border();
  QRect borderRect( geom.x() + b + parentView()->canvasXOffset(),
                    geom.y() + b + parentView()->canvasYOffset(),
                    geom.width() - b * 2,
                    geom.height() - b * 2 );
  setGeometry( borderRect );

  if(m_child)
  {
    // Set the child geometry from the frame geometry (R1 reversed)
    QRect borderLessRect( geom.x() + m_frame->leftBorder() + parentView()->canvasXOffset(),
                          geom.y() + m_frame->topBorder() + parentView()->canvasYOffset(),
                          geom.width() - m_frame->leftBorder() - m_frame->rightBorder(),
                          geom.height() - m_frame->topBorder() - m_frame->bottomBorder() );

    // We don't want to trigger slotDocGeometryChanged again
    lock();
    QRect childGeom = parentView()->reverseViewTransformations( borderLessRect );
    kDebug() << "KoChild::slotFrameGeometryChanged child geometry "
              << ( geometry() == childGeom ? "already " : "set to " )
              << childGeom << endl;
    m_child->setGeometry( childGeom );
    unlock();
  }
}

void KoViewChild::slotDocGeometryChanged()
{
  if ( locked() )
      return;
  // Set frame geometry from child geometry (R1)
  // The frame's resizeEvent will call slotFrameGeometryChanged.
  QRect geom = parentView()->applyViewTransformations( m_child->geometry() );
  QRect borderRect( geom.x() - m_frame->leftBorder() - parentView()->canvasXOffset(),
                    geom.y() - m_frame->topBorder() - parentView()->canvasYOffset(),
                    geom.width() + m_frame->leftBorder() + m_frame->rightBorder(),
                    geom.height() + m_frame->topBorder() + m_frame->bottomBorder() );
  kDebug() << "KoViewChild::slotDocGeometryChanged frame geometry "
            << ( m_frame->geometry() == borderRect ? "already " : "set to " )
            << borderRect << endl;

  m_frame->setGeometry( borderRect );
}

QPoint KoView::applyViewTransformations( const QPoint& p ) const
{
  return QPoint( qRound( p.x() * zoom() ), qRound( p.y() * zoom() ) );
}

QPoint KoView::reverseViewTransformations( const QPoint& v ) const
{
  return QPoint( qRound( v.x() / zoom() ), qRound( v.y() / zoom() ) );
}

QRect KoView::applyViewTransformations( const QRect& r ) const
{
  return QRect( applyViewTransformations( r.topLeft() ),
                applyViewTransformations( r.bottomRight() ) );
}

QRect KoView::reverseViewTransformations( const QRect& r ) const
{
  return QRect( reverseViewTransformations( r.topLeft() ),
                reverseViewTransformations( r.bottomRight() ) );
}

void KoViewChild::setInitialFrameGeometry()
{
    kDebug() << k_funcinfo << endl;

    // Connect only now, so that the GUI building doesn't move us around.
    connect( m_frame, SIGNAL( geometryChanged() ),
             this, SLOT( slotFrameGeometryChanged() ) );
    connect( m_child, SIGNAL( changed( KoChild * ) ),
             this, SLOT( slotDocGeometryChanged() ) );

    // Set frameGeometry from childGeometry
    slotDocGeometryChanged();
    // Set myGeometry from frameGeometry
    slotFrameGeometryChanged();

}

#include "KoView.moc"


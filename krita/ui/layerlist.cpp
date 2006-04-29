/*
  Copyright (c) 2005 Gábor Lehel <illissius@gmail.com>

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
  Boston, MA 02110-1301, USA.
*/

#include "layerlist.h"

#include <qtooltip.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <QEvent>
#include <QFrame>
#include <QList>
#include <QMouseEvent>
#include <Q3Header>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstringhandler.h>

class LayerItemIterator: public Q3ListViewItemIterator
{
public:
    LayerItemIterator( LayerList *list ): Q3ListViewItemIterator( list ) { }
    LayerItemIterator( LayerList *list, IteratorFlag flags ): Q3ListViewItemIterator( list, flags ) { }
    LayerItemIterator( LayerItem *item ): Q3ListViewItemIterator( item ) { }
    LayerItemIterator( LayerItem *item, IteratorFlag flags ): Q3ListViewItemIterator( item, flags ) { }
    LayerItem *operator*() { return static_cast<LayerItem*>( Q3ListViewItemIterator::operator*() ); }
};

struct LayerProperty
{
    QString name;
    QString displayName;
    QPixmap enabledIcon;
    QPixmap disabledIcon;
    bool defaultValue;
    bool validForFolders;

    LayerProperty(): defaultValue( false ), validForFolders( true ) { }
    LayerProperty( const QString &pname, const QString &pdisplayName, const QPixmap &enabled, const QPixmap &disabled,
                   bool pdefaultValue, bool pvalidForFolders )
        : name( pname ),
          displayName( pdisplayName ),
          enabledIcon( enabled ),
          disabledIcon( disabled ),
          defaultValue( pdefaultValue ),
          validForFolders( pvalidForFolders )
        { }
};

class LayerToolTip;
class LayerList::Private
{
public:
    LayerItem *activeLayer;
    bool foldersCanBeActive;
    bool previewsShown;
    int itemHeight;
    QList<LayerProperty> properties;
    KMenu contextMenu;
    //LayerToolTip *tooltip; XXX

    Private( QWidget *parent, LayerList *list );
    ~Private();
};

class LayerItem::Private
{
public:
    bool isFolder;
    int id;
    QList<bool> properties;
    QImage *previewImage;
    bool previewChanged;
    QPixmap scaledPreview;
    QSize previewSize;
    QPoint previewOffset;

    Private( int pid ): isFolder( false ), id( pid ), previewImage( 0 ), previewChanged( false )
    { }
};

static const int MAX_SIZE = 256;
#warning kde4 port to new tool tip api
// class LayerToolTip: public QToolTip, public Q3Frame
// {
//     LayerList *m_list;
//     LayerItem *m_item;
//     QPoint m_pos;
//     QTimer m_timer;
//     QImage m_img;
//
// public:
//     LayerToolTip( QWidget *parent, LayerList *list )
//         : QToolTip( parent ),
//           Q3Frame( 0, 0, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_Tool | Qt::WStyle_StaysOnTop | Qt::WX11BypassWM | Qt::WNoAutoErase ),
//           m_list( list )
//     {
//         Q3Frame::setPalette( QToolTip::palette() );
//         connect( &m_timer, SIGNAL( timeout() ), m_list, SLOT( hideTip() ) );
//         qApp->installEventFilter( this );
//     }
//
//     virtual void maybeTip( const QPoint &pos )
//     {
//         m_pos = pos;
//         LayerItem *prev = m_item;
//         m_item = static_cast<LayerItem*>(m_list->itemAt( m_pos ));
//         if( QToolTip::parentWidget() && m_list->showToolTips() && m_item )
//         {
//             if( m_item != prev )
//                 hideTip();
//             showTip();
//         }
//         else
//             hideTip();
//     }
//
//     void showTip()
//     {
//         m_img = m_item->tooltipPreview();
//         m_timer.start( 15000, true );
//         if( !isVisible() || sizeHint() != size() )
//         {
//             resize( sizeHint() );
//             position();
//         }
//         if( !isVisible() )
//             show();
//         else
//             update();
//     }
//
//     void hideTip()
//     {
//         if( !isVisible() )
//             return;
//         Q3Frame::hide();
//         QToolTip::hide();
//         m_timer.stop();
//         m_img.reset();
//         m_list->triggerUpdate();
//     }
//
//     virtual void drawContents( QPainter *painter )
//     {
//         QPixmap buf( width(), height() );
//         QPainter p( &buf );
//         buf.fill( colorGroup().background() );
//         p.setPen( colorGroup().foreground() );
//         p.drawRect( buf.rect() );
//
//         Q3SimpleRichText text( m_item->tooltip(), QToolTip::font() );
//         text.setWidth( QCOORD_MAX );
//
//         p.translate( 5, 5 );
//         if( !m_img.isNull() )
//         {
//             if( m_img.width() > MAX_SIZE || m_img.height() > MAX_SIZE )
//                 m_img = m_img.scale( MAX_SIZE, MAX_SIZE, Qt::KeepAspectRatio );
//             int y = 0;
//             if( m_img.height() < text.height() )
//                 y = text.height()/2 - m_img.height()/2;
//             p.drawImage( 0, y, m_img );
//             p.drawRect( -1, y-1, m_img.width()+2, m_img.height()+2 );
//             p.translate( m_img.width() + 10, 0 );
//         }
//
//         text.draw( &p, 0, 0, rect(), colorGroup() );
//
//         painter->drawPixmap( 0, 0, buf );
//     }
//
//     virtual QSize sizeHint() const
//     {
//         if( !m_item )
//             return QSize( 0, 0 );
//
//         Q3SimpleRichText text( m_item->tooltip(), QToolTip::font() );
//         text.setWidth( QCOORD_MAX );
//
//         int width = text.widthUsed();
//         if( !m_img.isNull() )
//             width += qMin( m_img.width(), MAX_SIZE ) + 10;
//         width += 10;
//
//         int height = text.height();
//         if( !m_img.isNull() && qMin( m_img.height(), MAX_SIZE ) > height )
//             height = qMin( m_img.height(), MAX_SIZE );
//         height += 10;
//
//         return QSize( width, height );
//     }
//
//     void position()
//     {
//         const QRect drect = QApplication::desktop()->availableGeometry( QToolTip::parentWidget() );
//         const QSize size = sizeHint();
//         const int width = size.width(), height = size.height();
//         const QRect tmp = m_item->rect();
//         const QRect irect( m_list->viewport()->mapToGlobal( m_list->contentsToViewport(tmp.topLeft()) ), tmp.size() );
//
//         int y;
//         if( irect.bottom() + height < drect.bottom() )
//             y = irect.bottom();
//         else
//             y = qMax( drect.top(), irect.top() - height );
//
//         int x = qMax( drect.x(), QToolTip::parentWidget()->mapToGlobal( m_pos ).x() - width/2 );
//         if( x + width > drect.right() )
//             x = drect.right() - width;
//
//         move( x, y );
//     }
//
//     virtual bool eventFilter( QObject *, QEvent *e )
//     {
//         if( isVisible() )
//             switch ( e->type() )
//             {
//                 case QEvent::KeyPress:
//                 case QEvent::KeyRelease:
//                 case QEvent::MouseButtonPress:
//                 case QEvent::MouseButtonRelease:
//                 //case QEvent::MouseMove:
//                 case QEvent::FocusIn:
//                 case QEvent::FocusOut:
//                 case QEvent::Wheel:
//                 case QEvent::Leave:
//                     hideTip();
//                 default: break;
//             }
//
//         return false;
//     }
// };

LayerList::Private::Private( QWidget *parent, LayerList *list )
    : activeLayer( 0 ), foldersCanBeActive( false ), previewsShown( false ), itemHeight( 32 )
// ,tooltip( new LayerToolTip( parent, list ) ) XXX
{ }

LayerList::Private::~Private()
{
//     delete tooltip; XXX
//     tooltip = 0;
}

static int getID()
{
    static int id = -2;
    return id--;
}

static QSize iconSize() { return QIcon::pixmapSize( QIcon::Small ); }


///////////////
// LayerList //
///////////////

LayerList::LayerList( QWidget *parent, const char *name )
    : super( parent ), d( new Private( viewport(), this ) )
{
    setObjectName(name);
    setSelectionMode( Q3ListView::Extended );
    setRootIsDecorated( true );
    setSorting( -1 );
    setSortColumn( -1 );
    setAllColumnsShowFocus( true );
    setFullWidth( true );
    setItemsRenameable( false );
    setDropHighlighter( true );
    setDefaultRenameAction( Q3ListView::Accept );
    setDragEnabled( true );
    setAcceptDrops( true );
    setItemsMovable( true );
    addColumn( QString() );
    header()->hide();

    this->setToolTip( i18n("Right-click to create folders. Click on the layername to change the layer's name. Click and drag to move layers."));

    setNumRows( 2 );

    connect( this, SIGNAL( itemRenamed( Q3ListViewItem*, const QString&, int ) ),
                 SLOT( slotItemRenamed( Q3ListViewItem*, const QString&, int ) ) );
    connect( this, SIGNAL( moved( QList<Q3ListViewItem*>&, QList<Q3ListViewItem*>&, QList<Q3ListViewItem*>& ) ),
             SLOT( slotItemMoved( QList<Q3ListViewItem*>&, QList<Q3ListViewItem*>&, QList<Q3ListViewItem*>& ) ) );
    connect( this, SIGNAL( onItem( Q3ListViewItem* ) ), SLOT( hideTip() ) );
    connect( this, SIGNAL( onViewport() ), SLOT( hideTip() ) );
}

LayerList::~LayerList()
{
    delete d;
}

void LayerList::addProperty( const QString &name, const QString &displayName, const QIcon &icon,
                             bool defaultValue, bool validForFolders )
{
    addProperty( name, displayName, icon.pixmap( QIcon::Small, QIcon::Normal ), icon.pixmap( QIcon::Small, QIcon::Disabled ), defaultValue, validForFolders );
}

void LayerList::addProperty( const QString &name, const QString &displayName, QPixmap enabled, QPixmap disabled,
                             bool defaultValue, bool validForFolders )
{
    d->properties.append( LayerProperty( name, displayName, enabled, disabled, defaultValue, validForFolders ) );

    for( LayerItemIterator it( this ); *it; ++it )
        (*it)->d->properties.append( defaultValue );

    //we do this only afterwards in case someone wants to access the other items in a connected slot...
    for( LayerItemIterator it( this ); *it; ++it )
        if( validForFolders || !(*it)->isFolder() )
        {
            emit propertyChanged( *it, name, defaultValue );
            emit propertyChanged( (*it)->id(), name, defaultValue );
        }

    triggerUpdate();
}

LayerItem *LayerList::layer( int id ) const
{
    if( !firstChild() || id == -1 )
        return 0;

    for( LayerItemIterator it( firstChild() ); *it; ++it )
        if( (*it)->id() == id )
            return (*it);

    return 0;
}

LayerItem *LayerList::folder( int id ) const
{
    if( !firstChild() || id == -1 )
        return 0;

    for( LayerItemIterator it( firstChild() ); *it; ++it )
        if( (*it)->id() == id && (*it)->isFolder() )
            return (*it);

    return 0;
}

LayerItem *LayerList::activeLayer() const
{
    return d->activeLayer;
}

int LayerList::activeLayerID() const
{
    if( activeLayer() )
        return activeLayer()->id();
    return -1;
}

QList<LayerItem*> LayerList::selectedLayers() const
{
    if( !firstChild() )
        return QList<LayerItem*>();

    QList<LayerItem*> layers;
    for( LayerItemIterator it( firstChild() ); *it; ++it )
        if( (*it)->isSelected() )
            layers.append( *it );

    return layers;
}

QList<int> LayerList::selectedLayerIDs() const
{
    const QList<LayerItem*> layers = selectedLayers();
    QList<int> ids;
    for( int i = 0, n = layers.count(); i < n; ++i )
        ids.append( layers[i]->id() );

    return ids;
}

bool LayerList::foldersCanBeActive() const
{
    return d->foldersCanBeActive;
}

bool LayerList::previewsShown() const
{
    return d->previewsShown;
}

int LayerList::itemHeight() const
{
    return d->itemHeight;
}

int LayerList::numRows() const
{
    if( itemHeight() < qMax( fontMetrics().height(), iconSize().height() ) )
        return 0;

    return ( itemHeight() - fontMetrics().height() ) / iconSize().height() + 1;
}

void LayerList::makeFolder( int id )
{
    LayerItem* const l = layer( id );
    if( l )
        l->makeFolder();
}

bool LayerList::isFolder( int id ) const
{
    LayerItem* const l = layer( id );
    if( !l )
        return false;

    return l->isFolder();
}

QString LayerList::displayName( int id ) const
{
    LayerItem* const l = layer( id );
    if( !l )
        return QString::null; //should be more severe...

    return l->displayName();
}

bool LayerList::property( int id, const QString &name ) const
{
    LayerItem* const l = layer( id );
    if( !l )
        return false; //should be more severe...

    return l->property( name );
}

KMenu *LayerList::contextMenu() const
{
    return &( d->contextMenu );
}

void LayerList::setFoldersCanBeActive( bool can ) //SLOT
{
    d->foldersCanBeActive = can;
    if( !can && activeLayer() && activeLayer()->isFolder() )
    {
        d->activeLayer = 0;
        emit activated( static_cast<LayerItem*>( 0 ) );
        emit activated( -1 );
    }
}

void LayerList::setPreviewsShown( bool show ) //SLOT
{
    d->previewsShown = show;
    triggerUpdate();
}

void LayerList::setItemHeight( int height ) //SLOT
{
    d->itemHeight = height;
    for( LayerItemIterator it( this ); *it; ++it )
        (*it)->setup();
    triggerUpdate();
}

void LayerList::setNumRows( int rows )
{
    if( rows < 1 )
        return;

    if( rows == 1 )
        setItemHeight( qMax( fontMetrics().height(), iconSize().height() ) );
    else
        setItemHeight( fontMetrics().height() + ( rows - 1 ) * iconSize().height() );
}

void LayerList::setActiveLayer( LayerItem *layer ) //SLOT
{
    if( !foldersCanBeActive() && layer && layer->isFolder() )
        return;

    ensureItemVisible( layer );

    if( d->activeLayer == layer )
        return;

    d->activeLayer = layer;

    if( currentItem() != layer )
        setCurrentItem( layer );
    else
    {
        int n = 0;
        for( LayerItemIterator it( this, LayerItemIterator::Selected ); n < 2 && (*it); ++it ) { n++; }
        if( n == 1 )
            (*LayerItemIterator( this, LayerItemIterator::Selected ))->setSelected( false );
        if( layer )
            layer->setSelected( true );
    }

    emit activated( layer );
    if( layer )
        emit activated( layer->id() );
    else
        emit activated( -1 );
}

void LayerList::setActiveLayer( int id ) //SLOT
{
    setActiveLayer( layer( id ) );
}

void LayerList::setLayerDisplayName( LayerItem *layer, const QString &displayName )
{
    if( !layer )
        return;

    layer->setDisplayName( displayName );
}

void LayerList::setLayerDisplayName( int id, const QString &displayName )
{
    setLayerDisplayName( layer( id ), displayName );
}

void LayerList::setLayerProperty( LayerItem *layer, const QString &name, bool on ) //SLOT
{
    if( !layer )
        return;

    layer->setProperty( name, on );
}

void LayerList::setLayerProperty( int id, const QString &name, bool on ) //SLOT
{
    setLayerProperty( layer( id ), name, on );
}

void LayerList::toggleLayerProperty( LayerItem *layer, const QString &name ) //SLOT
{
    if( !layer )
        return;

    layer->toggleProperty( name );
}

void LayerList::toggleLayerProperty( int id, const QString &name ) //SLOT
{
    toggleLayerProperty( layer( id ), name );
}

void LayerList::setLayerPreviewImage( LayerItem *layer, QImage *image )
{
    if( !layer )
        return;

    layer->setPreviewImage( image );
}

void LayerList::setLayerPreviewImage( int id, QImage *image )
{
    setLayerPreviewImage( layer( id ), image );
}

void LayerList::layerPreviewChanged( LayerItem *layer )
{
    if( !layer )
        return;

    layer->previewChanged();
}

void LayerList::layerPreviewChanged( int id )
{
    layerPreviewChanged( layer( id ) );
}

LayerItem *LayerList::addLayer( const QString &displayName, LayerItem *after, int id ) //SLOT
{
    return new LayerItem( displayName, this, after, id );
}

LayerItem *LayerList::addLayer( const QString &displayName, int afterID, int id ) //SLOT
{
    return new LayerItem( displayName, this, layer( afterID ), id );
}

//SLOT
LayerItem *LayerList::addLayerToParent( const QString &displayName, LayerItem *parent, LayerItem *after, int id )
{
    if( parent && parent->isFolder() )
        return parent->addLayer( displayName, after, id );
    else
        return 0;
}

LayerItem *LayerList::addLayerToParent( const QString &displayName, int parentID, int afterID, int id ) //SLOT
{
    return addLayerToParent( displayName, folder( parentID ), layer( afterID ), id );
}

void LayerList::moveLayer( LayerItem *layer, LayerItem *parent, LayerItem *after ) //SLOT
{
    if( !layer )
        return;

    if( parent && !parent->isFolder() )
        parent = 0;

    if( layer->parent() == parent && layer->prevSibling() == after )
        return;

    Q3ListViewItem *current = currentItem();

    moveItem( layer, parent, after );

    emit layerMoved( layer, parent, after );
    emit layerMoved( layer->id(), parent ? parent->id() : -1, after ? after->id() : -1 );

    setCurrentItem( current ); //HACK, sometimes Qt changes this under us
}

void LayerList::moveLayer( int id, int parentID, int afterID ) //SLOT
{
    moveLayer( layer( id ), folder( parentID ), layer( afterID ) );
}

void LayerList::removeLayer( LayerItem *layer ) //SLOT
{
    delete layer;
}

void LayerList::removeLayer( int id ) //SLOT
{
    delete layer( id );
}

void LayerList::contentsMousePressEvent( QMouseEvent *e )
{
    LayerItem *item = static_cast<LayerItem*>( itemAt( contentsToViewport( e->pos() ) ) );

    if( item )
    {
        QMouseEvent m( QEvent::MouseButtonPress, item->mapFromListView( e->pos() ), e->button(), e->state() );
        if( !item->mousePressEvent( &m ) )
            super::contentsMousePressEvent( e );
    }
    else
    {
        super::contentsMousePressEvent( e );
        if( e->button() == Qt::RightButton )
            showContextMenu();
    }
}

void LayerList::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
    super::contentsMouseDoubleClickEvent( e );
    if( LayerItem *layer = static_cast<LayerItem*>( itemAt( contentsToViewport( e->pos() ) ) ) )
    {
        if( !layer->iconsRect().contains( layer->mapFromListView( e->pos() ) ) )
        {
            emit requestLayerProperties( layer );
            emit requestLayerProperties( layer->id() );
        }
    }
    else
    {
        emit requestNewLayer( static_cast<LayerItem*>( 0 ), static_cast<LayerItem*>( 0 ) );
        emit requestNewLayer( -1, -1 );
    }
}

void LayerList::findDrop( const QPoint &pos, Q3ListViewItem *&parent, Q3ListViewItem *&after )
{
    LayerItem *item = static_cast<LayerItem*>( itemAt( contentsToViewport( pos ) ) );
    if( item && item->isFolder() )
    {
        parent = item;
        after = 0;
    }
    else
        super::findDrop( pos, parent, after );
}

void LayerList::showContextMenu()
{
    LayerItem *layer = static_cast<LayerItem*>( itemAt( viewport()->mapFromGlobal( QCursor::pos() ) ) );
    if( layer )
        setCurrentItem( layer );
    d->contextMenu.clear();
    constructMenu( layer );
#warning kde4 port
    //menuActivated( d->contextMenu.exec( QCursor::pos() ), layer );
}

void LayerList::hideTip()
{
    //d->tooltip->hideTip(); XXX
}

void LayerList::maybeTip()
{
    //d->tooltip->maybeTip( d->tooltip->QToolTip::parentWidget()->mapFromGlobal( QCursor::pos() ) ); XXX
}

void LayerList::constructMenu( LayerItem *layer )
{
    if( layer )
    {
        for( int i = 0, n = d->properties.count(); i < n; ++i )
            if( !layer->isFolder() || d->properties[i].validForFolders )
                d->contextMenu.insertItem( layer->d->properties[i] ? d->properties[i].enabledIcon : d->properties[i].disabledIcon, d->properties[i].displayName, MenuItems::COUNT + i );
        d->contextMenu.insertItem( SmallIconSet( "info" ), i18n( "&Properties" ), MenuItems::LayerProperties );
        d->contextMenu.insertSeparator();
        d->contextMenu.insertItem( SmallIconSet( "editdelete" ),
            selectedLayers().count() > 1 ? i18n( "Remove Layers" )
                   : layer->isFolder() ? i18n( "&Remove Folder" )
                                       : i18n( "&Remove Layer" ), MenuItems::RemoveLayer );
    }
    d->contextMenu.insertItem( SmallIconSet( "filenew" ), i18n( "&New Layer" ), MenuItems::NewLayer );
    d->contextMenu.insertItem( SmallIconSet( "folder" ), i18n( "New &Folder" ), MenuItems::NewFolder );
}

void LayerList::menuActivated( int id, LayerItem *layer )
{
    const QList<LayerItem*> selected = selectedLayers();

    LayerItem *parent = ( layer && layer->isFolder() ) ? layer : 0;
    LayerItem *after = 0;
    if( layer && !parent )
    {
        parent = layer->parent();
        after = layer->prevSibling();
    }
    switch( id )
    {
        case MenuItems::NewLayer:
            emit requestNewLayer( parent, after );
            emit requestNewLayer( parent ? parent->id() : -1, after ? after->id() : -1 );
            break;
        case MenuItems::NewFolder:
            emit requestNewFolder( parent, after );
            emit requestNewFolder( parent ? parent->id() : -1, after ? after->id() : -1 );
            break;
        case MenuItems::RemoveLayer:
            {
                QList<int> ids;
                for( int i = 0, n = selected.count(); i < n; ++i )
                {
                    ids.append( selected[i]->id() );
                    emit requestRemoveLayer( selected[i]->id() );
                }
                emit requestRemoveLayers( ids );
            }
            for( int i = 0, n = selected.count(); i < n; ++i )
                emit requestRemoveLayer( selected[i] );
            emit requestRemoveLayers( selected );
            break;
        case MenuItems::LayerProperties:
            if( layer )
            {
                emit requestLayerProperties( layer );
                emit requestLayerProperties( layer->id() );
            }
            break;
        default:
            if( id >= MenuItems::COUNT && layer )
                for( int i = 0, n = selected.count(); i < n; ++i )
                    selected[i]->toggleProperty( d->properties[ id - MenuItems::COUNT ].name );
    }
}

void LayerList::slotItemRenamed( Q3ListViewItem *item, const QString &text, int col )
{
    if( !item || col != 0 )
        return;

    emit displayNameChanged( static_cast<LayerItem*>( item ), text );
    emit displayNameChanged( static_cast<LayerItem*>( item )->id(), text );
}

void LayerList::slotItemMoved( QList<Q3ListViewItem*> &items, QList<Q3ListViewItem*> &/*afterBefore*/, QList<Q3ListViewItem*> &afterNow )
{
    for( int i = 0, n = items.count(); i < n; ++i )
    {
        LayerItem *l = static_cast<LayerItem*>( items.at(i) ), *a = static_cast<LayerItem*>( afterNow.at(i) );
        if( !l )
            continue;

        if( l->parent() )
            l->parent()->setOpen( true );

        emit layerMoved( l, l->parent(), a );
        emit layerMoved( l->id(), l->parent() ? l->parent()->id() : -1, a ? a->id() : -1 );
    }
}

void LayerList::setCurrentItem( Q3ListViewItem *item )
{
    if( !item )
        return;

    super::setCurrentItem( item );
    ensureItemVisible( item );
    int n = 0;
    for( LayerItemIterator it( this, LayerItemIterator::Selected ); n < 2 && (*it); ++it ) { n++; }
    if( n == 1 )
        (*LayerItemIterator( this, LayerItemIterator::Selected ))->setSelected( false );
    item->setSelected( true );
    if( activeLayer() != item )
        setActiveLayer( static_cast<LayerItem*>(item) );
}


///////////////
// LayerItem //
///////////////

LayerItem::LayerItem( const QString &displayName, LayerList *p, LayerItem *after, int id )
    : super( p, after ), d( new Private( id ) )
{
    init();
    setDisplayName( displayName );
}

LayerItem::LayerItem( const QString &displayName, LayerItem *p, LayerItem *after, int id )
    : super( ( p && p->isFolder() ) ? p : 0, after ), d( new Private( id ) )
{
    init();
    setDisplayName( displayName );
}

void LayerItem::init()
{
    if( d->id < 0 )
        d->id = getID();

    for( int i = 0, n = listView()->d->properties.count(); i < n; ++i )
        d->properties.append( listView()->d->properties[i].defaultValue );

    if( parent())
        parent()->setOpen( true );
}

LayerItem::~LayerItem()
{
    if (listView() && (listView()->activeLayer() == this || contains(listView()->activeLayer())))
        listView()->setActiveLayer( static_cast<LayerItem*>( 0 ) );
    delete d;
}

void LayerItem::makeFolder()
{
    d->isFolder = true;
    setPixmap( 0, SmallIcon( "folder", 16 ) );
    if( isActive() && !listView()->foldersCanBeActive() )
        listView()->setActiveLayer( static_cast<LayerItem*>( 0 ) );
}

bool LayerItem::isFolder() const
{
    return d->isFolder;
}

bool LayerItem::contains(const LayerItem *item)
{
    Q3ListViewItemIterator it(this);

    while (it.current()) {
        if (it.current() == item) {
            return true;
        }
        ++it;
    }
    return false;
}

int LayerItem::id() const
{
    return d->id;
}

QString LayerItem::displayName() const
{
    return text( 0 );
}

void LayerItem::setDisplayName( const QString &s )
{
    if( displayName() == s )
        return;
    setText( 0, s );
    emit listView()->displayNameChanged( this, s );
    emit listView()->displayNameChanged( id(), s );
}

bool LayerItem::isActive() const
{
    return listView()->activeLayer() == this;
}

void LayerItem::setActive()
{
    listView()->setActiveLayer( this );
}

bool LayerItem::property( const QString &name ) const
{
    int i = listView()->d->properties.count() - 1;
    while( i && listView()->d->properties[i].name != name )
        --i;

    if( i < 0 )
        return false; //should do something more severe... but what?

    return d->properties[i];
}

void LayerItem::setProperty( const QString &name, bool on )
{
    int i = listView()->d->properties.count() - 1;
    while( i && listView()->d->properties[i].name != name )
        --i;

    if( i < 0 || ( isFolder() && !listView()->d->properties[i].validForFolders ) )
        return;

    const bool notify = ( on != d->properties[i] );
    d->properties[i] = on;
    if( notify )
    {
        emit listView()->propertyChanged( this, name, on );
        emit listView()->propertyChanged( id(), name, on );
    }

    update();
}

void LayerItem::toggleProperty( const QString &name )
{
    int i = listView()->d->properties.count() - 1;
    while( i && listView()->d->properties[i].name != name )
        --i;

    if( i < 0 || ( isFolder() && !listView()->d->properties[i].validForFolders ) )
        return;

    d->properties[i] = !(d->properties[i]);
    emit listView()->propertyChanged( this, name, d->properties[i] );
    emit listView()->propertyChanged( id(), name, d->properties[i] );

    update();
}

void LayerItem::setPreviewImage( QImage *image )
{
    d->previewImage = image;
    previewChanged();
}

void LayerItem::previewChanged()
{
    d->previewChanged = true;
    update();
}

LayerItem *LayerItem::addLayer( const QString &displayName, LayerItem *after, int id )
{
    if( !isFolder() )
        return 0;
    return new LayerItem( displayName, this, after, id );
}

LayerItem *LayerItem::prevSibling() const
{
    LayerItem *item = parent() ? parent()->firstChild() : listView()->firstChild();
    if( !item || this == item )
        return 0;
    for(; item && this != item->nextSibling(); item = item->nextSibling() );
    return item;
}

int LayerItem::mapXFromListView( int x ) const
{
    return x - rect().left();
}

int LayerItem::mapYFromListView( int y ) const
{
    return y - rect().top();
}

QPoint LayerItem::mapFromListView( const QPoint &point ) const
{
    return QPoint( mapXFromListView( point.x() ), mapYFromListView( point.y() ) );
}

QRect LayerItem::mapFromListView( const QRect &rect ) const
{
    return QRect( mapFromListView( rect.topLeft() ), rect.size() );
}

int LayerItem::mapXToListView( int x ) const
{
    return x + rect().left();
}

int LayerItem::mapYToListView( int y ) const
{
    return y + rect().top();
}

QPoint LayerItem::mapToListView( const QPoint &point ) const
{
    return QPoint( mapXToListView( point.x() ), mapYToListView( point.y() ) );
}

QRect LayerItem::mapToListView( const QRect &rect ) const
{
    return QRect( mapToListView( rect.topLeft() ), rect.size() );
}

QRect LayerItem::rect() const
{
    const int indent = listView()->treeStepSize() * ( depth() + 1 );
    return QRect( listView()->header()->sectionPos( 0 )  + indent, itemPos(),
                  listView()->header()->sectionSize( 0 ) - indent, height() );
}

QRect LayerItem::textRect() const
{
    static QFont f;
    static int minbearing = 1337 + 666; //can be 0 or negative, 2003 is less likely
    if( minbearing == 2003 || f != font() )
    {
        f = font(); //getting your bearings can be expensive, so we cache them
        minbearing = fontMetrics().minLeftBearing() + fontMetrics().minRightBearing();
    }

    const int margin = listView()->itemMargin();
    int indent = previewRect().right() + margin;
    if( pixmap( 0 ) )
        indent += pixmap( 0 )->width() + margin;

    const int width = ( multiline() ? rect().right() : iconsRect().left() ) - indent - margin + minbearing;

    return QRect( indent, 0, width, fontMetrics().height() );
}

QRect LayerItem::iconsRect() const
{
    const QList<LayerProperty> &lp = listView()->d->properties;
    int propscount = 0;
    for( int i = 0, n = lp.count(); i < n; ++i )
        if( !lp[i].enabledIcon.isNull() && ( !multiline() || !isFolder() || lp[i].validForFolders ) )
            propscount++;

    const int iconswidth = propscount * iconSize().width() + (propscount - 1) * listView()->itemMargin();

    const int x = multiline() ? previewRect().right() + listView()->itemMargin() : rect().width() - iconswidth;
    const int y = multiline() ? fontMetrics().height() : 0;

    return QRect( x, y, iconswidth, iconSize().height() );
}

QRect LayerItem::previewRect() const
{
    return QRect( 0, 0, listView()->previewsShown() ? height() : 0, height() );
}

void LayerItem::drawText( QPainter *p, const QColorGroup &cg, const QRect &r )
{
    p->translate( r.left(), r.top() );

    p->setPen( isSelected() ? cg.highlightedText() : cg.text() );

    const QString text = KStringHandler::rPixelSqueeze( displayName(), p->fontMetrics(), r.width() );
    p->drawText( listView()->itemMargin(), 0, r.width(), r.height(), Qt::AlignLeft | Qt::AlignTop, text );

    p->translate( -r.left(), -r.top() );
}

void LayerItem::drawIcons( QPainter *p, const QColorGroup &/*cg*/, const QRect &r )
{
    p->translate( r.left(), r.top() );

    int x = 0;
    const QList<LayerProperty> &lp = listView()->d->properties;
    for( int i = 0, n = lp.count(); i < n; ++i )
        if( !lp[i].enabledIcon.isNull() && ( !multiline() || !isFolder() || lp[i].validForFolders ) )
        {
            if( !isFolder() || lp[i].validForFolders )
                p->drawPixmap( x, 0, d->properties[i] ? lp[i].enabledIcon : lp[i].disabledIcon );
            x += iconSize().width() + listView()->itemMargin();
        }

    p->translate( -r.left(), -r.top() );
}

void LayerItem::drawPreview( QPainter *p, const QColorGroup &/*cg*/, const QRect &r )
{
    if( !showPreview() )
        return;

    if( d->previewChanged || r.size() != d->previewSize )
    {      //TODO handle width() != height()
        const int size = qMin( r.width(), qMax( previewImage()->width(), previewImage()->height() ) );
        const QImage i = previewImage()->smoothScale( size, size, Qt::KeepAspectRatio );
        d->scaledPreview.convertFromImage( i );
        d->previewOffset.setX( r.width()/2 - i.width()/2 );
        d->previewOffset.setY( r.height()/2 - i.height()/2 );

        d->previewChanged = false;
        d->previewSize = r.size();
    }

    p->drawPixmap( r.topLeft() + d->previewOffset, d->scaledPreview );
}

bool LayerItem::showPreview() const
{
    return listView()->previewsShown() && previewImage() && !previewImage()->isNull();
}

bool LayerItem::multiline() const
{
    return height() >= fontMetrics().height() + iconSize().height();
}

QFont LayerItem::font() const
{
    if( isActive() )
    {
        QFont f = listView()->font();
        f.setBold( !f.bold() );
        f.setItalic( !f.italic() );
        return f;
    }
    else
        return listView()->font();
}

QFontMetrics LayerItem::fontMetrics() const
{
    return QFontMetrics( font() );
}

bool LayerItem::mousePressEvent( QMouseEvent *e )
{
    if( e->button() == Qt::RightButton )
    {
        if ( !(e->state() & Qt::ControlModifier) && !(e->state() & Qt::ShiftModifier) )
            setActive();
        QTimer::singleShot( 0, listView(), SLOT( showContextMenu() ) );
        return false;
    }

    const QRect ir = iconsRect(), tr = textRect();

    if( ir.contains( e->pos() ) )
    {
        const int iconWidth = iconSize().width();
        int x = e->pos().x() - ir.left();
        if( x % ( iconWidth + listView()->itemMargin() ) < iconWidth ) //it's on an icon, not a margin
        {
            const QList<LayerProperty> &lp = listView()->d->properties;
            int p = -1;
            for( int i = 0, n = lp.count(); i < n; ++i )
            {
                if( !lp[i].enabledIcon.isNull() && ( !multiline() || !isFolder() || lp[i].validForFolders ) )
                    x -= iconWidth + listView()->itemMargin();
                p += 1;
                if( x < 0 )
                    break;
            }
            toggleProperty( lp[p].name );
        }
        return true;
    }

    else if( tr.contains( e->pos() ) && isSelected() && !listView()->renameLineEdit()->isVisible() )
    {
        listView()->rename( this, 0 );
        QRect r( listView()->contentsToViewport( mapToListView( tr.topLeft() ) ), tr.size() );
        listView()->renameLineEdit()->setGeometry( r );
        return true;
    }

    if ( !(e->state() & Qt::ControlModifier) && !(e->state() & Qt::ShiftModifier) )
        setActive();

    return false;
}

QString LayerItem::tooltip() const
{
    QString tip;
    tip += "<table cellspacing=\"0\" cellpadding=\"0\">";
    tip += QString("<tr><td colspan=\"2\" align=\"center\"><b>%1</b></td></tr>").arg( displayName() );
    QString row = "<tr><td>%1</td><td>%2</td></tr>";
    for( int i = 0, n = listView()->d->properties.count(); i < n; ++i )
        if( !isFolder() || listView()->d->properties[i].validForFolders )
        {
            if( d->properties[i] )
                tip += row.arg( i18n( "%1:" ).arg( listView()->d->properties[i].displayName ) ).arg( i18n( "Yes" ) );
            else
                tip += row.arg( i18n( "%1:" ).arg( listView()->d->properties[i].displayName ) ).arg( i18n( "No" ) );
        }
    tip += "</table>";
    return tip;
}

QImage *LayerItem::previewImage() const
{
    return d->previewImage;
}

QImage LayerItem::tooltipPreview() const
{
    if( previewImage() )
        return *previewImage();
    return QImage();
}

int LayerItem::width( const QFontMetrics &fm, const Q3ListView *lv, int c ) const
{
    if( c != 0 )
        return super::width( fm, lv, c );

    const QList<LayerProperty> &lp = listView()->d->properties;
    int propscount = 0;
    for( int i = 0, n = d->properties.count(); i < n; ++i )
        if( !lp[i].enabledIcon.isNull() && ( !multiline() || !isFolder() || lp[i].validForFolders ) )
            propscount++;

    const int iconswidth = propscount * iconSize().width() + (propscount - 1) * listView()->itemMargin();

    if( multiline() )
        return qMax( super::width( fm, lv, 0 ), iconswidth );
    else
        return super::width( fm, lv, 0 ) + iconswidth;
}

void LayerItem::paintCell( QPainter *painter, const QColorGroup &cg, int column, int width, int align )
{
    if( column != 0 )
    {
        super::paintCell( painter, cg, column, width, align );
        return;
    }

    QPixmap buf( width, height() );
    QPainter p( &buf );

    p.setFont( font() );

    const QColorGroup cg_ = isEnabled() ? listView()->palette().active() : listView()->palette().disabled();

    const QColor bg = isSelected()  ? cg_.highlight()
                    : isAlternate() ? listView()->alternateBackground()
                    : listView()->viewport()->backgroundColor();

    buf.fill( bg );

    if( pixmap( 0 ) )
        p.drawPixmap( previewRect().right() + listView()->itemMargin(), 0, *pixmap( 0 ) );

    drawText( &p, cg_, textRect() );
    drawIcons( &p, cg_, iconsRect() );
    drawPreview( &p, cg_, previewRect() );

    painter->drawPixmap( 0, 0, buf );
}

void LayerItem::setup()
{
    super::setup();
    setHeight( listView()->d->itemHeight );
}

void LayerItem::setSelected( bool selected )
{
    if( !selected && ( isActive() || this == listView()->currentItem() ) )
        return;
    super::setSelected( selected );
}


/////////////////////////
// Convenience Methods //
/////////////////////////

LayerItem *LayerList::firstChild() const { return static_cast<LayerItem*>( super::firstChild() ); }
LayerItem *LayerList::lastChild() const { return static_cast<LayerItem*>( super::lastChild() ); }
LayerList *LayerItem::listView() const { return static_cast<LayerList*>( super::listView() ); }
void LayerItem::update() const { listView()->repaintItem( this ); }
LayerItem *LayerItem::firstChild() const { return static_cast<LayerItem*>( super::firstChild() ); }
LayerItem *LayerItem::nextSibling() const { return static_cast<LayerItem*>( super::nextSibling() ); }
LayerItem *LayerItem::parent() const { return static_cast<LayerItem*>( super::parent() ); }


#include "layerlist.moc"

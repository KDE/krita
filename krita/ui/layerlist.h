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


#ifndef LAYERLIST_H
#define LAYERLIST_H

#include <qiconset.h>
#include <klistview.h>

class QMouseEvent;
class QString;
class KPopupMenu;
class LayerItem;
class LayerFolder;

class LayerList: public KListView
{
    Q_OBJECT

public:
    LayerList( QWidget *parent = 0, const char *name = 0 );
    virtual ~LayerList();

    void addProperty( const QString &name, const QString &displayName, const QIconSet &icon = QIconSet(),
                      bool defaultValue = false, bool validForFolders = true );

    bool foldersCanBeActive() const;
    bool previewsShown() const;
    int itemHeight() const;
    int numRows() const;

    LayerItem *layer( int id ) const;
    LayerItem *folder( int id ) const; //returns 0 if not a folder

    LayerItem *activeLayer() const;
    int activeLayerID() const;

    QValueList<LayerItem*> selectedLayers() const;
    QValueList<int> selectedLayerIDs() const;

    void makeFolder( int id );
    bool isFolder( int id ) const;
    QString displayName( int id ) const;
    bool property( int id, const QString &name ) const;

    struct MenuItems
    {
        enum { NewLayer = 0, NewFolder, RemoveLayer, LayerProperties, COUNT };
    };
    KPopupMenu *contextMenu() const;

public slots:
    void setFoldersCanBeActive( bool can );
    void setPreviewsShown( bool show );
    void setItemHeight( int height );
    void setNumRows( int rows ); //how many rows of property icons can fit

    void setActiveLayer( LayerItem *layer );
    void setActiveLayer( int id );

    void setLayerDisplayName( LayerItem *layer, const QString &displayName );
    void setLayerDisplayName( int id, const QString &displayName );

    void setLayerProperty( LayerItem *layer, const QString &name, bool on );
    void setLayerProperty( int id, const QString &name, bool on );

    void toggleLayerProperty( LayerItem *layer, const QString &name );
    void toggleLayerProperty( int id, const QString &name );

    void setLayerPreviewImage( LayerItem *layer, QImage *image );
    void setLayerPreviewImage( int id, QImage *image );

    void setLayerPreviewPixmap( LayerItem *layer, QPixmap *pixmap );
    void setLayerPreviewPixmap( int id, QPixmap *pixmap );

    void layerPreviewChanged( LayerItem *layer );
    void layerPreviewChanged( int id );

    LayerItem *addLayer( const QString &displayName, LayerItem *after = 0, int id = -1 );
    LayerItem *addLayer( const QString &displayName, int afterID, int id = -1 );

    LayerItem *addLayerToParent( const QString &displayName, LayerItem *parent, LayerItem *after = 0, int id = -1 );
    LayerItem *addLayerToParent( const QString &displayName, int parentID, int afterID = -1, int id = -1 );

    void moveLayer( LayerItem *layer, LayerItem *parent, LayerItem *after );
    void moveLayer( int id, int parentID, int afterID );

    void removeLayer( LayerItem *layer );
    void removeLayer( int id );

signals:
    void activated( LayerItem *layer );
    void activated( int id );

    void displayNameChanged( LayerItem *layer, const QString &displayName );
    void displayNameChanged( int id, const QString &displayName );

    void propertyChanged( LayerItem *layer, const QString &name, bool on );
    void propertyChanged( int id, const QString &name, bool on );

    void layerMoved( LayerItem *layer, LayerItem *parent, LayerItem *after );
    void layerMoved( int id, int parentID, int afterID );

    void requestNewLayer( LayerItem *parent, LayerItem *after );
    void requestNewLayer( int parentID, int afterID );

    void requestNewFolder( LayerItem *parent, LayerItem *after );
    void requestNewFolder( int parentID, int afterID );

    void requestRemoveLayer( LayerItem *layer );
    void requestRemoveLayer( int id );

    void requestRemoveLayers( QValueList<LayerItem*> layers );
    void requestRemoveLayers( QValueList<int> ids );

    void requestLayerProperties( LayerItem *layer );
    void requestLayerProperties( int id );

public: //convenience
    LayerItem *firstChild() const;
    LayerItem *lastChild() const;

protected slots:
    virtual void constructMenu( LayerItem *layer );
    virtual void menuActivated( int id, LayerItem *layer );

private:
    typedef KListView super;
    friend class LayerItem;

    class Private;
    Private* const d;

private slots:
    void slotItemRenamed( QListViewItem *item, const QString &text, int col );
    void slotItemMoved( QListViewItem *item, QListViewItem*, QListViewItem* );
    void showContextMenu();

public: //reimplemented for internal reasons
    virtual void setCurrentItem( QListViewItem *i );

protected:
    virtual void contentsMousePressEvent( QMouseEvent *e );
    virtual void contentsMouseDoubleClickEvent ( QMouseEvent *e );
    virtual void findDrop( const QPoint &pos, QListViewItem *&parent, QListViewItem *&after );
};

class LayerItem: public KListViewItem
{
public:
    LayerItem( const QString &displayName, LayerList *parent, LayerItem *after = 0, int id = -1 );
    LayerItem( const QString &displayName, LayerItem *parent, LayerItem *after = 0, int id = -1 );
    virtual ~LayerItem();

    void makeFolder();
    bool isFolder() const;

    int id() const;

    QString displayName() const;
    void setDisplayName( const QString &displayName );

    bool isActive() const;
    void setActive();

    bool property( const QString &name ) const;
    void setProperty( const QString &name, bool on );
    void toggleProperty( const QString &name );

    void setPreviewImage( QImage *image );
    void setPreviewPixmap( QPixmap *pixmap );
    void previewChanged();

    LayerItem *addLayer( const QString &displayName, LayerItem *after = 0, int id = -1 );

    LayerItem *prevSibling() const;

public: //convenience
    LayerItem *nextSibling() const;
    LayerList *listView() const;
    LayerItem *firstChild() const;
    LayerItem *parent() const;
    void update() const; //like QWidget::update()

protected:
    virtual QRect rect() const;

    int    mapXFromListView( int x ) const;
    int    mapYFromListView( int y ) const;
    QPoint mapFromListView( const QPoint &point ) const;
    QRect  mapFromListView( const QRect &rect ) const;

    int    mapXToListView( int x ) const;
    int    mapYToListView( int y ) const;
    QPoint mapToListView( const QPoint &point ) const;
    QRect  mapToListView( const QRect &rect ) const;

    virtual QRect textRect() const;
    virtual QRect iconsRect() const;
    virtual QRect previewRect() const;

    virtual void drawText( QPainter *p, const QColorGroup &cg, const QRect &r );
    virtual void drawIcons( QPainter *p, const QColorGroup &cg, const QRect &r );
    virtual void drawPreview( QPainter *p, const QColorGroup &cg, const QRect &r );

    bool multiline() const;
    bool showPreview() const;
    virtual QFont font() const;
    QFontMetrics fontMetrics() const;

    virtual bool mousePressEvent( QMouseEvent *e );

private:
    typedef KListViewItem super;
    friend class LayerList;

    void init();

    class Private;
    Private* const d;

public: //reimplemented for internal reasons
    virtual int width( const QFontMetrics &fm, const QListView *lv, int c ) const;
    virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );
    virtual void setup();
};

class LayerFolder: public LayerItem
{
public:
    LayerFolder( const QString &displayName, LayerList *parent, LayerItem *after = 0, int id = -1 )
        : LayerItem( displayName, parent, after, id ) { makeFolder(); }
    LayerFolder( const QString &displayName, LayerItem *parent, LayerItem *after = 0, int id = -1 )
        : LayerItem( displayName, parent, after, id ) { makeFolder(); }
};


#endif

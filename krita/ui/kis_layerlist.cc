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

#include <QImage>
//Added by qt3to4:
#include <Q3ValueList>

#include <kaboutdata.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kinstance.h>

#include <KoPartSelectAction.h>

#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_part_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"
#include "kis_layerlist.h"


KisLayerList::KisLayerList( QWidget *parent, const char *name )
    : super( parent, name )
{
    m_partLayerAction = new KoPartSelectAction( i18n( "New &Object Layer" ), "gear"/*, this - KDE4*/);
}

static const int ADJUSTMENT_LAYER = 5384; //hack?

void KisLayerList::constructMenu( LayerItem *layer )
{
    super::constructMenu( layer );

    contextMenu()->removeItem( MenuItems::NewLayer );
    contextMenu()->removeItem( MenuItems::NewFolder );
    contextMenu()->changeItem( MenuItems::RemoveLayer, i18n( "&Remove Layer" ) );

    if( layer )
    {
        static KMenu submenu;
        submenu.clear();
        submenu.insertItem( SmallIconSet( "file" ), i18n( "&Layer..." ), MenuItems::NewLayer );
        submenu.insertItem( SmallIconSet( "folder" ), i18n( "&Group Layer..." ), MenuItems::NewFolder );
        submenu.insertItem( SmallIconSet( "tool_filter" ), i18n( "&Adjustment Layer..." ), ADJUSTMENT_LAYER );
        m_partLayerAction->setText( i18n( "&Object Layer" ) );
        m_partLayerAction->plug( &submenu );

        contextMenu()->insertItem( SmallIconSet( "filenew" ), i18n( "&New" ), &submenu );
    }
    else
    {
        contextMenu()->insertItem( SmallIconSet( "filenew" ), i18n( "&New Layer..." ), MenuItems::NewLayer );
        contextMenu()->insertItem( SmallIconSet( "folder" ), i18n( "New &Group Layer..." ), MenuItems::NewFolder );
        contextMenu()->insertItem( SmallIconSet( "tool_filter" ), i18n( "New &Adjustment Layer..." ), ADJUSTMENT_LAYER );
        m_partLayerAction->setText( i18n( "New &Object Layer" ) );
        m_partLayerAction->plug( contextMenu() );
    }
}

void KisLayerList::menuActivated( int id, LayerItem *layer )
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
        case ADJUSTMENT_LAYER:
            emit requestNewAdjustmentLayer( parent, after );
            emit requestNewAdjustmentLayer( parent ? parent->id() : -1, after ? after->id() : -1 );
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
                super::menuActivated( id, layer );
            else if( id != -1 ) //object layer was selected
            {
                emit requestNewObjectLayer( parent, after, m_partLayerAction->documentEntry() );
                emit requestNewObjectLayer( parent ? parent->id() : -1, after ? after->id() : -1, m_partLayerAction->documentEntry() );
            }
    }
}

KisLayerItem::KisLayerItem( LayerList* parent, KisLayer* layer )
    : super( layer->name(),
             parent,
             layer->prevSibling() ? parent->layer( layer->prevSibling()->id() ) : 0,
             layer->id() )
    , m_layer( layer )
{
    init();
}

KisLayerItem::KisLayerItem( LayerItem* parent, KisLayer* layer )
    : super( layer->name(),
             parent,
             layer->prevSibling() ? parent->listView()->layer( layer->prevSibling()->id() ) : 0,
             layer->id() )
    , m_layer( layer )
{
    init();
}

void KisLayerItem::init()
{
    setPreviewImage( &m_preview );
    sync();
}

KisLayer* KisLayerItem::layer() const
{
    return m_layer;
}

void KisLayerItem::sync()
{
    setProperty( "visible", layer()->visible() );
    setProperty( "locked", layer()->locked() );
    setDisplayName( layer()->name() );
    update();
}

bool KisLayerItem::updatePreview()
{
    m_preview = m_layer->createThumbnail( height()*2, height()*2 );
    m_preview = m_preview.convertToFormat(QImage::Format_ARGB32);
    previewChanged();
    return !m_preview.isNull();
}

QString KisLayerItem::tooltip() const
{
    QString text = super::tooltip();
    text = text.left( text.length() - 8 ); //HACK -- strip the </table>
    QString row = "<tr><td>%1</td><td>%2</td></tr>";
    text += row.arg( i18n( "Opacity:" ) ).arg( "%1%" ).arg( int( float( m_layer->opacity() * 100 ) / 255 + 0.5 ) );
    text += row.arg( i18n( "Composite mode:" ) ).arg( m_layer->compositeOp().id().name() );
    if( KisPaintLayer *player = dynamic_cast<KisPaintLayer*>( m_layer ) )
    {
        text += row.arg( i18n( "Colorspace:" ) ).arg( player->paintDevice()->colorSpace()->id().name() );
        if( KoColorProfile *profile = player->paintDevice()->colorSpace()->getProfile() )
            text += row.arg( i18n( "Profile:" ) ).arg( profile->productName() );
    }
    if( KisAdjustmentLayer *alayer = dynamic_cast<KisAdjustmentLayer*>( m_layer ) )
        text += row.arg( i18n( "Filter: " ) ).arg( KisFilterRegistry::instance()->get( alayer->filter()->name() )->id().name() );
    if( KisPartLayerImpl *player = dynamic_cast<KisPartLayerImpl*>( m_layer ) ) {
        QString type = player->docType();

        if( type.isEmpty() ) {
            type = player->childDoc()->document()->instance()->aboutData()->programName();
        }

        text += row.arg( i18n( "Document type: " ) ).arg( type );
    }
    text += "</table>";

    return text;
}

QImage KisLayerItem::tooltipPreview() const
{
    QImage img = m_layer->createThumbnail( 400, 400 );
    if( img.isNull() )
        return img; //so Qt doesn't complain
    img = img.convertToFormat(QImage::Format_ARGB32);
    const int size = qMin( 200, qMax( img.width(), img.height() ) );
    return img.scaled( size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
}

//void KisLayerItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );

#include "kis_layerlist.moc"

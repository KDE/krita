/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPADocumentModel.h"

#include "KoPADocument.h"
#include "KoPAPageBase.h"
#include "KoPAPageProvider.h"
#include <KoShapePainter.h>
#include <KoShapeManager.h>
#include <KoShapeBorderModel.h>
#include <KoShapeContainer.h>
#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoShapeControllerBase.h>
#include <KoSelection.h>
#include <KoShapeLayer.h>
#include <KoShapeGroup.h>
#include <KoShapeGroupCommand.h>
#include <KoShapeUngroupCommand.h>
#include <KoShapeRenameCommand.h>
#include <KoZoomHandler.h>

#include <klocale.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMimeData>

#include "commands/KoPAPageMoveCommand.h"

KoPADocumentModel::KoPADocumentModel( QObject* parent, KoPADocument *document )
: KoDocumentSectionModel( parent )
, m_master(false)
, m_lastContainer( 0 )
{
    setDocument( document );
    setSupportedDragActions( Qt::MoveAction );
}

void KoPADocumentModel::update()
{
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

int KoPADocumentModel::rowCount( const QModelIndex &parent ) const
{
    if(!m_document)
        return 0;

    // check if parent is root node
    if( ! parent.isValid() )
        return m_document->pages(m_master).count();

    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    KoShapeContainer *parentShape = dynamic_cast<KoShapeContainer*>( (KoShape*)parent.internalPointer() );
    if( ! parentShape )
        return 0;

    return parentShape->shapeCount();
}

int KoPADocumentModel::columnCount( const QModelIndex & ) const
{
    return 1;
}

QModelIndex KoPADocumentModel::index( int row, int column, const QModelIndex &parent ) const
{
    if( !m_document )
        return QModelIndex();
    
    // check if parent is root node
    if( ! parent.isValid() )
    {
        if( row >= 0 && row < m_document->pages(m_master).count() )
            return createIndex( row, column, m_document->pages(m_master).at(row) );
        else
            return QModelIndex();
    }

    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    KoShapeContainer *parentShape = dynamic_cast<KoShapeContainer*>( (KoShape*)parent.internalPointer() );
    if( ! parentShape )
        return QModelIndex();

    if( row < parentShape->shapeCount() )
        return createIndex( row, column, childFromIndex( parentShape, row ) );
    else
        return QModelIndex();
}

QModelIndex KoPADocumentModel::parent( const QModelIndex &child ) const
{
    // check if child is root node
    if( ! child.isValid() || !m_document )
        return QModelIndex();

    Q_ASSERT(child.model() == this);
    Q_ASSERT(child.internalPointer());

    KoShape *childShape = static_cast<KoShape*>( child.internalPointer() );
    if( ! childShape )
        return QModelIndex();

    // get the children's parent shape
    KoShapeContainer *parentShape = childShape->parent();
    if( ! parentShape )
        return QModelIndex();

    // get the grandparent to determine the row of the parent shape
    KoShapeContainer *grandParentShape = parentShape->parent();
    if( ! grandParentShape )
    {
        KoPAPageBase* page = dynamic_cast<KoPAPageBase*>( parentShape);
        return createIndex( m_document->pages(m_master).indexOf( page ), 0, parentShape );
    }

    return createIndex( indexFromChild( grandParentShape, parentShape ), 0, parentShape );
}

QVariant KoPADocumentModel::data( const QModelIndex &index, int role ) const
{
    if( ! index.isValid() || !m_document )
        return QVariant();

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KoShape *shape = static_cast<KoShape*>( index.internalPointer() );

    switch (role)
    {
        case Qt::DisplayRole:
        {
            QString name = shape->name();
            if( name.isEmpty() )
            {
                if ( dynamic_cast<KoPAPageBase *>( shape ) ) {
                    if(m_document->pageType() == KoPageApp::Slide ) {
                        name = i18n("Slide %1",  m_document->pageIndex(dynamic_cast<KoPAPageBase *>(shape)) + 1);
                    } else {
                        name = i18n("Page  %1", m_document->pageIndex(dynamic_cast<KoPAPageBase *>(shape)) + 1);
                    }
                }
                else if( dynamic_cast<KoShapeLayer*>( shape ) ) {
                    name = i18n("Layer") + QString(" (%1)").arg(shape->zIndex());
                }
                else if( dynamic_cast<KoShapeGroup*>( shape ) ) {
                    name = i18n("Group") + QString(" (%1)").arg(shape->zIndex());
                }
                else {
                    name = i18n("Shape") + QString(" (%1)").arg(shape->zIndex());
                }
            }
            return name;
        }
        case Qt::DecorationRole: return QVariant();//return shape->icon();
        case Qt::EditRole: return shape->name();
        case Qt::SizeHintRole:
        {
            KoPAPageBase *page = dynamic_cast<KoPAPageBase*>(shape);
            if (page) { // return actual page size for page
                KoPageLayout layout = page->pageLayout();
                return QSize(layout.width, layout.height);
            }
            else
                return shape->size();
        }
        case ActiveRole:
        {
            KoCanvasController * canvasController = KoToolManager::instance()->activeCanvasController();
            KoSelection * selection = canvasController->canvas()->shapeManager()->selection();
            if( ! selection )
                return false;

            /* KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>( shape );
            if( layer )
                return (layer == selection->activeLayer() );
            else */
            return selection->isSelected( shape );
        }
        case PropertiesRole: return QVariant::fromValue( properties( shape ) );
        case AspectRatioRole:
        {
            QTransform matrix = shape->absoluteTransformation( 0 );
            QRectF bbox = matrix.mapRect( shape->outline().boundingRect() );
            KoShapeContainer *container = dynamic_cast<KoShapeContainer*>( shape );
            if( container )
            {
                bbox = QRectF();
                foreach( KoShape* shape, container->shapes() )
                    bbox = bbox.united( shape->outline().boundingRect() );
            }
            return qreal(bbox.width()) / bbox.height();
        }
        default:
            if (role >= int(BeginThumbnailRole))
                return createThumbnail( shape, QSize( role - int(BeginThumbnailRole), role - int(BeginThumbnailRole) ) );
            else
                return QVariant();
    }
}

Qt::ItemFlags KoPADocumentModel::flags(const QModelIndex &index) const
{
    if( !m_document )
        return 0;

    if( ! index.isValid() )
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
    //if( dynamic_cast<KoShapeContainer*>( (KoShape*)index.internalPointer() ) )
        flags |= Qt::ItemIsDropEnabled;
    return flags;
}

bool KoPADocumentModel::setData(const QModelIndex &index, const QVariant &value, int role )
{
    if( ! index.isValid() || !m_document )
        return false;

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KoShape *shape = static_cast<KoShape*>( index.internalPointer() );
    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
        {
            QUndoCommand * cmd = new KoShapeRenameCommand( shape, value.toString() );
            // TODO 2.1 use different text for the command if e.g. it is a page/slide or layer
            m_document->addCommand( cmd );
        }   break;
        case PropertiesRole:
            setProperties( shape, value.value<PropertyList>());
            break;
        case ActiveRole:
            /* if (value.toBool())
            {
                KoCanvasController * canvasController = KoToolManager::instance()->activeCanvasController();
                KoSelection * selection = canvasController->canvas()->shapeManager()->selection();

                KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>( shape );
                if( layer && selection ) {
                    selection->setActiveLayer( layer );
                }
            } */
            break;
        default:
            return false;
    }

    emit dataChanged( index, index );
    return true;
}

KoDocumentSectionModel::PropertyList KoPADocumentModel::properties( KoShape* shape ) const
{
    PropertyList l;
    l << Property(i18n("Visible"), SmallIcon("14_layer_visible"), SmallIcon("14_layer_novisible"), shape->isVisible());
    l << Property(i18n("Locked"), SmallIcon("object-locked"), SmallIcon("object-unlocked"), shape->isGeometryProtected());
    return l;
}

void KoPADocumentModel::setProperties( KoShape* shape, const PropertyList &properties )
{
    bool oldVisibleState = shape->isVisible();
    bool oldLockedState = shape->isGeometryProtected();

    shape->setVisible( properties.at( 0 ).state.toBool() );
    shape->setGeometryProtected( properties.at( 1 ).state.toBool() );

    if( ( oldVisibleState != shape->isVisible() ) || ( oldLockedState != shape->isGeometryProtected() ) )
        shape->update();
}

QImage KoPADocumentModel::createThumbnail( KoShape* shape, const QSize &thumbSize ) const
{
    QSize size(thumbSize.width(), thumbSize.height());
    KoShapePainter shapePainter;

    QList<KoShape*> shapes;

    KoPAPageBase *page = dynamic_cast<KoPAPageBase*>(shape);
    if (page) { // We create a thumbnail with actual width / height ratio for page
        KoZoomHandler zoomHandler;
        KoPageLayout layout = page->pageLayout();
        qreal ratio = (zoomHandler.resolutionX() * layout.width) / (zoomHandler.resolutionY() * layout.height);
        if ( ratio > 1 ) {
            size.setHeight( size.width() / ratio );
        }
        else {
            size.setWidth( size.height() * ratio );
        }
        QPixmap pixmap = m_document->pageThumbnail( page, size );
        return pixmap.toImage();
    }

    shapes.append( shape );
    KoShapeContainer * container = dynamic_cast<KoShapeContainer*>( shape );
    if( container )
        shapes = container->shapes();

    shapePainter.setShapes( shapes );

    QImage thumb( size, QImage::Format_RGB32 );
    // draw the background of the thumbnail
    thumb.fill( QColor( Qt::white ).rgb() );
    shapePainter.paint(thumb);

    return thumb;
}

KoShape * KoPADocumentModel::childFromIndex( KoShapeContainer *parent, int row ) const
{
    return parent->shapes().at(row);

    if( parent != m_lastContainer )
    {
        m_lastContainer = parent;
        m_childs = parent->shapes();
        qSort( m_childs.begin(), m_childs.end(), KoShape::compareShapeZIndex );
    }
    return m_childs.at( row );
}

int KoPADocumentModel::indexFromChild( KoShapeContainer *parent, KoShape *child ) const
{
    if ( !m_document )
        return 0;

    return parent->shapes().indexOf( child );

    if( parent != m_lastContainer )
    {
        m_lastContainer = parent;
        m_childs = parent->shapes();
        qSort( m_childs.begin(), m_childs.end(), KoShape::compareShapeZIndex );
    }
    return m_childs.indexOf( child );
}

Qt::DropActions KoPADocumentModel::supportedDropActions () const
{
    return Qt::MoveAction | Qt::CopyAction;
}

QStringList KoPADocumentModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/x-kopalayermodeldatalist");
    return types;
}

QMimeData * KoPADocumentModel::mimeData( const QModelIndexList & indexes ) const
{
    // check if there is data to encode
    if( ! indexes.count() )
        return 0;

    // check if we support a format
    QStringList types = mimeTypes();
    if( types.isEmpty() )
        return 0;

    QMimeData *data = new QMimeData();
    QString format = types[0];
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);

    // encode the data
    QModelIndexList::ConstIterator it = indexes.begin();
    for( ; it != indexes.end(); ++it)
        stream << QVariant::fromValue( qulonglong( it->internalPointer() ) );

    data->setData(format, encoded);
    return data;
}

bool KoPADocumentModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
    Q_UNUSED( row );
    Q_UNUSED( column );

    // check if the action is supported
    if( ! data || action != Qt::MoveAction )
        return false;
    // check if the format is supported
    QStringList types = mimeTypes();
    if( types.isEmpty() )
        return false;
    QString format = types[0];
    if( ! data->hasFormat(format) )
        return false;

    QByteArray encoded = data->data( format );
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    QList<KoShape*> shapes;

    // decode the data
    while( ! stream.atEnd() )
    {
        QVariant v;
        stream >> v;
        shapes.append( static_cast<KoShape*>( (void*)v.value<qulonglong>() ) );
    }

    QList<KoShape*> toplevelShapes;
    QList<KoShapeLayer*> layers;
    QList<KoPAPageBase *> pages;
    // remove shapes having its parent in the list
    // and separate the layers
    foreach( KoShape * shape, shapes )
    {
        // check whether the selection contains page
        // by the UI rules, the selection should contains page only
        KoPAPageBase *page = dynamic_cast<KoPAPageBase *>( shape );
        if ( page ) {
            pages.append( page );
            continue;
        }

        KoShapeContainer *parentShape = shape->parent();
        bool hasParentInList = false;
        while ( parentShape ) {
            if ( shapes.contains( parentShape ) ) {
                hasParentInList = true;
                break;
            }
            parentShape = parentShape->parent();
        }
        if( hasParentInList )
            continue;

        KoShapeLayer * layer = dynamic_cast<KoShapeLayer*>( shape );
        if( layer )
            layers.append( layer );
        else
            toplevelShapes.append( shape );
    }

    // dropping to root, only page(s) is allowed
    if ( !parent.isValid() ) {
        if ( !pages.isEmpty() ) {
            if ( row < 0 ) {
                return false;
            }
            KoPAPageBase *after = ( row != 0 ) ? m_document->pageByIndex( row - 1, false ) : 0;
            KoPAPageMoveCommand *command = new KoPAPageMoveCommand( m_document, pages, after );
            m_document->addCommand( command );
            kDebug(30010) << "KoPADocumentModel::dropMimeData parent = root, dropping page(s) as root, moving page(s)";
            return true;
        }
        else {
            kDebug(30010) << "KoPADocumentModel::dropMimeData parent = root, dropping non-page as root, returning false";
            return false;
        }
    }

    KoShape *shape = static_cast<KoShape*>( parent.internalPointer() );
    KoShapeContainer * container = dynamic_cast<KoShapeContainer*>( shape );
    if( container ) {
        KoShapeGroup * group = dynamic_cast<KoShapeGroup*>( container );
        if( group )
        {
            kDebug(30010) <<"KoPADocumentModel::dropMimeData parent = group";
            if( ! toplevelShapes.count() )
                return false;

            emit layoutAboutToBeChanged();
            beginInsertRows( parent, group->shapeCount(), group->shapeCount()+toplevelShapes.count() );

            QUndoCommand * cmd = new QUndoCommand();
            cmd->setText( i18n("Reparent shapes") );

            foreach( KoShape * shape, toplevelShapes )
                new KoShapeUngroupCommand( shape->parent(), QList<KoShape*>() << shape, QList<KoShape*>(), cmd );

            new KoShapeGroupCommand( group, toplevelShapes, cmd );
            KoCanvasController * canvasController = KoToolManager::instance()->activeCanvasController();
            canvasController->canvas()->addCommand( cmd );

            endInsertRows();
            emit layoutChanged();
        }
        else
        {
            kDebug(30010) <<"KoPADocumentModel::dropMimeData parent = container";
            if( toplevelShapes.count() )
            {
                emit layoutAboutToBeChanged();
                beginInsertRows( parent, container->shapeCount(), container->shapeCount()+toplevelShapes.count() );

                QUndoCommand * cmd = new QUndoCommand();
                cmd->setText( i18n("Reparent shapes") );

                QList<bool> clipped;
                QList<bool> inheritsTransform;
                foreach( KoShape * shape, toplevelShapes )
                {
                    if( ! shape->parent() )
                    {
                        clipped.append( false );
                        inheritsTransform.append(false);
                        continue;
                    }

                    clipped.append( shape->parent()->isClipped( shape ) );
                    inheritsTransform.append(shape->parent()->inheritsTransform(shape));
                    new KoShapeUngroupCommand( shape->parent(), QList<KoShape*>() << shape, QList<KoShape*>(), cmd );
                }
                // shapes are dropped on a container, so add them to the container
                new KoShapeGroupCommand(container, toplevelShapes, clipped, inheritsTransform, cmd);
                KoCanvasController * canvasController = KoToolManager::instance()->activeCanvasController();
                canvasController->canvas()->addCommand( cmd );

                endInsertRows();
                emit layoutChanged();
            }
            else if( layers.count() )
            {
                KoShapeLayer * layer = dynamic_cast<KoShapeLayer*>( container );
                if( ! layer )
                    return false;

                // TODO layers are dropped on a layer, so change layer ordering
                return false;
            }
        }
    }
    else
    {
        kDebug(30010) <<"KoPADocumentModel::dropMimeData parent = shape";
        if( ! toplevelShapes.count() )
            return false;

        // TODO shapes are dropped on a shape, reorder them
        return false;
    }

    return true;
}

QModelIndex KoPADocumentModel::parentIndexFromShape( const KoShape * child )
{
    if( !m_document )
        return QModelIndex();

    // check if child shape is a layer, and return invalid model index if it is
    const KoShapeLayer *childlayer = dynamic_cast<const KoShapeLayer*>( child );
    if( childlayer )
        return QModelIndex();

    // get the children's parent shape
    KoShapeContainer *parentShape = child->parent();
    if( ! parentShape )
        return QModelIndex();

    // check if the parent is a layer
    KoShapeLayer *parentLayer = dynamic_cast<KoShapeLayer*>( parentShape );


    if( parentLayer ) {
        KoPAPageBase * page = dynamic_cast<KoPAPageBase*>( parentLayer->parent() );
        if ( page )
            return createIndex( m_document->pages(m_master).count() - 1 - m_document->pages(m_master).indexOf( page ), 0, parentLayer );
    }
    // get the grandparent to determine the row of the parent shape
    KoShapeContainer *grandParentShape = parentShape->parent();
    if( ! grandParentShape )
        return QModelIndex();

    return createIndex( indexFromChild( grandParentShape, parentShape ), 0, parentShape );
}

void KoPADocumentModel::setDocument( KoPADocument* document )
{
    m_document = document;

    if ( m_document )
    {
        connect( m_document, SIGNAL(pageAdded( KoPAPageBase* ) ), this, SLOT( update() ) );
        connect( m_document, SIGNAL(pageRemoved( KoPAPageBase* ) ), this, SLOT( update() ) );
        connect( m_document, SIGNAL(shapeAdded( KoShape* ) ), this, SLOT( update() ) );
        connect( m_document, SIGNAL(shapeRemoved( KoShape* ) ), this, SLOT( update() ) );
    }

    reset();
}

void KoPADocumentModel::setMasterMode(bool master)
{
    m_master = master;
    update(); // Rebuild the model
}

#include <KoPADocumentModel.moc>

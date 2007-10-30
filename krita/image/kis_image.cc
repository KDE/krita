/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_image.h"
#include <config-endian.h> // WORDS_BIGENDIAN

#include <stdlib.h>
#include <math.h>

#include <lcms.h>

#include <QImage>
#include <QPainter>
#include <QSize>
#include <QApplication>
#include <QThread>
#include <QDateTime>
#include <QRect>
#include <QRegion>

#include <klocale.h>

#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorConversionTransformation.h"
#include "colorprofiles/KoIccColorProfile.h"

#include "kis_action_recorder.h"
#include "kis_adjustment_layer.h"
#include "kis_annotation.h"
#include "kis_change_profile_visitor.h"
#include "kis_colorspace_convert_visitor.h"
#include "kis_count_visitor.h"
#include "kis_extent_visitor.h"
#include "kis_filter_strategy.h"
#include "kis_group_layer.h"
#include "kis_image_commands.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_merge_visitor.h"

#include "kis_nameserver.h"
#include "kis_paint_device.h"
#include "kis_paint_device_action.h"
#include "kis_paint_layer.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_perspective_grid.h"
#include "kis_projection.h"
#include "kis_selection.h"
#include "kis_shear_visitor.h"
#include "kis_transaction.h"
#include "kis_transform_visitor.h"
#include "kis_types.h"
#include "kis_crop_visitor.h"

class KisImage::KisImagePrivate {
public:
    KoColor backgroundColor;
    quint32 lockCount;
    bool sizeChangedWhileLocked;
    bool selectionChangedWhileLocked;
    KisPerspectiveGrid* perspectiveGrid;

    KUrl uri;

    qint32 width;
    qint32 height;

    double xres;
    double yres;

    KoUnit unit;

    KoColorSpace * colorSpace;

    KisGroupLayerSP rootLayer; // The layers are contained in here
    QList<KisLayer*> dirtyLayers; // for thumbnails

    KisProjectionSP projection;

    KisNameServer *nserver;
    KisUndoAdapter *adapter;
    KisActionRecorder *recorder;

    vKisAnnotationSP annotations;

    KisSelectionSP globalSelection; // XXX_SELECTION: Use the selection from the root layer for this

};




KisImage::KisImage(KisUndoAdapter *adapter, qint32 width, qint32 height,  KoColorSpace * colorSpace, const QString& name)
    : QObject(0)
    , KisShared()
    , m_d( new KisImagePrivate() )
{
    setObjectName(name);
    init(adapter, width, height, colorSpace);
}

KisImage::KisImage(const KisImage& rhs)
    : QObject()
    , KisNodeFacade( rhs )
    , KisNodeGraphListener( rhs )
    , KisShared( rhs )
    , m_d( new KisImagePrivate() )
{
    if (this != &rhs) {

        if ( rhs.m_d->perspectiveGrid )
            m_d->perspectiveGrid = new KisPerspectiveGrid(*rhs.m_d->perspectiveGrid);
        else
            m_d->perspectiveGrid = 0;

        m_d->uri = rhs.m_d->uri;
        m_d->width = rhs.m_d->width;
        m_d->height = rhs.m_d->height;
        m_d->xres = rhs.m_d->xres;
        m_d->yres = rhs.m_d->yres;
        m_d->unit = rhs.m_d->unit;
        m_d->colorSpace = rhs.m_d->colorSpace;
        m_d->adapter = rhs.m_d->adapter;
        m_d->globalSelection = 0;
        m_d->projection = new KisProjection( this );
        setRootLayer( static_cast<KisGroupLayer*>(rhs.m_d->rootLayer->clone().data()) );
        m_d->annotations = rhs.m_d->annotations; // XXX the annotations would probably need to be deep-copied



        m_d->nserver = new KisNameServer(rhs.m_d->nserver->currentSeed() + 1);
        Q_CHECK_PTR(m_d->nserver);

    }
}



KisImage::~KisImage()
{
    delete m_d->perspectiveGrid;
    delete m_d->nserver;
    delete m_d;
}


void KisImage::aboutToAddANode( KisNode *parent, int index )
{
    emit sigAboutToAddANode( parent, index );
}

void KisImage::nodeHasBeenAdded( KisNode *parent, int index )
{
    // XXX: Temporarily for compatibility

    KisLayer * layer = dynamic_cast<KisLayer*>( parent->at( index ).data() );
    if ( layer ) {

        KisPaintLayerSP player = KisPaintLayerSP( dynamic_cast<KisPaintLayer*>( layer ) );
        if (!player.isNull()) {
            // XXX: This should also be done whenever a layer grows!
            QList<KisPaintDeviceAction *> actions =
                KoColorSpaceRegistry::instance()->paintDeviceActionsFor(player->paintDevice()->colorSpace());
            for (int i = 0; i < actions.count(); i++) {
                actions.at(i)->act(player.data()->paintDevice(), width(), height());
            }

        }

        // The addition of temporary layers is not interesting
        KoProperties props;
        props.setProperty( "temporary", false );

        if ( !layer->check( props ) ) {
            emit sigLayerMoved( layer );
        }
    }
    emit sigNodeHasBeenAdded( parent, index );
}

void KisImage::aboutToRemoveANode( KisNode *parent, int index )
{
    emit sigAboutToRemoveANode( parent, index );
}

void KisImage::nodeHasBeenRemoved( KisNode *parent, int index )
{
    // XXX: Temporarily for compatibility
    emit sigNodeHasBeenRemoved( parent, index );

    KisLayer * l = dynamic_cast<KisLayer*>( parent->at( index ).data() );
    if ( l ) {
        // The removal of temporary layers is not interesting
        KoProperties props;
        props.setProperty( "temporary", false );

        if ( !l->check( props ) ) {
            emit sigLayerRemoved( l );
        }
    }
}

void KisImage::aboutToMoveNode( KisNode * parent, int oldIndex, int newIndex )
{
    Q_UNUSED( parent );
    Q_UNUSED( oldIndex );
    Q_UNUSED( newIndex );
}

void KisImage::nodeHasBeenMoved( KisNode * parent, int oldIndex, int newIndex )
{
    Q_UNUSED(oldIndex);

    KisLayer * l = dynamic_cast<KisLayer*>( parent->at( newIndex ).data() );
    if ( l ) {

        // The moving of temporary layers is not interesting

        KoProperties props;
        props.setProperty( "temporary", false );

        if ( !l->check( props ) ) {

            emit sigLayerMoved( l );
        }
    }
}


KisSelectionSP KisImage::globalSelection() const
{
    return m_d->globalSelection;
}

void KisImage::setGlobalSelection( KisSelectionSP globalSelection )
{
    if ( globalSelection == 0 )
        m_d->globalSelection = new KisSelection( m_d->rootLayer->projection() );
    else
        m_d->globalSelection = globalSelection;
}

void KisImage::removeGlobalSelection() {
    m_d->globalSelection = 0;
}

KoColor KisImage::backgroundColor() const
{
    return m_d->backgroundColor;
}

void KisImage::setBackgroundColor(const KoColor & color)
{
    m_d->backgroundColor = color;
}


QString KisImage::nextLayerName() const
{
    if (m_d->nserver->currentSeed() == 0) {
        m_d->nserver->number();
        return i18n("background");
    }

    return i18n("Layer %1", m_d->nserver->number());
}

void KisImage::rollBackLayerName()
{
    m_d->nserver->rollback();
}

void KisImage::init(KisUndoAdapter *adapter, qint32 width, qint32 height,  KoColorSpace * colorSpace)
{
    if (colorSpace == 0) {
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    }

    m_d->backgroundColor = KoColor(Qt::white, colorSpace);
    m_d->lockCount = 0;
    m_d->sizeChangedWhileLocked = false;
    m_d->selectionChangedWhileLocked = false;
    m_d->perspectiveGrid = 0;

    m_d->adapter = adapter;

    m_d->nserver = new KisNameServer(1);

    m_d->colorSpace = colorSpace;
    m_d->projection = new KisProjection( this );
    setRootLayer( new KisGroupLayer(this, "root", OPACITY_OPAQUE) );

    m_d->xres = 1.0;
    m_d->yres = 1.0;
    m_d->unit = KoUnit::Point;
    m_d->width = width;
    m_d->height = height;

    m_d->recorder = new KisActionRecorder(this);
}

bool KisImage::locked() const
{
    return m_d->lockCount != 0;
}

void KisImage::lock()
{
    if (!locked()) {
        if (m_d->projection)
            m_d->projection->lock();
        m_d->sizeChangedWhileLocked = false;
        m_d->selectionChangedWhileLocked = false;
        blockSignals( true );
    }
    m_d->lockCount++;
}

void KisImage::unlock()
{
    Q_ASSERT(locked());

    if (locked()) {
        m_d->lockCount--;

        if (m_d->lockCount == 0) {
            blockSignals( false );

            if (m_d->sizeChangedWhileLocked) {
                emit sigSizeChanged(m_d->width, m_d->height);
            }

            if (m_d->selectionChangedWhileLocked) {
                emit sigActiveSelectionChanged(KisImageSP(this));
            }

            if (m_d->projection)
                m_d->projection->unlock();

        }
    }
}

void KisImage::notifyLayerUpdated(KisLayerSP layer)
{
    // Add the layer to the list of layers that need to be
    // rescanned for the thumbnails in the layerbox
    KisLayer *l = layer.data();
    while( l )
    {
        if( !m_d->dirtyLayers.contains( l ) )
            m_d->dirtyLayers.append( l );
        l = dynamic_cast<KisLayer*>( l->parent().data() );
    }
}


void KisImage::resize(const QRect& rc, bool cropLayers)
{
    resize(rc.width(), rc.height(), rc.x(), rc.y(), cropLayers);
}

void KisImage::resize(qint32 w, qint32 h, qint32 x, qint32 y, bool cropLayers)
{
    if (w != width() || h != height()) {

        lock();

        if (undo()) {
            if (cropLayers)
                m_d->adapter->beginMacro(i18n("Crop Image"));
            else
                m_d->adapter->beginMacro(i18n("Resize Image"));

            m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), true));
            m_d->adapter->addCommand(new KisImageResizeCommand(KisImageSP(this), w, h, width(), height()));
        }

        m_d->width = w;
        m_d->height = h;

        if (cropLayers) {
            KisCropVisitor v(QRect(x, y, w, h), m_d->adapter);
            m_d->rootLayer->accept(v);
        }

        emitSizeChanged();

        unlock();

        if (undo()) {
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), false));
            m_d->adapter->endMacro();
        }
    }
}


void KisImage::emitSizeChanged()
{
    if (!locked()) {
        emit sigSizeChanged(m_d->width, m_d->height);
    } else {
        m_d->sizeChangedWhileLocked = true;
    }
}


void KisImage::scale(double sx, double sy, KoUpdater *progress, KisFilterStrategy *filterStrategy)
{
    QStringList list;
    list << "KisLayer";

    KisCountVisitor visitor(list, KoProperties());
    m_d->rootLayer->accept( visitor );

    if (visitor.count() == 0) return; // Nothing to scale

    // New image size. XXX: Pass along to discourage rounding errors?
    qint32 w, h;
    w = (qint32)(( width() * sx) + 0.5);
    h = (qint32)(( height() * sy) + 0.5);

    if (w != width() || h != height()) {

        lock();

        if (undo()) {
            m_d->adapter->beginMacro(i18n("Scale Image"));
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), true));
        }

        {
            KisTransformVisitor visitor (KisImageSP(this), sx, sy, 0.0, 0.0, 0.0, 0, 0, progress, filterStrategy);
            m_d->rootLayer->accept(visitor);
        }

        if (undo()) {
            m_d->adapter->addCommand(new KisImageResizeCommand(KisImageSP(this), w, h, width(), height()));
        }

        m_d->width = w;
        m_d->height = h;

        emitSizeChanged();

        unlock();

        if (undo()) {
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), false));
            m_d->adapter->endMacro();
        }
    }
}



void KisImage::rotate(double radians, KoUpdater *progress)
{
    lock();

    qint32 w = width();
    qint32 h = height();
    qint32 tx = qint32((w*cos(radians) - h*sin(radians) - w) / 2 + 0.5);
    qint32 ty = qint32((h*cos(radians) + w*sin(radians) - h) / 2 + 0.5);
    w = (qint32)(width()*QABS(cos(radians)) + height()*QABS(sin(radians)) + 0.5);
    h = (qint32)(height()*QABS(cos(radians)) + width()*QABS(sin(radians)) + 0.5);

    tx -= (w - width()) / 2;
    ty -= (h - height()) / 2;

    if (undo()) {
        m_d->adapter->beginMacro(i18n("Rotate Image"));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), true));
    }

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value("Triangle");
    KisTransformVisitor visitor (KisImageSP(this), 1.0, 1.0, 0, 0, radians, -tx, -ty, progress, filter);
    m_d->rootLayer->accept(visitor);

    if (undo()) m_d->adapter->addCommand(new KisImageResizeCommand(KisImageSP(this), w, h, width(), height()));

    m_d->width = w;
    m_d->height = h;

    emitSizeChanged();

    unlock();

    if (undo()) {
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), false));
        m_d->adapter->endMacro();
    }
}

void KisImage::shear(double angleX, double angleY, KoUpdater *progress)
{
    const double pi=3.1415926535897932385;

    //new image size
    qint32 w=width();
    qint32 h=height();


    if(angleX != 0 || angleY != 0){
        double deltaY=height()*QABS(tan(angleX*pi/180)*tan(angleY*pi/180));
        w = (qint32) ( width() + QABS(height()*tan(angleX*pi/180)) );
        //ugly fix for the problem of having two extra pixels if only a shear along one
        //axis is done. This has to be fixed in the cropping code in KisRotateVisitor!
        if (angleX == 0 || angleY == 0)
            h = (qint32) ( height() + QABS(w*tan(angleY*pi/180)) );
        else if (angleX > 0 && angleY > 0)
            h = (qint32) ( height() + QABS(w*tan(angleY*pi/180))- 2 * deltaY + 2 );
        else if (angleX < 0 && angleY < 0)
            h = (qint32) ( height() + QABS(w*tan(angleY*pi/180))- 2 * deltaY + 2 );
        else
            h = (qint32) ( height() + QABS(w*tan(angleY*pi/180)) );
    }

    if (w != width() || h != height()) {

        lock();

        if (undo()) {
            m_d->adapter->beginMacro(i18n("Shear Image"));
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), true));
        }

        KisShearVisitor v(angleX, angleY, progress);
        rootLayer()->accept(v);

        if (undo()) m_d->adapter->addCommand(new KisImageResizeCommand(KisImageSP(this), w, h, width(), height()));

        m_d->width = w;
        m_d->height = h;

        emitSizeChanged();

        unlock();

        if (undo()) {
            m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), false));
            m_d->adapter->endMacro();
        }
    }
}

void KisImage::convertTo(KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent)
{
    if ( m_d->colorSpace == dstColorSpace )
    {
        return;
    }

    lock();

    KoColorSpace * oldCs = m_d->colorSpace;

    if (undo()) {
        m_d->adapter->beginMacro(i18n("Convert Image Type"));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), true));
    }

    setColorSpace(dstColorSpace);

    KoColorSpaceConvertVisitor visitor(dstColorSpace, renderingIntent);
    m_d->rootLayer->accept(visitor);

    unlock();

    if (undo()) {

        m_d->adapter->addCommand(new KisImageConvertTypeCommand(KisImageSP(this), oldCs, dstColorSpace));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), false));
        m_d->adapter->endMacro();
    }
}

KoColorProfile * KisImage::profile() const
{
    return colorSpace()->profile();
}

void KisImage::setProfile(const KoColorProfile * profile)
{
    if (profile == 0) return;

    KoColorSpace * dstCs= KoColorSpaceRegistry::instance()->colorSpace( colorSpace()->id(),
                                                                                         profile);
    if (dstCs) {

        lock();

        KoColorSpace * oldCs = colorSpace();
        setColorSpace(dstCs);
        emit(sigProfileChanged(const_cast<KoColorProfile *>(profile)));

        KisChangeProfileVisitor visitor(oldCs, dstCs);
        m_d->rootLayer->accept(visitor);

        unlock();
    }
}

double KisImage::xRes() const
{
    return m_d->xres;
}

double KisImage::yRes() const
{
    return m_d->yres;
}

void KisImage::setResolution(double xres, double yres)
{
    m_d->xres = xres;
    m_d->yres = yres;
}

QPointF KisImage::documentToPixel(const QPointF &documentCoord) const
{
    return QPointF(documentCoord.x() * xRes(), documentCoord.y() * yRes());
}

QPoint KisImage::documentToIntPixel(const QPointF &documentCoord) const
{
    QPointF pixelCoord = documentToPixel(documentCoord);
    return QPoint((int)pixelCoord.x(), (int)pixelCoord.y());
}

QRectF KisImage::documentToPixel(const QRectF &documentRect) const
{
    return QRectF(documentToPixel(documentRect.topLeft()), documentToPixel(documentRect.bottomRight()));
}

QRect KisImage::documentToIntPixel(const QRectF &documentRect) const
{
    return documentToPixel(documentRect).toAlignedRect();
}

QPointF KisImage::pixelToDocument(const QPointF &pixelCoord) const
{
    return QPointF(pixelCoord.x() / xRes(), pixelCoord.y() / yRes());
}

QPointF KisImage::pixelToDocument(const QPoint &pixelCoord) const
{
    return QPointF((pixelCoord.x() + 0.5) / xRes(), (pixelCoord.y() + 0.5) / yRes());
}

qint32 KisImage::width() const
{
    return m_d->width;
}

qint32 KisImage::height() const
{
    return m_d->height;
}

KisGroupLayerSP KisImage::rootLayer() const
{
    Q_ASSERT( m_d->rootLayer );
    return m_d->rootLayer;
}

KisPaintDeviceSP KisImage::projection()
{
    Q_ASSERT( m_d->rootLayer );
    Q_ASSERT( m_d->rootLayer->projection() );
    return m_d->rootLayer->projection();
}

bool KisImage::addLayer(KisLayerSP layer, KisGroupLayerSP group)
{
    // XXX: Temporarily for compatibility
    if ( group )
        return addNode( layer.data(), group.data() );
    else {
        KisNode * root = rootLayer().data();
        return addNode( layer, root );
    }
}

bool KisImage::addLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis)
{
    if ( parent && aboveThis )
        return addNode( layer.data(), parent.data(), aboveThis.data() );
    else if ( parent )
        return addNode( layer.data(), parent.data() );
    else
        return addNode( layer.data(), rootLayer().data() );
}

bool KisImage::addLayer( KisLayerSP layer,  KisGroupLayerSP parent, int index )
{
    if ( parent )
        return addNode( layer.data(), parent.data(), index );
    else
        return addNode( layer.data(), rootLayer().data(), index );
}


bool KisImage::removeLayer(KisLayerSP layer)
{
    if (KisGroupLayerSP parent = dynamic_cast<KisGroupLayer*>( layer->parent().data() ) ) {

        KisNodeSP wasAbove = layer->nextSibling();
        KisNodeSP wasBelow = layer->prevSibling();

        const bool success = removeNode( layer.data() );

        if (success) {
            layer->setImage(0);
            if (!layer->temporary() && undo()) {
                m_d->adapter->addCommand(new KisImageLayerRemoveCommand(KisImageSP(this),
                                                                        layer,
                                                                        parent,
                                                                        wasAbove));
            }
        }
        return success;
    }

    return false;
}

bool KisImage::raiseLayer(KisLayerSP layer)
{
    return raiseNode( layer.data() );
}

bool KisImage::lowerLayer(KisLayerSP layer)
{
    return lowerNode( layer.data() );
}

bool KisImage::toTop(KisLayerSP layer)
{
    KisNode* node = dynamic_cast<KisNode*>( layer.data() );
    return KisNodeFacade::toTop( node );
}

bool KisImage::toBottom(KisLayerSP layer)
{
    KisNode* node = dynamic_cast<KisNode*>( layer.data() );
    return KisNodeFacade::toBottom( node );
}

bool KisImage::moveLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis)
{
    // XXX: Temporary until everything is moved to handling just nodes
    lock();
    KisGroupLayerSP wasParent = dynamic_cast<KisGroupLayer*>( layer->parent().data() );
    KisLayerSP wasAbove = dynamic_cast<KisLayer*>( layer->nextSibling().data() );
    if ( wasParent.data() == parent.data() && wasAbove.data() == aboveThis.data() )
        return false;
    bool success = moveNode( layer.data(), parent.data(), aboveThis.data() );
    if ( success ) {
        if (undo())
            m_d->adapter->addCommand(new KisImageLayerMoveCommand(KisImageSP(this),
                                                                  layer,
                                                                  wasParent,
                                                                  wasAbove));
    }

    unlock();

    return success;
}

qint32 KisImage::nlayers() const
{
    QStringList list;
    list << "KisLayer";

    KisCountVisitor visitor(list, KoProperties());
    m_d->rootLayer->accept( visitor );
    return visitor.count();
}

qint32 KisImage::nHiddenLayers() const
{
    QStringList list;
    list << "KisLayer";
    KoProperties properties;
    properties.setProperty( "visible", false );
    KisCountVisitor visitor(list, properties);
    m_d->rootLayer->accept( visitor );

    return visitor.count();
}

void KisImage::flatten()
{
    KisGroupLayerSP oldRootLayer = m_d->rootLayer;

    KisPaintLayer *dst = new KisPaintLayer(this, nextLayerName(), OPACITY_OPAQUE, colorSpace());
    Q_CHECK_PTR(dst);

    QRect rc = mergedImage()->extent();

    KisPainter gc(dst->paintDevice());
    gc.bitBlt(rc.x(), rc.y(), COMPOSITE_COPY, mergedImage(), OPACITY_OPAQUE, rc.left(), rc.top(), rc.width(), rc.height());

    setRootLayer( new KisGroupLayer(this, "", OPACITY_OPAQUE) );

    if (undo()) {
        m_d->adapter->beginMacro(i18n("Flatten Image"));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), true));
        m_d->adapter->addCommand(new KisImageChangeLayersCommand(KisImageSP(this), oldRootLayer, m_d->rootLayer, ""));
    }

    lock();

    addNode(dst, m_d->rootLayer.data(), 0);

    unlock();

    notifyLayersChanged();

    if (undo()) {
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), false));
        m_d->adapter->endMacro();
    }
}


void KisImage::mergeLayer(KisLayerSP layer)
{
    KisPaintLayer *newLayer = new KisPaintLayer(this, layer->name(), OPACITY_OPAQUE, colorSpace());
    Q_CHECK_PTR(newLayer);

    QRect rc = layer->extent() | layer->nextSibling()->extent();

    undoAdapter()->beginMacro(i18n("Merge with Layer Below"));

    //Abuse the merge visitor to only merge two layers (if either are groups they'll recursively merge)
    KisMergeVisitor visitor(newLayer->paintDevice(), rc);
    layer->nextSibling()->accept(visitor);
    layer->accept(visitor);

    removeNode(layer->nextSibling());
    addNode(newLayer, layer->parent(), layer.data());
    removeNode(layer.data());

    undoAdapter()->endMacro();
}


void KisImage::setModified()
{
    emit sigImageModified();
}

void KisImage::renderToPainter(qint32 srcX,
                               qint32 srcY,
                               qint32 dstX,
                               qint32 dstY,
                               qint32 width,
                               qint32 height,
                               QPainter &painter,
                               const KoColorProfile *  monitorProfile,
                               float exposure)
{
    QImage img = convertToQImage(srcX, srcY, width, height, monitorProfile, exposure);
    painter.drawImage(dstX, dstY, img, 0, 0, width, height);
}

QImage KisImage::convertToQImage(qint32 x,
                                 qint32 y,
                                 qint32 w,
                                 qint32 h,
                                 const KoColorProfile * profile,
                                 float exposure)
{
    KisPaintDeviceSP dev = m_d->rootLayer->projection();
    QImage img = dev->convertToQImage(const_cast<KoColorProfile*>( profile ), x, y, w, h, exposure);

    if (!img.isNull()) {

#ifdef WORDS_BIGENDIAN
        uchar * data = img.bits();
        for (int i = 0; i < w * h; ++i) {
            uchar r, g, b, a;
            a = data[0];
            b = data[1];
            g = data[2];
            r = data[3];
            data[0] = r;
            data[1] = g;
            data[2] = b;
            data[3] = a;
            data += 4;
        }
#endif

        return img;
    }

    return QImage();
}


QImage KisImage::convertToQImage(const QRect& r, const double xScale, const double yScale, const KoColorProfile *profile, float exposure, KisSelectionSP mask)
{
    Q_UNUSED( mask )

#ifdef __GNUC__
    #warning "KisImage::convertToQImage: Implement direct rendering of current mask onto scaled image pixels"
#endif

    if (r.isEmpty()) {
        return QImage();
    }

    quint32 pixelSize = colorSpace()->pixelSize();

    QRect srcRect;

    srcRect.setLeft(static_cast<int>(r.left() * xScale));
    srcRect.setRight(static_cast<int>(ceil((r.right() + 1) * xScale)) - 1);
    srcRect.setTop(static_cast<int>(r.top() * yScale));
    srcRect.setBottom(static_cast<int>(ceil((r.bottom() + 1) * yScale)) - 1);

    KisPaintDeviceSP mergedImage = m_d->rootLayer->projection();
    QTime t;
    t.start();

    quint8 *scaledImageData = new quint8[r.width() * r.height() * pixelSize];

    quint8 *imageRow = new quint8[srcRect.width() * pixelSize];
    const qint32 imageRowX = srcRect.x();

    t.restart();

    for (qint32 y = 0; y < r.height(); ++y) {

        qint32 dstY = r.y() + y;
        qint32 dstX = r.x();
        qint32 srcY = int(dstY * yScale);

        mergedImage->readBytes(imageRow, imageRowX, srcY, srcRect.width(), 1);

        quint8 *dstPixel = scaledImageData + (y * r.width() * pixelSize);
        quint32 columnsRemaining = r.width();

        while (columnsRemaining > 0) {

            qint32 srcX = int(dstX * xScale);

            memcpy(dstPixel, imageRow + ((srcX - imageRowX) * pixelSize), pixelSize);

            ++dstX;
            dstPixel += pixelSize;
            --columnsRemaining;
        }
    }

    t.restart();
    delete [] imageRow;

    QImage image = colorSpace()->convertToQImage(scaledImageData, r.width(), r.height(), const_cast<KoColorProfile*>( profile ), KoColorConversionTransformation::IntentPerceptual, exposure);
    delete [] scaledImageData;

#ifdef WORDS_BIGENDIAN
    uchar * data = image.bits();
    for (int i = 0; i < image.width() * image.height(); ++i) {
      uchar r, g, b, a;
      a = data[0];
      b = data[1];
      g = data[2];
      r = data[3];
      data[0] = r;
      data[1] = g;
      data[2] = b;
      data[3] = a;
      data += 4;
    }
#endif
    return image;
}

KisPaintDeviceSP KisImage::mergedImage()
{
    return m_d->rootLayer->projection();
}

KoColor KisImage::mergedPixel(qint32 x, qint32 y)
{
    return m_d->rootLayer->projection()->colorAt(x, y);
}

void KisImage::notifyLayersChanged()
{
    emit sigLayersChanged(rootLayer());
}

void KisImage::notifyPropertyChanged(KisLayerSP layer)
{
    emit sigLayerPropertiesChanged(layer);
}

QRect KisImage::bounds() const
{
    return QRect(0, 0, width(), height());
}


void KisImage::setUndoAdapter(KisUndoAdapter * adapter)
{
    m_d->adapter = adapter;
}


KisUndoAdapter* KisImage::undoAdapter() const
{
    return m_d->adapter;
}

KisActionRecorder* KisImage::actionRecorder() const
{
    return m_d->recorder;
}

bool KisImage::undo() const
{
    return (m_d->adapter && m_d->adapter->undo());
}


void KisImage::slotSelectionChanged()
{
    slotSelectionChanged(bounds());
}

void KisImage::slotSelectionChanged(const QRect& r)
{
    QRect r2(r.x() - 1, r.y() - 1, r.width() + 2, r.height() + 2);

    if (!locked()) {
        emit sigActiveSelectionChanged(KisImageSP(this));
    } else {
        m_d->selectionChangedWhileLocked = true;
    }
}

KoColorSpace * KisImage::colorSpace() const
{
    return m_d->colorSpace;
}

void KisImage::setColorSpace(KoColorSpace * colorSpace)
{
    m_d->colorSpace = colorSpace;
    m_d->rootLayer->resetProjection();
    emit sigColorSpaceChanged(colorSpace);
}

void KisImage::setRootLayer(KisGroupLayerSP rootLayer)
{
    m_d->rootLayer = rootLayer;
    m_d->rootLayer->disconnect();
    m_d->rootLayer->setGraphListener( this );
//     m_d->rootLayer->setProjectionManager( m_d->projection );
    m_d->projection->setRootLayer( rootLayer );
    setRoot( m_d->rootLayer.data() );
}

void KisImage::addAnnotation(KisAnnotationSP annotation)
{
    // Find the icc annotation, if there is one
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it)->type() == annotation->type()) {
            *it = annotation;
            return;
        }
        ++it;
    }
    m_d->annotations.push_back(annotation);
}

KisAnnotationSP KisImage::annotation(const QString& type)
{
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it)->type() == type) {
            return *it;
        }
        ++it;
    }
    return KisAnnotationSP(0);
}

void KisImage::removeAnnotation(const QString& type)
{
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it)->type() == type) {
            m_d->annotations.erase(it);
            return;
        }
        ++it;
    }
}

vKisAnnotationSP_it KisImage::beginAnnotations()
{
    KoColorProfile * profile = colorSpace()->profile();
    KisAnnotationSP annotation;

    if (profile)
    {
        // XXX we hardcode icc, this is correct for lcms?
        // XXX productName(), or just "ICC Profile"?
        if( profile->valid())
        {
            KoIccColorProfile* iccprofile = dynamic_cast<KoIccColorProfile*>(profile);
            if (!iccprofile->rawData().isEmpty())
            {
                annotation = new  KisAnnotation("icc", iccprofile->name(), iccprofile->rawData());
            }
        }
    }

    if (annotation)
         addAnnotation(annotation);
    else
        removeAnnotation("icc");

    return m_d->annotations.begin();
}

vKisAnnotationSP_it KisImage::endAnnotations()
{
    return m_d->annotations.end();
}

KisPerspectiveGrid* KisImage::perspectiveGrid()
{
    if ( m_d->perspectiveGrid == 0 )
        m_d->perspectiveGrid = new KisPerspectiveGrid();
    return m_d->perspectiveGrid;
}

void KisImage::slotProjectionUpdated( const QRect & rc )
{
    emit sigImageUpdated( rc );
}

#include "kis_image.moc"


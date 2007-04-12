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
#include "KoColorProfile.h"

#include "kis_annotation.h"
#include "kis_types.h"
#include "kis_meta_registry.h"
#include "kis_paint_device.h"
#include "kis_paint_device_action.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_paint_layer.h"
#include "kis_colorspace_convert_visitor.h"
#include "kis_nameserver.h"
#include "kis_merge_visitor.h"
#include "kis_transaction.h"
#include "kis_crop_visitor.h"
#include "kis_transform_visitor.h"
#include "kis_filter_strategy.h"
#include "kis_paint_layer.h"
#include "kis_change_profile_visitor.h"
#include "kis_group_layer.h"
#include "kis_iterators_pixel.h"
#include "kis_shear_visitor.h"
#include "kis_perspective_grid.h"
#include "kis_extent_visitor.h"
#include "kis_projection.h"
#include "kis_image_commands.h"


class KisImage::KisImagePrivate {
public:
    KoColor backgroundColor;
    quint32 lockCount;
    bool sizeChangedWhileLocked;
    bool selectionChangedWhileLocked;
    KisPerspectiveGrid* perspectiveGrid;

    KUrl uri;
    QString name;
    QString description;

    qint32 width;
    qint32 height;

    double xres;
    double yres;

    KoUnit unit;

    KoColorSpace * colorSpace;

    bool dirty;
    QRect dirtyRegion;

    KisGroupLayerSP rootLayer; // The layers are contained in here
    KisLayerSP activeLayer;
    QList<KisLayer*> dirtyLayers; // for thumbnails

    KisProjection * projection;

    KisNameServer *nserver;
    KisUndoAdapter *adapter;

    vKisAnnotationSP annotations;

};




KisImage::KisImage(KisUndoAdapter *adapter, qint32 width, qint32 height,  KoColorSpace * colorSpace, const QString& name)
    : QObject(0), KisShared()
{
    setObjectName(name);
    init(adapter, width, height, colorSpace, name);
    setName(name);
}

KisImage::KisImage(const KisImage& rhs) : QObject(), KisShared(rhs)
{
    if (this != &rhs) {
        m_d = new KisImagePrivate(*rhs.m_d);
        m_d->perspectiveGrid = new KisPerspectiveGrid(*rhs.m_d->perspectiveGrid);
        m_d->uri = rhs.m_d->uri;
        m_d->name.clear();
        m_d->width = rhs.m_d->width;
        m_d->height = rhs.m_d->height;
        m_d->xres = rhs.m_d->xres;
        m_d->yres = rhs.m_d->yres;
        m_d->unit = rhs.m_d->unit;
        m_d->colorSpace = rhs.m_d->colorSpace;
        m_d->dirty = rhs.m_d->dirty;
        m_d->adapter = rhs.m_d->adapter;


        m_d->rootLayer = static_cast<KisGroupLayer*>(rhs.m_d->rootLayer->clone().data());

        m_d->annotations = rhs.m_d->annotations; // XXX the annotations would probably need to be deep-copied

        m_d->projection = new KisProjection( this, m_d->rootLayer );

        m_d->nserver = new KisNameServer(rhs.m_d->nserver->currentSeed() + 1);
        Q_CHECK_PTR(m_d->nserver);

    }
}



KisImage::~KisImage()
{
    delete m_d->projection;
    delete m_d->perspectiveGrid;
    delete m_d->nserver;
    delete m_d;
}

QString KisImage::name() const
{
    return m_d->name;
}

void KisImage::setName(const QString& name)
{
    if (!name.isEmpty())
        m_d->name = name;
}

QString KisImage::description() const
{
    return m_d->description;
}

void KisImage::setDescription(const QString& description)
{
    if (!description.isEmpty())
        m_d->description = description;
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

void KisImage::init(KisUndoAdapter *adapter, qint32 width, qint32 height,  KoColorSpace * colorSpace, const QString& name)
{
    Q_ASSERT(colorSpace);

    if (colorSpace == 0) {
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        kWarning(41010) << "No colorspace specified: using RGBA\n";
    }

    m_d = new KisImagePrivate();
    m_d->backgroundColor = KoColor(Qt::white, colorSpace);
    m_d->lockCount = 0;
    m_d->sizeChangedWhileLocked = false;
    m_d->selectionChangedWhileLocked = false;
    m_d->perspectiveGrid = new KisPerspectiveGrid();

    m_d->adapter = adapter;

    m_d->nserver = new KisNameServer(1);
    m_d->name = name;

    m_d->colorSpace = colorSpace;

    m_d->rootLayer = new KisGroupLayer(this, "root", OPACITY_OPAQUE);
    m_d->projection = new KisProjection( this, m_d->rootLayer );

    m_d->xres = 1.0;
    m_d->yres = 1.0;
    m_d->unit = KoUnit::Point;
    m_d->dirty = false;
    m_d->width = width;
    m_d->height = height;

    blockSignals( this );
    lock();

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
    }
    m_d->lockCount++;
}

void KisImage::unlock()
{
    Q_ASSERT(locked());

    if (locked()) {
        m_d->lockCount--;

        if (m_d->lockCount == 0) {
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
        l = l->parent().data();
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


void KisImage::scale(double sx, double sy, KisProgressDisplayInterface *progress, KisFilterStrategy *filterStrategy)
{
    if (nlayers() == 0) return; // Nothing to scale

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



void KisImage::rotate(double radians, KisProgressDisplayInterface *progress)
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

void KisImage::shear(double angleX, double angleY, KisProgressDisplayInterface *progress)
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

void KisImage::convertTo(KoColorSpace * dstColorSpace, qint32 renderingIntent)
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

    emit sigLayerPropertiesChanged( m_d->activeLayer );

    if (undo()) {

        m_d->adapter->addCommand(new KisImageConvertTypeCommand(KisImageSP(this), oldCs, dstColorSpace));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), false));
        m_d->adapter->endMacro();
    }
}

KoColorProfile *  KisImage::profile() const
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

qint32 KisImage::width() const
{
    return m_d->width;
}

qint32 KisImage::height() const
{
    return m_d->height;
}

QRegion KisImage::extent() const
{
    KisExtentVisitor v(QRect(0, 0, width(), height()), false);
    m_d->rootLayer->accept(v);
    return v.region();
}

QRegion KisImage::dirtyRegion() const
{
    return QRegion(m_d->dirtyRegion);
}

KisPaintDeviceSP KisImage::activeDevice()
{
    if (KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_d->activeLayer.data())) {
        return layer->paintDeviceOrMask();
    }
    else if (KisAdjustmentLayer* layer = dynamic_cast<KisAdjustmentLayer*>(m_d->activeLayer.data())) {
        if (layer->selection()) {
            return KisPaintDeviceSP(layer->selection().data());
        }
    }
    else if (KisGroupLayer * layer = dynamic_cast<KisGroupLayer*>(m_d->activeLayer.data())) {
        // Find first child
        KisLayerSP child = layer->lastChild();
        while(child)
        {
            if (KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_d->activeLayer.data())) {
                return layer->paintDeviceOrMask();
            }
            child = child->prevSibling();
        }
    }
    // XXX: We're buggered!
    return KisPaintDeviceSP(0);
}

KisLayerSP KisImage::newLayer(const QString& name, quint8 opacity, const QString & compositeOp, KoColorSpace * cs)
{
    KisPaintLayer * layer;
    if (cs)
        layer = new KisPaintLayer(this, name, opacity, cs);
    else
        layer = new KisPaintLayer(this, name, opacity);
    Q_CHECK_PTR(layer);

    layer->setCompositeOp(cs->compositeOp(compositeOp));
    layer->setVisible(true);

    KisLayerSP layerSP(layer);

    if (!m_d->activeLayer.isNull()) {
        addLayer(layerSP, m_d->activeLayer->parent(), m_d->activeLayer->nextSibling());
    }
    else {
        addLayer(layerSP, m_d->rootLayer, KisLayerSP(0));
    }
    activateLayer(layerSP);

    return layerSP;
}

void KisImage::setLayerProperties(KisLayerSP layer, quint8 opacity, const KoCompositeOp* compositeOp, const QString& name, QBitArray channelFlags)
{
    layer->setName(name);
    layer->setOpacity(opacity);
    layer->setCompositeOp(compositeOp);
    layer->setChannelFlags( channelFlags );
}

KisGroupLayerSP KisImage::rootLayer() const
{
    return m_d->rootLayer;
}

KisLayerSP KisImage::activeLayer() const
{
    return m_d->activeLayer;
}

KisPaintDeviceSP KisImage::projection()
{
    return m_d->rootLayer->projection();
}

KisLayerSP KisImage::activateLayer(KisLayerSP layer)
{

//    if (layer != m_d->activeLayer) {

        if (m_d->activeLayer)
            m_d->activeLayer->deactivate();

        m_d->activeLayer = layer;

        if (m_d->activeLayer) {
            m_d->activeLayer->activate();

            emit sigLayerActivated(m_d->activeLayer);
        }

        emit sigMaskInfoChanged();
//    }

    return layer;
}

KisLayerSP KisImage::findLayer(const QString& name) const
{
    return rootLayer()->findLayer(name);
}

KisLayerSP KisImage::findLayer(int id) const
{
    return rootLayer()->findLayer(id);
}


void KisImage::preparePaintLayerAfterAdding( KisLayerSP layer )
{
    KisPaintLayerSP player = KisPaintLayerSP(dynamic_cast<KisPaintLayer*>(layer.data()));
    if (!player.isNull()) {

        // XXX: This should also be done whenever a layer grows!
        QList<KisPaintDeviceAction *> actions =
            KoColorSpaceRegistry::instance()->paintDeviceActionsFor(player->paintDevice()->colorSpace());
        for (int i = 0; i < actions.count(); i++) {
            actions.at(i)->act(player.data()->paintDevice(), width(), height());
        }

        connect(player.data(), SIGNAL(sigMaskInfoChanged()),
                this, SIGNAL(sigMaskInfoChanged()));
    }

    if (layer->extent().isValid()) layer->setDirty();

    if (!layer->temporary()) {
        emit sigLayerAdded(layer);
        activateLayer(layer);
    }


    if (!layer->temporary()) {
        QUndoCommand* cmd = new KisImageLayerAddCommand(KisImageSP(this), layer);
            cmd->redo();
    }
}

bool KisImage::addLayer(KisLayerSP layer, KisGroupLayerSP parent)
{
    if ( parent )
        return addLayer(layer, parent, parent->firstChild());
    else
        return addLayer( layer, rootLayer(), rootLayer()->firstChild() );
}

bool KisImage::addLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis)
{
    if (!parent)
        return false;

    const bool success = parent->addLayer(layer, aboveThis);
    if (success) preparePaintLayerAfterAdding( layer );

    return success;
}

bool KisImage::addLayer( KisLayerSP layer,  KisGroupLayerSP parent, int index )
{
    if (!parent)
        return false;

    const bool success = parent->addLayer(layer, index);
    if (success) preparePaintLayerAfterAdding( layer );

    return success;
}


bool KisImage::removeLayer(KisLayerSP layer)
{
    if (!layer || layer->image() != this)
        return false;

    if (KisGroupLayerSP parent = layer->parent()) {
        // Adjustment layers should mark the layers underneath them, whose rendering
        // they have cached, dirty on removal. Otherwise, the group won't be re-rendered.
        KisAdjustmentLayer * al = dynamic_cast<KisAdjustmentLayer*>(layer.data());
        if (al) {
            QRect r = al->extent();
            lock(); // Lock the image, because we are going to dirty a lot of layers
            KisLayerSP l = layer->nextSibling();
            while (l) {
                KisAdjustmentLayer * al2 = dynamic_cast<KisAdjustmentLayer*>(l.data());
                if (al2 != 0) break;
                l = l->nextSibling();
            }
            unlock();
        }
        KisPaintLayerSP player = dynamic_cast<KisPaintLayer*>(layer.data());
        if (player.isNull()) {
            disconnect(player.data(), SIGNAL(sigMaskInfoChanged()),
                       this, SIGNAL(sigMaskInfoChanged()));
        }

        KisLayerSP l = layer->prevSibling();
        QRect r = layer->extent();
        while (l) {
            l = l->prevSibling();
        }

        KisLayerSP wasAbove = layer->nextSibling();
        KisLayerSP wasBelow = layer->prevSibling();
        const bool wasActive = layer == activeLayer();
        // sigLayerRemoved can set it to 0, we don't want that in the else of wasActive!
        KisLayerSP actLayer = activeLayer();

        const bool success = parent->removeLayer(layer);

        if (success) {
            layer->setImage(0);
            if (!layer->temporary() && undo()) {
                m_d->adapter->addCommand(new KisImageLayerRemoveCommand(KisImageSP(this), layer, parent, wasAbove));
            }
            if (!layer->temporary()) {
                emit sigLayerRemoved(layer, parent, wasAbove);
                if (wasActive) {
                    if (wasBelow)
                        activateLayer(wasBelow);
                    else if (wasAbove)
                        activateLayer(wasAbove);
                    else if (parent != rootLayer())
                        activateLayer(KisLayerSP(parent.data()));
                    else
                        activateLayer(rootLayer()->firstChild());
                } else {
                    activateLayer(actLayer);
                }
            }
        }
        return success;
    }

    return false;
}

bool KisImage::raiseLayer(KisLayerSP layer)
{
    if (!layer)
        return false;
    return moveLayer(layer, layer->parent(), layer->prevSibling());
}

bool KisImage::lowerLayer(KisLayerSP layer)
{
    if (!layer)
        return false;
    if (KisLayerSP next = layer->nextSibling())
        return moveLayer(layer, layer->parent(), next->nextSibling());
    return false;
}

bool KisImage::toTop(KisLayerSP layer)
{
    if (!layer)
        return false;
    return moveLayer(layer, rootLayer(), rootLayer()->firstChild());
}

bool KisImage::toBottom(KisLayerSP layer)
{
    if (!layer)
        return false;
    return moveLayer(layer, rootLayer(), KisLayerSP(0));
}

bool KisImage::moveLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis)
{
    if (!parent)
        return false;

    KisGroupLayerSP wasParent = layer->parent();
    KisLayerSP wasAbove = layer->nextSibling();

    if (wasParent.data() == parent.data() && wasAbove.data() == aboveThis.data())
        return false;

    lock();

    if (!wasParent->removeLayer(layer)) {
        unlock();
        return false;
    }

    const bool success = parent->addLayer(layer, aboveThis);

    layer->setDirty();

    unlock();

    if (success)
    {
        emit sigLayerMoved(layer, wasParent, wasAbove);
        if (undo())
            m_d->adapter->addCommand(new KisImageLayerMoveCommand(KisImageSP(this), layer, wasParent, wasAbove));
    }
    else //we already removed the layer above, but re-adding it failed, so...
    {
        emit sigLayerRemoved(layer, wasParent, wasAbove);
        if (undo())
            m_d->adapter->addCommand(new KisImageLayerRemoveCommand(KisImageSP(this), layer, wasParent, wasAbove));
    }

    return success;
}

qint32 KisImage::nlayers() const
{
    return rootLayer()->numLayers() - 1;
}

qint32 KisImage::nHiddenLayers() const
{
    return rootLayer()->numLayers(KisLayer::Hidden);
}

void KisImage::flatten()
{
    KisGroupLayerSP oldRootLayer = m_d->rootLayer;

    KisPaintLayer *dst = new KisPaintLayer(this, nextLayerName(), OPACITY_OPAQUE, colorSpace());
    Q_CHECK_PTR(dst);

    QRect rc = mergedImage()->extent();

    KisPainter gc(dst->paintDevice());
    gc.bitBlt(rc.x(), rc.y(), COMPOSITE_COPY, mergedImage(), OPACITY_OPAQUE, rc.left(), rc.top(), rc.width(), rc.height());

    m_d->rootLayer = new KisGroupLayer(this, "", OPACITY_OPAQUE);
    m_d->projection->setRootLayer( m_d->rootLayer);

    if (undo()) {
        m_d->adapter->beginMacro(i18n("Flatten Image"));
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), true));
        m_d->adapter->addCommand(new KisImageChangeLayersCommand(KisImageSP(this), oldRootLayer, m_d->rootLayer, ""));
    }

    lock();

    addLayer(KisLayerSP(dst), m_d->rootLayer, KisLayerSP(0));
    activateLayer(KisLayerSP(dst));

    unlock();

    notifyLayersChanged();

    if (undo()) {
        m_d->adapter->addCommand(new KisImageLockCommand(KisImageSP(this), false));
        m_d->adapter->endMacro();
    }
}


void KisImage::mergeLayer(KisLayerSP layer)
{
    KisPaintLayer *player = new KisPaintLayer(this, layer->name(), OPACITY_OPAQUE, colorSpace());
    Q_CHECK_PTR(player);

    QRect rc = layer->extent() | layer->nextSibling()->extent();

    undoAdapter()->beginMacro(i18n("Merge with Layer Below"));

    //Abuse the merge visitor to only merge two layers (if either are groups they'll recursively merge)
    KisMergeVisitor visitor(player->paintDevice(), rc);
    layer->nextSibling()->accept(visitor);
    layer->accept(visitor);

    removeLayer(layer->nextSibling());
    addLayer(KisLayerSP(player), layer->parent(), layer);
    removeLayer(layer);

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
                               KoColorProfile *  monitorProfile,
                               float exposure)
{
    QImage img = convertToQImage(srcX, srcY, width, height, monitorProfile, exposure);
    painter.drawImage(dstX, dstY, img, 0, 0, width, height);
}

QImage KisImage::convertToQImage(qint32 x,
                                 qint32 y,
                                 qint32 w,
                                 qint32 h,
                                 KoColorProfile * profile,
                                 float exposure)
{
    KisPaintDeviceSP dev = m_d->rootLayer->projection();
    QImage img = dev->convertToQImage(profile, x, y, w, h, exposure);

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

#if 0
QImage KisImage::convertToQImage(const QRect& r, const double xScale, const double yScale, KoColorProfile *profile, float exposure)
{
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

    qDebug( "Created temporary memory areas: %d",  t.elapsed() );
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

    qDebug( "Created scaled image data : %d",  t.elapsed() );
    t.restart();
    delete [] imageRow;

    QImage image = colorSpace()->convertToQImage(scaledImageData, r.width(), r.height(), profile, INTENT_PERCEPTUAL, exposure);
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
    qDebug( "Converted to QImage: %d",  t.elapsed() );

    return image;
}
#endif

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
        emit sigSelectionChanged(KisImageSP(this));
    } else {
        m_d->selectionChangedWhileLocked = true;
    }
}

void KisImage::slotCommandExecuted()
{
    for( int i = 0, n = m_d->dirtyLayers.count(); i < n; ++i )
        m_d->dirtyLayers.at( i )->notifyCommandExecuted();
    m_d->dirtyLayers.clear();
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
    m_d->projection->setRootLayer( rootLayer );
    activateLayer(m_d->rootLayer->firstChild());
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
        if (!profile->rawData().isEmpty())
            annotation = new  KisAnnotation("icc", profile->productName(), profile->rawData());
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
    return m_d->perspectiveGrid;
}

void KisImage::slotProjectionUpdated( const QRect & rc )
{
    emit sigImageUpdated( rc );
}

#include "kis_image.moc"


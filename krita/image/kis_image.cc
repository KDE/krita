/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <stdlib.h>
#include <math.h>

#include <config.h>
#include <lcms.h>

#include <QImage>
#include <QPainter>
#include <QSize>
#include <QApplication>
#include <QThread>
#include <QDateTime>

#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>

#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorProfile.h"

#include "kis_annotation.h"
#include "kis_command.h"
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
#include "kis_undo_adapter.h"
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

namespace {

    class KisResizeImageCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisResizeImageCmd(KisUndoAdapter *adapter,
                          KisImageSP img,
                          qint32 width,
                          qint32 height,
                          qint32 oldWidth,
                          qint32 oldHeight) : super(i18n("Resize Image"))
            {
                m_adapter = adapter;
                m_img = img;
                m_before = QSize(oldWidth, oldHeight);
                m_after = QSize(width, height);
            }

        virtual ~KisResizeImageCmd()
            {
            }

    public:
        virtual void execute()
            {
                m_adapter->setUndo(false);
                m_img->resize(m_after.width(), m_after.height());
                m_adapter->setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter->setUndo(false);
                m_img->resize(m_before.width(), m_before.height());
                m_adapter->setUndo(true);
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        QSize m_before;
        QSize m_after;
    };

    // -------------------------------------------------------

    class KisChangeLayersCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisChangeLayersCmd(KisUndoAdapter *adapter, KisImageSP img,
                           KisGroupLayerSP oldRootLayer, KisGroupLayerSP newRootLayer, const QString& name)
            : super(name)
            {
                m_adapter = adapter;
                m_img = img;
                m_oldRootLayer = oldRootLayer;
                m_newRootLayer = newRootLayer;
            }

        virtual ~KisChangeLayersCmd()
            {
            }

    public:
        virtual void execute()
            {
                m_adapter->setUndo(false);
                m_img->setRootLayer(m_newRootLayer);
                m_adapter->setUndo(true);
                m_img->notifyLayersChanged();
            }

        virtual void unexecute()
            {
                m_adapter->setUndo(false);
                m_img->setRootLayer(m_oldRootLayer);
                m_adapter->setUndo(true);
                m_img->notifyLayersChanged();
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisGroupLayerSP m_oldRootLayer;
        KisGroupLayerSP m_newRootLayer;
    };


    // -------------------------------------------------------

    class KisConvertImageTypeCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisConvertImageTypeCmd(KisUndoAdapter *adapter, KisImageSP img,
                               KoColorSpace * beforeColorSpace, KoColorSpace * afterColorSpace
            ) : super(i18n("Convert Image Type"))
            {
                m_adapter = adapter;
                m_img = img;
                m_beforeColorSpace = beforeColorSpace;
                m_afterColorSpace = afterColorSpace;
            }

        virtual ~KisConvertImageTypeCmd()
            {
            }

    public:
        virtual void execute()
            {
                m_adapter->setUndo(false);

                m_img->setColorSpace(m_afterColorSpace);
                m_img->setProfile(m_afterColorSpace->getProfile());

                m_adapter->setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter->setUndo(false);

                m_img->setColorSpace(m_beforeColorSpace);
                m_img->setProfile(m_beforeColorSpace->getProfile());

                m_adapter->setUndo(true);
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KoColorSpace * m_beforeColorSpace;
        KoColorSpace * m_afterColorSpace;
    };


    // -------------------------------------------------------

    class KisImageCommand : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisImageCommand(const QString& name, KisImageSP image);
        virtual ~KisImageCommand() {}

        virtual void execute() = 0;
        virtual void unexecute() = 0;

    protected:
        void setUndo(bool undo);

        KisImageSP m_image;
    };

    KisImageCommand::KisImageCommand(const QString& name, KisImageSP image) :
        super(name), m_image(image)
    {
    }

    void KisImageCommand::setUndo(bool undo)
    {
        if (m_image->undoAdapter()) {
            m_image->undoAdapter()->setUndo(undo);
        }
    }


    // -------------------------------------------------------

    class KisLayerPositionCommand : public KisImageCommand {
        typedef KisImageCommand super;

    public:
        KisLayerPositionCommand(const QString& name, KisImageSP image, KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis) : super(name, image)
            {
                m_layer = layer;
                m_oldParent = layer->parent();
                m_oldAboveThis = layer->nextSibling();
                m_newParent = parent;
                m_newAboveThis = aboveThis;
           }

        virtual void execute()
            {
                setUndo(false);
                m_image->moveLayer(m_layer, m_newParent, m_newAboveThis);
                setUndo(true);
            }

        virtual void unexecute()
            {
                setUndo(false);
                m_image->moveLayer(m_layer, m_oldParent, m_oldAboveThis);
                setUndo(true);
            }

    private:
        KisLayerSP m_layer;
        KisGroupLayerSP m_oldParent;
        KisLayerSP m_oldAboveThis;
        KisGroupLayerSP m_newParent;
        KisLayerSP m_newAboveThis;
    };


    // -------------------------------------------------------

    class LayerAddCmd : public KisCommand {
        typedef KisCommand super;

    public:
        LayerAddCmd(KisUndoAdapter *adapter, KisImageSP img, KisLayerSP layer) : super(i18n("Add Layer"), adapter)
            {
                m_img = img;
                m_layer = layer;
                m_parent = layer->parent();
                m_aboveThis = layer->nextSibling();
            }

        virtual ~LayerAddCmd()
            {
            }

        virtual void execute()
            {
                adapter()->setUndo(false);
                m_img->addLayer(m_layer, m_parent, m_aboveThis);
                adapter()->setUndo(true);
            }

        virtual void unexecute()
            {
                adapter()->setUndo(false);
                m_img->removeLayer(m_layer);
                adapter()->setUndo(true);
            }

    private:
        KisImageSP m_img;
        KisLayerSP m_layer;
        KisGroupLayerSP m_parent;
        KisLayerSP m_aboveThis;
    };

    // -------------------------------------------------------

    class LayerRmCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        LayerRmCmd(KisUndoAdapter *adapter, KisImageSP img,
                   KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAbove)
            : super(i18n("Remove Layer"))
            {
                m_adapter = adapter;
                m_img = img;
                m_layer = layer;
                m_prevParent = wasParent;
                m_prevAbove = wasAbove;
            }

        virtual ~LayerRmCmd()
            {
            }

        virtual void execute()
            {
                m_adapter->setUndo(false);
                m_img->removeLayer(m_layer);
                m_adapter->setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter->setUndo(false);
                m_img->addLayer(m_layer, m_prevParent, m_prevAbove);
                m_adapter->setUndo(true);
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisLayerSP m_layer;
        KisGroupLayerSP m_prevParent;
        KisLayerSP m_prevAbove;
    };

    class LayerMoveCmd: public KNamedCommand {
        typedef KNamedCommand super;

    public:
        LayerMoveCmd(KisUndoAdapter *adapter, KisImageSP img,
                         KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAbove)
            : super(i18n("Move Layer"))
            {
                m_adapter = adapter;
                m_img = img;
                m_layer = layer;
                m_prevParent = wasParent;
                m_prevAbove = wasAbove;
                m_newParent = layer->parent();
                m_newAbove = layer->nextSibling();
            }

        virtual ~LayerMoveCmd()
            {
            }

        virtual void execute()
            {
                m_adapter->setUndo(false);
                m_img->moveLayer(m_layer, m_newParent, m_newAbove);
                m_adapter->setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter->setUndo(false);
                m_img->moveLayer(m_layer, m_prevParent, m_prevAbove);
                m_adapter->setUndo(true);
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisLayerSP m_layer;
        KisGroupLayerSP m_prevParent;
        KisLayerSP m_prevAbove;
        KisGroupLayerSP m_newParent;
        KisLayerSP m_newAbove;
    };


    // -------------------------------------------------------

    class LayerPropsCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        LayerPropsCmd(KisLayerSP layer,
                      KisImageSP img,
                      KisUndoAdapter *adapter,
                      const QString& name,
                      qint32 opacity,
                      const KoCompositeOp* compositeOp) : super(i18n("Layer Property Changes"))
            {
                m_layer = layer;
                m_img = img;
                m_adapter = adapter;
                m_name = name;
                m_opacity = opacity;
                m_compositeOp = compositeOp;
            }

        virtual ~LayerPropsCmd()
            {
            }

    public:
        virtual void execute()
            {
                QString name = m_layer->name();
                qint32 opacity = m_layer->opacity();
                m_compositeOp = m_layer->compositeOp();

                m_adapter->setUndo(false);
                m_img->setLayerProperties(m_layer,
                                            m_opacity,
                                            m_compositeOp,
                                            m_name);
                m_adapter->setUndo(true);
                m_name = name;
                m_opacity = opacity;
                m_layer->setDirty();
            }

        virtual void unexecute()
            {
                execute();
            }

    private:
        KisUndoAdapter *m_adapter;
        KisLayerSP m_layer;
        KisImageSP m_img;
        QString m_name;
        qint32 m_opacity;
        const KoCompositeOp * m_compositeOp;
    };

    // -------------------------------------------------------

    class LockImageCommand : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        LockImageCommand(KisImageSP img, bool lockImage) : super("lock image")  // Not for translation, this
            {                                                                   // is only ever used inside a macro command.
                m_img = img;
                m_lockImage = lockImage;
            }

        virtual ~LockImageCommand()
            {
            }

        virtual void execute()
            {
                if (m_lockImage) {
                    m_img->lock();
                } else {
                    m_img->unlock();
                }
            }

        virtual void unexecute()
            {
                if (m_lockImage) {
                    m_img->unlock();
                } else {
                    m_img->lock();
                }
            }

    private:
        KisImageSP m_img;
        bool m_lockImage;
    };
}

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

    KoUnit::Unit unit;

    KoColorSpace * colorSpace;

    bool dirty;
    QRect dirtyRect;

    KisGroupLayerSP rootLayer; // The layers are contained in here
    KisLayerSP activeLayer;
    QList<KisLayer*> dirtyLayers; // for thumbnails

    KisNameServer *nserver;
    KisUndoAdapter *adapter;

    vKisAnnotationSP annotations;

};




KisImage::KisImage(KisUndoAdapter *adapter, qint32 width, qint32 height,  KoColorSpace * colorSpace, const QString& name)
    : QObject(0), KShared()
{
    setObjectName(name);
    init(adapter, width, height, colorSpace, name);
    setName(name);
}

KisImage::KisImage(const KisImage& rhs) : QObject(), KShared(rhs)
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
        connect(m_d->rootLayer.data(), SIGNAL(sigDirty(QRect)), this, SIGNAL(sigImageUpdated(QRect)));

        m_d->annotations = rhs.m_d->annotations; // XXX the annotations would probably need to be deep-copied

        m_d->nserver = new KisNameServer(i18n("Layer %1"), rhs.m_d->nserver->currentSeed() + 1);
        Q_CHECK_PTR(m_d->nserver);

    }
}



KisImage::~KisImage()
{
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

    return m_d->nserver->name();
}

void KisImage::rollBackLayerName()
{
    m_d->nserver->rollback();
}

void KisImage::init(KisUndoAdapter *adapter, qint32 width, qint32 height,  KoColorSpace * colorSpace, const QString& name)
{
    Q_ASSERT(colorSpace);

    if (colorSpace == 0) {
        colorSpace = KisMetaRegistry::instance()->csRegistry()->rgb8();
        kWarning(41010) << "No colorspace specified: using RGBA\n";
    }

    m_d = new KisImagePrivate();
    m_d->backgroundColor = KoColor(Qt::white, colorSpace);
    m_d->lockCount = 0;
    m_d->sizeChangedWhileLocked = false;
    m_d->selectionChangedWhileLocked = false;
    m_d->perspectiveGrid = new KisPerspectiveGrid();

    m_d->adapter = adapter;

    m_d->nserver = new KisNameServer(i18n("Layer %1"), 1);
    m_d->name = name;

    m_d->colorSpace = colorSpace;
\
    m_d->rootLayer = new KisGroupLayer(this,"root", OPACITY_OPAQUE);
    connect(m_d->rootLayer.data(), SIGNAL(sigDirty(QRect)), this, SIGNAL(sigImageUpdated(QRect)));

    m_d->xres = 1.0;
    m_d->yres = 1.0;
    m_d->unit = KoUnit::U_PT;
    m_d->dirty = false;
    m_d->width = width;
    m_d->height = height;
}

bool KisImage::locked() const
{
    return m_d->lockCount != 0;
}

void KisImage::lock()
{
    if (!locked()) {
        if (m_d->rootLayer) disconnect(m_d->rootLayer.data(), SIGNAL(sigDirty(QRect)), this, SIGNAL(sigImageUpdated(QRect)));
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
                // A size change implies a full image update so only send this.
                emit sigSizeChanged(m_d->width, m_d->height);
            } else {
                if (m_d->rootLayer->dirty()) emit sigImageUpdated( m_d->rootLayer->dirtyRect() );
            }

            if (m_d->selectionChangedWhileLocked) {
                emit sigActiveSelectionChanged(KisImageSP(this));
            }

            if (m_d->rootLayer) connect(m_d->rootLayer.data(), SIGNAL(sigDirty(QRect)), this, SIGNAL(sigImageUpdated(QRect)));
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

void KisImage::notifyLayerUpdated(KisLayerSP layer, QRect rc)
{
    emit sigLayerUpdated(layer, rc);
    KisLayer *l = layer.data();
    while( l )
    {
        if( !m_d->dirtyLayers.contains( l ) )
            m_d->dirtyLayers.append( l );
        l = l->parent().data();
    }
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

            m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), true));
            m_d->adapter->addCommand(new KisResizeImageCmd(m_d->adapter, KisImageSP(this), w, h, width(), height()));
        }

        m_d->width = w;
        m_d->height = h;

        if (cropLayers) {
            KisCropVisitor v(QRect(x, y, w, h));
            m_d->rootLayer->accept(v);
        }

        emitSizeChanged();

        unlock();

        if (undo()) {
            m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), false));
            m_d->adapter->endMacro();
        }
    }
}

void KisImage::resize(const QRect& rc, bool cropLayers)
{
    resize(rc.width(), rc.height(), rc.x(), rc.y(), cropLayers);
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
            m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), true));
        }

        {
            KisTransformVisitor visitor (KisImageSP(this), sx, sy, 0.0, 0.0, 0.0, 0, 0, progress, filterStrategy);
            m_d->rootLayer->accept(visitor);
        }

        if (undo()) {
            m_d->adapter->addCommand(new KisResizeImageCmd(m_d->adapter, KisImageSP(this), w, h, width(), height()));
        }

        m_d->width = w;
        m_d->height = h;

        emitSizeChanged();

        unlock();

        if (undo()) {
            m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), false));
            m_d->adapter->endMacro();
        }
    }
}



void KisImage::rotate(double angle, KisProgressDisplayInterface *progress)
{
    lock();

    angle *= M_PI/180;
    qint32 w = width();
    qint32 h = height();
    qint32 tx = qint32((w*cos(angle) - h*sin(angle) - w) / 2 + 0.5);
    qint32 ty = qint32((h*cos(angle) + w*sin(angle) - h) / 2 + 0.5);
    w = (qint32)(width()*QABS(cos(angle)) + height()*QABS(sin(angle)) + 0.5);
    h = (qint32)(height()*QABS(cos(angle)) + width()*QABS(sin(angle)) + 0.5);

    tx -= (w - width()) / 2;
    ty -= (h - height()) / 2;

    if (undo()) {
        m_d->adapter->beginMacro(i18n("Rotate Image"));
        m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), true));
    }

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->get(KoID("Triangle"));
    KisTransformVisitor visitor (KisImageSP(this), 1.0, 1.0, 0, 0, angle, -tx, -ty, progress, filter);
    m_d->rootLayer->accept(visitor);

    if (undo()) m_d->adapter->addCommand(new KisResizeImageCmd(undoAdapter(), KisImageSP(this), w, h, width(), height()));

    m_d->width = w;
    m_d->height = h;

    emitSizeChanged();

    unlock();

    if (undo()) {
        m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), false));
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
            m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), true));
        }

        KisShearVisitor v(angleX, angleY, progress);
        v.setUndoAdapter(undoAdapter());
        rootLayer()->accept(v);

        if (undo()) m_d->adapter->addCommand(new KisResizeImageCmd(m_d->adapter, KisImageSP(this), w, h, width(), height()));

        m_d->width = w;
        m_d->height = h;

        emitSizeChanged();

        unlock();

        if (undo()) {
            m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), false));
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
        m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), true));
    }

    setColorSpace(dstColorSpace);

    KoColorSpaceConvertVisitor visitor(dstColorSpace, renderingIntent);
    m_d->rootLayer->accept(visitor);

    unlock();

    emit sigLayerPropertiesChanged( m_d->activeLayer );

    if (undo()) {

        m_d->adapter->addCommand(new KisConvertImageTypeCmd(undoAdapter(), KisImageSP(this),
                                                         oldCs, dstColorSpace));
        m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), false));
        m_d->adapter->endMacro();
    }
}

KoColorProfile *  KisImage::getProfile() const
{
    return colorSpace()->getProfile();
}

void KisImage::setProfile(const KoColorProfile * profile)
{
    if (profile == 0) return;

    KoColorSpace * dstCs= KisMetaRegistry::instance()->csRegistry()->colorSpace( colorSpace()->id(),
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

double KisImage::xRes()
{
    return m_d->xres;
}

double KisImage::yRes()
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
    activate(layerSP);

    return layerSP;
}

void KisImage::setLayerProperties(KisLayerSP layer, quint8 opacity, const KoCompositeOp* compositeOp, const QString& name)
{
    if (layer && (layer->opacity() != opacity || layer->compositeOp() != compositeOp || layer->name() != name)) {
        if (undo()) {
            QString oldname = layer->name();
            qint32 oldopacity = layer->opacity();
            const KoCompositeOp * oldCompositeOp = layer->compositeOp();
            layer->setName(name);
            layer->setOpacity(opacity);
            layer->setCompositeOp(compositeOp);
            m_d->adapter->addCommand(new LayerPropsCmd(layer, KisImageSP(this), m_d->adapter, oldname, oldopacity, oldCompositeOp));
        } else {
            layer->setName(name);
            layer->setOpacity(opacity);
            layer->setCompositeOp(compositeOp);
        }
    }
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
    return m_d->rootLayer->projection(QRect(0, 0, m_d->width, m_d->height));
}

KisLayerSP KisImage::activate(KisLayerSP layer)
{
    if (layer != m_d->activeLayer) {
        if (m_d->activeLayer) m_d->activeLayer->deactivate();
        m_d->activeLayer = layer;
        if (m_d->activeLayer) m_d->activeLayer->activate();
        emit sigLayerActivated(m_d->activeLayer);
        emit sigMaskInfoChanged();
    }

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


bool KisImage::addLayer(KisLayerSP layer, KisGroupLayerSP parent)
{
    return addLayer(layer, parent, parent->firstChild());
}

bool KisImage::addLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis)
{
    if (!parent)
        return false;

    const bool success = parent->addLayer(layer, aboveThis);
    if (success)
    {
        KisPaintLayerSP player = KisPaintLayerSP(dynamic_cast<KisPaintLayer*>(layer.data()));
        if (!player.isNull()) {

            // XXX: This should also be done whenever a layer grows!
            QList<KisPaintDeviceAction *> actions = KisMetaRegistry::instance() ->
                csRegistry()->paintDeviceActionsFor(player->paintDevice()->colorSpace());
            for (int i = 0; i < actions.count(); i++) {
                actions.at(i)->act(player.data()->paintDevice(), width(), height());
            }

            connect(player.data(), SIGNAL(sigMaskInfoChanged()),
                    this, SIGNAL(sigMaskInfoChanged()));
        }

        if (layer->extent().isValid()) layer->setDirty();

        if (!layer->temporary()) {
            emit sigLayerAdded(layer);
            activate(layer);
        }


        if (!layer->temporary() && undo()) {
            m_d->adapter->addCommand(new LayerAddCmd(m_d->adapter, KisImageSP(this), layer));
        }
    }

    return success;
}

bool KisImage::removeLayer(KisLayerSP layer)
{
    if (!layer || layer->image() != this)
        return false;

    if (KisGroupLayerSP parent = layer->parent()) {
        // Adjustment layers should mark the layers underneath them, whose rendering
        // they have cached, diryt on removal. Otherwise, the group won't be re-rendered.
        KisAdjustmentLayer * al = dynamic_cast<KisAdjustmentLayer*>(layer.data());
        if (al) {
            QRect r = al->extent();
            lock(); // Lock the image, because we are going to dirty a lot of layers
            KisLayerSP l = layer->nextSibling();
            while (l) {
                KisAdjustmentLayer * al2 = dynamic_cast<KisAdjustmentLayer*>(l.data());
                l->setDirty(r, false);
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
            l->setDirty(r, false);
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
                m_d->adapter->addCommand(new LayerRmCmd(m_d->adapter, KisImageSP(this), layer, parent, wasAbove));
            }
            if (!layer->temporary()) {
                emit sigLayerRemoved(layer, parent, wasAbove);
                if (wasActive) {
                    if (wasBelow)
                        activate(wasBelow);
                    else if (wasAbove)
                        activate(wasAbove);
                    else if (parent != rootLayer())
                        activate(KisLayerSP(parent.data()));
                    else
                        activate(rootLayer()->firstChild());
                } else {
                    activate(actLayer);
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
            m_d->adapter->addCommand(new LayerMoveCmd(m_d->adapter, KisImageSP(this), layer, wasParent, wasAbove));
    }
    else //we already removed the layer above, but re-adding it failed, so...
    {
        emit sigLayerRemoved(layer, wasParent, wasAbove);
        if (undo())
            m_d->adapter->addCommand(new LayerRmCmd(m_d->adapter, KisImageSP(this), layer, wasParent, wasAbove));
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
    disconnect(oldRootLayer.data(), SIGNAL(sigDirty(QRect)), this, SIGNAL(sigImageUpdated(QRect)));

    KisPaintLayer *dst = new KisPaintLayer(this, nextLayerName(), OPACITY_OPAQUE, colorSpace());
    Q_CHECK_PTR(dst);

    QRect rc = mergedImage()->extent();

    KisPainter gc(dst->paintDevice());
    gc.bitBlt(rc.x(), rc.y(), COMPOSITE_COPY, mergedImage(), OPACITY_OPAQUE, rc.left(), rc.top(), rc.width(), rc.height());

    m_d->rootLayer = new KisGroupLayer(this, "", OPACITY_OPAQUE);
    connect(m_d->rootLayer.data(), SIGNAL(sigDirty(QRect)), this, SIGNAL(sigImageUpdated(QRect)));

    if (undo()) {
        m_d->adapter->beginMacro(i18n("Flatten Image"));
        m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), true));
        m_d->adapter->addCommand(new KisChangeLayersCmd(m_d->adapter, KisImageSP(this), oldRootLayer, m_d->rootLayer, ""));
    }

    lock();

    addLayer(KisLayerSP(dst), m_d->rootLayer, KisLayerSP(0));
    activate(KisLayerSP(dst));

    unlock();

    notifyLayersChanged();

    if (undo()) {
        m_d->adapter->addCommand(new LockImageCommand(KisImageSP(this), false));
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
    KisPaintDeviceSP dev = m_d->rootLayer->projection(QRect(x, y, w, h));
    QImage img = dev->convertToQImage(profile, x, y, w, h, exposure);

    if (!img.isNull()) {

#ifdef __BIG_ENDIAN__
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

QImage KisImage::convertToQImage(const QRect& r, const QSize& scaledImageSize, KoColorProfile *profile, float exposure)
{
    if (r.isEmpty() || scaledImageSize.isEmpty()) {
        return QImage();
    }

    qint32 imageWidth = width();
    qint32 imageHeight = height();
    quint32 pixelSize = colorSpace()->pixelSize();

    double xScale = static_cast<double>(imageWidth) / scaledImageSize.width();
    double yScale = static_cast<double>(imageHeight) / scaledImageSize.height();

    QRect srcRect;

    srcRect.setLeft(static_cast<int>(r.left() * xScale));
    srcRect.setRight(static_cast<int>(ceil((r.right() + 1) * xScale)) - 1);
    srcRect.setTop(static_cast<int>(r.top() * yScale));
    srcRect.setBottom(static_cast<int>(ceil((r.bottom() + 1) * yScale)) - 1);

    KisPaintDeviceSP mergedImage = m_d->rootLayer->projection(srcRect);
    //QTime t;
    //t.start();

    quint8 *scaledImageData = new quint8[r.width() * r.height() * pixelSize];

    quint8 *imageRow = new quint8[srcRect.width() * pixelSize];
    const qint32 imageRowX = srcRect.x();

    for (qint32 y = 0; y < r.height(); ++y) {

        qint32 dstY = r.y() + y;
        qint32 dstX = r.x();
        qint32 srcY = (dstY * imageHeight) / scaledImageSize.height();

        mergedImage->readBytes(imageRow, imageRowX, srcY, srcRect.width(), 1);

        quint8 *dstPixel = scaledImageData + (y * r.width() * pixelSize);
        quint32 columnsRemaining = r.width();

        while (columnsRemaining > 0) {

            qint32 srcX = (dstX * imageWidth) / scaledImageSize.width();

            memcpy(dstPixel, imageRow + ((srcX - imageRowX) * pixelSize), pixelSize);

            ++dstX;
            dstPixel += pixelSize;
            --columnsRemaining;
        }
    }

    delete [] imageRow;

    QImage image = colorSpace()->convertToQImage(scaledImageData, r.width(), r.height(), profile, INTENT_PERCEPTUAL, exposure);
    delete [] scaledImageData;

#ifdef __BIG_ENDIAN__
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
    return m_d->rootLayer->projection(QRect(0, 0, m_d->width, m_d->height));
}

KoColor KisImage::mergedPixel(qint32 x, qint32 y)
{
    return m_d->rootLayer->projection(QRect(x, y, 1, 1))->colorAt(x, y);
}

void KisImage::notifyLayersChanged()
{
    emit sigLayersChanged(rootLayer());
}

void KisImage::notifyPropertyChanged(KisLayerSP layer)
{
    emit sigLayerPropertiesChanged(layer);
}

void KisImage::notifyImageLoaded()
{
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
    disconnect(m_d->rootLayer.data(), SIGNAL(sigDirty(QRect)), this, SIGNAL(sigImageUpdated(QRect)));

    m_d->rootLayer = rootLayer;

    if (!locked()) {
        connect(m_d->rootLayer.data(), SIGNAL(sigDirty(QRect)), this, SIGNAL(sigImageUpdated(QRect)));
    }
    activate(m_d->rootLayer->firstChild());
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
    KoColorProfile * profile = colorSpace()->getProfile();
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

#include "kis_image.moc"


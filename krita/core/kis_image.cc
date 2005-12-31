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
#include <stdlib.h>
#include <math.h>

#include <config.h>
#include LCMS_HEADER

#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qtl.h>
#include <qapplication.h>
#include <qthread.h>

#include <kcommand.h>
#include <kocommandhistory.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_image_iface.h"

#include "kis_annotation.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_color.h"
#include "kis_command.h"
#include "kis_types.h"
//#include "kis_guide.h"
#include "kis_image.h"
#include "kis_paint_device_impl.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_colorspace_convert_visitor.h"
#include "kis_background.h"
#include "kis_nameserver.h"
#include "kis_undo_adapter.h"
#include "kis_merge_visitor.h"
#include "kis_transaction.h"
#include "kis_scale_visitor.h"
#include "kis_profile.h"

#define DEBUG_IMAGES 0
const Q_INT32 RENDER_HEIGHT = 128;
const Q_INT32 RENDER_WIDTH = 128;

#if DEBUG_IMAGES
static int numImages = 0;
#endif

class KisImage::KisImagePrivate {
public:
    KisColor backgroundColor;
};


namespace {

    class KisResizeImageCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisResizeImageCmd(KisUndoAdapter *adapter,
                          KisImageSP img,
                          Q_INT32 width,
                          Q_INT32 height,
                          Q_INT32 oldWidth,
                          Q_INT32 oldHeight) : super(i18n("Resize Image"))
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
                m_adapter -> setUndo(false);
                m_img -> resize(m_after.width(), m_after.height());
                m_adapter -> setUndo(true);
                m_img -> notify(QRect( 0, 0, QMAX(m_before.width(), m_after.width()), QMAX(m_before.height(), m_after.height())) );
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);
                m_img -> resize(m_before.width(), m_before.height());
                m_adapter -> setUndo(true);
                m_img -> notify(QRect( 0, 0, QMAX(m_before.width(), m_after.width()), QMAX(m_before.height(), m_after.height())) );
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
                           KisLayerSP oldRootLayer, KisLayerSP newRootLayer, const QString& name)
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
                m_adapter -> setUndo(false);

// XXX LAYERREMOVE
                m_adapter -> setUndo(true);
                m_img -> notifyLayersChanged();
                m_img -> notify();
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);

// XXX LAYERREMOVE

                m_adapter -> setUndo(true);
                m_img -> notifyLayersChanged();
                m_img -> notify();
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisLayerSP m_oldRootLayer;
        KisLayerSP m_newRootLayer;
    };


    // -------------------------------------------------------

    class KisConvertImageTypeCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisConvertImageTypeCmd(KisUndoAdapter *adapter, KisImageSP img,
                               KisColorSpace * beforeColorSpace, KisColorSpace * afterColorSpace
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
                m_adapter -> setUndo(false);

                m_img -> setColorSpace(m_afterColorSpace);
                m_img -> setProfile(m_afterColorSpace -> getProfile());

                m_adapter -> setUndo(true);
                m_img -> notify();
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);

                m_img -> setColorSpace(m_beforeColorSpace);
                m_img -> setProfile(m_beforeColorSpace -> getProfile());

                m_adapter -> setUndo(true);
                m_img -> notify();
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisColorSpace * m_beforeColorSpace;
        KisColorSpace * m_afterColorSpace;
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
        if (m_image -> undoAdapter()) {
            m_image->undoAdapter()->setUndo(undo);
        }
    }


    // -------------------------------------------------------

    class KisLayerPositionCommand : public KisImageCommand {
        typedef KisImageCommand super;

    public:
        KisLayerPositionCommand(const QString& name, KisImageSP image, KisLayerSP layer, KisLayerSP parent, KisLayerSP aboveThis) : super(name, image)
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
                m_image -> moveLayer(m_layer, m_newParent, m_newAboveThis);
                setUndo(true);
            }

        virtual void unexecute()
            {
                setUndo(false);
                m_image -> moveLayer(m_layer, m_oldParent, m_oldAboveThis);
                setUndo(true);
            }

    private:
        KisLayerSP m_layer;
        KisLayerSP m_oldParent;
        KisLayerSP m_oldAboveThis;
        KisLayerSP m_newParent;
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
                adapter() -> setUndo(false);
                m_img -> addLayer(m_layer, m_parent.data(), m_aboveThis);
                adapter() -> setUndo(true);
            }

        virtual void unexecute()
            {
                adapter() -> setUndo(false);
                m_img -> removeLayer(m_layer);
                adapter() -> setUndo(true);
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
                m_adapter -> setUndo(false);
                m_img -> removeLayer(m_layer);
                m_adapter -> setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);
                m_img -> addLayer(m_layer, m_prevParent.data(), m_prevAbove);
                m_adapter -> setUndo(true);
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
                m_newParent = layer -> parent();
                m_newAbove = layer -> nextSibling();
            }

        virtual ~LayerMoveCmd()
            {
            }

        virtual void execute()
            {
                m_adapter -> setUndo(false);
                m_img -> moveLayer(m_layer, m_newParent.data(), m_newAbove);
                m_adapter -> setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);
                m_img -> moveLayer(m_layer, m_prevParent.data(), m_prevAbove);
                m_adapter -> setUndo(true);
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
                      Q_INT32 opacity,
                      const KisCompositeOp& compositeOp) : super(i18n("Layer Property Changes"))
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
                QString name = m_layer -> name();
                Q_INT32 opacity = m_layer -> opacity();
                KisCompositeOp compositeOp = m_layer -> compositeOp();

                m_adapter -> setUndo(false);
                m_img -> setLayerProperties(m_layer,
                                            m_opacity,
                                            m_compositeOp,
                                            m_name);
                m_adapter -> setUndo(true);
                m_name = name;
                m_opacity = opacity;
                m_compositeOp = compositeOp;
                m_img -> notify();
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
        Q_INT32 m_opacity;
        KisCompositeOp m_compositeOp;
    };
}

KisImage::KisImage(KisUndoAdapter *adapter, Q_INT32 width, Q_INT32 height,  KisColorSpace * colorSpace, const QString& name)
{
    init(adapter, width, height, colorSpace, name);
    setName(name);
    m_dcop = 0L;
}

KisImage::KisImage(const KisImage& rhs) : QObject(), KShared(rhs)
{
    m_dcop = 0L;
    if (this != &rhs) {
        m_private = new KisImagePrivate(*rhs.m_private);
        m_undoHistory = rhs.m_undoHistory;
        m_uri = rhs.m_uri;
        m_name = QString::null;
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        m_xres = rhs.m_xres;
        m_yres = rhs.m_yres;
        m_unit = rhs.m_unit;
        m_colorSpace = rhs.m_colorSpace;
        m_dirty = rhs.m_dirty;
        m_adapter = rhs.m_adapter;

        m_bkg = new KisBackground();
        Q_CHECK_PTR(m_bkg);

        m_projection = new KisPaintDeviceImpl(this, m_colorSpace);
        Q_CHECK_PTR(m_projection);

        m_rootLayer = rhs.m_rootLayer->clone();

        m_annotations = rhs.m_annotations; // XXX the annotations would probably need to be deep-copied

        m_nserver = new KisNameServer(i18n("Layer %1"), rhs.m_nserver -> currentSeed() + 1);
        Q_CHECK_PTR(m_nserver);

        //m_guides = rhs.m_guides;
    }
}



DCOPObject * KisImage::dcopObject()
{
    if (!m_dcop) {
        m_dcop = new KisImageIface(this);
        Q_CHECK_PTR(m_dcop);
    }
    return m_dcop;
}

KisImage::~KisImage()
{
    delete m_private;
    delete m_nserver;
    delete m_dcop;
}

QString KisImage::name() const
{
    return m_name;
}

void KisImage::setName(const QString& name)
{
    if (!name.isEmpty())
        m_name = name;
}

QString KisImage::description() const
{
    return m_description;
}

void KisImage::setDescription(const QString& description)
{
    if (!description.isEmpty())
        m_description = description;
}


KisColor KisImage::backgroundColor() const
{
    return m_private->backgroundColor;
}

void KisImage::setBackgroundColor(const KisColor & color)
{
    m_private->backgroundColor = color;
}


QString KisImage::nextLayerName() const
{
    if (m_nserver -> currentSeed() == 0) {
        m_nserver -> number();
        return i18n("background");
    }

    return m_nserver -> name();
}

void KisImage::init(KisUndoAdapter *adapter, Q_INT32 width, Q_INT32 height,  KisColorSpace * colorSpace, const QString& name)
{
    m_private = new KisImagePrivate();
    m_private->backgroundColor = KisColor(Qt::white, colorSpace);

    Q_ASSERT(colorSpace != 0);
    m_renderinit = false;
    m_adapter = adapter;

    m_nserver = new KisNameServer(i18n("Layer %1"), 1);
    Q_CHECK_PTR(m_nserver);
    m_name = name;

    m_colorSpace = colorSpace;
    m_bkg = new KisBackground();
    Q_CHECK_PTR(m_bkg);

    m_projection = new KisPaintDeviceImpl(this, m_colorSpace);
    Q_CHECK_PTR(m_projection);

    m_rootLayer = new KisGroupLayer(this,"root", OPACITY_OPAQUE);
    Q_CHECK_PTR(m_rootLayer);

    m_xres = 1.0;
    m_yres = 1.0;
    m_unit = KoUnit::U_PT;
    m_dirty = false;
    m_undoHistory = 0;
    m_width = width;
    m_height = height;

#ifdef __BIG_ENDIAN__
    cmsHPROFILE hProfile = cmsCreate_sRGBProfile();
    m_bigEndianTransform = cmsCreateTransform(hProfile ,
                                              TYPE_ABGR_8,
                                              hProfile ,
                                              TYPE_RGBA_8,
                                              INTENT_PERCEPTUAL,
                                              0);
#endif
}

void KisImage::resize(Q_INT32 , Q_INT32 , bool )
{
/*LAYERREMOVE
void KisImage::resize(Q_INT32 w, Q_INT32 h, bool cropLayers)
{
    if (w != width() || h != height()) {
        if (m_adapter && m_adapter -> undo()) {
            m_adapter -> beginMacro("Resize image");
            m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));
        }

        m_width = w;
        m_height = h;

        m_projection = new KisPaintDeviceImpl(this, colorSpace(), "projection");
        Q_CHECK_PTR(m_projection);

        if (cropLayers) {
            vKisLayerSP_it it;
            for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
                KisLayerSP layer = (*it);
                KisTransaction * t = new KisTransaction("crop", layer.data());
                Q_CHECK_PTR(t);
                layer -> crop(0, 0, w, h);
                if (m_adapter && m_adapter -> undo())
                    m_adapter->addCommand(t);
            }
        }

        if (m_adapter && m_adapter -> undo()) {
            m_adapter -> endMacro();
        }

        emit sigSizeChanged(KisImageSP(this), w, h);
        notify();
    }
*/
}

void KisImage::resize(const QRect& rc, bool cropLayers)
{
    resize(rc.width(), rc.height(), cropLayers);
    notify();
}


void KisImage::scale(double sx, double sy, KisProgressDisplayInterface *progress, KisFilterStrategy *filterStrategy)
{
    if (nlayers() == 0) return; // Nothing to scale

    // New image size. XXX: Pass along to discourage rounding errors?
    Q_INT32 w, h;
    w = (Q_INT32)(( width() * sx) + 0.5);
    h = (Q_INT32)(( height() * sy) + 0.5);

    if (w != width() || h != height()) {

        if (m_adapter && m_adapter -> undo()) {
            m_adapter->beginMacro("Scale image");
        }

        KisScaleVisitor visitor (this, sx, sy, progress, filterStrategy);
        m_rootLayer->accept(visitor);

        if (m_adapter && m_adapter -> undo()) {
            m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));
        }

        m_width = w;
        m_height = h;

        m_projection = new KisPaintDeviceImpl(this, m_colorSpace);
        
        if (m_adapter && m_adapter -> undo()) {
            m_adapter->endMacro();
        }
        notify();
        emit sigSizeChanged(w, h);

    }
}

void KisImage::rotate(double , KisProgressDisplayInterface *)
{
/*LAYERREMOVE
void KisImage::rotate(double angle, KisProgressDisplayInterface *m_progress)
{
    const double pi=3.1415926535897932385;

    if (m_layers.empty()) return; // Nothing to scale

    Q_INT32 w, h;
    w = (Q_INT32)(width()*QABS(cos(angle*pi/180)) + height()*QABS(sin(angle*pi/180)) + 0.5);
    h = (Q_INT32)(height()*QABS(cos(angle*pi/180)) + width()*QABS(sin(angle*pi/180)) + 0.5);

    Q_INT32 oldCentreToNewCentreXOffset = (w - width()) / 2;
    Q_INT32 oldCentreToNewCentreYOffset = (h - height()) / 2;

    m_adapter->beginMacro("Rotate image");

    vKisLayerSP_it it;
    for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
        KisLayerSP layer = (*it);

        KisTransaction * t = 0;
        if (undoAdapter() && m_adapter->undo()) {
            t = new KisTransaction("", layer.data());
            Q_CHECK_PTR(t);
        }

        layer -> rotate(angle, true, m_progress);

        if (t) {
            m_adapter->addCommand(t);
        }

        //XXX: This is very ugly.
        KNamedCommand *moveCommand = layer -> moveCommand(layer -> getX() + oldCentreToNewCentreXOffset,
                                                          layer -> getY() + oldCentreToNewCentreYOffset);
        if (undoAdapter() && m_adapter->undo()) {
            m_adapter->addCommand(moveCommand);
        } else {
            delete moveCommand;
        }
    }

    m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));

    m_width = w;
    m_height = h;

    m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
    Q_CHECK_PTR(m_projection);

    undoAdapter()->endMacro();

    emit sigSizeChanged(KisImageSP(this), w, h);
    notify();
*/
}

void KisImage::shear(double , double , KisProgressDisplayInterface *)
{
/*LAYERREMOVE
void KisImage::shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress)
{
    const double pi=3.1415926535897932385;

    if (m_layers.empty()) return; // Nothing to scale

    //new image size
    Q_INT32 w=width();
    Q_INT32 h=height();


    if(angleX != 0 || angleY != 0){
        double deltaY=height()*QABS(tan(angleX*pi/180)*tan(angleY*pi/180));
        w = (Q_INT32) ( width() + QABS(height()*tan(angleX*pi/180)) );
        //ugly fix for the problem of having two extra pixels if only a shear along one
        //axis is done. This has to be fixed in the cropping code in KisRotateVisitor!
        if (angleX == 0 || angleY == 0)
            h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180)) );
        else if (angleX > 0 && angleY > 0)
            h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180))- 2 * deltaY + 2 );
        else if (angleX < 0 && angleY < 0)
            h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180))- 2 * deltaY + 2 );
        else
            h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180)) );
    }

    if (w != width() || h != height()) {

        m_adapter->beginMacro("Shear image");

        vKisLayerSP_it it;
        for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
            KisLayerSP layer = (*it);

            KisTransaction * t = 0;
            if (undoAdapter() && m_adapter->undo()) {
                t = new KisTransaction("", layer.data());
                Q_CHECK_PTR(t);
            }

            layer -> shear(angleX, angleY, m_progress);

            if (t) {
                m_adapter->addCommand(t);
            }

        }

        m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));

        m_width = w;
        m_height = h;

        m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
        Q_CHECK_PTR(m_projection);

        undoAdapter()->endMacro();

        emit sigSizeChanged(KisImageSP(this), w, h);
        notify();
    }
*/
}

void KisImage::convertTo(KisColorSpace * dstColorSpace, Q_INT32 renderingIntent)
{
    if ( m_colorSpace == dstColorSpace )
    {
//         kdDebug(DBG_AREA_CORE) << "KisImage: NOT GOING TO CONVERT\n";
        return;
    }

    if (undoAdapter() && m_adapter->undo()) {
        m_adapter->beginMacro(i18n("Convert Image Type")); //XXX: fix when string freeze over
    }

    KisColorSpaceConvertVisitor visitor(dstColorSpace, renderingIntent);
    m_rootLayer->accept(visitor);

    m_projection->convertTo(dstColorSpace, renderingIntent);

    if (undoAdapter() && m_adapter->undo()) {

        m_adapter->addCommand(new KisConvertImageTypeCmd(undoAdapter(), this,
                                                         m_colorSpace, dstColorSpace));
        m_adapter->endMacro();
    }

    setColorSpace(dstColorSpace);

    notify();
}

KisProfile *  KisImage::getProfile() const
{
    return colorSpace()->getProfile();
}

void KisImage::setProfile(const KisProfile * profile)
{
    KisColorSpace * dstSpace = KisMetaRegistry::instance()->csRegistry()->getColorSpace( colorSpace()->id(), profile);
    //convertTo( dstSpace ); // XXX: We shouldn't convert here -- if you want to convert, use the conversion function.
    setColorSpace(dstSpace); 
    emit(sigProfileChanged(const_cast<KisProfile *>(profile)));
}

double KisImage::xRes()
{
    return m_xres;
}

double KisImage::yRes()
{
    return m_yres;
}

void KisImage::setResolution(double xres, double yres)
{
    m_xres = xres;
    m_yres = yres;
}

Q_INT32 KisImage::width() const
{
    return m_width;
}

Q_INT32 KisImage::height() const
{
    return m_height;
}

KisPaintDeviceImplSP KisImage::activeDevice()
{
    if (KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_activeLayer.data()))
        return layer -> paintDevice();
    return 0;
}

KisLayerSP KisImage::newLayer(const QString& name, Q_UINT8 opacity, const KisCompositeOp& compositeOp, KisColorSpace * colorstrategy)
{
    KisLayerSP layer;
    if (colorstrategy)
        layer = new KisPaintLayer(this, name, opacity, colorstrategy);
    else
        layer = new KisPaintLayer(this, name, opacity);
    Q_CHECK_PTR(layer);

    if (compositeOp.isValid())
        layer -> setCompositeOp(compositeOp);
    layer -> setVisible(true);

    kdDebug() << "active layer: " << m_activeLayer << "\n";
    
    if (m_activeLayer != 0) {
        addLayer(layer, m_activeLayer->parent().data(), m_activeLayer->nextSibling());
    }
    else {
        addLayer(layer, m_rootLayer, 0);
    }
    activate(layer);

    return layer;
}

void KisImage::setLayerProperties(KisLayerSP layer, Q_UINT8 opacity, const KisCompositeOp& compositeOp, const QString& name)
{
    if (layer && (layer->opacity() != opacity || layer->compositeOp() != compositeOp || layer->name() != name)) {
        if (m_adapter->undo()) {
            QString oldname = layer -> name();
            Q_INT32 oldopacity = layer -> opacity();
            KisCompositeOp oldCompositeOp = layer -> compositeOp();
            layer -> setName(name);
            layer -> setOpacity(opacity);
            layer -> setCompositeOp(compositeOp);
            m_adapter->addCommand(new LayerPropsCmd(layer, this, m_adapter, oldname, oldopacity, oldCompositeOp));
        } else {
            layer -> setName(name);
            layer -> setOpacity(opacity);
            layer -> setCompositeOp(compositeOp);
        }

        notify();
        emit sigLayerPropertiesChanged(layer);
    }
}

KisLayerSP KisImage::rootLayer() const
{
    return m_rootLayer;
}

KisLayerSP KisImage::activeLayer() const
{
    return m_activeLayer;
}

KisLayerSP KisImage::activate(KisLayerSP layer)
{
    if ( !layer )
        return 0;

    if (layer != m_activeLayer) {
        if (m_activeLayer) m_activeLayer->deactivate();
        m_activeLayer = layer;
        if (m_activeLayer) m_activeLayer->activate();
        emit sigLayerActivated(m_activeLayer);
    }

    return layer;
}

KisLayerSP KisImage::findLayer(const QString& name) const
{
    return rootLayer() -> findLayer(name);
}

KisLayerSP KisImage::findLayer(int id) const
{
    return rootLayer() -> findLayer(id);
}

bool KisImage::addLayer(KisLayerSP layer, KisLayerSP p, KisLayerSP aboveThis)
{
    KisGroupLayerSP parent = dynamic_cast<KisGroupLayer*>(p.data());
    if (!parent)
        return false;

    const bool success = parent->addLayer(layer, aboveThis);
    if (success)
    {
        if (m_adapter->undo())
            m_adapter->addCommand(new LayerAddCmd(m_adapter, this, layer));
        notify();

        activate(layer);

    /*LAYERREMOVE
        QValueVector<KisPaintDeviceAction *> actions = KisMetaRegistry::instance() ->
        csRegistry() -> paintDeviceActionsFor(cs);
        for (uint i = 0; i < actions.count(); i++)
        actions.at(i) -> act(layer.data(), img -> width(), img -> height());
    */

        emit sigLayerAdded(layer);
    }

    return success;
}

bool KisImage::removeLayer(KisLayerSP layer)
{
    if (KisGroupLayerSP parent = layer -> parent())
    {

        KisLayerSP wasAbove = layer -> nextSibling();
        const bool success = parent -> removeLayer(layer);
        if (success)
        {
            if (m_adapter->undo())
                m_adapter->addCommand(new LayerRmCmd(m_adapter, this, layer, parent, wasAbove));
            notify();
            emit sigLayerRemoved(layer, parent, wasAbove);
            activate(wasAbove ? wasAbove : parent != rootLayer() ? parent.data() : rootLayer() -> firstChild());
        }
        return success;
    }

    return false;
}

bool KisImage::raiseLayer(KisLayerSP layer)
{
    return moveLayer(layer, layer -> parent().data(), layer -> prevSibling());
}

bool KisImage::lowerLayer(KisLayerSP layer)
{
    if (KisLayerSP next = layer -> nextSibling())
        return moveLayer(layer, layer -> parent().data(), next -> nextSibling());
    return false;
}

bool KisImage::toTop(KisLayerSP layer)
{
    return moveLayer(layer, rootLayer(), rootLayer() -> firstChild());
}

bool KisImage::toBottom(KisLayerSP layer)
{
    return moveLayer(layer, rootLayer(), 0);
}

bool KisImage::moveLayer(KisLayerSP layer, KisLayerSP p, KisLayerSP aboveThis)
{
    KisGroupLayerSP parent = dynamic_cast<KisGroupLayer*>(p.data());
    if (!parent)
        return false;

    KisGroupLayerSP wasParent = layer -> parent();
    KisLayerSP wasAbove = layer -> nextSibling();

    if (wasParent.data() == p.data() && wasAbove.data() == aboveThis.data())
        return false;

    if (!wasParent -> removeLayer(layer))
        return false;

    const bool success = parent -> addLayer(layer, aboveThis);
    if (success)
    {
        if (m_adapter->undo())
            m_adapter->addCommand(new LayerMoveCmd(m_adapter, this, layer, wasParent, wasAbove));
        notify();
        emit sigLayerMoved(layer, wasParent, wasAbove);
    }
    else //we already removed the layer above, but re-adding it failed, so...
    {
        if (m_adapter->undo())
            m_adapter->addCommand(new LayerRmCmd(m_adapter, this, layer, wasParent, wasAbove));
        notify();
        emit sigLayerRemoved(layer, wasParent, wasAbove);
    }

    return success;
}

Q_INT32 KisImage::nlayers() const
{
    return rootLayer() -> numLayers() - 1;
}

Q_INT32 KisImage::nHiddenLayers() const
{
    return rootLayer() -> numLayers(KisLayer::Hidden);
}

void KisImage::flatten()
{
    KisLayerSP oldRootLayer = m_rootLayer;

    m_rootLayer = new KisGroupLayer(this, "", OPACITY_OPAQUE);

    KisPaintLayer *dst = new KisPaintLayer(this, nextLayerName(), OPACITY_OPAQUE, colorSpace());
    Q_CHECK_PTR(dst);

    KisPainter painter(dst->paintDevice());

    KisMergeVisitor visitor(this, &painter);
    oldRootLayer ->accept(visitor);

    addLayer(dst, m_rootLayer, 0);
    activate(dst); // LAYERREMOVE <- maybe addLayer should be the one to call activate but I don't understand the difference between addLayer and layerAdd </Cyrille>

    notify();
    notifyLayersChanged();

    if (m_adapter && m_adapter -> undo()) {
        m_adapter->addCommand(new KisChangeLayersCmd(m_adapter, this, oldRootLayer, m_rootLayer, i18n("Flatten Image")));
    }
}

void KisImage::mergeVisibleLayers()
{
/*
    vKisLayerSP beforeLayers = m_layers;

    QString newName = 0;
    if( activeLayer() && activeLayer()->visible())
        newName = activeLayer()->name();
    if( !newName )
        newName = nextLayerName();

    KisLayerSP dst = new KisLayer(this, newName, OPACITY_OPAQUE);
    Q_CHECK_PTR(dst);

    KisFillPainter painter(dst.data());

    vKisLayerSP mergeLayers = layers();
    KisMerge<isVisible, isVisible> visitor(this);
    visitor(painter, mergeLayers);

    int insertIndex = -1;

    if (visitor.insertMergedAboveLayer() != 0) {
        insertIndex = index(visitor.insertMergedAboveLayer());
    }

    add(dst, insertIndex);

    notify();
    notifyLayersChanged();

    if (m_adapter && m_adapter -> undo()) {
        m_adapter->addCommand(new KisChangeLayersCmd(m_adapter, this, beforeLayers, m_layers, i18n("Merge Visible Layers")));
    }
*/
}

void KisImage::mergeLinkedLayers()
{
/*
    vKisLayerSP beforeLayers = m_layers;

    KisLayerSP dst = new KisLayer(this, nextLayerName(), OPACITY_OPAQUE);
    Q_CHECK_PTR(dst);

    KisFillPainter painter(dst.data());

    vKisLayerSP mergeLayers = layers();
    KisMerge<isLinked, isLinked> visitor(this);
    visitor(painter, mergeLayers);

    int insertIndex = -1;

    if (visitor.insertMergedAboveLayer() != 0) {
        insertIndex = index(visitor.insertMergedAboveLayer());
    }

    add(dst, insertIndex);

    notify();
    notifyLayersChanged();

    if (m_adapter && m_adapter -> undo()) {
        m_adapter->addCommand(new KisChangeLayersCmd(m_adapter, this, beforeLayers, m_layers, i18n("Merge Linked Layers")));
    }
*/
}

void KisImage::mergeLayer(KisLayerSP /*l*/)
{
/*
    vKisLayerSP beforeLayers = m_layers;

    KisLayerSP dst = new KisLayer(this, l -> name(), OPACITY_OPAQUE);
    Q_CHECK_PTR(dst);

    KisFillPainter painter(dst.data());

    KisMerge<All, All> visitor(this);
    visitor(painter, layer(index(l) + 1));
    visitor(painter, l);

    int insertIndex = -1;

    if (visitor.insertMergedAboveLayer() != 0) {
        insertIndex = index(visitor.insertMergedAboveLayer());
    }

    add(dst, insertIndex);

    notify();
    notifyLayersChanged();

    if (m_adapter && m_adapter -> undo())
    {
        m_adapter->addCommand(new KisChangeLayersCmd(m_adapter, this, beforeLayers, m_layers, i18n("&Merge Layers")));
//XXX fix name after string freeze
    }
*/
}


void KisImage::enableUndo(KoCommandHistory *history)
{
    m_undoHistory = history;
}

void KisImage::setModified()
{
    emit sigImageModified();
}

void KisImage::renderToPainter(Q_INT32 x1,
                               Q_INT32 y1,
                               Q_INT32 x2,
                               Q_INT32 y2,
                               QPainter &painter,
                               KisProfile *  monitorProfile,
                               PaintFlags paintFlags,
                               float exposure)
{


//    QRect r = m_projection->extent();
//    kdDebug() << "projection extent: " << r.x() << ", " << r.y() << ", " << r.width() << ", " << r.height() << "...\n";

    Q_INT32 w = x2 - x1 + 1;
    Q_INT32 h = y2 - y1 + 1;

    if (!m_renderinit) {
        QRect rc(x1, y1, w, h);
        rc &= bounds();
        updateProjection(rc);
    }

    QImage img = m_projection -> convertToQImage(monitorProfile, x1, y1, w, h, exposure);

#ifdef __BIG_ENDIAN__
        //cmsDoTransform(m_bigEndianTransform, img.bits(), img.bits(), w * h);
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

    if (paintFlags & PAINT_BACKGROUND) {
        m_bkg -> paintBackground(img, x1, y1);
        img.setAlphaBuffer(false);
    }

    if (paintFlags & PAINT_SELECTION) {
        if (m_activeLayer != 0) {
            m_activeLayer -> paintSelection(img, x1, y1, w, h);
        }
    }

    if (paintFlags & PAINT_MASKINACTIVELAYERS) {
        if (m_activeLayer != 0) {
            m_activeLayer -> paintMaskInactiveLayers(img, x1, y1, w, h);
        }
    }

    painter.drawImage(x1, y1, img, 0, 0, w, h);
}

QImage KisImage::convertToQImage(Q_INT32 x1,
                                 Q_INT32 y1,
                                 Q_INT32 x2,
                                 Q_INT32 y2,
                                 KisProfile * profile,
                                 float exposure)
{
    Q_INT32 w = x2 - x1 + 1;
    Q_INT32 h = y2 - y1 + 1;

    QImage img = m_projection->convertToQImage(profile, x1, y1, w, h, exposure);

    if (!img.isNull()) {

#ifdef __BIG_ENDIAN__
        cmsDoTransform(m_bigEndianTransform, img.bits(), img.bits(), w * h);
#endif
        return img;
    }

    return QImage();
}

KisPaintDeviceImplSP KisImage::mergedImage()
{
    return m_projection;
}

KisColor KisImage::mergedPixel(Q_INT32 x, Q_INT32 y)
{
    return m_projection -> colorAt(x, y);
}

void KisImage::updateProjection(const QRect& rc)
{
    QRect rect = rc & QRect(0, 0, width(), height());

    // Composite the image
    KisFillPainter gc;
    gc.begin(m_projection);
    gc.eraseRect(rect);
    gc.end();

    KisPainter painter(m_projection);
    KisMergeVisitor visitor(this, &painter);
    visitor.setProjection(m_projection);
    m_rootLayer -> accept(visitor);

    m_renderinit = true;
}

void KisImage::notify()
{
    notify(QRect(0, 0, width(), height()));
}

void KisImage::notify(const QRect& rc)
{
    QRect rect = rc & QRect(0, 0, width(), height());

    updateProjection(rect);

    if (rect.isValid()) {
        emit sigImageUpdated(rect);
    }
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

KisUndoAdapter* KisImage::undoAdapter() const
{
    return m_adapter;
}

//KisGuideMgr *KisImage::guides() const
//{
//    return const_cast<KisGuideMgr*>(&m_guides);
//}

void KisImage::slotSelectionChanged()
{
//     kdDebug(DBG_AREA_CORE) << "KisImage::slotSelectionChanged\n";
    notify();
    emit sigActiveSelectionChanged(KisImageSP(this));
}

void KisImage::slotSelectionChanged(const QRect& r)
{
//     kdDebug(DBG_AREA_CORE) << "KisImage::slotSelectionChanged rect\n";
    QRect r2(r.x() - 1, r.y() - 1, r.width() + 2, r.height() + 2);

    notify(r2);
    emit sigActiveSelectionChanged(KisImageSP(this));
}

KisColorSpace * KisImage::colorSpace() const
{
    return m_colorSpace;
}

void KisImage::setColorSpace(KisColorSpace * colorSpace)
{
    m_colorSpace = colorSpace;

    m_projection = new KisPaintDeviceImpl(this, colorSpace);
    Q_CHECK_PTR(m_projection);

    emit sigColorSpaceChanged(colorSpace);

    notify();
}

void KisImage::addAnnotation(KisAnnotationSP annotation)
{
    // Find the icc annotation, if there is one
    vKisAnnotationSP_it it = m_annotations.begin();
    while (it != m_annotations.end()) {
        if ((*it) -> type() == annotation -> type()) {
            *it = annotation;
            return;
        }
        ++it;
    }
    m_annotations.push_back(annotation);
}

KisAnnotationSP KisImage::annotation(QString type)
{
    vKisAnnotationSP_it it = m_annotations.begin();
    while (it != m_annotations.end()) {
        if ((*it) -> type() == type) {
            return *it;
        }
        ++it;
    }
    return 0;
}

void KisImage::removeAnnotation(QString type)
{
    vKisAnnotationSP_it it = m_annotations.begin();
    while (it != m_annotations.end()) {
        if ((*it) -> type() == type) {
            m_annotations.erase(it);
            return;
        }
        ++it;
    }
}

vKisAnnotationSP_it KisImage::beginAnnotations()
{
    KisProfile * profile = colorSpace()->getProfile();
    KisAnnotationSP annotation;

    if (profile)
        annotation =  profile -> annotation();

    if (annotation)
         addAnnotation(annotation);
    else
        removeAnnotation("icc");

    return m_annotations.begin();
}

vKisAnnotationSP_it KisImage::endAnnotations()
{
    return m_annotations.end();
}

KisBackgroundSP KisImage::background() const
{
    return m_bkg;
}
#include "kis_image.moc"


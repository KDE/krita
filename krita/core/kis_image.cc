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

#include <kcommand.h>
#include <kocommandhistory.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_image_iface.h"

#include "kis_command.h"
#include "kis_types.h"
#include "kis_guide.h"
#include "kis_image.h"
#include "kis_paint_device_impl.h"
#include "kis_paint_device_visitor.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_layer.h"
#include "kis_background.h"
#include "kis_doc.h"
#include "kis_nameserver.h"
#include "visitors/kis_flatten.h"
#include "visitors/kis_merge.h"
#include "kis_transaction.h"
#include "kis_scale_visitor.h"
#include "kis_profile.h"
#include "kis_config.h"
#include "kis_colorspace_registry.h"

#define DEBUG_IMAGES 0
const Q_INT32 RENDER_HEIGHT = 128;
const Q_INT32 RENDER_WIDTH = 128;

#if DEBUG_IMAGES
static int numImages = 0;
#endif

namespace {

    // -------------------------------------------------------

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
        KisChangeLayersCmd(KisUndoAdapter *adapter, KisImageSP img, vKisLayerSP& beforeLayers, vKisLayerSP& afterLayers, const QString& name) : super(name)
            {
                m_adapter = adapter;
                m_img = img;
                m_beforeLayers = beforeLayers;
                m_afterLayers = afterLayers;
            }

        virtual ~KisChangeLayersCmd()
            {
            }

    public:
        virtual void execute()
            {
                m_adapter -> setUndo(false);

                for (vKisLayerSP::const_iterator it = m_beforeLayers.begin(); it != m_beforeLayers.end(); ++it) {
                    m_img -> rm(*it);
                }

                for (vKisLayerSP::const_iterator it = m_afterLayers.begin(); it != m_afterLayers.end(); ++it) {
                    m_img -> add(*it, -1);
                }

                m_adapter -> setUndo(true);
                m_img -> notify();
                m_img -> notifyLayersChanged();
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);

                for (vKisLayerSP::const_iterator it = m_afterLayers.begin(); it != m_afterLayers.end(); ++it) {
                    m_img -> rm(*it);
                }

                for (vKisLayerSP::const_iterator it = m_beforeLayers.begin(); it != m_beforeLayers.end(); ++it) {
                    m_img -> add(*it, -1);
                }

                m_adapter -> setUndo(true);
                m_img -> notify();
                m_img -> notifyLayersChanged();
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        vKisLayerSP m_beforeLayers;
        vKisLayerSP m_afterLayers;
    };


    // -------------------------------------------------------

    class KisConvertImageTypeCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisConvertImageTypeCmd(KisUndoAdapter *adapter, KisImageSP img,
                               KisColorSpace * beforeColorSpace, KisProfile *  beforeProfile,
                               KisColorSpace * afterColorSpace, KisProfile *  afterProfile
            ) : super(i18n("Convert Image Type"))
            {
                m_adapter = adapter;
                m_img = img;
                m_beforeColorSpace = beforeColorSpace;
                m_beforeProfile = beforeProfile;
                m_afterColorSpace = afterColorSpace;
                m_afterProfile = afterProfile;
            }

        virtual ~KisConvertImageTypeCmd()
            {
            }

    public:
        virtual void execute()
            {
                m_adapter -> setUndo(false);

                m_img -> setColorSpace(m_afterColorSpace);
                m_img -> setProfile(m_afterProfile);

                m_adapter -> setUndo(true);
                m_img -> notify();
                m_img -> notifyLayersChanged();
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);

                m_img -> setColorSpace(m_beforeColorSpace);
                m_img -> setProfile(m_beforeProfile);

                m_adapter -> setUndo(true);
                m_img -> notify();
                m_img -> notifyLayersChanged();
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisColorSpace * m_beforeColorSpace;
        KisColorSpace * m_afterColorSpace;
        KisProfile *  m_beforeProfile;
        KisProfile *  m_afterProfile;
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
        KisLayerPositionCommand(const QString& name, KisImageSP image, KisLayerSP layer, Q_INT32 position) : super(name, image)
            {
                m_layer = layer;
                m_oldPosition = image -> index(layer);
                m_newPosition = position;
            }

        virtual void execute()
            {
                setUndo(false);
                m_image -> setLayerPosition(m_layer, m_newPosition);
                m_image -> notifyLayersChanged();
                setUndo(true);
            }

        virtual void unexecute()
            {
                setUndo(false);
                m_image -> setLayerPosition(m_layer, m_oldPosition);
                m_image -> notifyLayersChanged();
                setUndo(true);
            }

    private:
        KisLayerSP m_layer;
        Q_INT32 m_oldPosition;
        Q_INT32 m_newPosition;
    };


    // -------------------------------------------------------

    class LayerAddCmd : public KisCommand {
        typedef KisCommand super;

    public:
        LayerAddCmd(KisUndoAdapter *adapter, KisImageSP img, KisLayerSP layer) : super(i18n("Add Layer"), adapter)
            {
                m_img = img;
                m_layer = layer;
                m_index = img->index(layer);
            }

        virtual ~LayerAddCmd()
            {
            }

        virtual void execute()
            {
                adapter() -> setUndo(false);
                m_img -> layerAdd(m_layer, m_index);
                adapter() -> setUndo(true);
            }

        virtual void unexecute()
            {
                adapter() -> setUndo(false);
                m_img -> layerRemove(m_layer);
                adapter() -> setUndo(true);
            }

    private:
        KisImageSP m_img;
        KisLayerSP m_layer;
        Q_INT32 m_index;
    };

    // -------------------------------------------------------

    class LayerRmCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        LayerRmCmd(KisUndoAdapter *adapter, KisImageSP img, KisLayerSP layer) : super(i18n("Remove Layer"))
            {
                m_adapter = adapter;
                m_img = img;
                m_layer = layer;
                m_index = img -> index(layer);
            }

        virtual ~LayerRmCmd()
            {
            }

        virtual void execute()
            {
                m_adapter -> setUndo(false);
                m_img -> layerRemove(m_layer);
                m_adapter -> setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);
                m_img -> layerAdd(m_layer, m_index);
                m_adapter -> setUndo(true);
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisLayerSP m_layer;
        Q_INT32 m_index;
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

KisImage::KisImage(KisDoc *doc, Q_INT32 width, Q_INT32 height,  KisColorSpace * colorSpace, const QString& name)
{
    m_profile = 0;
    init(doc, width, height, colorSpace, name);
    setName(name);
    m_dcop = 0L;
}

KisImage::KisImage(const KisImage& rhs) : QObject(), KShared(rhs)
{
    m_dcop = 0L;
    if (this != &rhs) {
        m_doc = rhs.m_doc;
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
        m_profile = rhs.m_profile;

        m_bkg = new KisBackground(this, rhs.width(), rhs.height());
        Q_CHECK_PTR(m_bkg);

        m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
        Q_CHECK_PTR(m_projection);

        m_layers.reserve(rhs.m_layers.size());

        for (vKisLayerSP_cit it = rhs.m_layers.begin(); it != rhs.m_layers.end(); ++it) {
            KisLayerSP layer = new KisLayer(**it);
            Q_CHECK_PTR(layer);

            layer -> setImage(this);
            m_layers.push_back(layer);
            m_layerStack.push_back(layer);
            m_activeLayer = layer;
        }

        m_annotations = rhs.m_annotations; // XXX the annotations would probably need to be deep-copied


        m_nserver = new KisNameServer(i18n("Layer %1"), rhs.m_nserver -> currentSeed() + 1);
        Q_CHECK_PTR(m_nserver);

        m_guides = rhs.m_guides;

    }

}



KisImageIface *KisImage::dcopObject()
{
    if (!m_dcop) {
        m_dcop = new KisImageIface(this);
        Q_CHECK_PTR(m_dcop);
    }
    return m_dcop;
}

KisImage::~KisImage()
{
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


QString KisImage::nextLayerName() const
{
    if (m_nserver -> currentSeed() == 0) {
        m_nserver -> number();
        return i18n("background");
    }

    return m_nserver -> name();
}

void KisImage::init(KisDoc *doc, Q_INT32 width, Q_INT32 height,  KisColorSpace * colorSpace, const QString& name)
{
    Q_ASSERT(colorSpace != 0);

    m_doc = doc;

    if (m_doc != 0) {
        m_adapter = m_doc -> undoAdapter();
        Q_ASSERT(m_adapter != 0);
    } else {
        m_adapter = 0;
    }

    m_nserver = new KisNameServer(i18n("Layer %1"), 1);
    Q_CHECK_PTR(m_nserver);
    m_name = name;

    m_colorSpace = colorSpace;
    m_bkg = new KisBackground(this, width, height);
    Q_CHECK_PTR(m_bkg);

    m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
    Q_CHECK_PTR(m_projection);

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
                                              TYPE_BGRA_8,
                                              hProfile ,
                                              TYPE_ARGB_8,
                                              INTENT_PERCEPTUAL,
                                              0);
#endif
}

void KisImage::resize(Q_INT32 w, Q_INT32 h, bool cropLayers)
{
    if (w != width() || h != height()) {
        if (m_adapter && m_adapter -> undo()) {
            m_adapter -> beginMacro("Resize image");
            m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));
        }

        m_width = w;
        m_height = h;

        m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
        Q_CHECK_PTR(m_projection);

        m_bkg = new KisBackground(this, w, h);
        Q_CHECK_PTR(m_bkg);

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
}

void KisImage::resize(const QRect& rc, bool cropLayers)
{
    resize(rc.width(), rc.height(), cropLayers);
}

void KisImage::scale(double sx, double sy, KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy)
{
    if (m_layers.empty()) return; // Nothing to scale

    // New image size. XXX: Pass along to discourage rounding errors?
    Q_INT32 w, h;
    w = (Q_INT32)(( width() * sx) + 0.5);
    h = (Q_INT32)(( height() * sy) + 0.5);

    if (w != width() || h != height()) {

        if (m_adapter && m_adapter -> undo()) {
            m_adapter->beginMacro("Scale image");
        }

        vKisLayerSP_it it;
        for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
            KisLayerSP layer = (*it);
            KisTransaction *cmd = 0;

            if (m_adapter && m_adapter -> undo()) {
                cmd = new KisTransaction("", layer.data());
                Q_CHECK_PTR(cmd);
            }

            layer -> scale(sx, sy, m_progress, filterStrategy);

            if (m_adapter && undoAdapter() -> undo()) {
                m_adapter->addCommand(cmd);
            }
        }

        if (m_adapter && m_adapter -> undo()) {
            m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));
        }

        m_width = w;
        m_height = h;

        m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
        Q_CHECK_PTR(m_projection);

        m_bkg = new KisBackground(this, w, h);
        Q_CHECK_PTR(m_bkg);

        if (m_adapter && m_adapter -> undo()) {
            m_adapter->endMacro();
        }
        notify();
        emit sigSizeChanged(KisImageSP(this), w, h);

    }
}

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

    m_bkg = new KisBackground(this, w, h);
    Q_CHECK_PTR(m_bkg);

    undoAdapter()->endMacro();

    emit sigSizeChanged(KisImageSP(this), w, h);
    notify();
}

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

        m_bkg = new KisBackground(this, w, h);
        Q_CHECK_PTR(m_bkg);

        undoAdapter()->endMacro();

        emit sigSizeChanged(KisImageSP(this), w, h);
        notify();
    }
}

void KisImage::convertTo(KisColorSpace * dstColorSpace, KisProfile *  dstProfile, Q_INT32 renderingIntent)
{
    // XXX profile() == profile() will mostly result in extra work being done here, but there doesn't seem to be a better way?
    if ( (m_colorSpace -> id() == dstColorSpace -> id())
         && profile()
         && dstProfile
         && (profile() == dstProfile) )
    {
//         kdDebug(DBG_AREA_CORE) << "KisImage: NOT GOING TO CONVERT\n";
        return;
    }

    if (undoAdapter() && m_adapter->undo()) {
        m_adapter->beginMacro(i18n("Convert Image Type")); //XXX: fix when string freeze over
    }

    vKisLayerSP_it it;
    for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
//         kdDebug() << "Converting layer " << ( *it )->name() << " from " << ( *it )->colorSpace()->id().name()
//                   << " to " << dstColorSpace->id().name() << "\n";

        (*it) -> convertTo(dstColorSpace, dstProfile, renderingIntent);
    }

    setProfile(dstProfile);

    m_projection->convertTo(dstColorSpace, dstProfile, renderingIntent);
    m_bkg->convertTo(dstColorSpace, dstProfile, renderingIntent);

    if (undoAdapter() && m_adapter->undo()) {

        m_adapter->addCommand(new KisConvertImageTypeCmd(undoAdapter(), this,
                                                         m_colorSpace, m_profile,  dstColorSpace, dstProfile));
        m_adapter->endMacro();
    }


    setColorSpace(dstColorSpace);

    notify();
    notifyLayersChanged();
}

KisProfile *  KisImage::profile() const
{
    return m_profile;
}

void KisImage::setProfile(const KisProfile * profile)
{
    // XXX: When we set a new profile, we should do a transform!
    if (profile && profile -> valid()) {
        m_profile = const_cast<KisProfile *>( profile );
        m_projection -> setProfile(const_cast<KisProfile *>( profile ));
    }
    else {
        m_profile = 0;
        m_projection -> setProfile(m_profile);
    }
    notify();
    emit(sigProfileChanged(m_profile));
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


bool KisImage::empty() const
{
    return m_layers.size() > 0;
}

vKisLayerSP KisImage::layers()
{
    return m_layers;
}

const vKisLayerSP& KisImage::layers() const
{
    return m_layers;
}

KisPaintDeviceImplSP KisImage::activeDevice()
{
    if (m_activeLayer) {
        return m_activeLayer.data();
    }

    return 0;
}

KisLayerSP KisImage::layerAdd(const QString& name, Q_UINT8 devOpacity)
{
    KisLayerSP layer;
    layer = new KisLayer(this, name, devOpacity);
    Q_CHECK_PTR(layer);

    // No need to fill the layer with something; it's empty and transparent
    // and doesn't have any pixels by default.
    if (layer && add(layer, -1)) {
        layer = activate(layer);

        if (layer) {
            top(layer);

            if (m_adapter->undo())
                m_adapter->addCommand(new LayerAddCmd(m_adapter, this, layer));
            m_doc->setModified(true);
            layer -> setVisible(true);
            emit sigLayersUpdated(this);
        }
    }

    return layer;
}

KisLayerSP KisImage::layerAdd(const QString& name, const KisCompositeOp& compositeOp, Q_UINT8 opacity, KisColorSpace * colorstrategy)
{
    KisLayerSP layer;
    layer = new KisLayer(colorstrategy, name);
    Q_CHECK_PTR(layer);

    layer -> setOpacity(opacity);
    layer -> setCompositeOp(compositeOp);

    if (layer && add(layer, -1)) {
        layer = activate(layer);

        if (layer) {
            top(layer);

            if (m_adapter->undo())
                m_adapter->addCommand(new LayerAddCmd(m_adapter, this, layer));
            m_doc->setModified(true);
            layer -> setVisible(true);
            emit sigLayersUpdated(this);
        }
    }

    return layer;
}


KisLayerSP KisImage::layerAdd(KisLayerSP l, Q_INT32 position)
{
    if (!add(l, -1))
        return 0;

    m_doc->setModified(true);
    setLayerPosition(l, position);

    if (m_adapter->undo())
        m_adapter->addCommand(new LayerAddCmd(m_adapter, this, l));

    l -> setVisible(true);
    emit sigLayersUpdated(this);

    return l;
}

void KisImage::layerRemove(KisLayerSP layer)
{
    if (layer) {
        m_doc->setModified(true);

        if (m_adapter->undo())
            m_adapter->addCommand(new LayerRmCmd(m_adapter, this, layer));

        rm(layer);
        emit sigLayersUpdated(this);
    }
}

void KisImage::layerNext(KisLayerSP l)
{
    if (l) {
        Q_INT32 npos = index(l);
        Q_INT32 n = nlayers();

        if (npos < 0 || npos >= n - 1)
            return;

        npos--;
        l = layer(npos);

        if (!l)
            return;

        if (!l -> visible()) {
            l -> setVisible(true);
            m_doc->setModified(true);
        }

        if (!activate(l))
            return;

        for (Q_INT32 i = 0; i < npos; i++) {
            l = layer(i);

            if (l) {
                l->setVisible(false);
            }
        }

        m_doc->setModified(true);
        emit sigLayersUpdated(this);
    }
}

void KisImage::layerPrev(KisLayerSP l)
{
    if (l) {
        Q_INT32 npos = index(l);
        Q_INT32 n = nlayers();

        if (npos < 0 || npos >= n - 1)
            return;

        npos++;
        l = layer(npos);

        if (!l)
            return;

        if (!l -> visible()) {
            l -> setVisible(true);
            m_doc->setModified(true);
        }

        if (!activate(l))
            return;

        for (Q_INT32 i = 0; i < npos; i++) {
            l = layer(i);

            if (l) {
                l->setVisible(false);
            }
        }

        m_doc->setModified(true);
        emit sigLayersUpdated(this);
    }
}

void KisImage::setLayerProperties(KisLayerSP layer, Q_UINT8 opacity, const KisCompositeOp& compositeOp,    const QString& name)
{
    if (layer) {
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

        m_doc->setModified(true);
        emit sigLayersUpdated(this);

        notify();
    }
}


const KisLayerSP KisImage::activeLayer() const
{
    return m_activeLayer;
}

KisLayerSP KisImage::activeLayer()
{
    return m_activeLayer;
}

KisLayerSP KisImage::activate(KisLayerSP layer)
{
    vKisLayerSP_it it;

    if (m_layers.empty() || !layer)
        return 0;

    it = qFind(m_layers.begin(), m_layers.end(), layer);

    if (it == m_layers.end()) {
        layer = m_layers[0];
        it = m_layers.begin();
    }

    if (layer) {
        it = qFind(m_layerStack.begin(), m_layerStack.end(), layer);

        if (it != m_layerStack.end())
            m_layerStack.erase(it);

        m_layerStack.insert(m_layerStack.begin() + 0, layer);
    }

    if (layer != m_activeLayer) {
        if (m_activeLayer) m_activeLayer->deactivate();
        m_activeLayer = layer;
        if (m_activeLayer) m_activeLayer->activate();
    }


    return layer;
}

KisLayerSP KisImage::activateLayer(Q_INT32 n)
{
    if (n < 0 || static_cast<Q_UINT32>(n) > m_layers.size())
        return 0;

    return activate(m_layers[n]);
}

KisLayerSP KisImage::layer(Q_INT32 n)
{
    if (n < 0 || static_cast<Q_UINT32>(n) > m_layers.size())
        return 0;

    return m_layers[n];

}

KisLayerSP KisImage::findLayer(const QString & name)
{
    for (Q_UINT32 i = 0; i < m_layers.size(); i++) {
        if (m_layers[i]->name() == name)
            return m_layers[i];
    }
    return 0;
}


Q_INT32 KisImage::index(const KisLayerSP &layer)
{
    for (Q_UINT32 i = 0; i < m_layers.size(); i++) {
        if (m_layers[i] == layer)
            return i;
    }

    return -1;
}

KisLayerSP KisImage::layer(const QString& name)
{
    for (vKisLayerSP_it it = m_layers.begin(); it != m_layers.end(); ++it) {
        if ((*it) -> name() == name)
            return *it;
    }

    return 0;
}

KisLayerSP KisImage::layer(Q_UINT32 npos)
{
    if (npos >= m_layers.size())
        return 0;

    return m_layers[npos];
}

bool KisImage::add(KisLayerSP layer, Q_INT32 position)
{
    if (layer == 0)
        return false;

    if (layer -> image() && layer -> image() != KisImageSP(this))
        return false;

    if (qFind(m_layers.begin(), m_layers.end(), layer) != m_layers.end())
        return false;

    layer -> setImage(KisImageSP(this));

    if (position == -1) {
        // Add to bottom of layer stack
        position = m_layers.size();
    }

    m_layers.insert(m_layers.begin() + position, layer);
    activate(layer);

    m_layerStack.push_back(layer);
    return true;
}

void KisImage::rm(KisLayerSP layer)
{
    if (layer == 0)
        return;

    vKisLayerSP_it it = qFind(m_layers.begin(), m_layers.end(), layer);

    if (it == m_layers.end())
        return;

    m_layers.erase(it);
    it = qFind(m_layerStack.begin(), m_layerStack.end(), layer);

    if (it != m_layerStack.end())
        m_layerStack.erase(it);

    layer -> setImage(0);

    if (layer == m_activeLayer) {
        if (m_layers.empty()) {
            m_activeLayer = 0;
        } else {
            activate(m_layerStack[0]);
        }
    }
}

bool KisImage::raise(KisLayerSP layer)
{
    Q_INT32 position;

    if (layer == 0)
        return false;

    position = index(layer);

    if (position <= 0)
        return false;

    return setLayerPosition(layer, position - 1);
}

KCommand *KisImage::raiseLayerCommand(KisLayerSP layer)
{
    return new KisLayerPositionCommand(i18n("Raise Layer"), this, layer, index(layer) - 1);
}

bool KisImage::lower(KisLayerSP layer)
{
    Q_INT32 position;
    Q_INT32 size;

    if (layer == 0)
        return false;

    position = index(layer);
    size = m_layers.size();

    if (position >= size)
        return false;

    return setLayerPosition(layer, position + 1);
}

KCommand *KisImage::lowerLayerCommand(KisLayerSP layer)
{
    return new KisLayerPositionCommand(i18n("Lower Layer"), this, layer, index(layer) + 1);
}

bool KisImage::top(KisLayerSP layer)
{
    Q_INT32 position;

    if (layer == 0)
        return false;

    position = index(layer);

    if (position == 0)
        return false;

    return setLayerPosition(layer, 0);
}

KCommand *KisImage::topLayerCommand(KisLayerSP layer)
{
    return new KisLayerPositionCommand(i18n("Layer Top"), this, layer, 0);
}

bool KisImage::bottom(KisLayerSP layer)
{
    Q_INT32 position;
    Q_INT32 size;

    if (layer == 0)
        return false;

    position = index(layer);
    size = m_layers.size();

    if (position >= size - 1)
        return false;

    return setLayerPosition(layer, size - 1);
}

KCommand *KisImage::bottomLayerCommand(KisLayerSP layer)
{
    return new KisLayerPositionCommand(i18n("Layer Bottom"), this, layer, nlayers() - 1);
}

bool KisImage::setLayerPosition(KisLayerSP layer, Q_INT32 position)
{
    Q_INT32 old;
    Q_INT32 nlayers;

    if (layer == 0)
        return false;

    old = index(layer);

    if (old < 0)
        return false;

    nlayers = m_layers.size();

    if (position < 0)
        position = 0;

    if (position >= nlayers)
        position = nlayers - 1;

    if (old == position)
        return true;

    if (position < old) {
        m_layers.erase(m_layers.begin() + old);
        m_layers.insert(m_layers.begin() + position, layer);
    }
    else {
        m_layers.insert(m_layers.begin() + position + 1, layer);
        m_layers.erase(m_layers.begin() + old);
    }

    return true;
}

Q_INT32 KisImage::nlayers() const
{
    return m_layers.size();
}

Q_INT32 KisImage::nHiddenLayers() const
{
    Q_INT32 n = 0;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it) {
        const KisLayerSP& layer = *it;

        if (!layer -> visible()) {
            n++;
        }
    }

    return n;
}

Q_INT32 KisImage::nLinkedLayers() const
{
    Q_INT32 n = 0;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it) {
        const KisLayerSP& layer = *it;

        if (layer -> linked()) {
            n++;
        }
    }

    return n;
}

void KisImage::flatten()
{
    vKisLayerSP beforeLayers = m_layers;

    if (m_layers.empty()) return;

    KisLayerSP dst = new KisLayer(this, nextLayerName(), OPACITY_OPAQUE);
    Q_CHECK_PTR(dst);

    KisFillPainter painter(dst.data());

    vKisLayerSP mergeLayers = layers();

    KisLayerSP bottomLayer = mergeLayers.back();
    QString bottomName =  bottomLayer -> name();

    KisMerge<isVisible, All> visitor(this);
    visitor(painter, mergeLayers);
    dst -> setName(bottomName);

    add(dst, -1);

    notify();
    notifyLayersChanged();

    if (m_adapter && m_adapter -> undo()) {
        m_adapter->addCommand(new KisChangeLayersCmd(m_adapter, this, beforeLayers, m_layers, i18n("Flatten Image")));
    }
}

void KisImage::mergeVisibleLayers()
{
    vKisLayerSP beforeLayers = m_layers;

    KisLayerSP dst = new KisLayer(this, nextLayerName(), OPACITY_OPAQUE);
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
}

void KisImage::mergeLinkedLayers()
{
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
}

void KisImage::mergeLayer(KisLayerSP l)
{
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
}


void KisImage::enableUndo(KoCommandHistory *history)
{
    m_undoHistory = history;
}

void KisImage::renderToPainter(Q_INT32 x1,
                               Q_INT32 y1,
                               Q_INT32 x2,
                               Q_INT32 y2,
                               QPainter &painter,
                               KisProfile *  monitorProfile,
                               float exposure)
{

//    QRect r = m_projection->extent();
//    kdDebug() << "projection extent: " << r.x() << ", " << r.y() << ", " << r.width() << ", " << r.height() << "...\n";

    Q_INT32 w = x2 - x1 + 1;
    Q_INT32 h = y2 - y1 + 1;

    QImage img = m_projection->convertToQImage(monitorProfile, x1, y1, w, h, exposure);

    if (m_activeLayer != 0 && m_activeLayer -> hasSelection()) {
        m_activeLayer -> selection()->paintSelection(img, x1, y1, w, h);
    }

    if (!img.isNull()) {

#ifdef __BIG_ENDIAN__
        cmsDoTransform(m_bigEndianTransform, img.bits(), img.bits(), w * h);
#endif
        painter.drawImage(x1, y1, img, 0, 0, w, h);
    }


}

KisPaintDeviceImplSP KisImage::mergedImage()
{
    KisPaintDeviceImplSP dev = new KisPaintDeviceImpl(colorSpace(), "merged image");
    dev -> setProfile(profile());

    KisPainter gc;

    gc.begin(dev.data());

    if (!m_layers.empty()) {
        KisFlatten<flattenAllVisible> visitor(0, 0, width(), height());
        visitor(gc, m_layers);
    }

    gc.end();

    return dev;
}

KisColor KisImage::mergedPixel(Q_INT32 x, Q_INT32 y)
{
    KisPaintDeviceImplSP dev = new KisPaintDeviceImpl(colorSpace(), "merged pixel");
    dev -> setProfile(profile());

    KisPainter gc;

    gc.begin(dev.data());

    if (!m_layers.empty()) {
        KisFlatten<flattenAllVisible> visitor(x, y, 1, 1);

        visitor(gc, m_layers);
    }

    gc.end();

    return dev -> colorAt(x, y);
}

void KisImage::notify()
{
    notify(QRect(0, 0, width(), height()));
}

void KisImage::notify(const QRect& rc)
{
    // Composite the image
    KisPainter gc;

    gc.begin(m_projection.data());

    gc.bitBlt(rc.x(), rc.y(), COMPOSITE_COPY, m_bkg.data(), rc.x(), rc.y(), rc.width(), rc.height());

    if (!m_layers.empty()) {
        KisFlatten<flattenAllVisible> visitor(rc.x(), rc.y(), rc.width(), rc.height());
        visitor(gc, m_layers);
    }

    gc.end();


    if (rc.isValid()) {
        emit sigImageUpdated(KisImageSP(this), rc);
    }

}

void KisImage::notifyLayersChanged()
{
    emit sigLayersChanged(KisImageSP(this));
}

QRect KisImage::bounds() const
{
    return QRect(0, 0, width(), height());
}

KisUndoAdapter* KisImage::undoAdapter() const
{
    return m_adapter;
}

KisGuideMgr *KisImage::guides() const
{
    return const_cast<KisGuideMgr*>(&m_guides);
}

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

    m_bkg = new KisBackground(this, m_width, m_height);
    Q_CHECK_PTR(m_bkg);

    m_projection = new KisLayer(this, "projection", OPACITY_OPAQUE);
    Q_CHECK_PTR(m_projection);
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
    if (m_profile) {
        addAnnotation(m_profile -> annotation());
    } else {
        removeAnnotation("icc");
    }

    return m_annotations.begin();
}

vKisAnnotationSP_it KisImage::endAnnotations()
{
    return m_annotations.end();
}

#include "kis_image.moc"


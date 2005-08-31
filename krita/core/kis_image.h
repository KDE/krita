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
#ifndef KIS_IMAGE_H_
#define KIS_IMAGE_H_

#include <qobject.h>
#include <qstring.h>
#include <qvaluevector.h>
#include <qmutex.h>

#include <ksharedptr.h>
#include <kurl.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_guide.h"
#include "kis_scale_visitor.h"
#include "resources/kis_profile.h"
#include "kis_annotation.h"


#include <koffice_export.h>

class KoCommandHistory;
class KisNameServer;
class KisUndoAdapter;
class KisPainter;
class DCOPObject;
class KisDoc;
class KCommand;
class KisCompositeOp;
class KisColor;
class KisFilterStrategy;
class KisImageIface;

class KRITACORE_EXPORT KisImage : public QObject, public KShared {
    Q_OBJECT

public:
    KisImage(KisDoc *doc, Q_INT32 width, Q_INT32 height,
         KisAbstractColorSpace * colorStrategy, const QString& name);
    KisImage(const KisImage& rhs);
    virtual ~KisImage();
    virtual KisImageIface *dcopObject();

public:
    // Composite the specified tile onto the projection layer.
    virtual void renderToProjection(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

    /// Paint the specified rect onto the painter, adjusting the colors using the
    /// given profile. The exposure setting is used if the image has a high dynamic range.
    virtual void renderToPainter(Q_INT32 x1,
                     Q_INT32 y1,
                     Q_INT32 x2,
                     Q_INT32 y2,
                     QPainter &painter,
                     KisProfileSP profile,
                     float exposure = 0.0f);


public:
    QString name() const;
    void setName(const QString& name);

    QString description() const;
    void setDescription(const QString& description);

    QString nextLayerName() const;

    void resize(Q_INT32 w, Q_INT32 h, bool cropLayers = false);
    void resize(const QRect& rc, bool cropLayers = false);

    void scale(double sx, double sy, KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy);
        void rotate(double angle, KisProgressDisplayInterface *m_progress);
        void shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress);

    void convertTo(KisAbstractColorSpace * dstColorStrategy, KisProfileSP dstProfile, Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

    // Get the profile associated with this image
    KisProfileSP profile() const;

    // Set the profile associated with this image
    void setProfile(const KisProfileSP& profile);


    void enableUndo(KoCommandHistory *history);

    KisAbstractColorSpace * colorStrategy() const;
    void setColorStrategy(KisAbstractColorSpace * colorStrategy);

    KURL uri() const;
    void uri(const KURL& uri);

    KoUnit::Unit unit() const;
    void unit(const KoUnit::Unit& u);

    // Resolution of the image == XXX: per inch?
        double xRes();
    double yRes();
    void setResolution(double xres, double yres);

    Q_INT32 width() const;
    Q_INT32 height() const;

    bool empty() const;

    vKisLayerSP layers();
    const vKisLayerSP& layers() const;

    /** returns a paintdevice that contains the merged layers of this image, within
     * the bounds of this image (with the colorspace and profile of this image) */
    KisPaintDeviceImplSP mergedImage();

    /*
     * Returns the colour of the merged image at pixel (x, y).
     */
    KisColor mergedPixel(Q_INT32 x, Q_INT32 y);

    // Get the active painting device
    KisPaintDeviceImplSP activeDevice();

    KisLayerSP layerAdd(const QString& name, QUANTUM devOpacity);
    KisLayerSP layerAdd(const QString& name, const KisCompositeOp& compositeOp,  QUANTUM opacity,  KisAbstractColorSpace * colorstrategy);
    KisLayerSP layerAdd(KisLayerSP layer, Q_INT32 position);

    void layerRemove(KisLayerSP layer);
    void layerNext(KisLayerSP layer);
    void layerPrev(KisLayerSP layer);

    void setLayerProperties(KisLayerSP layer, QUANTUM opacity, const KisCompositeOp& compositeOp, const QString& name);

    KisLayerSP activeLayer();
    const KisLayerSP activeLayer() const;

    KisLayerSP activate(KisLayerSP layer);
    KisLayerSP activateLayer(Q_INT32 n);

    KisLayerSP layer(Q_INT32 n);
    KisLayerSP findLayer(const QString & name);

    Q_INT32 index(const KisLayerSP &layer);

    KisLayerSP layer(const QString& name);
    KisLayerSP layer(Q_UINT32 npos);

    bool add(KisLayerSP layer, Q_INT32 position);

    void rm(KisLayerSP layer);

    bool raise(KisLayerSP layer);
    bool lower(KisLayerSP layer);
    bool top(KisLayerSP layer);
    bool bottom(KisLayerSP layer);

    bool setLayerPosition(KisLayerSP layer, Q_INT32 position);

    Q_INT32 nlayers() const;
    Q_INT32 nHiddenLayers() const;
    Q_INT32 nLinkedLayers() const;

    KCommand *raiseLayerCommand(KisLayerSP layer);
    KCommand *lowerLayerCommand(KisLayerSP layer);
    KCommand *topLayerCommand(KisLayerSP layer);
    KCommand *bottomLayerCommand(KisLayerSP layer);


    // Merge all visible layers and discard hidden ones.
    void flatten();

    void mergeVisibleLayers();
    void mergeLinkedLayers();

    /**
     * Merge the specified layer with the layer
     * below this layer, remove the specified layer.
     */
    void mergeLayer(KisLayerSP l);

    QRect bounds() const;

    void notify();
    void notify(Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height);
    void notify(const QRect& rc);

    void notifyLayersChanged();

    KisUndoAdapter *undoAdapter() const;
    KisGuideMgr *guides() const;

    /**
     * Add an annotation for this image. This can be anything: Gamma, EXIF, etc.
     * Note that the "icc" annotation is reserved for the colour strategies.
     * If the annotation already exists, overwrite it with this one.
     */
    void addAnnotation(KisAnnotationSP annotation);
    /** get the annotation with the given type, can return 0 */
    KisAnnotationSP annotation(QString type);
    /** delete the annotation, if the image contains it */
    void removeAnnotation(QString type);
    /** start of an iteration over the annotations of this image (including the ICC Profile) */
    vKisAnnotationSP_it beginAnnotations();
    /** end of an iteration over the annotations of this image */
    vKisAnnotationSP_it endAnnotations();

signals:
    void activeSelectionChanged(KisImageSP image);
    void selectionChanged(KisImageSP image);
    void update(KisImageSP image, const QRect& rc);
    void layersChanged(KisImageSP image);
    void layersUpdated(KisImageSP image);
    void imageUpdated(KisImageSP img);
    void sizeChanged(KisImageSP image, Q_INT32 w, Q_INT32 h);
    void profileChanged(KisProfileSP profile);


public slots:
    void slotSelectionChanged();
    void slotSelectionChanged(const QRect& r);

private:
    KisImage& operator=(const KisImage& rhs);
    void init(KisDoc *doc, Q_INT32 width, Q_INT32 height,  KisAbstractColorSpace * colorStrategy, const QString& name);

private:
    KisDoc *m_doc;
    KoCommandHistory *m_undoHistory;
    KURL m_uri;
    QString m_name;
    QString m_description;

    KisProfileSP m_profile;

    Q_INT32 m_width;
    Q_INT32 m_height;

    double m_xres;
    double m_yres;

    KoUnit::Unit m_unit;

    KisAbstractColorSpace * m_colorStrategy;

    bool m_dirty;
    QRect m_dirtyRect;

    KisBackgroundSP m_bkg;
    KisLayerSP m_projection;
    vKisLayerSP m_layers; // Contains the list of all layers
    vKisLayerSP m_layerStack; // Contains a stack of layers in
                  // order of activation, so that when
                  // we remove a layer can activate
                  // the previously activated layer
                  // instead of the bottom or topmost
                  // layer.
    KisLayerSP m_activeLayer;

    KisNameServer *m_nserver;
    KisUndoAdapter *m_adapter;
    KisGuideMgr m_guides;

    KisImageIface *m_dcop;

    QPixmap m_pixmap;

    vKisAnnotationSP m_annotations;

#ifdef __BIG_ENDIAN__
    cmsHTRANSFORM m_bigEndianTransform;
#endif
};

#endif // KIS_IMAGE_H_

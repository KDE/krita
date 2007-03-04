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

#include <QObject>
#include <QString>
#include <QPainter>
#include <QRect>
#include <QRegion>

#include <kis_shared.h>
#include <kurl.h>

#include "KoUnit.h"

#include "kis_global.h"
#include "kis_types.h"

#include <krita_export.h>

class KCommand;

class KoCommandHistory;
class KoColorSpace;
class KoCompositeOp;
class KoColor;

class KisNameServer;
class KisUndoAdapter;
class KisPainter;
class KisFilterStrategy;
class KisImageIface;
class KoColorProfile;
class KisProgressDisplayInterface;
class KisPaintLayer;
class KisPerspectiveGrid;

/**
 * This is the image class, it contains a tree of KisLayer stack and meta information about the image. And it also provides some functions
 * to manipulate the whole image.
 */
class KRITAIMAGE_EXPORT KisImage : public QObject, public KisShared {

    Q_OBJECT

public:
    KisImage(KisUndoAdapter * adapter, qint32 width, qint32 height, KoColorSpace * colorSpace, const QString& name);
    KisImage(const KisImage& rhs);
    virtual ~KisImage();

public:

    /**
     * Paint the specified rect onto the painter, adjusting the colors
     * using the given profile. The exposure setting is used if the image
     * has a high dynamic range.
     */
    virtual void renderToPainter(qint32 srcX,
                                 qint32 srcY,
                                 qint32 dstX,
                                 qint32 dstY,
                                 qint32 width,
                                 qint32 height,
                                 QPainter &painter,
                                 KoColorProfile *profile,
                                 float exposure = 0.0f);

    /**
     * Render the projection onto a QImage.
     */
     virtual QImage convertToQImage(qint32 x1,
                                    qint32 y1,
                                    qint32 width,
                                    qint32 height,
                                    KoColorProfile * profile,
                                    float exposure = 0.0f);
#if 0
     /**
      * Render the projection scaled onto a QImage. Use this when
      * zoom < 100% to avoid color-adjusting pixels that will be
      * filtered away anyway.
      */
     virtual QImage convertToQImage(const QRect& r,
                                    const double xScale, const double yScale,
                                    KoColorProfile *profile,
                                    float exposure = 0.0f);
#endif
    /**
     * Lock the image to make sure no recompositing-causing signals get emitted
     * while you're messing with the layers. Don't forget to unlock again.
     */
    void lock();

    /**
     * Unlock the image to make sure the rest of Krita learns about changes in the image
     * again. If the rootLayer is dirty on unlocking, an imgUpdated signal is sent out
     * immediately.
     */
    void unlock();

    /**
     * Returns true if the image is locked.
     */
    bool locked() const;

    KoColor backgroundColor() const;
    void setBackgroundColor(const KoColor & color);

    QString name() const;
    void setName(const QString& name);

    QString description() const;
    void setDescription(const QString& description);

    /**
     * Retrieve the next automatic layername.
     */
    QString nextLayerName() const;

    /**
     * Set the automatic layer name counter one back.
     */
    void rollBackLayerName();

    /**
     * @return the perspective grid associated to this image
     */
    KisPerspectiveGrid* perspectiveGrid();

    /**
     * Resize the image to the specified width and height. The resize
     * method handles the creating on an undo step itself.
     *
     * @param w the new width of the image
     * @param h the new height of the image
     * @param x the x position of the crop on all layer if cropLayers is true
     * @param y the y position of the crop on all layer if cropLayers is true
     * @param cropLayers if true, all layers are cropped to the new size.
     */
    void resize(qint32 w, qint32 h, qint32 x = 0, qint32 y = 0,  bool cropLayers = false);

    /**
     * Resize the image to the specified width and height. The resize
     * method handles the creating on an undo step itself.
     *
     * @param rc the rect describing the new width and height of the image
     * @param cropLayers if true, all layers are cropped to the new rect
     */
    void resize(const QRect& rc, bool cropLayers = false);

    /**
     * Execute a scale transform on all layers in this image.
     */
    void scale(double sx, double sy, KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy);

    /**
     * Execute a rotate transform on all layers in this image.
     */
    void rotate(double radians, KisProgressDisplayInterface *m_progress);

    /**
     * Execute a shear transform on all layers in this image.
     */
    void shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress);

    /**
     * Convert the image and all its layers to the dstColorSpace
     */
    void convertTo(KoColorSpace * dstColorSpace, qint32 renderingIntent = INTENT_PERCEPTUAL);

    // Get the profile associated with this image
    KoColorProfile *  profile() const;

    /**
     * Set the profile of the image to the new profile and do the same for
     * all layers that have the same colorspace and profile as the image.
     * It doesn't do any pixel conversion.
     *
     * This is essential if you have loaded an image that didn't
     * have an embedded profile to which you want to attach the right profile.
     */
    void setProfile(const KoColorProfile * profile);

    /**
     * Replace the current undo adapter with the specified undo adapter.
     * The current undo adapter will _not_ be deleted.
     */
    void setUndoAdapter(KisUndoAdapter * undoAdapter);

    /**
     * Returns the current undo adapter. You can add new commands to the
     * undo stack using the adapter
     */
    KisUndoAdapter *undoAdapter() const;

    /**
     * Returns true if this image wants undo information, false otherwise
     */
    bool undo() const;

    /**
     * Tell the image it's modified; this emits the sigImageModified signal. This happens
     *  when the image needs to be saved
     */
    void setModified();

    /**
     * The default colorspace of this image: new layers will have this colorspace
     * and the projection will have this colorspace.
     */
    KoColorSpace * colorSpace() const;

    /**
     * X resolution in pixels per pt
     */
    double xRes() const;

    /**
     * Y resolution in pixels per pt
     */
    double yRes() const;

    /**
     * Set the resolution in pixels per pt.
     */
    void setResolution(double xres, double yres);

    /**
     * Return the width of the image
     */
    qint32 width() const;

    /**
     * Return the height of the image
     */
    qint32 height() const;

    /**
     * Return the size of the image
     */
    QSize size() const { return QSize( width(), height() ); }

    /**
     * @return a region built from the extents of all layers in this image.
     */
    QRegion extent() const;

    /**
     * @return a region built from all the dirty rects in all layers in this image
     */
    QRegion dirtyRegion() const;

    /**
     *  returns a paintdevice that contains the merged layers of this image, within
     * the bounds of this image (with the colorspace and profile of this image)
     */
    KisPaintDeviceSP mergedImage();

    /*
     * Returns the color of the merged image at pixel (x, y).
     */
    KoColor mergedPixel(qint32 x, qint32 y);

    // This was only used from the performance tests?
    KisLayerSP newLayer(const QString& name, quint8 opacity, const QString & compositeOp, KoColorSpace * colorspace);

    /// Get the active painting device. Returns 0 if the active layer does not have a paint device.
    KisPaintDeviceSP activeDevice();

    void setLayerProperties(KisLayerSP layer, quint8 opacity, const KoCompositeOp * compositeOp, const QString& name);

    KisGroupLayerSP rootLayer() const;
    KisLayerSP activeLayer() const;

    /// Return the projection; that is, the complete, composited representation
    /// of this image.
    KisPaintDeviceSP projection();

    KisLayerSP activateLayer(KisLayerSP layer);
    KisLayerSP findLayer(const QString& name) const;
    KisLayerSP findLayer(int id) const;

    /// Move layer to specified position
    bool moveLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis);

    /**
     * Add an already existing layer to the image. The layer is put on top
     * of the layers in the specified layergroup
     * @param layer the layer to be added
     * @param parent the parent layer
     */
    bool addLayer(KisLayerSP layer, KisGroupLayerSP parent);

    /**
     * Add already existing layer to image.
     *
     * @param layer the layer to be added
     * @param parent the parent layer
     * @param aboveThis in the list with child layers of the specified
     *                  parent, add this layer above the specified sibling.
     *                  if 0, the layer is put in the lowermost position in
     *                  its group.
     * @param notify If true, the image is immediately recomposited, if false,
     *               no recomposition is done yet. The added layer is all
     *
     * returns false if adding the layer didn't work, true if the layer got added
     */
    bool addLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis);

    /// Remove layer
    bool removeLayer(KisLayerSP layer);

    /// Move layer up one slot
    bool raiseLayer(KisLayerSP layer);

    /// Move layer down one slot
    bool lowerLayer(KisLayerSP layer);

    /// Move layer to top slot
    bool toTop(KisLayerSP layer);

    /// Move layer to bottom slot
    bool toBottom(KisLayerSP layer);

    qint32 nlayers() const;
    qint32 nHiddenLayers() const;

    KCommand *raiseLayerCommand(KisLayerSP layer);
    KCommand *lowerLayerCommand(KisLayerSP layer);
    KCommand *topLayerCommand(KisLayerSP layer);
    KCommand *bottomLayerCommand(KisLayerSP layer);

    /**
     * Merge all visible layers and discard hidden ones.
     * The resulting layer will be activated.
     */
    void flatten();

    /**
     * Merge the specified layer with the layer
     * below this layer, remove the specified layer.
     */
    void mergeLayer(KisLayerSP l);

    QRect bounds() const;

    /// use if the layers have changed _completely_ (eg. when flattening)
    void notifyLayersChanged();

    void notifyPropertyChanged(KisLayerSP layer);

    /**
       Called whenever a layer has changed. The layer is added to a
       list of dirty layers and as soon as the document stores the
       command that is now in progress, the image will be notified.
       Then the image will notify the dirty layers, the dirty layers
       will notify their parents & emit a signal that will be caught
       by the layer box, which will request a new thumbnail.
    */
    void notifyLayerUpdated(KisLayerSP layer);

    void setColorSpace(KoColorSpace * colorSpace);
    void setRootLayer(KisGroupLayerSP rootLayer);

    /**
     * Add an annotation for this image. This can be anything: Gamma, EXIF, etc.
     * Note that the "icc" annotation is reserved for the color strategies.
     * If the annotation already exists, overwrite it with this one.
     */
    void addAnnotation(KisAnnotationSP annotation);

    /** get the annotation with the given type, can return 0 */
    KisAnnotationSP annotation(const QString& type);

    /** delete the annotation, if the image contains it */
    void removeAnnotation(const QString& type);

    /**
     * Start of an iteration over the annotations of this image (including the ICC Profile)
     */
    vKisAnnotationSP_it beginAnnotations();

    /** end of an iteration over the annotations of this image */
    vKisAnnotationSP_it endAnnotations();

signals:

    void sigActiveSelectionChanged(KisImageSP image);
    void sigSelectionChanged(KisImageSP image);
    void sigSelectionChanged(KisImageSP image, const QRect& rect);

    /// Emitted after a different layer is made active.
    void sigLayerActivated(KisLayerSP layer);

    /// Emitted after a layer is added: you can find out where by asking it for its parent(), et al.
    void sigLayerAdded(KisLayerSP layer);

    /** Emitted after a layer is removed.
        It's no longer in the image, but still exists, so @p layer is valid.

        @param layer the removed layer
        @param parent the parent of the layer, before it was removed
        @param wasAboveThis the layer it was above, before it was removed.
    */
    void sigLayerRemoved(KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAboveThis);

    /** Emitted after a layer is moved to a different position under its parent layer, or its parent changes.

        @param previousParent the parent of the layer, before it was moved
        @param wasAboveThis the layer it was above, before it was moved.
    */
    void sigLayerMoved(KisLayerSP layer, KisGroupLayerSP previousParent, KisLayerSP wasAboveThis);

    /// Emitted after a layer's properties (visible, locked, opacity, composite op, name, ...) change
    void sigLayerPropertiesChanged(KisLayerSP layer);

    /** Emitted when the list of layers has changed completely.
        This means e.g. when the image is flattened, but not when it is rotated,
        as the layers only change internally then.
    */
    void sigLayersChanged(KisGroupLayerSP rootLayer);

    /**
       Emitted whenever an action has caused the image to be
       recomposited.

       @param rc The recty that has been recomposited.
     */
    void sigImageUpdated( const QRect & );

    /**
       Emitted whenever the image has been modified, so that it
       doesn't match with the version saved on disk.
     */
    void sigImageModified();

    void sigSizeChanged(qint32 w, qint32 h);
    void sigProfileChanged(KoColorProfile *  profile);
    void sigColorSpaceChanged(KoColorSpace*  cs);
    /// Emitted when any layer's mask info got updated (or when the current layer changes)
    void sigMaskInfoChanged();

public slots:
    void slotSelectionChanged();
    void slotSelectionChanged(const QRect& r);

   void slotProjectionUpdated( const QRect & rc );
    /**
       Called whenever a KisCommand has been executed. This notifies
       the layers, who then notify the layerbox that they are ready
       for new thumbnails
    */
    void slotCommandExecuted();


private:
    KisImage& operator=(const KisImage& rhs);
    void init(KisUndoAdapter * adapter, qint32 width, qint32 height,  KoColorSpace * colorSpace, const QString& name);
    void emitSizeChanged();


private:
    class KisImagePrivate;
    KisImagePrivate * m_d;
};

#endif // KIS_IMAGE_H_

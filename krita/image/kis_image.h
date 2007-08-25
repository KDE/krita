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
#include <QBitArray>

#include <kdebug.h>
#include <kurl.h>

#include <KoColorConversionTransformation.h>
#include "KoUnit.h"

#include "kis_global.h"
#include "kis_types.h"
#include "kis_shared.h"
#include "kis_node_graph_listener.h"
#include "kis_node_facade.h"
#include <krita_export.h>

class KoColorSpace;
class KoCompositeOp;
class KoColor;

class KisActionRecorder;
class KisUndoAdapter;
class KisFilterStrategy;
class KoColorProfile;
class KisProgressDisplayInterface;
class KisPerspectiveGrid;

/**
 * This is the image class, it contains a tree of KisLayer stack and meta information about the image. And it also provides some functions
 * to manipulate the whole image.
 */
class KRITAIMAGE_EXPORT KisImage : public QObject, public KisNodeFacade, public KisNodeGraphListener, public KisShared {

    Q_OBJECT

public:
    KisImage(KisUndoAdapter * adapter, qint32 width, qint32 height, KoColorSpace * colorSpace, const QString& name);
    KisImage(const KisImage& rhs);
    virtual ~KisImage();

public: // KisNodeGraphListener implementation

    void aboutToAddANode( KisNode *parent, int index );
    void nodeHasBeenAdded( KisNode *parent, int index );
    void aboutToRemoveANode( KisNode *parent, int index );
    void nodeHasBeenRemoved( KisNode *parent, int index );
    void aboutToMoveNode( KisNode * parent, int oldIndex, int newIndex );
    void nodeHasBeenMoved( KisNode * parent, int oldIndex, int newIndex );

public:

    /**
     * Paint the specified rect onto the painter, adjusting the colors
     * using the given profile. The exposure setting is used if the image
     * has a high dynamic range.
     */
    void renderToPainter(qint32 srcX,
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
    QImage convertToQImage(qint32 x1,
                           qint32 y1,
                           qint32 width,
                           qint32 height,
                           KoColorProfile * profile,
                           float exposure = 0.0f);

     /**
      * Render the projection scaled onto a QImage. Use this when
      * zoom < 100% to avoid color-adjusting pixels that will be
      * filtered away anyway. It uses nearest-neighbour sampling, so
      * the result is inaccurate and ugly. Set the option "fast_zoom"
      * to true to make Krita use this.
      *
      * XXX: Implement the mask option to draw the mask onto the
      * scaled image.
      *
      * @param r the source rectangle in pixels that needs to be drawn
      * @param xScale the X axis scale (1.0 == 100%)
      * @param yScale the Y axis scale (1.0 == 100%)
      * @param projection the display profile
      * @param exposure the exposure (for hdr images)
      * @param mask the mask that will be rendered on top of the image
      * @return a qimage containing the sampled image pixels
      */
     QImage convertToQImage(const QRect& r,
                            const double xScale, const double yScale,
                            KoColorProfile *profile,
                            float exposure = 0.0f,
                            KisSelectionSP mask = 0);

    /**
     * Lock the image to make sure no recompositing-causing signals get emitted
     * while you're messing with the layers. Don't forget to unlock again.
     */
    void lock();

    /**
     * Unlock the image to make sure the rest of Krita learns about changes in the image
     * again.
     */
    void unlock();

    /**
     * Returns true if the image is locked.
     */
    bool locked() const;

    KoColor backgroundColor() const;
    void setBackgroundColor(const KoColor & color);

    /**
     * @return the global selection object or 0 if there is none. The
     * global selection is always read-write.
     */
    KisSelectionSP globalSelection() const;

    /**
     * Replaces the current global selection with globalSelection. If
     * globalSelection is empty, a new selection object will be
     * created that is by default completely deselected.
     */
    void setGlobalSelection( KisSelectionSP globalSelection = 0 );

    /**
     * Removes the global selection.
     */
    void removeGlobalSelection();

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
    void convertTo(KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual);

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
     * @return the action recorder associated with this image
     */
    KisActionRecorder* actionRecorder() const;
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
     * Convert a document coordinate to a pixel coordinate.
     *
     * @param documentCoord PostScript Pt coordinate to convert.
     */
    QPointF documentToPixel(const QPointF &documentCoord) const;

    /**
     * Convert a document coordinate to an integer pixel coordinate.
     *
     * @param documentCoord PostScript Pt coordinate to convert.
     */
    QPoint documentToIntPixel(const QPointF &documentCoord) const;

    /**
     * Convert a document rectangle to a pixel rectangle.
     *
     * @param documentRect PostScript Pt rectangle to convert.
     */
    QRectF documentToPixel(const QRectF &documentRect) const;

    /**
     * Convert a document rectangle to an integer pixel rectangle.
     *
     * @param documentRect PostScript Pt rectangle to convert.
     */
    QRect documentToIntPixel(const QRectF &documentRect) const;

    /**
     * Convert a pixel coordinate to a document coordinate.
     *
     * @param pixelCoord pixel coordinate to convert.
     */
    QPointF pixelToDocument(const QPointF &pixelCoord) const;

    /**
     * Convert an integer pixel coordinate to a document coordinate.
     * The document coordinate is at the centre of the pixel.
     *
     * @param pixelCoord pixel coordinate to convert.
     */
    QPointF pixelToDocument(const QPoint &pixelCoord) const;

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
     *  returns a paintdevice that contains the merged layers of this image, within
     * the bounds of this image (with the colorspace and profile of this image)
     */
    KisPaintDeviceSP mergedImage();

    /*
     * Returns the color of the merged image at pixel (x, y).
     */
    KoColor mergedPixel(qint32 x, qint32 y);

    KisGroupLayerSP rootLayer() const;

    /// Return the projection; that is, the complete, composited representation
    /// of this image.
    KisPaintDeviceSP projection();

    /// Move layer to specified position
    bool KDE_DEPRECATED moveLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis);

    /**
     * Add an already existing layer to the image. The layer is put on top
     * of the layers in the specified layergroup
     * @param layer the layer to be added
     * @param parent the parent layer
     */
    bool KDE_DEPRECATED addLayer(KisLayerSP layer, KisGroupLayerSP parent = 0);

    /**
     * Add already existing layer to image.
     *
     * @param layer the layer to be added
     * @param parent the parent layer
     * @param aboveThis in the list with child layers of the specified
     *                  parent, add this layer above the specified sibling.
     *                  if 0, the layer is put in the lowermost position in
     *                  its group.
     * returns false if adding the layer didn't work, true if the layer got added
     */
    bool KDE_DEPRECATED addLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis);

    /**
     * Adds the layer to this group at the specified index.
     * childCount() is a valid index and appends to the end. Fails and
     * returns false if the layer is already in this group or any
     * other (remove it first.)
     */
    bool KDE_DEPRECATED addLayer( KisLayerSP layer,  KisGroupLayerSP parent, int index );

    /// Remove layer
    bool KDE_DEPRECATED removeLayer(KisLayerSP layer);

    /// Move layer up one slot
    bool KDE_DEPRECATED raiseLayer(KisLayerSP layer);

    /// Move layer down one slot
    bool KDE_DEPRECATED lowerLayer(KisLayerSP layer);

    /// Move layer to top slot
    bool KDE_DEPRECATED toTop(KisLayerSP layer);

    /// Move layer to bottom slot
    bool KDE_DEPRECATED toBottom(KisLayerSP layer);

    /**
     * Return the number of layers (not other nodes) that are in this
     * image.
     */
    qint32 KDE_DEPRECATED nlayers() const;

    /**
     * Return the number of layers (not other node types) that are in
     * this image and that are hidden.
     */
    qint32 KDE_DEPRECATED nHiddenLayers() const;

    /**
     * Merge all visible layers and discard hidden ones.
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

    /// Emitted after a layer's properties (visible, locked, opacity, composite op, name, ...) change
    void sigLayerPropertiesChanged(KisLayerSP layer);

    /**
     * Emitted when the list of layers has changed completely.
     * This means e.g. when the image is flattened, but not when it is rotated,
     * as the layers only change internally then.
     */
    void sigLayersChanged(KisGroupLayerSP rootLayer);

    void sigLayerMoved( KisLayerSP layer );
    void sigLayerRemoved( KisLayerSP layer );

    /**
     *  Emitted whenever an action has caused the image to be
     *  recomposited.
     *
     * @param rc The recty that has been recomposited.
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

    /**
     * Inform the model that we're going to add a layer.
     */
    void sigAboutToAddANode( KisNode *parent, int index );

    /**
     * Inform the model we're done adding a layer.
     */
    void sigNodeHasBeenAdded( KisNode *parent, int index );

    /**
     * Inform the model we're going to remove a layer.
     */
    void sigAboutToRemoveANode( KisNode *parent, int index );

    /**
     * Inform the model we're done removing a layer.
     */
    void sigNodeHasBeenRemoved( KisNode *parent, int index );


public slots:
    void slotSelectionChanged();
    void slotSelectionChanged(const QRect& r);

   void slotProjectionUpdated( const QRect & rc );
    /**
       Called whenever a KisCommand has been executed. This notifies
       the layers, who then notify the layerbox that they are ready
       for new thumbnails
    */

private:
    KisImage& operator=(const KisImage& rhs);
    void init(KisUndoAdapter * adapter, qint32 width, qint32 height,  KoColorSpace * colorSpace);
    void emitSizeChanged();
    void preparePaintLayerAfterAdding( KisLayerSP layer );

private:
    class KisImagePrivate;
    KisImagePrivate * const m_d;
};

#endif // KIS_IMAGE_H_

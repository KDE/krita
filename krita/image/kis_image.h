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
#ifndef KIS_IMAGE_H_
#define KIS_IMAGE_H_

#include <QObject>
#include <QString>
#include <QPainter>
#include <QRect>
#include <QRegion>
#include <QBitArray>

#include <KoColorConversionTransformation.h>

#include "kis_types.h"
#include "kis_shared.h"
#include "kis_node_graph_listener.h"
#include "kis_node_facade.h"

class KoColorSpace;
class KoCompositeOp;
class KoColor;

class KisActionRecorder;
class KisUndoAdapter;
class KisFilterStrategy;
class KoColorProfile;
class KoUpdater;
class KisPerspectiveGrid;

namespace KisMetaData
{
class MergeStrategy;
}

/**
 * This is the image class, it contains a tree of KisLayer stack and
 * meta information about the image. And it also provides some
 * functions to manipulate the whole image.
 */
class KRITAIMAGE_EXPORT KisImage : public QObject, public KisNodeFacade, public KisNodeGraphListener, public KisShared
{

    Q_OBJECT

public:

    KisImage(KisUndoAdapter * adapter, qint32 width, qint32 height, const KoColorSpace * colorSpace, const QString& name, bool startProjection = true);
    KisImage(const KisImage& rhs);
    virtual ~KisImage();

public: // KisNodeGraphListener implementation

    void aboutToAddANode(KisNode *parent, int index);
    void nodeHasBeenAdded(KisNode *parent, int index);
    void aboutToRemoveANode(KisNode *parent, int index);
    void nodeHasBeenRemoved(KisNode *parent, int index);
    void aboutToMoveNode(KisNode * parent, int oldIndex, int newIndex);
    void nodeHasBeenMoved(KisNode * parent, int oldIndex, int newIndex);

public:

    /**
     * Paint the specified rect onto the painter, adjusting the colors
     * using the given profile.
     */
    void renderToPainter(qint32 srcX,
                         qint32 srcY,
                         qint32 dstX,
                         qint32 dstY,
                         qint32 width,
                         qint32 height,
                         QPainter &painter,
                         const KoColorProfile *profile);

    /**
     * Render the projection onto a QImage.
     */
    QImage convertToQImage(qint32 x1,
                           qint32 y1,
                           qint32 width,
                           qint32 height,
                           const KoColorProfile * profile);


    /**
     * XXX: docs!
     */
    QImage convertToQImage(const QRect& scaledRect, const QSize& scaledImageSize, const KoColorProfile *profile);

    /**
     * Lock the image to make sure no recompositing-causing signals
     * get emitted while you're messing with the layers. Don't forget
     * to unlock again.
     */
    void lock();

    /**
     * Unlock the image to make sure the rest of Krita learns about
     * changes in the image again.
     */
    void unlock();

    /**
     * Returns true if the image is locked.
     */
    bool locked() const;

    /**
     * @return the image that is used as background tile.
     */
    KisBackgroundSP backgroundPattern() const;

    /**
     * Set a 64x64 tile for the background of the image.
     */
    void setBackgroundPattern(KisBackgroundSP image);

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
    void setGlobalSelection(KisSelectionSP globalSelection = 0);

    /**
     * Removes the global selection.
     */
    void removeGlobalSelection();

    /**
     * @return the deselected global selection or 0 if no global selection was deselected
     */
    KisSelectionSP deleselectedGlobalSelection();

    /**
     * Set deselected global selection
     */
    void setDeleselectedGlobalSelection(KisSelectionSP selection);

    /**
     * Retrieve the next automatic layername (XXX: fix to add option to return Mask X)
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
     * Resize the image to the specified width and height. The previous
     * image is offset by the amount specified.
     *
     * @param w the width of the image
     * @param h the height of the image
     * @param xOffset the horizontal offset of the previous image
     * @param yOffset the vertical offset of the previous image
     */
    void resizeWithOffset(qint32 w, qint32 h, qint32 xOffset = 0, qint32 yOffset = 0);

    /**
     * Execute a scale transform on all layers in this image.
     */
    void scale(double sx, double sy, KoUpdater *m_progress, KisFilterStrategy *filterStrategy);

    /**
     * Execute a rotate transform on all layers in this image.
     */
    void rotate(double radians, KoUpdater *m_progress);

    /**
     * Execute a shear transform on all layers in this image.
     */
    void shear(double angleX, double angleY, KoUpdater *m_progress);

    /**
     * Convert the image and all its layers to the dstColorSpace
     */
    void convertTo(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual);

    // Get the profile associated with this image
    const KoColorProfile *  profile() const;

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
     * Tell the image it's modified; this emits the sigImageModified
     * signal. This happens when the image needs to be saved
     */
    void setModified();

    /**
     * The default colorspace of this image: new layers will have this
     * colorspace and the projection will have this colorspace.
     */
    const KoColorSpace * colorSpace() const;

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
     * Convert a document rectangle to an integer pixel rectangle.
     *
     * @param pixelCoord pixel coordinate to convert.
     */
    QRectF pixelToDocument(const QRectF &pixelCoord) const;

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
    QSize size() const {
        return QSize(width(), height());
    }

    /**
     * returns a paintdevice that contains the merged layers of this
     * image, within the bounds of this image (with the colorspace and
     * profile of this image)
     */
    KisPaintDeviceSP mergedImage();

    /**
     * @return the root node of the image node graph
     */
    KisGroupLayerSP rootLayer() const;

    /**
     * Return the projection; that is, the complete, composited
     * representation of this image.
     */
    KisPaintDeviceSP projection();

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
    KisLayerSP mergeLayer(KisLayerSP l, const KisMetaData::MergeStrategy* strategy);

    /**
     * flatten the layer: that is, the projection becomes the layer
     * and all subnodes are removed. If this is not a paint layer, it will morph
     * into a paint layer.
     */
    KisLayerSP flattenLayer(KisLayerSP layer);


    QRect bounds() const;

    /// use if the layers have changed _completely_ (eg. when flattening)
    void notifyLayersChanged();

    void notifyPropertyChanged(KisLayerSP layer);

    /**
     * Called whenever a layer has changed. The layer is added to a
     * list of dirty layers and as soon as the document stores the
     * command that is now in progress, the image will be notified.
     * Then the image will notify the dirty layers, the dirty layers
     * will notify their parents & emit a signal that will be caught
     * by the layer box, which will request a new thumbnail.
    */
    void notifyLayerUpdated(KisLayerSP layer);

    void setColorSpace(const KoColorSpace * colorSpace);

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

    /// Emitted after a layer's properties (visible, locked, opacity, composite op, name, ...) change
    void sigLayerPropertiesChanged(KisLayerSP layer);

    /**
     * Emitted when the list of layers has changed completely.
     * This means e.g. when the image is flattened, but not when it is rotated,
     * as the layers only change internally then.
     */
    void sigLayersChanged(KisGroupLayerSP rootLayer);
    /**
     * Emitted when the list of layers has changed completely, and emitted after \ref sigLayersChanged has been
     * emitted.
     */
    void sigPostLayersChanged(KisGroupLayerSP rootLayer);

    void sigLayerMoved(KisLayerSP layer);
    void sigLayerRemoved(KisLayerSP layer);

    /**
     *  Emitted whenever an action has caused the image to be
     *  recomposited.
     *
     * @param rc The rect that has been recomposited.
     */
    void sigImageUpdated(const QRect &);

    /**
       Emitted whenever the image has been modified, so that it
       doesn't match with the version saved on disk.
     */
    void sigImageModified();

    void sigSizeChanged(qint32 w, qint32 h);
    void sigProfileChanged(KoColorProfile *  profile);
    void sigColorSpaceChanged(const KoColorSpace*  cs);
    void sigResolutionChanged(double xRes, double yRes);

    /**
     * Inform the model that we're going to add a layer.
     */
    void sigAboutToAddANode(KisNode *parent, int index);

    /**
     * Inform the model we're done adding a layer.
     */
    void sigNodeHasBeenAdded(KisNode *parent, int index);

    /**
     * Inform the model we're going to remove a layer.
     */
    void sigAboutToRemoveANode(KisNode *parent, int index);

    /**
     * Inform the model we're done removing a layer.
     */
    void sigNodeHasBeenRemoved(KisNode *parent, int index);

public slots:

    void slotProjectionUpdated(const QRect & rc);
    void updateProjection(KisNodeSP node, const QRect& rc);
    void refreshGraph();

private:
    KisImage& operator=(const KisImage& rhs);
    void init(KisUndoAdapter * adapter, qint32 width, qint32 height, const KoColorSpace * colorSpace);
    void emitSizeChanged();
    void preparePaintLayerAfterAdding(KisLayerSP layer);

private:
    class KisImagePrivate;
    KisImagePrivate * const m_d;
};

#endif // KIS_IMAGE_H_

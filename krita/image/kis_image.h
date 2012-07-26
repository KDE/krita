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

#include "kis_paint_device.h" // msvc cannot handle forward declarations, so include kis_paint_device here
#include "kis_types.h"
#include "kis_shared.h"
#include "kis_node_graph_listener.h"
#include "kis_node_facade.h"
#include "kis_default_bounds.h"
#include "kis_image_interfaces.h"


class KoDocument;
class KoColorSpace;
class KoCompositeOp;
class KoColor;

class KisCompositeProgressProxy;
class KisActionRecorder;
class KisUndoStore;
class KisUndoAdapter;
class KisImageSignalRouter;
class KisPostExecutionUndoAdapter;
class KisFilterStrategy;
class KoColorProfile;
class KoUpdater;
class KisPerspectiveGrid;
class KisLayerComposition;

namespace KisMetaData
{
class MergeStrategy;
}

/**
 * This is the image class, it contains a tree of KisLayer stack and
 * meta information about the image. And it also provides some
 * functions to manipulate the whole image.
 */
class KRITAIMAGE_EXPORT KisImage : public QObject,
        public KisStrokesFacade,
        public KisUpdatesFacade,
        public KisProjectionUpdateListener,
        public KisNodeFacade,
        public KisNodeGraphListener,
        public KisShared
{

    Q_OBJECT

public:

    /// @param colorSpace can be null. in that case it will be initialised to a default color space.
    KisImage(KisUndoStore *undoStore, qint32 width, qint32 height, const KoColorSpace * colorSpace, const QString& name, bool startProjection = true);
    virtual ~KisImage();

public: // KisNodeGraphListener implementation

    void nodeHasBeenAdded(KisNode *parent, int index);
    void aboutToRemoveANode(KisNode *parent, int index);
    void nodeChanged(KisNode * node);
    void requestProjectionUpdate(KisNode *node, const QRect& rect);

public: // KisProjectionUpdateListener implementation
    void notifyProjectionUpdated(const QRect &rc);

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
     * Render the projection onto a QImage.
     * (this is an overloaded function)
     */
    QImage convertToQImage(QRect imageRect,
                           const KoColorProfile * profile);


    /**
     * XXX: docs!
     */
    QImage convertToQImage(const QRect& scaledRect, const QSize& scaledImageSize, const KoColorProfile *profile);

    /**
     * Calls KisUpdateScheduler::lock
     */
    void lock();

    /**
     * Calls KisUpdateScheduler::unlock
     */
    void unlock();

    /**
     * Returns true if lock() has been called more often than unlock().
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
     * Resize the image to the specified rect. The resize
     * method handles the creating on an undo step itself.
     *
     * @param newRect the rect describing the new width, height and offset
     *        of the image
     */
    void resizeImage(const QRect& newRect);

    /**
     * Crop the image to the specified rect. The crop
     * method handles the creating on an undo step itself.
     *
     * @param newRect the rect describing the new width, height and offset
     *        of the image
     */
    void cropImage(const QRect& newRect);


    /**
     * Crop a node to @newRect. The node will *not* be moved anywhere,
     * it just drops some content
     */
    void cropNode(KisNodeSP node, const QRect& newRect);

    void scaleImage(const QSize &size, qreal xres, qreal yres, KisFilterStrategy *filterStrategy);

    /**
     * Execute a rotate transform on all layers in this image.
     * Image is resized to fit rotated image.
     */
    void rotateImage(double radians);

    /**
     * Execute a rotate transform on on a subtree of this image.
     * Image is not resized.
     */
    void rotateNode(KisNodeSP node, double radians);

    /**
     * Execute a shear transform on all layers in this image.
     */
    void shear(double angleX, double angleY);

    /**
     * Shear a node and all its children.
     * @param angleX, @param angleY are given in degrees.
     */
    void shearNode(KisNodeSP node, double angleX, double angleY);

    /**
     * Convert the image and all its layers to the dstColorSpace
     */
    void convertImageColorSpace(const KoColorSpace *dstColorSpace,
                                KoColorConversionTransformation::Intent renderingIntent,
                                KoColorConversionTransformation::ConversionFlags conversionFlags);

    /**
     * Set the color space of  the projection (and the root layer)
     * to dstColorSpace. No conversion is done for other layers,
     * their colorspace can differ.
     * NOTE: Note conversion is done, only regeneration, so no rendering
     * intent needed
     */
    void convertProjectionColorSpace(const KoColorSpace *dstColorSpace);

    // Get the profile associated with this image
    const KoColorProfile *  profile() const;

    /**
     * Set the profile of the image to the new profile and do the same for
     * all layers that have the same colorspace and profile of the image.
     * It doesn't do any pixel conversion.
     *
     * This is essential if you have loaded an image that didn't
     * have an embedded profile to which you want to attach the right profile.
     *
     * This does not create an undo action; only call it when creating or
     * loading an image.
     */
    void assignImageProfile(const KoColorProfile *profile);

    /**
     * Returns the current undo adapter. You can add new commands to the
     * undo stack using the adapter. This adapter is used for a backward
     * compatibility for old commands created before strokes. It blocks
     * all the porcessing at the scheduler, waits until it's finished
     * adn executes commands exclusively.
     */
    KisUndoAdapter* undoAdapter() const;

    /**
     * This adapter is used by the strokes system. The commands are added
     * to it *after* redo() is done (in the scheduler context). They are
     * wrapped into a special command and added to the undo stack. redo()
     * in not called.
     */
    KisPostExecutionUndoAdapter* postExecutionUndoAdapter() const;

    /**
     * Replace current undo store with the new one. The old store
     * will be deleted.
     * This method is used by KisDoc2 for dropping all the commands
     * during file loading.
     */
    void setUndoStore(KisUndoStore *undoStore);

    /**
     * Return current undo store of the image
     */
    KisUndoStore* undoStore();

    /**
     * @return the action recorder associated with this image
     */
    KisActionRecorder* actionRecorder() const;

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
     * Starting form 2.3 mergedImage() is declared deprecated.
     * If you want to get a projection of the image, please use
     * something like:
     *
     * image->lock();
     * read_something_from_the_image(image->projection());
     * image->unlock();
     *
     * or if you want to get a full refresh of the image graph
     * performed beforehand (do you really want it?) (sure?) then
     * you can add a call to image->refreshGraph() before locking
     * the image.
     */
    KDE_DEPRECATED KisPaintDeviceSP mergedImage();

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
    qint32 nlayers() const;

    /**
     * Return the number of layers (not other node types) that are in
     * this image and that are hidden.
     */
    qint32 nHiddenLayers() const;

    /**
     * Merge all visible layers and discard hidden ones.
     */
    void flatten();

    /**
     * Merge the specified layer with the layer
     * below this layer, remove the specified layer.
     */
    KisLayerSP mergeDown(KisLayerSP l, const KisMetaData::MergeStrategy* strategy);

    /**
     * flatten the layer: that is, the projection becomes the layer
     * and all subnodes are removed. If this is not a paint layer, it will morph
     * into a paint layer.
     */
    KisLayerSP flattenLayer(KisLayerSP layer);


    /// This overrides interface for KisDefaultBounds
    /// @return the exact bounds of the image in pixel coordinates.
    QRect bounds() const;

    /// use if the layers have changed _completely_ (eg. when flattening)
    void notifyLayersChanged();

    /**
     * Called whenever a layer has changed. The layer is added to a
     * list of dirty layers and as soon as the document stores the
     * command that is now in progress, the image will be notified.
     * Then the image will notify the dirty layers, the dirty layers
     * will notify their parents & emit a signal that will be caught
     * by the layer box, which will request a new thumbnail.
    */
    void notifyLayerUpdated(KisLayerSP layer);

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

    /**
     * Called before the image is delted and sends the sigAboutToBeDeleted signal
     */
    void notifyAboutToBeDeleted();

    KisImageSignalRouter* signalRouter();

    /**
     * Returns whether we can reselect current global selection
     *
     * \see reselectGlobalSelection()
     */
    bool canReselectGlobalSelection();

    /**
     * Returns the layer compositions for the image
     */
    QList<KisLayerComposition*> compositions();

    /**
     * Adds a new layer composition, will be saved with the image
     */
    void addComposition(KisLayerComposition* composition);

    /**
     * Remove the layer compostion
     */
    void removeComposition(KisLayerComposition* composition);

signals:

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
    void sigProfileChanged(const KoColorProfile *  profile);
    void sigColorSpaceChanged(const KoColorSpace*  cs);
    void sigResolutionChanged(double xRes, double yRes);

    /**
     * Inform the model that a node was changed
     */
    void sigNodeChanged(KisNodeSP node);

    /**
     * Inform that the image is going to be deleted
     */
    void sigAboutToBeDeleted();

    /**
     * The signal is emitted right after a node has been connected
     * to the graph of the nodes.
     *
     * WARNING: you must not request any graph-related information
     * about the node being run in a not-scheduler thread. If you need
     * information about the parent/siblings of the node connect
     * with Qt::DirectConnection, get needed information and then
     * emit another Qt::AutoConnection signal to pass this information
     * to your thread. See details of the implementation
     * in KisDummiesfacadeBase.
     */
    void sigNodeAddedAsync(KisNodeSP node);

    /**
     * This signal is emitted right before a node is going to removed
     * from the graph of the nodes.
     *
     * WARNING: you must not request any graph-related information
     * about the node being run in a not-scheduler thread.
     *
     * \see comment in sigNodeAddedAsync()
     */
    void sigRemoveNodeAsync(KisNodeSP node);

    /**
     * Emitted when the root node of the image has changed.
     * It happens, e.g. when we flatten the image. When
     * this happens the receiver should reload information
     * about the image
     */
    void sigLayersChangedAsync();


public slots:
    KisCompositeProgressProxy* compositeProgressProxy();

    void barrierLock();
    bool tryBarrierLock();
    void waitForDone();

    KisStrokeId startStroke(KisStrokeStrategy *strokeStrategy);
    void addJob(KisStrokeId id, KisStrokeJobData *data);
    void endStroke(KisStrokeId id);
    bool cancelStroke(KisStrokeId id);

    void blockUpdates();
    void unblockUpdates();

    void disableUIUpdates();
    void enableUIUpdates();

    void refreshGraphAsync(KisNodeSP root = 0);
    void refreshGraphAsync(KisNodeSP root, const QRect &rc);
    void refreshGraphAsync(KisNodeSP root, const QRect &rc, const QRect &cropRect);

    /**
     * Triggers synchronous recomposition of the projection
     */
    void refreshGraph(KisNodeSP root = 0);
    void refreshGraph(KisNodeSP root, const QRect& rc, const QRect &cropRect);
    void initialRefreshGraph();

private:
    KisImage(const KisImage& rhs);
    KisImage& operator=(const KisImage& rhs);
    void init(KisUndoStore *undoStore, qint32 width, qint32 height, const KoColorSpace * colorSpace);
    void emitSizeChanged();

    void resizeImageImpl(const QRect& newRect, bool cropLayers);
    void rotateImpl(const QString &actionName, KisNodeSP rootNode,
                    bool resizeImage, double radians);
    void shearImpl(const QString &actionName, KisNodeSP rootNode,
                   bool resizeImage, double angleX, double angleY,
                   const QPointF &origin);

    void refreshHiddenArea(KisNodeSP rootNode, const QRect &preparedArea);
    static QRect realNodeExtent(KisNodeSP rootNode, QRect currentRect = QRect());

    friend class KisImageResizeCommand;
    void setSize(const QSize& size);

    friend class KisImageSetProjectionColorSpaceCommand;
    void setProjectionColorSpace(const KoColorSpace * colorSpace);


    friend class KisDeselectGlobalSelectionCommand;
    friend class KisReselectGlobalSelectionCommand;
    friend class KisSetGlobalSelectionCommand;
    friend class KisImageTest;

    /**
     * Replaces the current global selection with globalSelection. If
     * \p globalSelection is empty, removes the selection object, so that
     * \ref globalSelection() will return 0 after that.
     */
    void setGlobalSelection(KisSelectionSP globalSelection);

    /**
     * Deselects current global selection.
     * \ref globalSelection() will return 0 after that.
     */
    void deselectGlobalSelection();

    /**
     * Reselects current deselected selection
     *
     * \see deselectGlobalSelection()
     */
    void reselectGlobalSelection();

private:
    class KisImagePrivate;
    KisImagePrivate * const m_d;
};

#endif // KIS_IMAGE_H_

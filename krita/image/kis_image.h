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
#include "kis_image_interfaces.h"

#include <kritaimage_export.h>

class KisDocument;
class KoColorSpace;
class KoColor;

class KisCompositeProgressProxy;
class KisActionRecorder;
class KisUndoStore;
class KisUndoAdapter;
class KisImageSignalRouter;
class KisPostExecutionUndoAdapter;
class KisFilterStrategy;
class KoColorProfile;
class KisPerspectiveGrid;
class KisLayerComposition;
class KisSpontaneousJob;
class KisImageAnimationInterface;
class KUndo2MagicString;


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
    KisImage(KisUndoStore *undoStore, qint32 width, qint32 height, const KoColorSpace * colorSpace, const QString& name);
    virtual ~KisImage();

public: // KisNodeGraphListener implementation

    void aboutToAddANode(KisNode *parent, int index);
    void nodeHasBeenAdded(KisNode *parent, int index);
    void aboutToRemoveANode(KisNode *parent, int index);
    void nodeChanged(KisNode * node);
    void invalidateAllFrames();
    void notifySelectionChanged();
    void requestProjectionUpdate(KisNode *node, const QRect& rect);
    void invalidateFrames(const KisTimeRange &range, const QRect &rect);
    void requestTimeSwitch(int time);

public: // KisProjectionUpdateListener implementation
    void notifyProjectionUpdated(const QRect &rc);

public:

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
    QImage convertToQImage(const QSize& scaledImageSize, const KoColorProfile *profile);

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

    /// XXX: ApiDox
    void scaleImage(const QSize &size, qreal xres, qreal yres, KisFilterStrategy *filterStrategy);

    /// XXX: ApiDox
    void scaleNode(KisNodeSP node, qreal scaleX, qreal scaleY, KisFilterStrategy *filterStrategy);

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
     *
     * @returns false if the profile could not be assigned
     */
    bool assignImageProfile(const KoColorProfile *profile);

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
     * This method is used by KisDocument for dropping all the commands
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
     * @return the root node of the image node graph
     */
    KisGroupLayerSP rootLayer() const;

    /**
     * Return the projection; that is, the complete, composited
     * representation of this image.
     */
    KisPaintDeviceSP projection() const;

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
    void mergeDown(KisLayerSP l, const KisMetaData::MergeStrategy* strategy);

    /**
     * flatten the layer: that is, the projection becomes the layer
     * and all subnodes are removed. If this is not a paint layer, it will morph
     * into a paint layer.
     */
    void flattenLayer(KisLayerSP layer);

    /**
     * Merges layers in \p mergedLayers and creates a new layer above
     * \p putAfter
     */
    void mergeMultipleLayers(QList<KisNodeSP> mergedLayers, KisNodeSP putAfter);

    /// This overrides interface for KisDefaultBounds
    /// @return the exact bounds of the image in pixel coordinates.
    QRect bounds() const;

    /// use if the layers have changed _completely_ (eg. when flattening)
    void notifyLayersChanged();

    /**
     * Sets the default color of the root layer projection. All the layers
     * will be merged on top of this very color
     */
    void setDefaultProjectionColor(const KoColor &color);

    /**
     * \see setDefaultProjectionColor()
     */
    KoColor defaultProjectionColor() const;

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

    /**
     * Permit or deny the wrap-around mode for all the paint devices
     * of the image. Note that permitting the wraparound mode will not
     * necessarily activate it right now. To be activated the wrap
     * around mode should be 1) permitted; 2) supported by the
     * currently running stroke.
     */
    void setWrapAroundModePermitted(bool value);

    /**
     * \return whether the wrap-around mode is permitted for this
     *         image. If the wrap around mode is permitted and the
     *         currently running stroke supports it, the mode will be
     *         activated for all paint devices of the image.
     *
     * \see setWrapAroundMode
     */
    bool wrapAroundModePermitted() const;


    /**
     * \return whether the wraparound mode is activated for all the
     *         devices of the image. The mode is activated when both
     *         factors are true: the user permitted it and the stroke
     *         supports it
     */
    bool wrapAroundModeActive() const;

    /**
     * \return curent level of detail which is used when processing the image.
     * Current working zoom = 2 ^ (- currentLevelOfDetail()). Default value is
     * null, which means we work on the original image.
     */
    int currentLevelOfDetail() const;

    /**
     * Notify KisImage which level of detail should be used in the
     * lod-mode. Setting the mode does not guarantee the LOD to be
     * used. It will be activated only when the stokes supports it.
     */
    void setDesiredLevelOfDetail(int lod);

public Q_SLOTS:

    /**
     * Explicitly start regeneration of LoD planes of all the devices
     * in the image. This call should be performed when the user is idle,
     * just to make the quality of image updates better.
     */
    void explicitRegenerateLevelOfDetail();

public:

    /**
     * Blocks usage of level of detail functionality. After this method
     * has been called, no new strokes will use LoD.
     */
    void setLevelOfDetailBlocked(bool value);

    /**
     * \see setLevelOfDetailBlocked()
     */
    bool levelOfDetailBlocked() const;

    /**
     * Notifies that the node collapsed state has changed
     */
    void notifyNodeCollpasedChanged();

    KisImageAnimationInterface *animationInterface() const;

public:
    bool startIsolatedMode(KisNodeSP node);
    void stopIsolatedMode();
    KisNodeSP isolatedModeRoot() const;

Q_SIGNALS:

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

    /**
     * The signal is emitted when the size of the image is changed.
     * \p oldStillPoint and \p newStillPoint give the receiver the
     * hint about how the new and old rect of the image correspond to
     * each other. They specify the point of the image around which
     * the conversion was done. This point will stay still on the
     * user's screen. That is the \p newStillPoint of the new image
     * will be painted at the same screen position, where \p
     * oldStillPoint of the old image was painted.
     *
     * \param oldStillPoint is a still point represented in *old*
     *                      image coordinates
     *
     * \param newStillPoint is a still point represented in *new*
     *                      image coordinates
     */
    void sigSizeChanged(const QPointF &oldStillPoint, const QPointF &newStillPoint);

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

    /**
     * Emitted when the UI has requested the undo of the last stroke's
     * operation. The point is, we cannot deal with the internals of
     * the stroke without its creator knowing about it (which most
     * probably cause a crash), so we just forward this request from
     * the UI to the creator of the stroke.
     *
     * If your tool supports undoing part of its work, just listen to
     * this signal and undo when it comes
     */
    void sigUndoDuringStrokeRequested();

    /**
     * Emitted when the UI has requested the cancellation of
     * the stroke. The point is, we cannot cancel the stroke
     * without its creator knowing about it (which most probably
     * cause a crash), so we just forward this request from the UI
     * to the creator of the stroke.
     *
     * If your tool supports cancelling of its work in the middle
     * of operation, just listen to this signal and cancel
     * the stroke when it comes
     */
    void sigStrokeCancellationRequested();

    /**
     * Emitted when the image decides that the stroke should better
     * be ended. The point is, we cannot just end the stroke
     * without its creator knowing about it (which most probably
     * cause a crash), so we just forward this request from the UI
     * to the creator of the stroke.
     *
     * If your tool supports long  strokes that may involve multiple
     * mouse actions in one stroke, just listen to this signal and
     * end the stroke when it comes.
     */
    void sigStrokeEndRequested();

    /**
     * Same as sigStrokeEndRequested() but is not emitted when the active node
     * is changed.
     */
    void sigStrokeEndRequestedActiveNodeFiltered();

    /**
     * Emitted when the isolated mode status has changed.
     *
     * Can be used by the receivers to catch a fact of forcefully
     * stopping the isolated mode by the image when some complex
     * action was requested
     */
    void sigIsolatedModeChanged();

    /**
     * Emitted when one or more nodes changed the collapsed state
     *
     */
    void sigNodeCollapsedChanged();

public Q_SLOTS:
    KisCompositeProgressProxy* compositeProgressProxy();

    bool isIdle();

    void barrierLock();
    bool tryBarrierLock();
    void waitForDone();

    KisStrokeId startStroke(KisStrokeStrategy *strokeStrategy);
    void addJob(KisStrokeId id, KisStrokeJobData *data);
    void endStroke(KisStrokeId id);
    bool cancelStroke(KisStrokeId id);

    void blockUpdates();
    void unblockUpdates();

    /**
     * Disables notification of the UI about the changes in the image.
     * This feature is used by KisProcessingApplicator. It is needed
     * when we change the size of the image. In this case, the whole
     * image will be reloaded into UI by sigSizeChanged(), so there is
     * no need to inform the UI about individual dirty rects.
     */
    void disableUIUpdates();

    /**
     * \see disableUIUpdates
     */
    void enableUIUpdates();

    /**
     * Disables the processing of all the setDirty() requests that
     * come to the image. The incoming requests are effectively
     * *dropped*.
     *
     * This feature is used by KisProcessingApplicator. For many cases
     * it provides its own updates interface, which recalculates the
     * whole subtree of nodes. But while we change any particular
     * node, it can ask for an update itself. This method is a way of
     * blocking such intermediate (and excessive) requests.
     *
     * NOTE: this is a convenience function for setProjectionUpdatesFilter()
     *       that installs a predefined filter that eats everything. Please
     *       note that these calls are *not* recursive
     */
    void disableDirtyRequests();

    /**
     * \see disableDirtyRequests()
     */
    void enableDirtyRequests();

    /**
     * Installs a filter object that will filter all the incoming projection update
     * requests. If the filter return true, the incoming update is dropped.
     *
     * NOTE: you cannot set filters recursively!
     */
    void setProjectionUpdatesFilter(KisProjectionUpdatesFilterSP filter);

    /**
     * \see setProjectionUpdatesFilter()
     */
    KisProjectionUpdatesFilterSP projectionUpdatesFilter() const;

    void refreshGraphAsync(KisNodeSP root = 0);
    void refreshGraphAsync(KisNodeSP root, const QRect &rc);
    void refreshGraphAsync(KisNodeSP root, const QRect &rc, const QRect &cropRect);

    /**
     * Triggers synchronous recomposition of the projection
     */
    void refreshGraph(KisNodeSP root = 0);
    void refreshGraph(KisNodeSP root, const QRect& rc, const QRect &cropRect);
    void initialRefreshGraph();

    /**
     * Initiate a stack regeneration skipping the recalculation of the
     * filthy node's projection.
     *
     * Works exactly as pseudoFilthy->setDirty() with the only
     * exception that pseudoFilthy::updateProjection() will not be
     * called. That is used by KisRecalculateTransformMaskJob to avoid
     * cyclic dependencies.
     */
    void requestProjectionUpdateNoFilthy(KisNodeSP pseudoFilthy, const QRect &rc, const QRect &cropRect);

    /**
     * Adds a spontaneous job to the updates queue.
     *
     * A spontaneous job may do some trivial tasks in the background,
     * like updating the outline of selection or purging unused tiles
     * from the existing paint devices.
     */
    void addSpontaneousJob(KisSpontaneousJob *spontaneousJob);

    /**
     * This method is called by the UI (*not* by the creator of the
     * stroke) when it thinks the current stroke should undo its last
     * action, for example, when the user presses Ctrl+Z while some
     * stroke is active.
     *
     * If the creator of the stroke supports undoing of intermediate
     * actions, it will be notified about this request and can undo
     * its last action.
     */
    void requestUndoDuringStroke();

    /**
     * This method is called by the UI (*not* by the creator of the
     * stroke) when it thinks current stroke should be cancelled. If
     * there is a running stroke that has already been detached from
     * its creator (ended or cancelled), it will be forcefully
     * cancelled and reverted. If there is an open stroke present, and
     * if its creator supports cancelling, it will be notified about
     * the request and the stroke will be cancelled
     */
    void requestStrokeCancellation();

    /**
     * This method is called when image or some other part of Krita
     * (*not* the creator of the stroke) decides that the stroke
     * should be ended. If the creator of the stroke supports it, it
     * will be notified and the stroke will be cancelled
     */
    void requestStrokeEnd();

    /**
     * Same as requestStrokeEnd() but is called by view manager when
     * the current node is changed. Use to dintinguish
     * sigStrokeEndRequested() and
     * sigStrokeEndRequestedActiveNodeFiltered() which are used by
     * KisNodeJugglerCompressed
     */
    void requestStrokeEndActiveNode();

private:

    KisImage(const KisImage& rhs);
    KisImage& operator=(const KisImage& rhs);

    void emitSizeChanged();

    void resizeImageImpl(const QRect& newRect, bool cropLayers);
    void rotateImpl(const KUndo2MagicString &actionName, KisNodeSP rootNode,
                    bool resizeImage, double radians);
    void shearImpl(const KUndo2MagicString &actionName, KisNodeSP rootNode,
                   bool resizeImage, double angleX, double angleY,
                   const QPointF &origin);

    void safeRemoveTwoNodes(KisNodeSP node1, KisNodeSP node2);

    void refreshHiddenArea(KisNodeSP rootNode, const QRect &preparedArea);

    void requestProjectionUpdateImpl(KisNode *node,
                                     const QRect& rect,
                                     const QRect &cropRect);

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
    KisImagePrivate * m_d;
};

#endif // KIS_IMAGE_H_

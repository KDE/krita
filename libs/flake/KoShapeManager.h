/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2007, 2009 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSHAPEMANAGER_H
#define KOSHAPEMANAGER_H

#include <QList>
#include <QObject>
#include <QSet>
#include <QRect>

#include "KoFlake.h"
#include "kritaflake_export.h"

#include <memory>
#include <vector>

class KoShape;
class KoSelection;
class KoViewConverter;
class KoCanvasBase;
class KoPointerEvent;
class KoShapePaintingContext;

class QPainter;
class QPointF;
class QRectF;

/**
 * The shape manager hold a list of all shape which are in scope.
 * There is one shape manager per canvas. This makes the shape manager
 * different from QGraphicsScene, which contains the datamodel for all
 * graphics items: KoShapeManager only contains the subset of shapes
 * that are shown in its canvas.
 *
 * The selection in the different views can be different.
 */
class KRITAFLAKE_EXPORT KoShapeManager : public QObject
{
    Q_OBJECT

public:
    /// enum for add()
    enum Repaint {
        PaintShapeOnAdd,    ///< Causes each shapes 'update()' to be called after being added to the shapeManager
        AddWithoutRepaint   ///< Avoids each shapes 'update()' to be called for faster addition when its possible.
    };

    /**
     * Constructor.
     */
    explicit KoShapeManager(KoCanvasBase *canvas);
    /**
     * Constructor that takes a list of shapes, convenience version.
     * @param shapes the shapes to start out with, see also setShapes()
     * @param canvas the canvas this shape manager is working on.
     */
    KoShapeManager(KoCanvasBase *canvas, const QList<KoShape *> &shapes);
    ~KoShapeManager() override;


    /**
     * Remove all previously owned shapes and make the argument list the new shapes
     * to be managed by this manager.
     * @param shapes the new shapes to manage.
     * @param repaint if true it will trigger a repaint of the shapes
     */
    void setShapes(const QList<KoShape *> &shapes, Repaint repaint = PaintShapeOnAdd);

    /// returns the list of maintained shapes
    QList<KoShape*> shapes() const;

    /**
     * Get a list of all shapes that don't have a parent.
     */
    QList<KoShape*> topLevelShapes() const;

public Q_SLOTS:
    /**
     * Add a KoShape to be displayed and managed by this manager.
     * This will trigger a repaint of the shape.
     * @param shape the shape to add
     * @param repaint if true it will trigger a repaint of the shape
     */
    void addShape(KoShape *shape, KoShapeManager::Repaint repaint = PaintShapeOnAdd);


    /**
     * Remove a KoShape from this manager
     * @param shape the shape to remove
     */
    void remove(KoShape *shape);

public:
    /// return the selection shapes for this shapeManager
    KoSelection *selection() const;

    struct PaintJob {
        using ShapesStorage = std::vector<std::unique_ptr<KoShape>>;
        using SharedSafeStorage = std::shared_ptr<ShapesStorage>;

        PaintJob() = default;
        PaintJob(QRectF _docUpdateRect, QRect _viewUpdateRect)
            : docUpdateRect(_docUpdateRect),
              viewUpdateRect(_viewUpdateRect)
        {
        }

        bool isEmpty() const {
            return shapes.isEmpty();
        }

        QRectF docUpdateRect;
        QRect viewUpdateRect;

        QList<KoShape*> shapes;
        SharedSafeStorage allClonedShapes;
    };

    struct PaintJobsOrder
    {
        QRect uncroppedViewUpdateRect;
        QList<PaintJob> jobs;

        inline void clear() {
            jobs.clear();
            uncroppedViewUpdateRect = QRect();
        }

        inline bool isEmpty() const {
            return jobs.isEmpty();
        }
    };

    /**
     * The update signals are usually emitted when the owned of the manager
     * calls to preparePaintJobs() in the beginning of the rendering cycle.
     * This way we avoid too many signals to be emitted. But some of the
     * canvases (e.g. KisShapeSelectionCanvas) doesn't do any rendering (yet?),
     * so the signals should be emitted explicitly.
     */
    void explicitlyIssueShapeChangedSignals();

    /**
     * Prepare a shallow copy of all the shapes and the jobs to be rendered
     * asynchronoursly later. The copies are stored in jobs, so that the user
     * could later pass these jobs into paintJob() in a separate thread.
     *
     * @param jobs a list of rects that are going to be updated. docUpdateRect
     *             and viewUpdateRect should be preinitialized by the caller.
     * @param excludeRoot the root shape which should not be copied. It is basically
     *                    a hack to avoid copying of KisShapeLayer, which is not
     *                    copiable.
     * \see paintJob()
     * \see a comment in KisShapeLayerCanvas::slotStartAsyncRepaint()
     */
    void preparePaintJobs(PaintJobsOrder &jobsOrder, KoShape *excludeRoot);

    /**
     * Render a \p job on \p painter. No mutable internals of the shape
     * manager are accessed, so calling this method is safe in multithreading
     * environment.
     *
     * @param painter a painter to paint on. Clip rect of the painter is expected to be setup correctly.
     * @param job a job to paint.
     * @param forPrint not used in Krita.
     *
     * \see preparePaintJobs
     */
    void paintJob(QPainter &painter, const KoShapeManager::PaintJob &job, bool forPrint);

    /**
     * Paint all shapes and their selection handles etc.
     * @param painter the painter to paint to.
     * @param forPrint if true, make sure only actual content is drawn and no decorations.
     * @param converter to convert between document and view coordinates.
     */
    void paint(QPainter &painter, bool forPrint);

    /**
     * Returns the shape located at a specific point in the document.
     * If more than one shape is located at the specific point, the given selection type
     * controls which of them is returned.
     * @param position the position in the document coordinate system.
     * @param selection controls which shape is returned when more than one shape is at the specific point
     * @param omitHiddenShapes if true, only visible shapes are considered
     */
    KoShape *shapeAt(const QPointF &position, KoFlake::ShapeSelection selection = KoFlake::ShapeOnTop, bool omitHiddenShapes = true);

    /**
     * Returns the shapes which intersects the specific rect in the document.
     * @param rect the rectangle in the document coordinate system.
     * @param omitHiddenShapes if @c true, only visible shapes are considered
     * @param containedMode if @c true use contained mode
     */
    QList<KoShape *> shapesAt(const QRectF &rect, bool omitHiddenShapes = true, bool containedMode = false);

    /**
     * Request a repaint to be queued.
     * The repaint will be restricted to the parameters rectangle, which is expected to be
     * in points (the document coordinates system of KoShape) and it is expected to be
     * normalized and based in the global coordinates, not any local coordinates.
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     * @param rect the rectangle (in pt) to queue for repaint.
     * @param shape the shape that is going to be redrawn; only needed when selectionHandles=true
     * @param selectionHandles if true; find out if the shape is selected and repaint its
     *   selection handles at the same time.
     */
    void update(const QRectF &rect, const KoShape *shape = 0, bool selectionHandles = false);

    /**
     * Block all updates initiated with update() call. The incoming updates will
     * be dropped completely.
     */
    void setUpdatesBlocked(bool value);

    /**
     * \see setUpdatesBlocked()
     */
    bool updatesBlocked() const;

    /**
     * Update the tree for finding the shapes.
     * This will remove the shape from the tree and will reinsert it again.
     * The update to the tree will be posponed until it is needed so that successive calls
     * will be merged into one.
     * @param shape the shape to updated its position in the tree.
     */
    void notifyShapeChanged(KoShape *shape);

    /**
     * @brief renderSingleShape renders a shape on \p painter. This method includes all the
     * needed steps for painting a single shape: setting transformations, clipping and masking.
     */
    static void renderSingleShape(KoShape *shape, QPainter &painter, KoShapePaintingContext &paintContext);

    /**
     * A special interface for KoShape to use during shape destruction. Don't use this
     * interface directly unless you are KoShape.
     */
    struct ShapeInterface {
        ShapeInterface(KoShapeManager *_q);

        /**
         * Called by a shape when it is destructed. Please note that you cannot access
         * any shape's method type or information during this call because the shape might be
         * semi-destroyed.
         */
        void notifyShapeDestructed(KoShape *shape);

    protected:
        KoShapeManager *q;
    };

    ShapeInterface* shapeInterface();

Q_SIGNALS:
    /// emitted when the selection is changed
    void selectionChanged();
    /// emitted when an object in the selection is changed (moved/rotated etc)
    void selectionContentChanged();
    /// emitted when any object changed (moved/rotated etc)
    void contentChanged();

private:
    KoCanvasBase *canvas();


    class Private;
    Private * const d;
    Q_PRIVATE_SLOT(d, void updateTree())
    Q_PRIVATE_SLOT(d, void forwardCompressedUdpate())
};

#endif

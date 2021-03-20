/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSNAPGUIDE_H
#define KOSNAPGUIDE_H

#include "kritaflake_export.h"

#include <QScopedPointer>
#include <QList>
#include <Qt>

class KoSnapStrategy;
class KoShape;
class KoPathPoint;
class KoViewConverter;
class KoCanvasBase;
class QPainter;
class QPointF;
class QRectF;

/**
 * This class is the place where all the snapping (i.e. snap to grid) is handled.
 *
 * What this class does is snapping a given position (i.e. mouse position) to various
 * snapping targets like grid, boundbox etc.
 * The snap guide does not know anything about the specific snapping target. This
 * is handled by the different snapping strategies which are derived from KoSnapStrategy.
 * Snapping strategies can be enabled/disabled by passing a mask of corresponding
 * snapping ids to KoSnapGuide::enableSnapStrategies. There can be one or more snapping
 * strategies enabled at the same time. The best result (with the nearest distance to the
 * original position) is then returned to the caller of KoSnapGuide::snap.
 *
 * The snap guide is part of the KoCanvasBase class and thus can be accessed by any tool
 * or application via the canvas pointer.
 * For letting the user manage which snap stratgies to enable, there is a snap guide config
 * widget in guiutils.
 *
 */

class KRITAFLAKE_EXPORT KoSnapGuide
{
public:
    /// the different possible snap Strategies
    enum Strategy
    {
        OrthogonalSnapping = 1,
        NodeSnapping = 2,
        ExtensionSnapping = 4,
        IntersectionSnapping = 8,
        GridSnapping = 0x10,
        BoundingBoxSnapping = 0x20,
        GuideLineSnapping = 0x40,
        DocumentBoundsSnapping = 0x80,
        DocumentCenterSnapping = 0x100,
        CustomSnapping = 0x200,
        PixelSnapping = 0x400
    };
    Q_DECLARE_FLAGS(Strategies, Strategy)

    /// Creates the snap guide to work on the given canvas
    explicit KoSnapGuide(KoCanvasBase *canvas);

    virtual ~KoSnapGuide();

    /// snaps the mouse position, returns if mouse was snapped
    QPointF snap(const QPointF &mousePosition, Qt::KeyboardModifiers modifiers);

    QPointF snap(const QPointF &mousePosition, const QPointF &dragOffset, Qt::KeyboardModifiers modifiers);

    /// paints the guide
    void paint(QPainter &painter, const KoViewConverter &converter);

    /// returns the bounding rect of the guide
    QRectF boundingRect();

    /// Adds an additional shape to snap to (useful when creating a path)
    void setAdditionalEditedShape(KoShape *shape);

    /// returns the extra shapes to use
    KoShape *additionalEditedShape() const;

    void enableSnapStrategy(Strategy type, bool value);
    bool isStrategyEnabled(Strategy type) const;

    /// enables the strategies used for snapping
    void enableSnapStrategies(Strategies strategies);

    /// returns the enabled snap strategies
    KoSnapGuide::Strategies enabledSnapStrategies() const;

    /**
     * Adds a custom snap strategy
     *
     * The snap guide take ownership of the strategy. All custom strategies
     * are destroyed when calling reset().
     */
    bool addCustomSnapStrategy(KoSnapStrategy *customStrategy);

    /**
     * Overrides the first entry of a strategy \p type with a strategy
     * \p strategy. Note that basically strategy->type() may not be equal
     * to type and that is ok. \p strategy may also be null.
     */
    void overrideSnapStrategy(Strategy type, KoSnapStrategy *strategy);

    /// enables the snapping guides
    void enableSnapping(bool on);

    /// returns if snapping is enabled
    bool isSnapping() const;

    /// sets the snap distances in pixels
    void setSnapDistance(int distance);

    /// returns the snap distance in pixels
    int snapDistance() const;

    /// returns the canvas the snap guide is working on
    KoCanvasBase *canvas() const;

    /// Sets a list of path points to ignore
    void setIgnoredPathPoints(const QList<KoPathPoint*> &ignoredPoints);

    /// Returns list of ignored points
    QList<KoPathPoint*> ignoredPathPoints() const;

    /// Sets list of ignored shapes
    void setIgnoredShapes(const QList<KoShape*> &ignoredShapes);

    /// Returns list of ignored shapes
    QList<KoShape*> ignoredShapes() const;

    /// Resets the snap guide
    void reset();

private:
    class Private;
    const QScopedPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoSnapGuide::Strategies)

#endif // KOSNAPGUIDE_H

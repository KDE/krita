/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHTOOLSELECTION_H
#define KOPATHTOOLSELECTION_H

#include <KoToolSelection.h>
#include <KoPathShape.h>

class KoPathTool;
class KoPathPoint;
class KoPathPointData;
class KoViewConverter;
class QPainter;

/**
* @brief Handle the selection of points
*
* This class handles the selection of points. It makes sure
* the canvas is repainted when the selection changes.
*/
class KRITAFLAKE_EXPORT KoPathToolSelection : public KoToolSelection, public KoPathShape::PointSelectionChangeListener
{
    Q_OBJECT

public:
    explicit KoPathToolSelection(KoPathTool *tool);

    ~KoPathToolSelection() override;

    /// @brief Draw the selected points
    void paint(QPainter &painter, const KoViewConverter &converter, qreal handleRadius);

    /**
    * @brief Add a point to the selection
    *
    * @param point to add to the selection
    * @param clear if true the selection will be cleared before adding the point
    */
    void add(KoPathPoint *point, bool clear);

    /**
    * @brief Remove a point form the selection
    *
    * @param point to remove from the selection
    */
    void remove(KoPathPoint *point);

    /**
    * @brief Clear the selection
    */
    void clear();

    /**
     * @brief Select points in rect
     *
     * @param rect the selection rectangle in document coordinates
     * @param clearSelection if set clear the current selection before the selection
     */
    void selectPoints(const QRectF &rect, bool clearSelection);

    /**
    * @brief Get the number of path objects in the selection
    *
    * @return number of path object in the point selection
    */
    int objectCount() const;

    /**
    * @brief Get the number of path points in the selection
    *
    * @return number of points in the selection
    */
    int size() const;

    /**
    * @brief Check if a point is in the selection
    *
    * @return true when the point is in the selection, false otherwise
    */
    bool contains(KoPathPoint *point);

    /**
    * @brief Get all selected points
    *
    * @return set of selected points
    */
    const QSet<KoPathPoint *> &selectedPoints() const;

    /**
    * @brief Get the point data of all selected points
    *
    * This is subject to change
    */
    QList<KoPathPointData> selectedPointsData() const;

    /**
    * @brief Get the point data of all selected segments
    *
    * This is subject to change
    */
    QList<KoPathPointData> selectedSegmentsData() const;

    /// Returns list of selected shapes
    QList<KoPathShape*> selectedShapes() const;

    /// Sets list of selected shapes
    void setSelectedShapes(const QList<KoPathShape*> shapes);

    /**
    * @brief Update the selection to contain only valid points
    *
    * This function checks which points are no longer valid and removes them
    * from the selection.
    * If e.g. some points are selected and the shape which contains the points
    * is removed by undo, the points are no longer valid and have therefore to
    * be removed from the selection.
    */
    void update();

    /// reimplemented from KoToolSelection
    bool hasSelection() override;


    void recommendPointSelectionChange(KoPathShape *shape, const QList<KoPathPointIndex> &newSelection) override;
    void notifyPathPointsChanged(KoPathShape *shape) override;
    void notifyShapeChanged(KoShape::ChangeType type, KoShape *shape) override;

Q_SIGNALS:
    void selectionChanged();

private:
    typedef QMap<KoPathShape *, QSet<KoPathPoint *> > PathShapePointMap;

    QSet<KoPathPoint *> m_selectedPoints;
    PathShapePointMap m_shapePointMap;
    KoPathTool *m_tool;
    QList<KoPathShape*> m_selectedShapes;
};

#endif // PATHTOOLSELECTION_H

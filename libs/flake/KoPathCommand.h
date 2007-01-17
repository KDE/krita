/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOPATHCOMMAND_H
#define KOPATHCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QMap>
#include <QPointF>
#include "KoPathShape.h"
#include <koffice_export.h>

class KoParameterShape;
class KoShapeController;

/// the base command for commands altering a path shape
class KoPathBaseCommand : public QUndoCommand {
public:
    /**
     * @param parent the parent command used for macro commands
     */
    KoPathBaseCommand( QUndoCommand *parent = 0 );
    /** initialize the base command with a single shape
     * @param parent the parent command used for macro commands
     */
    explicit KoPathBaseCommand( KoPathShape *shape, QUndoCommand *parent = 0 );
protected:
    /**
     * Shedules repainting of all shapes control point rects.
     * @param normalizeShapes controls if paths are normalized before painting
     */
    void repaint( bool normalizeShapes );

    QSet<KoPathShape*> m_shapes; ///< the shapes the command operates on
};

/// The undo / redo command for path point moving.
class KoPointMoveCommand : public QUndoCommand 
{
public:
    /**
     * Command to move path point.
     * @param pointMap map of the path point to move
     * @param offset the offset by which the point is moved in document coordinates
     * @param parent the parent command used for macro commands
     */
    KoPointMoveCommand( const KoPathShapePointMap &pointMap, const QPointF &offset, QUndoCommand *parent = 0 );

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoPathShapePointMap m_pointMap;
    QPointF m_offset;
};

/// The undo / redo command for path point moving.
class KoControlPointMoveCommand : public QUndoCommand
{
public:
    /**
     * Command to move one control path point.
     * @param offset the offset by which the point is moved in document coordinates
     * @param pointType the type of the point to move
     * @param parent the parent command used for macro commands
     */
    KoControlPointMoveCommand( const KoPathPointData &pointData, const QPointF &offset, KoPathPoint::KoPointType pointType,
                               QUndoCommand *parent = 0 );
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoPathPointData m_pointData;
    // the offset in shape coordinates
    QPointF m_offset;
    KoPathPoint::KoPointType m_pointType;
};

/// The undo / redo command for changing the path point type.
class KoPointTypeCommand : public KoPathBaseCommand
{
public:
    /// The type of the point
    enum PointType
    {
        Corner,
        Smooth,
        Symmetric
    };
    /**
     * Command to change the type of the given points
     * @param pointDataList List of point for changing the points
     * @param pointType the new point type to set
     * @param parent the parent command used for macro commands
     */
    KoPointTypeCommand( const QList<KoPathPointData> & pointDataList, PointType pointType, QUndoCommand *parent = 0 );
    ~KoPointTypeCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:    
    // used for storing the data for undo
    struct PointData
    {
        PointData( const KoPathPointData pointData )
        : m_pointData( pointData )
        {}
        KoPathPointData m_pointData;
        // old control points in document coordinates
        QPointF m_oldControlPoint1;
        QPointF m_oldControlPoint2;
        KoPathPoint::KoPointProperties m_oldProperties;
    };

    PointType m_pointType;
    QList<PointData> m_oldPointData;
};

/// The undo / redo command for removing path points.
class KoPointRemoveCommand : public QUndoCommand 
{
public:
    /**
     * @brief Create command for removing points from path shapes
     *
     * This will create the command for removing points from path shapes. If all
     * points from a path shape are deleted it will delete the path shape. If all 
     * points from a subpath are deleted it will delete the subpath.
     *
     * @param pointDataList List of point data to remove
     * @param shapeController shape controller in charge
     * @param parent the parent command used for macro commands
     */
    static QUndoCommand * createCommand( const QList<KoPathPointData> & pointDataList, KoShapeController * shapeController, 
                                         QUndoCommand *parent = 0 );

    /**
     * @brief Command to remove a points from path shapes
     *
     * Don't use this directly use createCommand instead.
     *
     * @param pointDataList List of point data to remove.
     * @param parent the parent command used for macro commands
     */
    explicit KoPointRemoveCommand( const QList<KoPathPointData> & pointDataList, QUndoCommand *parent = 0 );
    ~KoPointRemoveCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    QList<KoPathPointData> m_pointDataList;
    QList<KoPathPoint*> m_points;
    bool m_deletePoints;
};

/// The undo / redo command for removing a subpath
class KoRemoveSubpathCommand : public QUndoCommand
{
public:
    KoRemoveSubpathCommand( KoPathShape *pathShape, int subpathIndex, QUndoCommand *parent = 0 );
    ~KoRemoveSubpathCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    KoPathShape * m_pathShape;
    int m_subpathIndex;
    KoSubpath * m_subpath;
};

/// The undo / redo command for splitting a path segment
class KoSplitSegmentCommand : public QUndoCommand
{
public:    
    /**
     * Command to split a path segment
     *
     * This splits the segments at the given split position by inserting new points.
     * The De Casteljau algorithm is used for calculating the position of the new 
     * points.
     *
     * @param pointDataList describing the segments to split
     * @param splitPosition the position to split at [0..1]
     * @param parent the parent command used for macro commands
     */
    KoSplitSegmentCommand( const QList<KoPathPointData> & pointDataList, double splitPosition, QUndoCommand *parent = 0 );
    virtual ~KoSplitSegmentCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    QList<KoPathPointData> m_pointDataList;
    QList<KoPathPoint*> m_points;
    QList<QPair<QPointF, QPointF> > m_controlPoints;
    bool m_deletePoints;
};

/// The undo / redo command for joining two subpath end points
class KoSubpathJoinCommand : public QUndoCommand
{
public:
    /**
     * Command to join two subpath end points.
     *
     * The points have to be from the same path shape.
     *
     * @param pointData1 the data of the first point to join
     * @param pointData2 the data of the second point to join
     * @param parent the parent command used for macro commands
     */
    KoSubpathJoinCommand( const KoPathPointData &pointData1, const KoPathPointData &pointData2, QUndoCommand *parent = 0 );
    ~KoSubpathJoinCommand();
    
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoPathPointData m_pointData1;
    KoPathPointData m_pointData2;
    KoPathPointIndex m_splitIndex;
    // the control points have to be stored in document positions
    QPointF m_oldControlPoint1;
    QPointF m_oldControlPoint2;
    KoPathPoint::KoPointProperties m_oldProperties1;
    KoPathPoint::KoPointProperties m_oldProperties2;
    enum Reverse
    {
        ReverseFirst = 1,
        ReverseSecond = 2
    };
    int m_reverse;
};

/// The undo / redo command for breaking a subpath by removing the segment
class KoBreakSegmentCommand : public QUndoCommand 
{
public:
    /**
     * Command to break a subpath by removing the segement
     *
     * The segment following the given point will be removed.
     *
     * @param pointData describing the point 
     * @param parent the parent command used for macro commands
     */
    KoBreakSegmentCommand( const KoPathPointData & pointData, QUndoCommand *parent = 0 );
    ~KoBreakSegmentCommand();
    
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoPathPointData m_pointData;
    KoPathPointIndex m_startIndex;
    bool m_broken;
};

/// Command to break a subpath at points.
class KoBreakAtPointCommand : public QUndoCommand
{
public:
    /**
     * Command to break a subpath at points.
     *
     * The pathes are broken at the given points. New points will be inserted after
     * the given points and then the pathes will be split after the given points.
     *
     * @param pointDataList List of point data where the path should be split.
     * @param parent the parent command used for macro commands
     */
    KoBreakAtPointCommand( const QList<KoPathPointData> & pointDataList, QUndoCommand *parent = 0 );
    ~KoBreakAtPointCommand();
    
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:    
    QList<KoPathPointData> m_pointDataList;
    QList<KoPathPoint*> m_points;
    // used for storing where to open the subpath. In case it not used for the open 
    // status use .second to the store offset caused by a open of a subpath.
    QList<KoPathPointIndex> m_closedIndex;
    bool m_deletePoints;
};

/// The undo / redo command for changing segments to curves/lines
class KoSegmentTypeCommand : public QUndoCommand
{
public:
    /// Segment Types
    enum SegmentType
    {
        Curve = 1,
        Line = 2
    };

    /**
     * Command for changing the segment type ( curve/line )
     * @param pointDataList List of point data identifying the segements that should be changed.
     * @param segmentType to which the segements should be changed to
     * @param parent the parent command used for macro commands
     */
    KoSegmentTypeCommand( const QList<KoPathPointData> & pointDataList, SegmentType segmentType, QUndoCommand *parent = 0 );
    ~KoSegmentTypeCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    // used for storing the data for undo
    struct SegmentTypeData
    {
        // old control points in document coordinates
        QPointF m_controlPoint1;
        QPointF m_controlPoint2;
        KoPathPoint::KoPointProperties m_properties1;
        KoPathPoint::KoPointProperties m_properties2;
    };

    QList<KoPathPointData> m_pointDataList;
    QList<SegmentTypeData> m_segmentData;
    SegmentType m_segmentType;
};

class KoShapeControllerBase;

/// The undo / redo command for combining two or more paths into one
class FLAKE_EXPORT KoPathCombineCommand : public QUndoCommand
{
public:
    /**
     * Command for combining a list of paths into one single path.
     * @param controller the controller to used for removing/inserting.
     * @param paths the list of paths to combine
     * @param parent the parent command used for macro commands
     */
    KoPathCombineCommand( KoShapeControllerBase *controller, const QList<KoPathShape*> &paths, QUndoCommand *parent = 0 );
    virtual ~KoPathCombineCommand();
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoShapeControllerBase *m_controller;
    QList<KoPathShape*> m_paths;
    KoPathShape *m_combinedPath;
    bool m_isCombined;
};

/// The undo / redo command for changing a parameter
class KoParameterChangeCommand : public QUndoCommand
{
public:
    KoParameterChangeCommand( KoParameterShape *shape, int handleId, const QPointF &startPoint, const QPointF &endPoint,
                              QUndoCommand *parent = 0 );
    virtual ~KoParameterChangeCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoParameterShape *m_shape;
    int m_handleId;
    QPointF m_startPoint;
    QPointF m_endPoint;
};

/// The undo / redo command for changing a KoParameterShape into a KoPathShape
class KoParameterToPathCommand : public QUndoCommand
{
public:
    KoParameterToPathCommand( KoParameterShape *shape, QUndoCommand *parent = 0 );
    KoParameterToPathCommand( const QList<KoParameterShape*> &shapes, QUndoCommand *parent = 0 );
    virtual ~KoParameterToPathCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    QList<KoParameterShape*> m_shapes;
    QList<KoSubpathList> m_oldSubpaths;
    QList<KoSubpathList> m_newSubpaths;
    bool m_newPointsActive;
};

/// The undo / redo command for separating subpaths into different paths
class FLAKE_EXPORT KoPathSeparateCommand : public QUndoCommand
{
public:
    /**
     * Command for separating subpaths of a list of paths into different paths.
     * @param controller the controller to used for removing/inserting.
     * @param paths the list of paths to separate
     * @param parent the parent command used for macro commands
     */
    KoPathSeparateCommand( KoShapeControllerBase *controller, const QList<KoPathShape*> &paths,
                           QUndoCommand *parent = 0 );
    virtual ~KoPathSeparateCommand();
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoShapeControllerBase *m_controller;
    QList<KoPathShape*> m_paths;
    QList<KoPathShape*> m_separatedPaths;
    bool m_isSeparated;
};

#endif

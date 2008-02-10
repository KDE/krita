/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KOPATHTOOL_H
#define KOPATHTOOL_H

#include "KoPathShape.h"
#include "KoTool.h"
#include "KoPathToolSelection.h"

class QButtonGroup;
class KoCanvasBase;
class KoInteractionStrategy;
class KoPathToolHandle;

class QAction;

/// The tool for editing a KoPathShape or a KoParameterShape
class FLAKE_TEST_EXPORT KoPathTool : public KoTool {
    Q_OBJECT
public:
    explicit KoPathTool(KoCanvasBase *canvas);
    ~KoPathTool();

    /// reimplemented
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented
    virtual void repaintDecorations();

    /// reimplemented
    virtual void mousePressEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented
    virtual void keyReleaseEvent(QKeyEvent *event);

    /// reimplemented
    virtual void activate (bool temporary=false);
    /// reimplemented
    virtual void deactivate();

    /// reimplemented
    virtual void deleteSelection();

    /// reimplemented
    virtual KoToolSelection* selection();

    /// returns a handle rect at the given position
    QRectF handleRect( const QPointF &p );

    /// repaints the specified rect
    void repaint( const QRectF &repaintRect );

signals:
    void typeChanged(int types);
    void pathChanged(KoPathShape* path);
protected:
    /// reimplemented
    virtual QWidget * createOptionWidget();

private:
    void updateOptionsWidget();

    // needed for interaction strategy
    QPointF m_lastPoint;

private slots:
    void pointTypeChanged( QAction *type );
    void insertPoints();
    void removePoints();
    void segmentToLine();
    void segmentToCurve();
    void convertToPath();
    void joinPoints();
    void breakAtPoint();
    void breakAtSegment();
    void resourceChanged( int key, const QVariant & res );
    void pointSelectionChanged();
    void updateActions();

private:

    KoPathToolHandle * m_activeHandle;       ///< the currently active handle
    int m_handleRadius;                ///< the radius of the control point handles
    /// the point selection
    KoPathToolSelection m_pointSelection;

    // make a frind so that it can test private member/methods
    friend class TestPathTool;

    KoInteractionStrategy *m_currentStrategy; ///< the rubber selection strategy

    QButtonGroup *m_pointTypeGroup;

    QAction *m_actionPathPointCorner;
    QAction *m_actionPathPointSmooth;
    QAction *m_actionPathPointSymmetric;
    QAction *m_actionLineSegment;
    QAction *m_actionCurveSegment;
    QAction *m_actionAddPoint;
    QAction *m_actionRemovePoint;
    QAction *m_actionBreakPoint;
    QAction *m_actionBreakSegment;
    QAction *m_actionJoinSegment;
    QAction *m_actionMergePoints;
    QAction *m_actionConvertToPath;

    QCursor m_selectCursor;
    QCursor m_moveCursor;
};

#endif

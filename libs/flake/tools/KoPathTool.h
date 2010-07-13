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
#include "KoToolBase.h"
#include "KoPathToolSelection.h"
#include <QMap>

class QButtonGroup;
class KoCanvasBase;
class KoInteractionStrategy;
class KoPathToolHandle;

class KAction;

/// The tool for editing a KoPathShape or a KoParameterShape
class FLAKE_TEST_EXPORT KoPathTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit KoPathTool(KoCanvasBase *canvas);
    ~KoPathTool();

    /// reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    /// reimplemented
    virtual void repaintDecorations();

    /// reimplemented
    virtual void mousePressEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void mouseMoveEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented
    virtual void keyReleaseEvent(QKeyEvent *event);
    /// reimplemented
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    /// reimplemented
    virtual void deactivate();

    /// reimplemented
    virtual void deleteSelection();

    /// reimplemented
    virtual KoToolSelection* selection();

    /// repaints the specified rect
    void repaint(const QRectF &repaintRect);

signals:
    void typeChanged(int types);
    void pathChanged(KoPathShape* path); // TODO this is unused, can we remove this one?
protected:
    /// reimplemented
    virtual QMap<QString, QWidget *>  createOptionWidgets();


private:
    void updateOptionsWidget();
    bool segmentAtPoint( const QPointF &point, KoPathShape* &shape, KoPathPoint* &segmentStart, qreal &pointParam );

private slots:
    void pointTypeChanged(QAction *type);
    void insertPoints();
    void removePoints();
    void segmentToLine();
    void segmentToCurve();
    void convertToPath();
    void joinPoints();
    void mergePoints();
    void breakAtPoint();
    void breakAtSegment();
    void resourceChanged(int key, const QVariant & res);
    void pointSelectionChanged();
    void updateActions();
    void pointToLine();
    void pointToCurve();
    void activate();

private:

    KoPathToolHandle * m_activeHandle;       ///< the currently active handle
    int m_handleRadius;    ///< the radius of the control point handles
    uint m_grabSensitivity; ///< the grab sensitivity
    /// the point selection
    KoPathToolSelection m_pointSelection;
    // needed for interaction strategy
    QPointF m_lastPoint;

    // make a frind so that it can test private member/methods
    friend class TestPathTool;

    KoInteractionStrategy *m_currentStrategy; ///< the rubber selection strategy

    QButtonGroup *m_pointTypeGroup;

    KAction *m_actionPathPointCorner;
    KAction *m_actionPathPointSmooth;
    KAction *m_actionPathPointSymmetric;
    KAction *m_actionCurvePoint;
    KAction *m_actionLinePoint;
    KAction *m_actionLineSegment;
    KAction *m_actionCurveSegment;
    KAction *m_actionAddPoint;
    KAction *m_actionRemovePoint;
    KAction *m_actionBreakPoint;
    KAction *m_actionBreakSegment;
    KAction *m_actionJoinSegment;
    KAction *m_actionMergePoints;
    KAction *m_actionConvertToPath;
    QCursor m_selectCursor;
    QCursor m_moveCursor;

    Q_DECLARE_PRIVATE(KoToolBase)
};

#endif

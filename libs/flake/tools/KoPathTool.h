/* This file is part of the KDE project
 * Copyright (C) 2006-2012 Jan Hambrecht <jaham@gmx.net>
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
#include <QList>
#include <QCursor>

class QButtonGroup;
class KoCanvasBase;
class KoInteractionStrategy;
class KoPathToolHandle;
class KoParameterShape;

class QAction;

/// The tool for editing a KoPathShape or a KoParameterShape.
/// See KoCreatePathTool for code handling the initial path creation.
class KRITAFLAKE_EXPORT KoPathTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit KoPathTool(KoCanvasBase *canvas);
    ~KoPathTool() override;

    /// reimplemented
    void paint(QPainter &painter, const KoViewConverter &converter) override;

    /// reimplemented
    void repaintDecorations() override;

    /// reimplemented
    void mousePressEvent(KoPointerEvent *event) override;
    /// reimplemented
    void mouseMoveEvent(KoPointerEvent *event) override;
    /// reimplemented
    void mouseReleaseEvent(KoPointerEvent *event) override;
    /// reimplemented
    void keyPressEvent(QKeyEvent *event) override;
    /// reimplemented
    void keyReleaseEvent(QKeyEvent *event) override;
    /// reimplemented
    void mouseDoubleClickEvent(KoPointerEvent *event) override;
    /// reimplemented
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    /// reimplemented
    void deactivate() override;

    /// reimplemented
    void deleteSelection() override;

    /// reimplemented
    KoToolSelection* selection() override;

    /// repaints the specified rect
    void repaint(const QRectF &repaintRect);

public Q_SLOTS:
    void documentResourceChanged(int key, const QVariant & res) override;

Q_SIGNALS:
    void typeChanged(int types);
    void pathChanged(KoPathShape* path); // TODO this is unused, can we remove this one?
protected:
    /// reimplemented
    QList<QPointer<QWidget> >  createOptionWidgets() override;

private:
    struct PathSegment;

    void updateOptionsWidget();
    PathSegment* segmentAtPoint(const QPointF &point);

private Q_SLOTS:
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
    void pointSelectionChanged();
    void updateActions();
    void pointToLine();
    void pointToCurve();
    void activate();

protected:
    KoPathToolSelection m_pointSelection; ///< the point selection
    QCursor m_selectCursor;

private:

    KoPathToolHandle * m_activeHandle;       ///< the currently active handle
    int m_handleRadius;    ///< the radius of the control point handles
    uint m_grabSensitivity; ///< the grab sensitivity
    QPointF m_lastPoint; ///< needed for interaction strategy
    PathSegment *m_activeSegment;

    // make a frind so that it can test private member/methods
    friend class TestPathTool;

    KoInteractionStrategy *m_currentStrategy; ///< the rubber selection strategy

    QButtonGroup *m_pointTypeGroup;

    QAction *m_actionPathPointCorner;
    QAction *m_actionPathPointSmooth;
    QAction *m_actionPathPointSymmetric;
    QAction *m_actionCurvePoint;
    QAction *m_actionLinePoint;
    QAction *m_actionLineSegment;
    QAction *m_actionCurveSegment;
    QAction *m_actionAddPoint;
    QAction *m_actionRemovePoint;
    QAction *m_actionBreakPoint;
    QAction *m_actionBreakSegment;
    QAction *m_actionJoinSegment;
    QAction *m_actionMergePoints;
    QAction *m_actionConvertToPath;
    QCursor m_moveCursor;

    Q_DECLARE_PRIVATE(KoToolBase)
};

#endif

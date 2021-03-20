/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2012 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHTOOL_H
#define KOPATHTOOL_H

#include "KoPathShape.h"
#include "KoToolBase.h"
#include "KoPathToolSelection.h"
#include "kis_signal_auto_connection.h"
#include <QList>
#include <QCursor>
#include <KoShapeFillResourceConnector.h>

class QActionGroup;
class QButtonGroup;
class KoCanvasBase;
class KoInteractionStrategy;
class KoPathToolHandle;
class KoParameterShape;
class KUndo2Command;

class QAction;
class QMenu;


/// The tool for editing a KoPathShape or a KoParameterShape.
/// See KoCreatePathTool for code handling the initial path creation.
class KRITAFLAKE_EXPORT KoPathTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit KoPathTool(KoCanvasBase *canvas);
    ~KoPathTool() override;

    void paint(QPainter &painter, const KoViewConverter &converter) override;
    void repaintDecorations();
    QRectF decorationsRect() const override;
    void mousePressEvent(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void mouseReleaseEvent(KoPointerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(KoPointerEvent *event) override;
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void deleteSelection() override;
    KoToolSelection* selection() override;
    void requestUndoDuringStroke() override;
    void requestStrokeCancellation() override;
    void requestStrokeEnd() override;
    void explicitUserStrokeEndRequest() override;

    QMenu* popupActionsMenu() override;

    // for KoPathToolSelection
    void notifyPathPointsChanged(KoPathShape *shape);

public Q_SLOTS:
    void documentResourceChanged(int key, const QVariant & res) override;

Q_SIGNALS:
    void typeChanged(int types);
    void singleShapeChanged(KoPathShape* path);

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
    void slotSelectionChanged();

private:
    void clearActivePointSelectionReferences();
    void initializeWithShapes(const QList<KoShape*> shapes);
    KUndo2Command* createPointToCurveCommand(const QList<KoPathPointData> &points);
    void mergePointsImpl(bool doJoin);

protected:
    KoPathToolSelection m_pointSelection; ///< the point selection
    QCursor m_selectCursor;

private:
    QScopedPointer<KoPathToolHandle> m_activeHandle;       ///< the currently active handle
    QPointF m_lastPoint; ///< needed for interaction strategy
    QScopedPointer<PathSegment> m_activeSegment;

    // make a frind so that it can test private member/methods
    friend class TestPathTool;

    QScopedPointer<KoInteractionStrategy> m_currentStrategy; ///< the rubber selection strategy

    QButtonGroup *m_pointTypeGroup;
    QActionGroup *m_points;
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
    QScopedPointer<QMenu> m_contextMenu;
    KisSignalAutoConnectionsStore m_canvasConnections;
    KoShapeFillResourceConnector m_shapeFillResourceConnector;

    Q_DECLARE_PRIVATE(KoToolBase)
};

#endif

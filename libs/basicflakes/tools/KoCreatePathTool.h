/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOCREATEPATHTOOL_H
#define KOCREATEPATHTOOL_H

#include "kritabasicflakes_export.h"

#include <KoFlakeTypes.h>
#include <KoToolBase.h>

#include <QList>

class KoPathShape;
class KoShapeStroke;

class KoCreatePathToolPrivate;

#define KoCreatePathTool_ID "CreatePathTool"

/**
 * Tool for creating path shapes.
 */
class KRITABASICFLAKES_EXPORT KoCreatePathTool : public KoToolBase
{
    Q_OBJECT
public:
    /**
     * Constructor for the tool that allows you to create new paths by hand.
     * @param canvas the canvas this tool will be working for.
     */
    explicit KoCreatePathTool(KoCanvasBase * canvas);
    ~KoCreatePathTool() override;

    QRectF decorationsRect() const override;

    /// reimplemented
    void paint(QPainter &painter, const KoViewConverter &converter) override;

    /// reimplemented
    void mousePressEvent(KoPointerEvent *event) override;
    /// reimplemented
    void mouseDoubleClickEvent(KoPointerEvent *event) override;
    /// reimplemented
    void mouseMoveEvent(KoPointerEvent *event) override;
    /// reimplemented
    void mouseReleaseEvent(KoPointerEvent *event) override;

    /**
    * Returns true if path has been started
    */
    bool pathStarted() const;

    bool tryMergeInPathShape(KoPathShape *pathShape);

    void setEnableClosePathShortcut(bool value);

public Q_SLOTS:
    /// reimplemented
    void activate(const QSet<KoShape*> &shapes) override;
    /// reimplemented
    void deactivate() override;
    /// reimplemented
    void documentResourceChanged(int key, const QVariant & res) override;

Q_SIGNALS:
    void sigUpdateAutoSmoothCurvesGUI(bool value);

protected:
    /**
     * Add path shape to document.
     * This method can be overridden and change the behaviour of the tool. In that case the subclass takes ownership of pathShape.
     * It gets only called if there are two or more points in the path.
     */
    virtual void addPathShape(KoPathShape* pathShape);

protected:
    /**
      * This method is called to paint the path. Decorations are drawn by KoCreatePathTool afterwards.
      */
    virtual void paintPath(KoPathShape& pathShape, QPainter &painter, const KoViewConverter &converter);

    void endPath();
    void endPathWithoutLastPoint();
    void cancelPath();
    void removeLastPoint();

    bool addPathShapeImpl(KoPathShape* pathShape, bool tryMergeOnly);

    /// reimplemented
    QList<QPointer<QWidget> > createOptionWidgets() override;

private:
    Q_DECLARE_PRIVATE(KoCreatePathTool)
    Q_PRIVATE_SLOT(d_func(), void angleDeltaChanged(qreal))
    Q_PRIVATE_SLOT(d_func(), void angleSnapChanged(int))
    Q_PRIVATE_SLOT(d_func(), void autoSmoothCurvesChanged(bool))
};
#endif

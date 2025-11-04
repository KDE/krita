/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISROOTSURFACETRACKERBASE_H
#define KISROOTSURFACETRACKERBASE_H

#include <kritawidgetutils_export.h>

#include <QObject>
#include <QPointer>

class QWidget;
class QWindow;
class QEvent;

/**
 * KisRootSurfaceProxyBase is a special proxy object for toplevel
 * native window attached to the current QWidget
 *
 * When created, the proxy does the following:
 *
 * 1) Finds the top-level widget that \p watched belongs to.
 *    (is \p watched changed its parent in the meantime, e.g.
 *     during construction, then the proxy will handle that
 *     as well)
 *
 * 2) Finds the QWindow that this toplevel widget is painted on
 *    (if platform window is not created yet, subscribes to
 *     QPlatformSurfaceEvent to attach when platform window is
 *     finally created)
 *
 * 3) When found, calls implementation of connectToNativeWindow()
 *    that would connect to the actual native window
 */
class KRITAWIDGETUTILS_EXPORT KisRootSurfaceTrackerBase : public QObject
{
    Q_OBJECT
public:
    KisRootSurfaceTrackerBase(QWidget *watched, QObject *parent = nullptr);
    ~KisRootSurfaceTrackerBase();

    QWidget* trackedWidget() const;

protected:
    virtual void connectToNativeWindow(QWindow *nativeWindow) = 0;
    virtual void disconnectFromNativeWindow() = 0;
    void initialize();

private:

    bool eventFilter(QObject *watched, QEvent *event) override;

    void tryUpdateHierarchy();
    void tryReconnectToNativeWindow();

    QVector<QPointer<QObject>> getCurrentHierarchy(QWidget *wdg);

    void reconnectToHierarchy(const QVector<QPointer<QObject>> newHierarchy);

private:
    QWidget *m_watched {nullptr};
    QVector<QPointer<QObject>> m_watchedHierarchy;

    QPointer<QWidget> m_topLevelWidgetWithSurface;

    QPointer<QWindow> m_topLevelNativeWindow;
    QPointer<QObject> m_childChangedFilter;
};

#endif /* KISROOTSURFACETRACKERBASE_H */

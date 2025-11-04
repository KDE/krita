/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENMIGRATIONTRACKER_H
#define KISSCREENMIGRATIONTRACKER_H

#include <KisRootSurfaceTrackerBase.h>

//#include <QObject>
//#include <QPointer>
//#include <kritawidgetutils_export.h>
#include <kis_signal_auto_connection.h>


class QScreen;
class QWidget;
class QWindow;
class KisSignalCompressor;

/**
 * A simple class that tracks the current screen assigned to the widget. When
 * the widget migrates to a different screen, a signal is emitted.
 *
 * If KisScreenMigrationTracker is created **before** the actual window for
 * the widget is created, then it subscribes to widget's QEvent::Show event
 * and waits until the widget is displayed.
 */
class KRITAWIDGETUTILS_EXPORT KisScreenMigrationTracker : public KisRootSurfaceTrackerBase
{
    Q_OBJECT
public:
    KisScreenMigrationTracker(QWidget *trackedWidget, QObject *parent = nullptr);

    /**
     * Return the screen currently assigned to the tracked widget. If the widget
     * has no native window associated, then the function asserts.
     */
    QScreen* currentScreen() const;

    /**
     * Return the screen currently assigned to the tracked widget or the default
     * screen if the widget has no native window association (usually it means that
     * the widget hasn't yet been added into the window hierarchy).
     */
    QScreen* currentScreenSafe() const;

private Q_SLOTS:
    void slotScreenChanged(QScreen *screen);
    void slotScreenResolutionChanged(qreal value);
    void slotScreenLogicalResolutionChanged(qreal value);
    void slotResolutionCompressorTriggered();

Q_SIGNALS:
    /**
     * Emitted when the widget migrates to a different screen
     */
    void sigScreenChanged(QScreen *screen);

    /**
     * Emitted when the widget migrates to a different screen or screen resolution
     * changes. This signal is useful for adjusting the display scale factor.
     */
    void sigScreenOrResolutionChanged(QScreen *screen);

private:
    void connectScreenSignals(QScreen *screen);

protected:
    void connectToNativeWindow(QWindow *window) override;
    void disconnectFromNativeWindow() override;

private:
    Q_DISABLE_COPY_MOVE(KisScreenMigrationTracker)

    QPointer<QWindow> m_connectedTopLevelWindow;
    QMetaObject::Connection m_topLevelWindowConnection;

    KisSignalAutoConnectionsStore m_screenConnections;
    KisSignalCompressor *m_resolutionChangeCompressor;
};

#endif // KISSCREENMIGRATIONTRACKER_H

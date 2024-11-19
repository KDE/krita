/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENMIGRATIONTRACKER_H
#define KISSCREENMIGRATIONTRACKER_H

#include <QObject>
#include <kritawidgetutils_export.h>
#include <kis_signal_auto_connection.h>


class QScreen;
class QWidget;
class QWindow;
class KisSignalCompressor;

class KRITAWIDGETUTILS_EXPORT KisScreenMigrationTracker : public QObject
{
    Q_OBJECT
public:
    KisScreenMigrationTracker(QWidget *trackedWidget, QObject *parent = nullptr);

    QScreen* currentScreen() const;
    QScreen* currentScreenSafe() const;

private Q_SLOTS:
    void slotScreenChanged(QScreen *screen);
    void slotScreenResolutionChanged(qreal value);
    void slotScreenLogicalResolutionChanged(qreal value);
    void slotResolutionCompressorTriggered();

Q_SIGNALS:
    void sigScreenChanged(QScreen *screen);
    void sigScreenOrResolutionChanged(QScreen *screen);

private:
    void connectScreenSignals(QScreen *screen);
    void connectTopLevelWindow(QWindow *window);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(KisScreenMigrationTracker)

    QWidget *m_trackedWidget {nullptr};
    QPointer<QWindow> m_trackedTopLevelWindow;
    KisSignalAutoConnectionsStore m_screenConnections;
    KisSignalCompressor *m_resolutionChangeCompressor;
};

#endif // KISSCREENMIGRATIONTRACKER_H

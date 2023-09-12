/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENMIGRATIONTRACKER_H
#define KISSCREENMIGRATIONTRACKER_H

#include <QObject>
#include <kritawidgetutils_export.h>

class QScreen;
class QWidget;

class KRITAWIDGETUTILS_EXPORT KisScreenMigrationTracker : public QObject
{
    Q_OBJECT
public:
    KisScreenMigrationTracker(QWidget *trackedWidget);

private Q_SLOTS:
    void slotScreenChanged(QScreen *screen);
    void slotScreenResolutionChanged();

Q_SIGNALS:
    void sigScreenChanged(QScreen *screen);
    void sigScreenOrResolutionChanged(QScreen *screen);

private:
    Q_DISABLE_COPY_MOVE(KisScreenMigrationTracker)

    QWidget *m_trackedWidget {nullptr};
    QMetaObject::Connection m_screenConnection;
};

#endif // KISSCREENMIGRATIONTRACKER_H

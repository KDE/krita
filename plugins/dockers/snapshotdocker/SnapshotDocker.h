/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef SNAPSHOT_DOCKER_H_
#define SNAPSHOT_DOCKER_H_

#include <QDockWidget>
#include <QScopedPointer>

#include <kis_mainwindow_observer.h>
#include <klocalizedstring.h>

#include <KoShapeController.h>
#include <KoCanvasBase.h>

class SnapshotDocker : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    SnapshotDocker();
    ~SnapshotDocker() override;

    QString observerName() override { return "SnapshotDocker"; }

    void setViewManager(KisViewManager* viewManager) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private Q_SLOTS:
    void slotBnAddClicked();
    void slotBnSwitchToClicked();
    void slotBnRemoveClicked();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif

/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef CHANNELDOCKER_DOCK_H
#define CHANNELDOCKER_DOCK_H

#include <QPointer>
#include <QDockWidget>

#include <KoCanvasObserverBase.h>

#include <kis_canvas2.h>
#include "ChannelDockerWidget.h"

class ChannelDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    ChannelDockerDock();

    QString observerName() override { return "ChannelDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private:
    ChannelDockerWidget *m_widget {nullptr};
    QPointer<KisCanvas2> m_canvas;
};


#endif

/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef _HISTORY_DOCK_H_
#define _HISTORY_DOCK_H_

#include <kddockwidgets/DockWidget.h>
#include <QToolButton>

#include "KisUndoView.h"

#include <KoCanvasObserverBase.h>
#include <klocalizedstring.h>
#include <kundo2stack.h>

#include <KoShapeController.h>
#include <KoCanvasBase.h>
#include "kis_types.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_image.h"
#include "kis_paint_device.h"

class HistoryDock : public KDDockWidgets::DockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    HistoryDock();
    QString observerName() override { return "HistoryDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private Q_SLOTS:
    void configure();
private:
    KisUndoView *m_undoView;
    QToolButton *m_bnConfigure;
    KoCanvasBase *m_historyCanvas;
};


#endif

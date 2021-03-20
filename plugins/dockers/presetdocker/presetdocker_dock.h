/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _PRESETDOCKER_DOCK_H_
#define _PRESETDOCKER_DOCK_H_

#include <QPointer>
#include <kddockwidgets/DockWidget.h>
#include <KoCanvasObserverBase.h>
#include <kis_canvas2.h>

class KisPaintOpPresetsChooserPopup;

class PresetDockerDock : public KDDockWidgets::DockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    PresetDockerDock( );
    QString observerName() override { return "PresetDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override { m_canvas = 0; setEnabled(false);}
public Q_SLOTS:
    void canvasResourceChanged(int key = 0, const QVariant& v = QVariant());
private:
    QPointer<KisCanvas2> m_canvas;
    KisPaintOpPresetsChooserPopup* m_presetChooser;
};


#endif

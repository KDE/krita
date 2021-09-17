/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "presetdocker_dock.h"

#include <QHBoxLayout>
#include <QPushButton>

#include <klocalizedstring.h>

#include <KoCanvasResourceProvider.h>
#include <KoCanvasBase.h>

#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_paintop_box.h"
#include "kis_paintop_presets_chooser_popup.h"
#include "kis_canvas_resource_provider.h"
#include <brushengine/kis_paintop_preset.h>


PresetDockerDock::PresetDockerDock( )
    : QDockWidget(i18nc("@title:window Brush presets chooser docker", "Brush Presets"))
    , m_canvas(0)
{
    m_presetChooser = new KisPaintOpPresetsChooserPopup(this);
    m_presetChooser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_presetChooser->showButtons(false);
    setWidget(m_presetChooser);
}

void PresetDockerDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_presetChooser->disconnect(m_canvas->viewManager()->paintOpBox());
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (!m_canvas || !m_canvas->viewManager() || !m_canvas->resourceManager()) return;

    connect(m_presetChooser, SIGNAL(resourceSelected(KoResourceSP )),
            m_canvas->viewManager()->paintOpBox(), SLOT(resourceSelected(KoResourceSP )));
    connect(m_presetChooser, SIGNAL(resourceClicked(KoResourceSP )),
            m_canvas->viewManager()->paintOpBox(), SLOT(resourceSelected(KoResourceSP )));
    connect(canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
            this, SLOT(canvasResourceChanged(int,QVariant)));


    connect(m_canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()), m_presetChooser, SLOT(slotThemeChanged()), Qt::UniqueConnection);
    m_presetChooser->slotThemeChanged();

    canvasResourceChanged(KoCanvasResource::CurrentPaintOpPreset);
}

void PresetDockerDock::canvasResourceChanged(int key, const QVariant& /*v*/)
{
    if (key == KoCanvasResource::CurrentPaintOpPreset) {
        if (m_canvas && m_canvas->resourceManager()) {
            if (sender()) sender()->blockSignals(true);
            KisPaintOpPresetSP preset = m_canvas->resourceManager()->resource(KoCanvasResource::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
            if(preset)
                m_presetChooser->canvasResourceChanged(preset);
            if (sender()) sender()->blockSignals(false);
        }
    }
}




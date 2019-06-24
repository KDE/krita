/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    : QDockWidget(i18n("Brush Presets"))
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


    connect(m_canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()), m_presetChooser, SLOT(slotThemeChanged()));

    canvasResourceChanged();
}

void PresetDockerDock::canvasResourceChanged(int /*key*/, const QVariant& /*v*/)
{
    if (m_canvas && m_canvas->resourceManager()) {
        if (sender()) sender()->blockSignals(true);
        KisPaintOpPresetSP preset = m_canvas->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
        if(preset)
            m_presetChooser->canvasResourceChanged(preset);
        if (sender()) sender()->blockSignals(false);
        m_presetChooser->updateViewSettings();
    }
}




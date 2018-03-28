/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_painting_assistants_manager.h"

#include "KisViewManager.h"
#include "kis_action_manager.h"
#include "kis_action.h"

#include "kis_canvas2.h"
#include <klocalizedstring.h>
#include <kguiitem.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>


KisPaintingAssistantsManager::KisPaintingAssistantsManager(KisViewManager* view) : QObject(view)
    , m_imageView(0)
{

}

KisPaintingAssistantsManager::~KisPaintingAssistantsManager()
{

}

void KisPaintingAssistantsManager::setup(KisActionManager * actionManager)
{
    m_toggleAssistant = actionManager->createAction("view_toggle_painting_assistants");
    m_togglePreview   = actionManager->createAction("view_toggle_assistant_previews");

    updateAction();
}

void KisPaintingAssistantsManager::setView(QPointer<KisView> imageView)
{

    // set view is called twice when a document is open, so we need to disconnect the original signals
    // if m_imageView has already been created. This prevents double signal events firing
    if (m_imageView) {
        m_toggleAssistant->disconnect();
        m_togglePreview->disconnect();

        if (decoration()) {
            decoration()->disconnect(this);
        }
    }

    m_imageView = imageView;


    if (m_imageView && !decoration()) {
        KisPaintingAssistantsDecoration* deco = new KisPaintingAssistantsDecoration(m_imageView);
        m_imageView->canvasBase()->addDecoration(deco);
    }
    if (m_imageView && decoration()) {
        connect(m_toggleAssistant, SIGNAL(triggered()), decoration(), SLOT(toggleAssistantVisible()));
        connect(m_togglePreview, SIGNAL(triggered()), decoration(), SLOT(toggleOutlineVisible()));
        connect(decoration(), SIGNAL(assistantChanged()), SLOT(updateAction()));
    }
    updateAction();
}

void KisPaintingAssistantsManager::updateAction()
{
    if (decoration()) {
        bool enabled = !decoration()->assistants().isEmpty();
        m_toggleAssistant->setChecked(decoration()->visible());
        m_toggleAssistant->setEnabled(enabled);
        m_togglePreview->setChecked(decoration()->outlineVisibility());
        m_togglePreview->setEnabled(enabled);
    } else {
        m_toggleAssistant->setEnabled(false);
    }
}

KisPaintingAssistantsDecorationSP KisPaintingAssistantsManager::decoration()
{
    if (m_imageView) {
        return m_imageView->canvasBase()->paintingAssistantsDecoration();
    }
    return 0;
}


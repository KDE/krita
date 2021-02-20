/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDecorationsManager.h"

#include "KisViewManager.h"
#include "kis_action_manager.h"
#include "kis_action.h"

#include "kis_canvas2.h"
#include <klocalizedstring.h>
#include <kguiitem.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <KisDocument.h>


KisDecorationsManager::KisDecorationsManager(KisViewManager* view)
    : QObject(view)
    , m_imageView(0)
{
}

KisDecorationsManager::~KisDecorationsManager()
{
}

void KisDecorationsManager::setup(KisActionManager * actionManager)
{
    m_toggleAssistant = actionManager->createAction("view_toggle_painting_assistants");
    m_togglePreview   = actionManager->createAction("view_toggle_assistant_previews");
    m_toggleReferenceImages = actionManager->createAction("view_toggle_reference_images");

    updateAction();
}

void KisDecorationsManager::setView(QPointer<KisView> imageView)
{

    // set view is called twice when a document is open, so we need to disconnect the original signals
    // if m_imageView has already been created. This prevents double signal events firing
    if (m_imageView) {
        m_toggleAssistant->disconnect();
        m_togglePreview->disconnect();

        if (assistantsDecoration()) {
            assistantsDecoration()->disconnect(this);
        }
        if (referenceImagesDecoration()) {
            referenceImagesDecoration()->disconnect(this);
        }
    }

    m_imageView = imageView;

    if (m_imageView && !referenceImagesDecoration()) {
        KisReferenceImagesDecoration *deco = new KisReferenceImagesDecoration(m_imageView, imageView->document());
        m_imageView->canvasBase()->addDecoration(deco);
    }

    if (m_imageView && !assistantsDecoration()) {
        KisPaintingAssistantsDecoration *deco = new KisPaintingAssistantsDecoration(m_imageView);
        m_imageView->canvasBase()->addDecoration(deco);
    }

    if (m_imageView && assistantsDecoration()) {
        connect(m_toggleAssistant, SIGNAL(triggered()), assistantsDecoration(), SLOT(toggleAssistantVisible()));
        connect(m_togglePreview, SIGNAL(triggered()), assistantsDecoration(), SLOT(toggleOutlineVisible()));
        connect(assistantsDecoration(), SIGNAL(assistantChanged()), SLOT(updateAction()));
        connect(m_imageView->document(), &KisDocument::sigAssistantsChanged,
                assistantsDecoration(), &KisPaintingAssistantsDecoration::slotUpdateDecorationVisibility);
        connect(m_imageView->document(), &KisDocument::sigAssistantsChanged,
                m_imageView->canvasBase(), QOverload<>::of(&KisCanvas2::updateCanvas));
    }

    if (m_imageView && referenceImagesDecoration()) {
        connect(m_toggleReferenceImages, SIGNAL(triggered(bool)), referenceImagesDecoration(), SLOT(setVisible(bool)), Qt::UniqueConnection);
    }


    updateAction();
}

void KisDecorationsManager::updateAction()
{
    if (assistantsDecoration()) {
        bool enabled = !assistantsDecoration()->assistants().isEmpty();
        m_toggleAssistant->setChecked(assistantsDecoration()->visible());
        m_toggleAssistant->setEnabled(enabled);
        m_togglePreview->setChecked(assistantsDecoration()->outlineVisibility());
        m_togglePreview->setEnabled(enabled);
    } else {
        m_toggleAssistant->setEnabled(false);
    }

    if (referenceImagesDecoration()) {
       m_toggleReferenceImages->setEnabled(referenceImagesDecoration()->documentHasReferenceImages());
       m_toggleReferenceImages->setChecked(referenceImagesDecoration()->visible());
    }
}

KisPaintingAssistantsDecorationSP KisDecorationsManager::assistantsDecoration() const
{
    if (m_imageView) {
        return m_imageView->canvasBase()->paintingAssistantsDecoration();
    }
    return 0;
}

KisReferenceImagesDecorationSP KisDecorationsManager::referenceImagesDecoration() const
{
    if (m_imageView) {
        return m_imageView->canvasBase()->referenceImagesDecoration();
    }
    return 0;
}


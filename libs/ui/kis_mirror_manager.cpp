/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mirror_manager.h"
#include "KisViewManager.h"
#include <kis_canvas_controller.h>
#include <kis_icon.h>

#include <klocalizedstring.h>
#include <kguiitem.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <QAction>

#include "kis_canvas2.h"
#include "kis_mirror_axis.h"
#include <KisMirrorAxisConfig.h>
#include <KisDocument.h>
#include <kis_signals_blocker.h>
#include <kis_types.h>

KisMirrorManager::KisMirrorManager(KisViewManager* view) : QObject(view)
    , m_imageView(0)
{
}

KisMirrorManager::~KisMirrorManager()
{
}

void KisMirrorManager::setup(KActionCollection * collection)
{
    m_mirrorCanvas = new KToggleAction(i18n("Mirror View"), this);
    m_mirrorCanvas->setChecked(false);
    m_mirrorCanvas->setIcon(KisIconUtils::loadIcon("mirror-view"));

    collection->addAction("mirror_canvas", m_mirrorCanvas);
    collection->setDefaultShortcut(m_mirrorCanvas, QKeySequence(Qt::Key_M));

    updateAction();
}

void KisMirrorManager::setView(QPointer<KisView> imageView)
{
    if (m_imageView) {
        m_mirrorCanvas->disconnect();
        m_imageView->document()->disconnect(this);

        KisMirrorAxisSP canvasDecoration = this->decoration();
        if (canvasDecoration) {
            canvasDecoration->disconnect();
        }
    }

    m_imageView = imageView;

    if (m_imageView)  {
        connect(m_mirrorCanvas, SIGNAL(toggled(bool)), dynamic_cast<KisCanvasController*>(m_imageView->canvasController()), SLOT(mirrorCanvas(bool)));
        connect(m_imageView->document(), SIGNAL(sigMirrorAxisConfigChanged()), this, SLOT(slotDocumentConfigChanged()), Qt::UniqueConnection);

        KisMirrorAxisSP canvasDecoration = this->decoration();
        if (!canvasDecoration) {
            KisMirrorAxis* decoration = new KisMirrorAxis(m_imageView->viewManager()->canvasResourceProvider(), m_imageView);
            connect(decoration, SIGNAL(sigConfigChanged()), this, SLOT(slotMirrorAxisConfigChanged()), Qt::UniqueConnection);
            m_imageView->canvasBase()->addDecoration(decoration);
        } else {
            connect(canvasDecoration.data(), SIGNAL(sigConfigChanged()), this, SLOT(slotMirrorAxisConfigChanged()), Qt::UniqueConnection);
        }

        setDecorationConfig();
    }

    updateAction();
}

void KisMirrorManager::updateAction()
{
    if (m_imageView) {
        m_mirrorCanvas->setEnabled(true);
        m_mirrorCanvas->setChecked(m_imageView->canvasIsMirrored());
    }
    else {
        m_mirrorCanvas->setEnabled(false);
        m_mirrorCanvas->setChecked(false);
    }
}

void KisMirrorManager::slotDocumentConfigChanged()
{
    setDecorationConfig();
}

void KisMirrorManager::slotMirrorAxisConfigChanged()
{
    if (m_imageView && m_imageView->document()) {
        KisSignalsBlocker blocker(m_imageView->document());

        KisMirrorAxisSP canvasDecoration = this->decoration();
        if (canvasDecoration) {
            m_imageView->document()->setMirrorAxisConfig(canvasDecoration->mirrorAxisConfig());
        }
    }
}

KisMirrorAxisSP KisMirrorManager::decoration() const
{
    if (m_imageView) {
        return qobject_cast<KisMirrorAxis*>(m_imageView->canvasBase()->decoration("mirror_axis").data());
    } else {
        return 0;
    }
}

void KisMirrorManager::setDecorationConfig()
{
    if (m_imageView && m_imageView->document()) {
        KisMirrorAxisConfig config = m_imageView->document()->mirrorAxisConfig();

        KisMirrorAxisSP canvasDecoration = this->decoration();
        if (canvasDecoration) {
            canvasDecoration->setMirrorAxisConfig(config);
        }
    }
}

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
    }
    m_imageView = imageView;
    if (m_imageView)  {
        connect(m_mirrorCanvas, SIGNAL(toggled(bool)), dynamic_cast<KisCanvasController*>(m_imageView->canvasController()), SLOT(mirrorCanvas(bool)));

        if (!hasDecoration()) {
            m_imageView->canvasBase()->addDecoration(new KisMirrorAxis(m_imageView->viewManager()->resourceProvider(), m_imageView));
        }
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

KisMirrorAxis* KisMirrorManager::hasDecoration() {

    if (m_imageView && m_imageView->canvasBase() && m_imageView->canvasBase()->decoration("mirror_axis")) {
        return dynamic_cast<KisMirrorAxis*>(m_imageView->canvasBase()->decoration("mirror_axis").data());
    }
    return 0;
}

/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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

#include "kis_image_manager.h"

#include <QString>

#include <kstandardaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <KoIcon.h>
#include <KoFilterManager.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
typedef QPointer<KoUpdater> KoUpdaterPtr;

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>

#include "kis_layer_manager.h"
#include "kis_statusbar.h"
#include "kis_import_catcher.h"
#include "kis_view2.h"
#include "kis_doc2.h"
#include "dialogs/kis_dlg_image_properties.h"
#include "commands/kis_image_commands.h"
#include "kis_progress_widget.h"

KisImageManager::KisImageManager(KisView2 * view)
        : m_view(view)
{
}

void KisImageManager::setup(KActionCollection * actionCollection)
{
    KAction *action  = new KAction(i18n("I&nsert Image as Layer..."), this);
    actionCollection->addAction("insert_image_as_layer", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotInsertImageAsLayer()));

    action  = new KAction(koIcon("document-properties"), i18n("Properties..."), this);
    actionCollection->addAction("image_properties", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImageProperties()));
}


void KisImageManager::updateGUI()
{
}


void KisImageManager::slotInsertImageAsLayer()
{
    if (importImage() > 0)
        m_view->image()->setModified();

}

qint32 KisImageManager::importImage(const KUrl& urlArg)
{
    KisImageWSP currentImage = m_view->image();

    if (!currentImage) {
        return 0;
    }

    KUrl::List urls;
    qint32 rc = 0;

    if (urlArg.isEmpty()) {
        QString mimelist = KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Import).join(" ");
        urls = KFileDialog::getOpenUrls(KUrl(QString()), mimelist, 0, i18n("Import Image"));
    } else {
        urls.push_back(urlArg);
    }

    if (urls.empty())
        return 0;

    for (KUrl::List::iterator it = urls.begin(); it != urls.end(); ++it) {
        new KisImportCatcher(*it, m_view);
    }

    m_view->canvas()->update();

    return rc;
}

void KisImageManager::resizeCurrentImage(qint32 w, qint32 h, qint32 xOffset, qint32 yOffset)
{
    if (!m_view->image()) return;

    m_view->image()->cropImage(QRect(-xOffset, -yOffset, w, h));
}

void KisImageManager::scaleCurrentImage(const QSize &size, qreal xres, qreal yres, KisFilterStrategy *filterStrategy)
{
    if (!m_view->image()) return;
    m_view->image()->scaleImage(size, xres, yres, filterStrategy);
}

void KisImageManager::rotateCurrentImage(double radians)
{
    if (!m_view->image()) return;
    m_view->image()->rotateImage(radians);
}

void KisImageManager::shearCurrentImage(double angleX, double angleY)
{
    if (!m_view->image()) return;
    m_view->image()->shear(angleX, angleY);
}


void KisImageManager::slotImageProperties()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    QPointer<KisDlgImageProperties> dlg = new KisDlgImageProperties(image, m_view);
    if (dlg->exec() == QDialog::Accepted) {
        image->convertProjectionColorSpace(dlg->colorSpace());
    }
    delete dlg;
}


#include "kis_image_manager.moc"

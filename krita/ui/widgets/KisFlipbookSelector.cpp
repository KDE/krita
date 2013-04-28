/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisFlipbookSelector.h"

#include <kis_doc2.h>
#include <kis_part2.h>
#include <kis_flipbook.h>
#include <kis_flipbook_item.h>
#include <kis_image.h>

#include <KoIcon.h>
#include <KoFilterManager.h>
#include <KoServiceProvider.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kurl.h>

#include <QDesktopServices>
#include <QListWidget>
#include <QListWidgetItem>

KisFlipbookSelector::KisFlipbookSelector(QWidget *parent, KisDoc2 *document)
    : QWidget(parent)
    , m_document(document)
{
    setupUi(this);

    connect(bnCreateNewFlipbook, SIGNAL(clicked()), SLOT(createImage()));
}

void KisFlipbookSelector::createImage()
{
    const QStringList mimeFilter = KoFilterManager::mimeFilter(KoServiceProvider::readNativeFormatMimeType(),
                                   KoFilterManager::Import,
                                   KoServiceProvider::readExtraNativeMimeTypes());

    QStringList urls = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///OpenDialog"),
                                                     mimeFilter.join(" "),
                                                     this, i18n("Select files to add to flipbook"));

    if (urls.size() < 1) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    KisFlipbook *flipbook = new KisFlipbook();
    foreach(QString url, urls) {
        if (QFile::exists(url)) {
            flipbook->addItem(url);
        }
    }

    flipbook->setName(txtFlipbookName->text());

    m_document->setUrl(urls[0]);
    m_document->setCurrentImage(static_cast<KisFlipbookItem*>(flipbook->item(0))->document()->image());

    static_cast<KisPart2*>(m_document->documentPart())->setFlipbook(flipbook);
    QApplication::restoreOverrideCursor();
    emit documentSelected();
}

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

#include "flipbookdocker_dock.h"

#include <QListWidget>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QDesktopServices>

#include <klocale.h>
#include <kactioncollection.h>
#include <kurl.h>

#include <KoIcon.h>
#include <KoCanvasBase.h>
#include <KoZoomController.h>
#include <KoZoomMode.h>
#include <KoFilterManager.h>
#include <KoServiceProvider.h>
#include <KoMainWindow.h>
#include <KoView.h>

#include <kis_image.h>
#include <kis_view2.h>
#include <kis_canvas2.h>
#include <kis_flipbook.h>
#include <kis_flipbook_item.h>
#include <kis_doc2.h>
#include <kis_part2.h>
#include <kis_zoom_manager.h>
#include <kis_canvas_controller.h>

#include "FlipbookView.h"
#include "sequence_viewer.h"

FlipbookDockerDock::FlipbookDockerDock( )
    : QDockWidget(i18n("Flipbook"))
    , m_canvas(0)
    , m_flipbook(0)
    , m_animating(false)
    , m_animationWidget(0)
    , m_canvasWidget(0)
{
    QWidget* widget = new QWidget(this);
    setupUi(widget);
    setWidget(widget);

    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(updateLayout(Qt::DockWidgetArea)));

    bnSaveFlipbook->setIcon(koIcon("document-save"));
    bnSaveFlipbook->setToolTip(i18n("Save the current flipbook"));
    connect(bnSaveFlipbook, SIGNAL(clicked()), SLOT(saveFlipbook()));

    bnNewFlipbook->setIcon(koIcon("document-new"));
    bnNewFlipbook->setToolTip(i18n("Create a new flipbook"));
    connect(bnNewFlipbook, SIGNAL(clicked()), SLOT(newFlipbook()));

    bnLoadFlipbook->setIcon(koIcon("document-open"));
    bnLoadFlipbook->setToolTip(i18n("Open a flipbook file"));
    connect(bnLoadFlipbook, SIGNAL(clicked()), SLOT(openFlipbook()));

    bnFirstItem->setIcon(koIcon("arrow-up-double"));
    bnFirstItem->setToolTip(i18n("Go to the first image in the current flipbook"));
    connect(bnFirstItem, SIGNAL(clicked()), SLOT(goFirst()));

    bnPreviousItem->setIcon(koIcon("arrow-up"));
    bnPreviousItem->setToolTip(i18n("Show previous image"));
    connect(bnPreviousItem, SIGNAL(clicked()), SLOT(goPrevious()));

    bnNextItem->setIcon(koIcon("arrow-down"));
    bnNextItem->setToolTip(i18n("Show next image"));
    connect(bnNextItem, SIGNAL(clicked()), SLOT(goNext()));

    bnLastItem->setIcon(koIcon("arrow-down-double"));
    bnLastItem->setToolTip(i18n("Go to the last image in the current flipblook"));
    connect(bnLastItem, SIGNAL(clicked()), SLOT(goLast()));

    bnAddItem->setIcon(koIcon("list-add"));
    bnAddItem->setToolTip(i18n("Add one or more images to the current flipbook"));
    connect(bnAddItem, SIGNAL(clicked()), SLOT(addImage()));

    bnDeleteItem->setIcon(koIcon("list-remove"));
    bnDeleteItem->setToolTip(i18n("Remove selected images from the current flipbook"));
    connect(bnDeleteItem, SIGNAL(clicked()), SLOT(removeImage()));

    connect(listFlipbook, SIGNAL(currentItemChanged(const QModelIndex &)), SLOT(selectImage(const QModelIndex&)));
}

FlipbookDockerDock::~FlipbookDockerDock()
{
}

void FlipbookDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas && m_canvas->view()) {
         m_canvas->view()->actionCollection()->disconnect(this);
         foreach(KXMLGUIClient* client, m_canvas->view()->childClients()) {
            client->actionCollection()->disconnect(this);
        }
    }
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if (m_canvas && m_canvas->view() && m_canvas->view()->document() && m_canvas->view()->document()->documentPart()) {
        m_flipbook = dynamic_cast<KisPart2*>(m_canvas->view()->document()->documentPart())->flipbook();
        if (!m_flipbook) {
            listFlipbook->setModel(0);
            txtName->clear();
        }
        else {
            listFlipbook->setModel(m_flipbook);
            txtName->setText(m_flipbook->name());
        }
    }
}

void FlipbookDockerDock::unsetCanvas()
{
    m_canvas = 0;
}

void FlipbookDockerDock::updateLayout(Qt::DockWidgetArea area)
{
    listFlipbook->setWrapping(false);
    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea) {
        listFlipbook->setFlow(QListView::TopToBottom);
        bnFirstItem->setIcon(koIcon("arrow-up-double"));
        bnPreviousItem->setIcon(koIcon("arrow-up"));
        bnNextItem->setIcon(koIcon("arrow-down"));
        bnLastItem->setIcon(koIcon("arrow-down-double"));
    }
    else {
        listFlipbook->setFlow(QListView::LeftToRight);
        bnFirstItem->setIcon(koIcon("arrow-left-double"));
        bnPreviousItem->setIcon(koIcon("arrow-left"));
        bnNextItem->setIcon(koIcon("arrow-right"));
        bnLastItem->setIcon(koIcon("arrow-right-double"));
    }
}

void FlipbookDockerDock::saveFlipbook()
{
    QString filename = KFileDialog::getSaveFileName(KUrl("kfiledialog:///OpenDialog"),
                                                    "*.flipbook", this, i18n("Save flipbook"));
    if (!filename.isEmpty()) {
        m_flipbook->setName(txtName->text());
        m_flipbook->save(filename);
    }
    m_canvas->view()->document()->documentPart()->addRecentURLToAllShells(filename);
}


void FlipbookDockerDock::newFlipbook()
{
    if (!m_canvas) return;
    if (!m_canvas->view()) return;
    if (!m_canvas->view()->document()) return;
    if (!m_canvas->view()->document()->documentPart()) return;

    const QStringList mimeFilter = KoFilterManager::mimeFilter(KoServiceProvider::readNativeFormatMimeType(),
                                   KoFilterManager::Import,
                                   KoServiceProvider::readExtraNativeMimeTypes());

    QStringList urls = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///OpenDialog"),
                                                     mimeFilter.join(" "),
                                                     this, i18n("Select files to add to flipbook"));

    if (urls.size() < 1) return;

    KisFlipbook *flipbook = new KisFlipbook();
    foreach(QString url, urls) {
        if (QFile::exists(url)) {
            flipbook->addItem(url);
        }
    }

    txtName->setText("");

    static_cast<KisPart2*>(m_canvas->view()->document()->documentPart())->setFlipbook(flipbook);

    KisFlipbook *old = m_flipbook;
    m_flipbook = flipbook;

    listFlipbook->setModel(m_flipbook);
    selectImage(m_flipbook->index(0, 0));
    delete old;

}

void FlipbookDockerDock::openFlipbook()
{
    QString filename = KFileDialog::getOpenFileName(KUrl("kfiledialog:///OpenDialog"), "*.flipbook", this, i18n("Load flipbook"));
    if (!QFile::exists(filename)) return;

    KisFlipbook *flipbook = new KisFlipbook();
    flipbook->load(filename);
    txtName->setText(flipbook->name());

    static_cast<KisPart2*>(m_canvas->view()->document()->documentPart())->setFlipbook(flipbook);

    KisFlipbook *old = m_flipbook;
    m_flipbook = flipbook;

    listFlipbook->setModel(m_flipbook);
    selectImage(m_flipbook->index(0, 0));
    delete old;
}

void FlipbookDockerDock::addImage()
{
    const QStringList mimeFilter = KoFilterManager::mimeFilter(KoServiceProvider::readNativeFormatMimeType(),
                                   KoFilterManager::Import,
                                   KoServiceProvider::readExtraNativeMimeTypes());

    QStringList urls = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///OpenDialog"),
                                                     mimeFilter.join(" "),
                                                     this, i18n("Select files to add to flipbook"));

    if (urls.size() < 1) return;

    foreach(QString url, urls) {
        if (QFile::exists(url)) {
            m_flipbook->addItem(url);
        }
    }

}

void FlipbookDockerDock::removeImage()
{
    QModelIndex idx = listFlipbook->currentIndex();
    QStandardItem *item = m_flipbook->itemFromIndex(idx);
    for (int i = 0; i < m_flipbook->rowCount(); ++i) {
        if (m_flipbook->item(i) == item) {
            delete m_flipbook->takeItem(i);
        }
    }
    listFlipbook->reset();
}

void FlipbookDockerDock::goFirst()
{
    listFlipbook->scrollToTop();
    listFlipbook->setCurrentIndex(m_flipbook->index(0, 0));
}

void FlipbookDockerDock::goPrevious()
{
    listFlipbook->goPrevious();
}

void FlipbookDockerDock::goNext()
{
    listFlipbook->goNext();
}

void FlipbookDockerDock::goLast()
{
    listFlipbook->scrollToBottom();
    listFlipbook->setCurrentIndex(m_flipbook->index(m_flipbook->rowCount() -1, 0));
}

void FlipbookDockerDock::selectImage(const QModelIndex &index)
{
    if (!index.isValid()) return;
    if (!m_canvas) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    KisFlipbookItem *item = dynamic_cast<KisFlipbookItem*>(m_flipbook->itemFromIndex(index));

    if (item && item->document() && item->document()->image()) {
        if (m_canvas->view()->document()->isModified()) {
            m_canvas->view()->document()->documentPart()->save();
            m_canvas->view()->document()->setModified(false);
        }
        m_canvas->view()->document()->setCurrentImage(item->document()->image());
        m_canvas->view()->document()->setUrl(item->filename());
        m_canvas->view()->zoomController()->setZoomMode(KoZoomMode::ZOOM_PAGE);
    }
    QApplication::restoreOverrideCursor();
}

void FlipbookDockerDock::toggleAnimation()
{
//    if (!m_animationWidget) {
//        m_animationWidget = new SequenceViewer(m_canvas->view()->shell());
//        m_animationWidget->hide();
//    }

//    if (!m_canvasWidget) {
//        m_canvasWidget = m_canvas->view()->findChild<KisCanvasController*>();
//    }

//    if (!m_animating) {
//        qDebug() << "start animation";
//        m_animating = true;
//        bnAnimate->setIcon(koIcon("media-playback-stop"));
//        m_canvas->view()->
//        m_mainWindow->setCentralWidget(m_animationWidget);
//        m_animationWidget->show();
//        m_canvasWidget->hide();
//    }
//    else {
//        qDebug() << "stopt animation";
//        m_animating = false;
//        bnAnimate->setIcon(koIcon("media-playback-start"));
//        m_mainWindow->setCentralWidget(m_canvasWidget);
//        m_animationWidget->hide();
//        m_canvasWidget->show();
//    }
}


#include "flipbookdocker_dock.moc"

/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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

#include "compositiondocker_dock.h"

#include <QGridLayout>
#include <QListView>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QInputDialog>
#include <QThread>
#include <QAction>
#include <QDesktopServices>
#include <QMenu>

#include <klocale.h>
#include <kactioncollection.h>

#include <KoIcon.h>
#include <KoCanvasBase.h>
#include <KoFileDialog.h>

#include <KisPart.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_painter.h>
#include <kis_paint_layer.h>
#include <kis_action.h>
#include <kis_action_manager.h>

#include "compositionmodel.h"


CompositionDockerDock::CompositionDockerDock( ) : QDockWidget(i18n("Compositions")), m_canvas(0)
{
    QWidget* widget = new QWidget(this);
    setupUi(widget);
    m_model = new CompositionModel(this);
    compositionView->setModel(m_model);
    compositionView->installEventFilter(this);
    deleteButton->setIcon(themedIcon("edit-delete"));
    saveButton->setIcon(themedIcon("list-add"));
    exportButton->setIcon(themedIcon("document-export"));

    deleteButton->setToolTip(i18n("Delete Composition"));
    saveButton->setToolTip(i18n("New Composition"));
    exportButton->setToolTip(i18n("Export Composition"));


    setWidget(widget);

    connect( compositionView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(activated ( const QModelIndex & ) ) );

    compositionView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( compositionView, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(customContextMenuRequested(QPoint)));

    connect( deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteClicked()));
    connect( saveButton, SIGNAL(clicked(bool)), this, SLOT(saveClicked()));
    connect( exportButton, SIGNAL(clicked(bool)), this, SLOT(exportClicked()));
    saveNameEdit->setPlaceholderText(i18n("Insert Name"));

    updateAction  = new KisAction(i18n("Update Composition"), this);
    updateAction->setObjectName("update_composition");
    connect(updateAction, SIGNAL(triggered()), this, SLOT(updateComposition()));

    renameAction  = new KisAction(i18n("Rename Composition..."), this);
    renameAction->setObjectName("rename_composition");
    connect(renameAction, SIGNAL(triggered()), this, SLOT(renameComposition()));
    m_actions.append(renameAction);
}

CompositionDockerDock::~CompositionDockerDock()
{
    
}

void CompositionDockerDock::setCanvas(KoCanvasBase * canvas)
{
    unsetCanvas();
    setEnabled(canvas != 0);

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if (m_canvas) {
        foreach(KisAction *action, m_actions) {
            m_canvas->viewManager()->actionManager()->addAction(action->objectName(), action);
        }
        updateModel();
    }
}

void CompositionDockerDock::unsetCanvas()
{
    setEnabled(false);
    if (m_canvas) {
        foreach(KisAction *action, m_actions) {
            m_canvas->viewManager()->actionManager()->takeAction(action);
        }
    }
    m_canvas = 0;
    m_model->setCompositions(QList<KisLayerComposition*>());
}

void CompositionDockerDock::activated(const QModelIndex& index)
{
    KisLayerComposition* composition = m_model->compositionFromIndex(index);
    composition->apply();
}

void CompositionDockerDock::deleteClicked()
{
    QModelIndex index = compositionView->currentIndex();
    if(index.isValid()) {
        KisLayerComposition* composition = m_model->compositionFromIndex(index);
        m_canvas->viewManager()->image()->removeComposition(composition);
        updateModel();
    }
}

void CompositionDockerDock::saveClicked()
{
    KisImageWSP image = m_canvas->viewManager()->image();
    // format as 001, 002 ...
    QString name = saveNameEdit->text();
    if (name.isEmpty()) {
        bool found = false;
        int i = 1;
        do {
            name = QString("%1").arg(i, 3, 10, QChar('0'));
            found = false;
            foreach(KisLayerComposition* composition, m_canvas->viewManager()->image()->compositions()) {
                if (composition->name() == name) {
                    found = true;
                    break;
                }
            }
            i++;
        } while(found && i < 1000);
    }
    KisLayerComposition* composition = new KisLayerComposition(image, name);
    composition->store();
    image->addComposition(composition);
    saveNameEdit->clear();
    updateModel();
    compositionView->setCurrentIndex(m_model->index(image->compositions().count()-1, 0));
    image->setModified();
}

void CompositionDockerDock::updateModel()
{
    m_model->setCompositions(m_canvas->viewManager()->image()->compositions());
}

void CompositionDockerDock::exportClicked()
{
	QString path;

    KoFileDialog dialog(0, KoFileDialog::OpenDirectory, "krita/compositiondockerdock");
    dialog.setCaption(i18n("Select a Directory"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    path = dialog.filename();


    if (path.isNull()) return;

    if (!path.endsWith('/')) {
		path.append('/');
	}

    KisImageWSP image = m_canvas->viewManager()->image();
    QString filename = m_canvas->viewManager()->document()->localFilePath();
    if (!filename.isEmpty()) {
        QFileInfo info(filename);
        path += info.baseName() + '_';
    }

    foreach(KisLayerComposition* composition, m_canvas->viewManager()->image()->compositions()) {
        if (!composition->isExportEnabled()) {
            continue;
        }

        composition->apply();
        image->refreshGraph();
        image->lock();
#if 0
        image->rootLayer()->projection()->convertToQImage(0, 0, 0, image->width(), image->height()).save(path + composition->name() + ".png");
#else
        QRect r = image->bounds();

        KisDocument *d = KisPart::instance()->createDocument();

        d->prepareForImport();

        KisImageWSP dst = new KisImage(d->createUndoStore(), r.width(), r.height(), image->colorSpace(), composition->name());
        dst->setResolution(image->xRes(), image->yRes());
        d->setCurrentImage(dst);
        KisPaintLayer* paintLayer = new KisPaintLayer(dst, "projection", OPACITY_OPAQUE_U8);
        KisPainter gc(paintLayer->paintDevice());
        gc.bitBlt(QPoint(0, 0), image->rootLayer()->projection(), r);
        dst->addNode(paintLayer, dst->rootLayer(), KisLayerSP(0));

        dst->refreshGraph();

        d->setOutputMimeType("image/png");
        d->setSaveInBatchMode(true);

        d->exportDocument(KUrl(path + composition->name() + ".png"));

        delete d;

#endif
        image->unlock();
    }
}

bool CompositionDockerDock::eventFilter(QObject* obj, QEvent* event)
{
     if (event->type() == QEvent::KeyPress ) {
         QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
         if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
             // new index will be set after the method is called
             QTimer::singleShot(0, this, SLOT(activateCurrentIndex()));
         }
         return false;
     } else {
         return QObject::eventFilter(obj, event);
     }
}

void CompositionDockerDock::activateCurrentIndex()
{
    QModelIndex index = compositionView->currentIndex();
    if (index.isValid()) {
        activated(index);
    }
}

void CompositionDockerDock::customContextMenuRequested(QPoint pos)
{
    QMenu menu;
    menu.addAction(updateAction);
    menu.addAction(renameAction);
    menu.exec(compositionView->mapToGlobal(pos));
}

void CompositionDockerDock::updateComposition()
{
    QModelIndex index = compositionView->currentIndex();
    if (index.isValid()) {
        KisLayerComposition* composition = m_model->compositionFromIndex(index);
        composition->store();
        m_canvas->image()->setModified();
    }
}

void CompositionDockerDock::renameComposition()
{
    dbgKrita << "rename";
    QModelIndex index = compositionView->currentIndex();
    if (index.isValid()) {
        KisLayerComposition* composition = m_model->compositionFromIndex(index);
        bool ok;
        QString name = QInputDialog::getText(this, i18n("Rename Composition"),
                                             i18n("New Name:"), QLineEdit::Normal,
                                             composition->name(), &ok);
        if (ok && !name.isEmpty()) {
            composition->setName(name);
            m_canvas->image()->setModified();
        }
    }
}



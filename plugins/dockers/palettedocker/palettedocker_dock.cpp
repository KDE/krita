/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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


#include "palettedocker_dock.h"

#include <QPainter>
#include <QGridLayout>
#include <QTableView>
#include <QHeaderView>
#include <QWheelEvent>
#include <QColorDialog>
#include <QCompleter>
#include <QComboBox>

#include <klocalizedstring.h>

#include <KoResourceServerProvider.h>
#include <KoColorSpaceRegistry.h>

#include <kis_icon.h>
#include <kis_layer.h>
#include <kis_node_manager.h>
#include <kis_config.h>
#include <kis_workspace_resource.h>
#include <kis_canvas_resource_provider.h>
#include <KisMainWindow.h>
#include <kis_canvas_resource_provider.h>
#include <KisViewManager.h>
#include <kis_display_color_converter.h>
#include <kis_canvas2.h>
#include <KoDialog.h>
#include <kis_color_button.h>
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <squeezedcombobox.h>

#include "KisPaletteModel.h"
#include "KisPaletteDelegate.h"
#include "kis_palette_view.h"
#include <KisPaletteListWidget.h>

#include "ui_wdgpalettedock.h"

PaletteDockerDock::PaletteDockerDock( )
    : QDockWidget(i18n("Palette"))
    , m_wdgPaletteDock(new Ui_WdgPaletteDock())
    , m_currentColorSet(0)
    , m_resourceProvider(0)
    , m_canvas(0)
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_wdgPaletteDock->setupUi(mainWidget);
    m_wdgPaletteDock->bnAdd->setIcon(KisIconUtils::loadIcon("list-add"));
    m_wdgPaletteDock->bnAdd->setIconSize(QSize(16, 16));
    m_wdgPaletteDock->bnRemove->setIcon(KisIconUtils::loadIcon("edit-delete"));
    m_wdgPaletteDock->bnRemove->setIconSize(QSize(16, 16));
    m_wdgPaletteDock->bnAdd->setEnabled(false);
    m_wdgPaletteDock->bnRemove->setEnabled(false);
    // m_wdgPaletteDock->bnAddGroup->setIcon(KisIconUtils::loadIcon("groupLayer"));

    m_model = new KisPaletteModel(this);
    m_wdgPaletteDock->paletteView->setPaletteModel(m_model);

    connect(m_wdgPaletteDock->bnAdd, SIGNAL(clicked(bool)), this, SLOT(slotAddColor()));
    connect(m_wdgPaletteDock->bnRemove, SIGNAL(clicked(bool)), this, SLOT(slotRemoveColor()));
    connect(m_wdgPaletteDock->bnRename, SIGNAL(clicked(bool)), this, SLOT(slotEditEntry()));

    connect(m_wdgPaletteDock->paletteView, SIGNAL(sigColorSelected(const KoColor &)),
            this, SLOT(slotSetForegroundColor(const KoColor &)));
    connect(m_wdgPaletteDock->paletteView, SIGNAL(entrySelectedBackGround(const KisSwatch &)),
            this, SLOT(entrySelectedBack(const KisSwatch &)));

    connect(m_wdgPaletteDock->paletteView, SIGNAL(sigSetEntry(QModelIndex)),
            this, SLOT(slotSetEntryByForeground(QModelIndex)));

    KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
    m_serverAdapter = QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoColorSet>(rServer));
    m_serverAdapter->connectToResourceServer();
    rServer->addObserver(this);

    m_paletteChooser = new KisPaletteListWidget(this);
    connect(m_paletteChooser, SIGNAL(sigPaletteSelected(KoColorSet*)), this, SLOT(slotSetColorSet(KoColorSet*)));

    m_wdgPaletteDock->bnColorSets->setIcon(KisIconUtils::loadIcon("hi16-palette_library"));
    m_wdgPaletteDock->bnColorSets->setToolTip(i18n("Choose palette"));
    m_wdgPaletteDock->bnColorSets->setPopupWidget(m_paletteChooser);

    connect(m_wdgPaletteDock->cmbNameList, SIGNAL(currentIndexChanged(int)), this, SLOT(setColorFromNameList(int)));
    KisConfig cfg;
    QString defaultPaletteName = cfg.defaultPalette();
    KoColorSet* defaultPalette = rServer->resourceByName(defaultPaletteName);
    if (defaultPalette) {
        slotSetColorSet(defaultPalette);
    }
}

PaletteDockerDock::~PaletteDockerDock()
{
    KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
    rServer->removeObserver(this);

    if (m_currentColorSet) {
        KisConfig cfg;
        cfg.setDefaultPalette(m_currentColorSet->name());
    }

    delete m_wdgPaletteDock->paletteView->itemDelegate();
    delete m_wdgPaletteDock;
}

void PaletteDockerDock::setMainWindow(KisViewManager* kisview)
{
    m_resourceProvider = kisview->resourceProvider();
    connect(m_resourceProvider, SIGNAL(sigSavingWorkspace(KisWorkspaceResource*)), SLOT(saveToWorkspace(KisWorkspaceResource*)));
    connect(m_resourceProvider, SIGNAL(sigLoadingWorkspace(KisWorkspaceResource*)), SLOT(loadFromWorkspace(KisWorkspaceResource*)));
    connect(m_resourceProvider, SIGNAL(sigFGColorChanged(KoColor)),m_wdgPaletteDock->paletteView, SLOT(trySelectClosestColor(KoColor)));

    kisview->nodeManager()->disconnect(m_model);


}

void PaletteDockerDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
    if (canvas) {
        KisCanvas2 *cv = qobject_cast<KisCanvas2*>(canvas);
        m_model->setDisplayRenderer(cv->displayColorConverter()->displayRendererInterface());
    }
    m_canvas = static_cast<KisCanvas2*>(canvas);
}


void PaletteDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_model->setDisplayRenderer(0);
    m_canvas = 0;
}

void PaletteDockerDock::unsetResourceServer()
{
    KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
    rServer->removeObserver(this);
}

void PaletteDockerDock::removingResource(KoColorSet *resource)
{
    if (resource == m_currentColorSet) {
        slotSetColorSet(0);
    }
}

void PaletteDockerDock::resourceChanged(KoColorSet *resource)
{
    slotSetColorSet(resource);
}


void PaletteDockerDock::slotSetColorSet(KoColorSet* colorSet)
{
    m_currentColorSet = colorSet;
    m_model->setColorSet(colorSet);
    m_wdgPaletteDock->bnColorSets->setText(colorSet->name());
    resetNameList(colorSet);
}

void PaletteDockerDock::resetNameList(const KoColorSet *colorSet)
{
    m_wdgPaletteDock->cmbNameList->clear();

    if (!colorSet || !colorSet->colorCount()>0) {
        return;
    }

    for (quint32 x = 0; x < colorSet->columnCount(); x++) {
        for (quint32 y = 0; y < colorSet->rowCount(); y++) {
            KoColorSetEntry entry = colorSet->getColorGlobal(x, y);
            QPixmap colorSquare = QPixmap(32, 32);
            if (entry.spotColor()) {
                QImage img = QImage(32, 32, QImage::Format_ARGB32);
                QPainter circlePainter;
                img.fill(Qt::transparent);
                circlePainter.begin(&img);
                QBrush brush = QBrush(Qt::SolidPattern);
                brush.setColor(entry.color().toQColor());
                circlePainter.setBrush(brush);
                QPen pen = circlePainter.pen();
                pen.setColor(Qt::transparent);
                pen.setWidth(0);
                circlePainter.setPen(pen);
                circlePainter.drawEllipse(0, 0, 32, 32);
                circlePainter.end();
                colorSquare = QPixmap::fromImage(img);
            } else {
                colorSquare.fill(entry.color().toQColor());
            }
            QString name = entry.name();
            if (!entry.id().isEmpty()){
                name = entry.id() + " - " + entry.name();
            }
            m_wdgPaletteDock->cmbNameList->addSqueezedItem(QIcon(colorSquare), name);
        }
    }

    QCompleter *completer = new QCompleter(m_wdgPaletteDock->cmbNameList->model());
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    m_wdgPaletteDock->cmbNameList->setCompleter(completer);
    if (colorSet && colorSet->removable()) {
        m_wdgPaletteDock->bnAdd->setEnabled(true);
        m_wdgPaletteDock->bnRemove->setEnabled(true);
    } else {
        m_wdgPaletteDock->bnAdd->setEnabled(false);
        m_wdgPaletteDock->bnRemove->setEnabled(false);
    }
}

void PaletteDockerDock::setColorFromNameList(int index)
{
    if (m_model && m_currentColorSet) {
        entrySelected(m_currentColorSet->getColorGlobal(index));
        m_wdgPaletteDock->paletteView->blockSignals(true);
        m_wdgPaletteDock->cmbNameList->blockSignals(true);
        m_wdgPaletteDock->cmbNameList->setCurrentIndex(index);
        m_wdgPaletteDock->paletteView->selectionModel()->clearSelection();
        m_wdgPaletteDock->paletteView->selectionModel()->setCurrentIndex(m_model->indexFromId(index), QItemSelectionModel::Select);
        m_wdgPaletteDock->paletteView->blockSignals(false);
        m_wdgPaletteDock->cmbNameList->blockSignals(false);
    }
}

void PaletteDockerDock::slotAddColor()
{
    if (m_resourceProvider) {
        m_wdgPaletteDock->paletteView->addEntryWithDialog(m_resourceProvider->fgColor());
    }
}

void PaletteDockerDock::slotRemoveColor()
{
    QModelIndex index = m_wdgPaletteDock->paletteView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_wdgPaletteDock->paletteView->removeEntryWithDialog(index);
}

void PaletteDockerDock::slotSetForegroundColor(const KoColor &color)
{
    if (m_resourceProvider) {
        m_resourceProvider->setFGColor(color);
    }
    if (m_currentColorSet->removable()) {
        m_wdgPaletteDock->bnRemove->setEnabled(true);
    }
}

void PaletteDockerDock::entrySelectedBack(KoColorSetEntry entry)
{
    if (m_wdgPaletteDock->paletteView->currentIndex().isValid()) {
        quint32 index = m_model->idFromIndex(m_wdgPaletteDock->paletteView->currentIndex());
        m_wdgPaletteDock->cmbNameList->setCurrentIndex(index);
    }
    if (m_resourceProvider) {
        m_resourceProvider->setBGColor(entry.color());
    }
    if (m_currentColorSet->removable()) {
        m_wdgPaletteDock->bnRemove->setEnabled(true);
    }
}

void PaletteDockerDock::saveToWorkspace(KisWorkspaceResource* workspace)
{
    if (m_currentColorSet) {
        workspace->setProperty("palette", m_currentColorSet->name());
    }
}

void PaletteDockerDock::loadFromWorkspace(KisWorkspaceResource* workspace)
{
    if (workspace->hasProperty("palette")) {
        KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
        KoColorSet* colorSet = rServer->resourceByName(workspace->getString("palette"));
        if (colorSet) {
            slotSetColorSet(colorSet);
        }
    }
}

void PaletteDockerDock::slotSetEntryByForeground(const QModelIndex &index)
{
    m_model->setEntry(KisSwatch(m_resourceProvider->fgColor()), index);
}

void PaletteDockerDock::slotEditEntry()
{
    QModelIndex index = m_wdgPaletteDock->paletteView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_wdgPaletteDock->paletteView->modifyEntry(index);
}

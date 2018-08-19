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
#include "ui_wdgpalettedock.h"
#include "kis_palette_delegate.h"
#include "kis_palette_view.h"
#include <KisColorsetChooser.h>

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
    m_wdgPaletteDock->bnAddDialog->setVisible(false);
    m_wdgPaletteDock->bnAddGroup->setIcon(KisIconUtils::loadIcon("groupLayer"));
    m_wdgPaletteDock->bnAddGroup->setIconSize(QSize(16, 16));


    m_model = new KisPaletteModel(this);
    m_wdgPaletteDock->paletteView->setPaletteModel(m_model);

    connect(m_wdgPaletteDock->bnAdd, SIGNAL(clicked(bool)), this, SLOT(addColorForeground()));
    connect(m_wdgPaletteDock->bnRemove, SIGNAL(clicked(bool)), this, SLOT(removeColor()));
    connect(m_wdgPaletteDock->bnAddGroup, SIGNAL(clicked(bool)), m_wdgPaletteDock->paletteView, SLOT(addGroupWithDialog()));

    connect(m_wdgPaletteDock->paletteView, SIGNAL(entrySelected(KoColorSetEntry)), this, SLOT(entrySelected(KoColorSetEntry)));
    connect(m_wdgPaletteDock->paletteView, SIGNAL(entrySelectedBackGround(KoColorSetEntry)), this, SLOT(entrySelectedBack(KoColorSetEntry)));

    KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
    m_serverAdapter = QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoColorSet>(rServer));
    m_serverAdapter->connectToResourceServer();
    rServer->addObserver(this);

    m_paletteChooser = new KisColorsetChooser(this);
    connect(m_paletteChooser, SIGNAL(paletteSelected(KoColorSet*)), this, SLOT(setColorSet(KoColorSet*)));

    m_wdgPaletteDock->bnColorSets->setIcon(KisIconUtils::loadIcon("hi16-palette_library"));
    m_wdgPaletteDock->bnColorSets->setToolTip(i18n("Choose palette"));
    m_wdgPaletteDock->bnColorSets->setPopupWidget(m_paletteChooser);

    connect(m_wdgPaletteDock->cmbNameList, SIGNAL(currentIndexChanged(int)), this, SLOT(setColorFromNameList(int)));
    KisConfig cfg(true);
    QString defaultPalette = cfg.defaultPalette();
    KoColorSet* defaultColorSet = rServer->resourceByName(defaultPalette);
    if (defaultColorSet) {
        setColorSet(defaultColorSet);
    }
}

PaletteDockerDock::~PaletteDockerDock()
{
    KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
    rServer->removeObserver(this);

    if (m_currentColorSet) {
        KisConfig cfg(true);
        cfg.setDefaultPalette(m_currentColorSet->name());
    }

    delete m_wdgPaletteDock->paletteView->itemDelegate();
    delete m_wdgPaletteDock;
}

void PaletteDockerDock::setViewManager(KisViewManager* kisview)
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
        setColorSet(0);
    }
}

void PaletteDockerDock::resourceChanged(KoColorSet *resource)
{
    setColorSet(resource);
}


void PaletteDockerDock::setColorSet(KoColorSet* colorSet)
{
    m_model->setColorSet(colorSet);
    m_wdgPaletteDock->paletteView->updateView();
    m_wdgPaletteDock->paletteView->updateRows();
    m_wdgPaletteDock->cmbNameList->clear();



    if (colorSet && colorSet->nColors()>0) {
        for (quint32 i = 0; i< colorSet->nColors(); i++) {
            KoColorSetEntry entry = colorSet->getColorGlobal(i);
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
    m_currentColorSet = colorSet;
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

void PaletteDockerDock::addColorForeground()
{
    if (m_resourceProvider) {
        //setup dialog
        m_wdgPaletteDock->paletteView->addEntryWithDialog(m_resourceProvider->fgColor());
    }
}

void PaletteDockerDock::removeColor()
{
    QModelIndex index = m_wdgPaletteDock->paletteView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_wdgPaletteDock->paletteView->removeEntryWithDialog(index);
}

void PaletteDockerDock::entrySelected(KoColorSetEntry entry)
{
    if (m_wdgPaletteDock->paletteView->currentIndex().isValid()) {
        quint32 index = m_model->idFromIndex(m_wdgPaletteDock->paletteView->currentIndex());
        m_wdgPaletteDock->cmbNameList->setCurrentIndex(index);
    }
    if (m_resourceProvider) {
        m_resourceProvider->setFGColor(entry.color());
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
            setColorSet(colorSet);
        }
    }
}



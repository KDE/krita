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
#include <QAction>
#include <QMenu>
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>

#include <klocalizedstring.h>
#include <KoResourceServerProvider.h>
#include <KoColorSpaceRegistry.h>
#include <KoFileDialog.h>
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
#include <squeezedcombobox.h>

#include "KisPaletteModel.h"
#include "KisPaletteDelegate.h"
#include "kis_palette_view.h"
#include <KisPaletteListWidget.h>

#include "ui_wdgpalettedock.h"

PaletteDockerDock::PaletteDockerDock( )
    : QDockWidget(i18n("Palette"))
    , m_ui(new Ui_WdgPaletteDock())
    , m_currentColorSet(0)
    , m_view(0)
    , m_resourceProvider(0)
    , m_canvas(0)
    , m_saver(new PaletteListSaver(this))
    , m_actAdd(new QAction(KisIconUtils::loadIcon("list-add"), i18n("Add foreground color")))
    , m_actAddWithDlg(new QAction(KisIconUtils::loadIcon("list-add"), i18n("Choose a color to add")))
    , m_actSwitch(new QAction(i18n("Switch with another spot")))
    , m_actRemove(new QAction(KisIconUtils::loadIcon("edit-delete"), i18n("Delete color")))
    , m_actModify(new QAction(KisIconUtils::loadIcon("edit-rename"), i18n("Modify this spot")))
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    m_ui->setupUi(mainWidget);
    m_ui->bnAdd->setDefaultAction(m_actAdd.data());
    m_ui->bnRemove->setDefaultAction(m_actRemove.data());
    m_ui->bnRename->setDefaultAction(m_actModify.data());

    // to make sure their icons have the same size
    m_ui->bnRemove->setIconSize(QSize(16, 16));
    m_ui->bnRename->setIconSize(QSize(16, 16));
    m_ui->bnAdd->setIconSize(QSize(16, 16));

    m_ui->bnAdd->setEnabled(false);
    m_ui->bnRename->setEnabled(false);
    m_ui->bnRemove->setEnabled(false);
    // m_wdgPaletteDock->bnAddGroup->setIcon(KisIconUtils::loadIcon("groupLayer"));

    m_model = new KisPaletteModel(this);
    m_ui->paletteView->setPaletteModel(m_model);
    m_ui->paletteView->setAllowModification(true);
    m_ui->cmbNameList->setPaletteModel(m_model);

    connect(m_actAdd.data(), SIGNAL(triggered()), SLOT(slotAddColor()));
    connect(m_actRemove.data(), SIGNAL(triggered()), SLOT(slotRemoveColor()));
    connect(m_actModify.data(), SIGNAL(triggered()), SLOT(slotEditEntry()));
    connect(m_ui->paletteView, SIGNAL(sigIndexSelected(QModelIndex)),
            SLOT(slotPaletteIndexSelected(QModelIndex)));
    connect(m_ui->paletteView, SIGNAL(sigIndexSelected(QModelIndex)),
            m_ui->cmbNameList, SLOT(slotSwatchSelected(QModelIndex)));
    connect(m_ui->cmbNameList, SIGNAL(sigColorSelected(KoColor)),
            SLOT(slotNameListSelection(KoColor)));

    m_viewContextMenu.addAction(m_actAddWithDlg.data());
    m_viewContextMenu.addAction(m_actRemove.data());
    m_viewContextMenu.addAction(m_actModify.data());
    m_viewContextMenu.addAction(m_actSwitch.data());

    m_paletteChooser = new KisPaletteListWidget(this);
    connect(m_paletteChooser, SIGNAL(sigPaletteSelected(KoColorSet*)), SLOT(slotSetColorSet(KoColorSet*)));

    m_ui->bnColorSets->setIcon(KisIconUtils::loadIcon("hi16-palette_library"));
    m_ui->bnColorSets->setToolTip(i18n("Choose palette"));
    m_ui->bnColorSets->setPopupWidget(m_paletteChooser);

    KisConfig cfg(true);
    QString defaultPaletteName = cfg.defaultPalette();
    KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
    KoColorSet* defaultPalette = rServer->resourceByName(defaultPaletteName);
    if (defaultPalette) {
        slotSetColorSet(defaultPalette);
    }

    connect(m_paletteChooser, SIGNAL(sigPaletteListChanged()), m_saver.data(), SLOT(slotSetPaletteList()));
}

PaletteDockerDock::~PaletteDockerDock()
{
    if (m_currentColorSet) {
        KisConfig cfg(true);
        cfg.setDefaultPalette(m_currentColorSet->name());
    }

    delete m_ui;
}

void PaletteDockerDock::setViewManager(KisViewManager* kisview)
{
    m_view = kisview;
    m_resourceProvider = kisview->resourceProvider();
    connect(m_resourceProvider, SIGNAL(sigSavingWorkspace(KisWorkspaceResource*)), SLOT(saveToWorkspace(KisWorkspaceResource*)));
    connect(m_resourceProvider, SIGNAL(sigLoadingWorkspace(KisWorkspaceResource*)), SLOT(loadFromWorkspace(KisWorkspaceResource*)));
    connect(m_resourceProvider, SIGNAL(sigFGColorChanged(KoColor)),
            m_ui->paletteView, SLOT(slotChosenColorChanged(KoColor)));

    kisview->nodeManager()->disconnect(m_model);
}

void PaletteDockerDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
    if (canvas) {
        KisCanvas2 *cv = qobject_cast<KisCanvas2*>(canvas);
        m_ui->paletteView->setDisplayRenderer(cv->displayColorConverter()->displayRendererInterface());
    }
    m_canvas = static_cast<KisCanvas2*>(canvas);
}

void PaletteDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_ui->paletteView->setDisplayRenderer(Q_NULLPTR);
    m_canvas = Q_NULLPTR;
}

void PaletteDockerDock::slotSetColorSet(KoColorSet* colorSet)
{
    if (colorSet->isEditable()) {
        m_ui->bnAdd->setEnabled(true);
        m_ui->bnRename->setEnabled(true);
        m_ui->bnRemove->setEnabled(true);
    } else {
        m_ui->bnAdd->setEnabled(false);
        m_ui->bnRename->setEnabled(false);
        m_ui->bnRemove->setEnabled(false);
    }

    m_currentColorSet = colorSet;
    m_model->setColorSet(colorSet);
    m_ui->bnColorSets->setText(colorSet->name());
}

void PaletteDockerDock::slotAddColor()
{
    if (m_currentColorSet->isEditable()) {
        if (m_resourceProvider) {
            m_ui->paletteView->addEntryWithDialog(m_resourceProvider->fgColor());
        }
    }
}

void PaletteDockerDock::slotRemoveColor()
{
    if (m_currentColorSet->isEditable()) {
        QModelIndex index = m_ui->paletteView->currentIndex();
        if (!index.isValid()) {
            return;
        }
        m_ui->paletteView->removeEntryWithDialog(index);
    }
}

void PaletteDockerDock::slotSetFGColorByPalette(const KisSwatch &entry)
{
    if (m_resourceProvider) {
        m_resourceProvider->setFGColor(entry.color());
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

void PaletteDockerDock::slotPaletteIndexSelected(const QModelIndex &index)
{
    bool slotEmpty = !(qvariant_cast<bool>(index.data(KisPaletteModel::CheckSlotRole)));
    if (slotEmpty) {
        if (!m_currentColorSet->isEditable()) { return; }
        slotSetEntryByForeground(index);
    } else {
        KisSwatch entry = m_model->getEntry(index);
        slotSetFGColorByPalette(entry);
    }
}

void PaletteDockerDock::slotSetEntryByForeground(const QModelIndex &index)
{
    m_model->setEntry(KisSwatch(m_resourceProvider->fgColor()), index);
    if (m_currentColorSet->isEditable()) {
        m_ui->bnRemove->setEnabled(true);
        emit sigPaletteSelected(m_currentColorSet);
    }
}

void PaletteDockerDock::slotEditEntry()
{
    if (m_currentColorSet->isEditable()) {
        QModelIndex index = m_ui->paletteView->currentIndex();
        if (!index.isValid()) {
            return;
        }
        m_ui->paletteView->modifyEntry(index);
        emit sigPaletteSelected(m_currentColorSet);
    }
}

void PaletteDockerDock::slotImportPalette()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "OpenColorSet");
    dialog.setDefaultDir(m_currentColorSet->filename());
    dialog.setMimeTypeFilters(QStringList() << "application/x-gimp-color-palette");
    QString fileName = dialog.filename();
    KoColorSet *colorSet = new KoColorSet(fileName);
    colorSet->load();
}

void PaletteDockerDock::slotNameListSelection(const KoColor &color)
{
    m_resourceProvider->setFGColor(color);
}

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

#include <KisSqueezedComboBox.h>
#include <klocalizedstring.h>
#include <KoResourceServerProvider.h>
#include <KoColorSpaceRegistry.h>
#include <KoFileDialog.h>
#include <kis_icon.h>
#include <kis_config.h>
#include <kis_node_manager.h>
#include <kis_workspace_resource.h>
#include <kis_canvas_resource_provider.h>
#include <KisMainWindow.h>
#include <KisViewManager.h>
#include <kis_display_color_converter.h>
#include <kis_canvas2.h>
#include <KoDialog.h>
#include <kis_color_button.h>
#include <KisDocument.h>
#include <KisPart.h>

#include <KisPaletteModel.h>
#include <KisPaletteDelegate.h>
#include <kis_palette_view.h>
#include <KisPaletteListWidget.h>

#include <KisPaletteEditor.h>
#include <dialogs/KisDlgPaletteEditor.h>

#include "ui_wdgpalettedock.h"

PaletteDockerDock::PaletteDockerDock( )
    : QDockWidget(i18n("Palette"))
    , m_ui(new Ui_WdgPaletteDock())
    , m_model(new KisPaletteModel(this))
    , m_paletteChooser(new KisPaletteListWidget(this))
    , m_view(Q_NULLPTR)
    , m_resourceProvider(Q_NULLPTR)
    , m_rServer(KoResourceServerProvider::instance()->paletteServer())
    , m_activeDocument(Q_NULLPTR)
    , m_paletteEditor(new KisPaletteEditor)
    , m_actAdd(new QAction(KisIconUtils::loadIcon("list-add"), i18n("Add a color")))
    , m_actRemove(new QAction(KisIconUtils::loadIcon("edit-delete"), i18n("Delete color")))
    , m_actModify(new QAction(KisIconUtils::loadIcon("edit-rename"), i18n("Modify this spot")))
    , m_actEditPalette(new QAction(KisIconUtils::loadIcon("groupLayer"), i18n("Edit this palette")))
    , m_colorSelfUpdate(false)
{
    QWidget *mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_ui->setupUi(mainWidget);

    m_ui->bnAdd->setDefaultAction(m_actAdd.data());
    m_ui->bnRemove->setDefaultAction(m_actRemove.data());
    m_ui->bnRename->setDefaultAction(m_actModify.data());
    m_ui->bnEditPalette->setDefaultAction(m_actEditPalette.data());

    // to make sure their icons have the same size
    m_ui->bnRemove->setIconSize(QSize(16, 16));
    m_ui->bnRename->setIconSize(QSize(16, 16));
    m_ui->bnAdd->setIconSize(QSize(16, 16));
    m_ui->bnEditPalette->setIconSize(QSize(16, 16));

    m_ui->paletteView->setPaletteModel(m_model);
    m_ui->paletteView->setAllowModification(true);
    m_ui->cmbNameList->setCompanionView(m_ui->paletteView);

    m_paletteEditor->setPaletteModel(m_model);

    connect(m_actAdd.data(), SIGNAL(triggered()), SLOT(slotAddColor()));
    connect(m_actRemove.data(), SIGNAL(triggered()), SLOT(slotRemoveColor()));
    connect(m_actModify.data(), SIGNAL(triggered()), SLOT(slotEditEntry()));
    connect(m_actEditPalette.data(), SIGNAL(triggered()), SLOT(slotEditPalette()));
    connect(m_ui->paletteView, SIGNAL(sigIndexSelected(QModelIndex)),
            SLOT(slotPaletteIndexSelected(QModelIndex)));
    connect(m_ui->paletteView, SIGNAL(clicked(QModelIndex)),
            SLOT(slotPaletteIndexClicked(QModelIndex)));
    connect(m_ui->paletteView, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(slotPaletteIndexDoubleClicked(QModelIndex)));

    m_viewContextMenu.addAction(m_actModify.data());
    m_viewContextMenu.addAction(m_actRemove.data());
    connect(m_ui->paletteView, SIGNAL(pressed(QModelIndex)), SLOT(slotContextMenu(QModelIndex)));

    m_paletteChooser->setAllowModification(true);
    connect(m_paletteChooser, SIGNAL(sigPaletteSelected(KoColorSet*)), SLOT(slotSetColorSet(KoColorSet*)));
    connect(m_paletteChooser, SIGNAL(sigAddPalette()), SLOT(slotAddPalette()));
    connect(m_paletteChooser, SIGNAL(sigImportPalette()), SLOT(slotImportPalette()));
    connect(m_paletteChooser, SIGNAL(sigRemovePalette(KoColorSet*)), SLOT(slotRemovePalette(KoColorSet*)));
    connect(m_paletteChooser, SIGNAL(sigExportPalette(KoColorSet*)), SLOT(slotExportPalette(KoColorSet*)));

    m_ui->bnColorSets->setIcon(KisIconUtils::loadIcon("hi16-palette_library"));
    m_ui->bnColorSets->setToolTip(i18n("Choose palette"));
    m_ui->bnColorSets->setPopupWidget(m_paletteChooser);

    KisConfig cfg(true);
    QString defaultPaletteName = cfg.defaultPalette();
    KoColorSet* defaultPalette = m_rServer->resourceByName(defaultPaletteName);
    if (defaultPalette) {
        slotSetColorSet(defaultPalette);
    } else {
        m_ui->bnAdd->setEnabled(false);
        m_ui->bnRename->setEnabled(false);
        m_ui->bnRemove->setEnabled(false);
        m_ui->bnEditPalette->setEnabled(false);
        m_ui->paletteView->setAllowModification(false);
    }
}

PaletteDockerDock::~PaletteDockerDock()
{ }

void PaletteDockerDock::setViewManager(KisViewManager* kisview)
{
    m_view = kisview;
    m_resourceProvider = kisview->canvasResourceProvider();
    connect(m_resourceProvider, SIGNAL(sigSavingWorkspace(KisWorkspaceResource*)),
            SLOT(saveToWorkspace(KisWorkspaceResource*)));
    connect(m_resourceProvider, SIGNAL(sigLoadingWorkspace(KisWorkspaceResource*)),
            SLOT(loadFromWorkspace(KisWorkspaceResource*)));
    connect(m_resourceProvider, SIGNAL(sigFGColorChanged(KoColor)),
            this, SLOT(slotFGColorResourceChanged(KoColor)));

    kisview->nodeManager()->disconnect(m_model);
}

void PaletteDockerDock::slotContextMenu(const QModelIndex &)
{
    if (QApplication::mouseButtons() == Qt::RightButton) {
        m_viewContextMenu.exec(QCursor::pos());
    }
}

void PaletteDockerDock::slotAddPalette()
{
    m_paletteEditor->addPalette();
}

void PaletteDockerDock::slotRemovePalette(KoColorSet *cs)
{
    m_paletteEditor->removePalette(cs);
}

void PaletteDockerDock::slotImportPalette()
{
    m_paletteEditor->importPalette();
}

void PaletteDockerDock::slotExportPalette(KoColorSet *palette)
{
    KoFileDialog dialog(this, KoFileDialog::SaveFile, "Save Palette");
    dialog.setDefaultDir(palette->filename());
    dialog.setMimeTypeFilters(QStringList() << "krita/x-colorset");
    QString newPath;
    bool isStandAlone = palette->isGlobal();
    QString oriPath = palette->filename();
    if ((newPath = dialog.filename()).isEmpty()) { return; }
    palette->setFilename(newPath);
    palette->setIsGlobal(true);
    palette->save();
    palette->setFilename(oriPath);
    palette->setIsGlobal(isStandAlone);
}

void PaletteDockerDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != Q_NULLPTR);
    if (canvas) {
        KisCanvas2 *cv = qobject_cast<KisCanvas2*>(canvas);
        m_ui->paletteView->setDisplayRenderer(cv->displayColorConverter()->displayRendererInterface());
    }

    if (m_activeDocument) {
        for (KoColorSet * &cs : m_activeDocument->paletteList()) {
            KoColorSet *tmpAddr = cs;
            cs = new KoColorSet(*cs);
            m_rServer->removeResourceFromServer(tmpAddr);
        }
    }

    if (m_view && m_view->document()) {
        m_activeDocument = m_view->document();
        m_paletteEditor->setView(m_view);

        for (KoColorSet *cs : m_activeDocument->paletteList()) {
            m_rServer->addResource(cs);
        }
    }

    if (!m_currentColorSet) {
        slotSetColorSet(Q_NULLPTR);
    }
}

void PaletteDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_ui->paletteView->setDisplayRenderer(Q_NULLPTR);
    m_paletteEditor->setView(Q_NULLPTR);

    for (KoResource *r : m_rServer->resources()) {
        KoColorSet *c = static_cast<KoColorSet*>(r);
        if (!c->isGlobal()) {
            m_rServer->removeResourceFromServer(c);
        }
    }
    if (!m_currentColorSet) {
        slotSetColorSet(Q_NULLPTR);
    }
}

void PaletteDockerDock::slotSetColorSet(KoColorSet* colorSet)
{
    if (colorSet && colorSet->isEditable()) {
        m_ui->bnAdd->setEnabled(true);
        m_ui->bnRename->setEnabled(true);
        m_ui->bnRemove->setEnabled(true);
        m_ui->bnEditPalette->setEnabled(true);
        m_ui->paletteView->setAllowModification(true);
    } else {
        m_ui->bnAdd->setEnabled(false);
        m_ui->bnRename->setEnabled(false);
        m_ui->bnRemove->setEnabled(false);
        m_ui->bnEditPalette->setEnabled(false);
        m_ui->paletteView->setAllowModification(false);
    }

    m_currentColorSet = colorSet;
    m_model->setPalette(colorSet);
    if (colorSet) {
        KisConfig cfg(true);
        cfg.setDefaultPalette(colorSet->name());
        m_ui->lblPaletteName->setTextElideMode(Qt::ElideLeft);
        m_ui->lblPaletteName->setText(colorSet->name());
    } else {
        m_ui->lblPaletteName->setText("");
    }
}

void PaletteDockerDock::slotEditPalette()
{
    KisDlgPaletteEditor dlg;
    if (!m_currentColorSet) { return; }
    dlg.setPaletteModel(m_model);
    dlg.setView(m_view);
    if (dlg.exec() != QDialog::Accepted){ return; }

    slotSetColorSet(m_currentColorSet); // update GUI
}


void PaletteDockerDock::slotAddColor()
{
    if (m_resourceProvider) {
        m_paletteEditor->addEntry(m_resourceProvider->fgColor());
    }
}

void PaletteDockerDock::slotRemoveColor()
{
    QModelIndex index = m_ui->paletteView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_paletteEditor->removeEntry(index);
    m_ui->bnRemove->setEnabled(false);
}

void PaletteDockerDock::setFGColorByPalette(const KisSwatch &entry)
{
    if (m_resourceProvider) {
        m_colorSelfUpdate = true;
        m_resourceProvider->setFGColor(entry.color());
        m_colorSelfUpdate = false;
    }
}

void PaletteDockerDock::saveToWorkspace(KisWorkspaceResource* workspace)
{
    if (!m_currentColorSet.isNull()) {
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

void PaletteDockerDock::slotFGColorResourceChanged(const KoColor &color)
{
    if (!m_colorSelfUpdate) {
        m_ui->paletteView->slotFGColorResourceChanged(color);
    }
}

void PaletteDockerDock::slotPaletteIndexSelected(const QModelIndex &index)
{
    bool occupied = qvariant_cast<bool>(index.data(KisPaletteModel::CheckSlotRole));
    if (occupied) {
        if (!qvariant_cast<bool>(index.data(KisPaletteModel::IsGroupNameRole))) {
            m_ui->bnRemove->setEnabled(true);
            KisSwatch entry = m_model->getEntry(index);
            setFGColorByPalette(entry);
        }
    }
    if (!m_currentColorSet->isEditable()) { return; }
    m_ui->bnRemove->setEnabled(occupied);
}

void PaletteDockerDock::slotPaletteIndexClicked(const QModelIndex &index)
{
    if (!(qvariant_cast<bool>(index.data(KisPaletteModel::CheckSlotRole)))) {
        setEntryByForeground(index);
    }
}

void PaletteDockerDock::slotPaletteIndexDoubleClicked(const QModelIndex &index)
{
    m_paletteEditor->modifyEntry(index);
}

void PaletteDockerDock::setEntryByForeground(const QModelIndex &index)
{
    m_paletteEditor->setEntry(m_resourceProvider->fgColor(), index);
    if (m_currentColorSet->isEditable()) {
        m_ui->bnRemove->setEnabled(true);
    }
}

void PaletteDockerDock::slotEditEntry()
{
    QModelIndex index = m_ui->paletteView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_paletteEditor->modifyEntry(index);
}

void PaletteDockerDock::slotNameListSelection(const KoColor &color)
{
    m_colorSelfUpdate = true;
    m_resourceProvider->setFGColor(color);
    m_colorSelfUpdate = false;
}

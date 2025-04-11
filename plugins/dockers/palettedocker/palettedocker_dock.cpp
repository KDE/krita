/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "palettedocker_dock.h"

#include <QPainter>
#include <QGridLayout>
#include <QTableView>
#include <QHeaderView>
#include <QWheelEvent>
#include <QCompleter>
#include <QComboBox>
#include <QAction>
#include <QMenu>
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QString>

#include <kundo2stack.h>
#include <KisSqueezedComboBox.h>
#include <klocalizedstring.h>
#include <KoResourceServerProvider.h>
#include <KisResourceLocator.h>
#include <KoColorSpaceRegistry.h>
#include <KoFileDialog.h>
#include <kis_floating_message.h>
#include <kis_icon.h>
#include <kis_config.h>
#include <kis_node_manager.h>
#include <kis_canvas_resource_provider.h>
#include <KisMainWindow.h>
#include <KisViewManager.h>
#include <kis_display_color_converter.h>
#include <kis_canvas2.h>
#include <KoDialog.h>
#include <kis_color_button.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <KisPaletteEditor.h>

#include <KisStorageModel.h>

#include <KisPaletteModel.h>
#include <KisPaletteDelegate.h>
#include <kis_palette_view.h>
#include <KisPaletteChooser.h>

#include <KisPaletteEditor.h>
#include <dialogs/KisDlgPaletteEditor.h>

#include "ui_wdgpalettedock.h"

PaletteDockerDock::PaletteDockerDock( )
    : QDockWidget(i18n("Palette"))
    , m_ui(new Ui_WdgPaletteDock())
    , m_model(new KisPaletteModel(this))
    , m_paletteChooser(new KisPaletteChooser(this))
    , m_view(0)
    , m_resourceProvider(0)
    , m_rServer(KoResourceServerProvider::instance()->paletteServer())
    , m_activeDocument(0)
    , m_paletteEditor(new KisPaletteEditor)
    , m_actAdd(new QAction(KisIconUtils::loadIcon("list-add"), i18n("Add a new color swatch")))
    , m_actRemove(new QAction(KisIconUtils::loadIcon("edit-delete"), i18n("Remove swatch or group")))
    , m_actModify(new QAction(KisIconUtils::loadIcon("document-edit"), i18n("Edit swatch or group")))
    , m_actEditPalette(new QAction(KisIconUtils::loadIcon("palette-edit"), i18n("Edit current palette")))
    , m_actSavePalette(new QAction(KisIconUtils::loadIcon("document-save-16"), i18n("Save current palette")))
    , m_colorSelfUpdate(false)
{
    QWidget *mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_ui->setupUi(mainWidget);

    connect(KisResourceLocator::instance(), SIGNAL(storageRemoved(QString)), this, SLOT(slotStoragesChanged(QString)));

    m_ui->bnAdd->setDefaultAction(m_actAdd.data());
    m_ui->bnRemove->setDefaultAction(m_actRemove.data());
    m_ui->bnRename->setDefaultAction(m_actModify.data());
    m_ui->bnEditPalette->setDefaultAction(m_actEditPalette.data());
    m_ui->bnSavePalette->setDefaultAction(m_actSavePalette.data());

    // to make sure their icons have the same size
    m_ui->bnRemove->setIconSize(QSize(16, 16));
    m_ui->bnRename->setIconSize(QSize(16, 16));
    m_ui->bnAdd->setIconSize(QSize(16, 16));
    m_ui->bnEditPalette->setIconSize(QSize(16, 16));
    m_ui->bnSavePalette->setIconSize(QSize(16, 16));


    m_ui->paletteView->setPaletteModel(m_model);
    m_ui->paletteView->setAllowModification(true);
    m_ui->cmbNameList->setCompanionView(m_ui->paletteView);

    m_paletteEditor->setPaletteModel(m_model);

    connect(m_actAdd.data(), SIGNAL(triggered()), SLOT(slotAddColor()));
    connect(m_actRemove.data(), SIGNAL(triggered()), SLOT(slotRemoveColor()));
    connect(m_actModify.data(), SIGNAL(triggered()), SLOT(slotEditEntry()));
    connect(m_actEditPalette.data(), SIGNAL(triggered()), SLOT(slotEditPalette()));
    connect(m_actSavePalette.data(), SIGNAL(triggered()), SLOT(slotSavePalette()));
    connect(m_ui->paletteView, SIGNAL(sigIndexSelected(QModelIndex)), SLOT(slotPaletteIndexSelected(QModelIndex)));
    connect(m_ui->paletteView, SIGNAL(clicked(QModelIndex)), SLOT(slotPaletteIndexClicked(QModelIndex)));
    connect(m_ui->paletteView, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(slotPaletteIndexDoubleClicked(QModelIndex)));
    connect(m_model, SIGNAL(sigPaletteModified()), SLOT(slotUpdateLblPaletteName()));
    connect(m_ui->cmbNameList, SIGNAL(sigColorSelected(const KoColor&)), SLOT(slotNameListSelection(const KoColor&)));
    connect(m_ui->bnLock, SIGNAL(toggled(bool)), SLOT(slotLockPalette(bool)));

    m_viewContextMenu.addAction(m_actModify.data());
    m_viewContextMenu.addAction(m_actRemove.data());
    connect(m_ui->paletteView, SIGNAL(pressed(QModelIndex)), SLOT(slotContextMenu(QModelIndex)));

    connect(m_paletteChooser, SIGNAL(sigPaletteSelected(KoColorSetSP)), SLOT(slotSetColorSet(KoColorSetSP)));
    connect(m_paletteChooser, SIGNAL(sigAddPalette()), SLOT(slotAddPalette()));
    connect(m_paletteChooser, SIGNAL(sigImportPalette()), SLOT(slotImportPalette()));
    connect(m_paletteChooser, SIGNAL(sigRemovePalette(KoColorSetSP)), SLOT(slotRemovePalette(KoColorSetSP)));
    connect(m_paletteChooser, SIGNAL(sigExportPalette(KoColorSetSP)), SLOT(slotExportPalette(KoColorSetSP)));

    m_ui->bnColorSets->setIcon(KisIconUtils::loadIcon("palette-library"));
    m_ui->bnColorSets->setToolTip(i18n("Load a palette"));
    m_ui->bnColorSets->setPopupWidget(m_paletteChooser);

    KisConfig cfg(true);
    QString defaultPaletteName = cfg.defaultPalette();
    KoColorSetSP defaultPalette = m_rServer->resource("", "", defaultPaletteName);
    if (defaultPalette) {
        slotSetColorSet(defaultPalette);
        m_paletteChooser->setCurrentItem(defaultPalette);
    } else {
        m_ui->bnAdd->setEnabled(false);
        m_ui->bnUndo->setEnabled(false);
        m_ui->bnRedo->setEnabled(false);
        m_ui->bnRename->setEnabled(false);
        m_ui->bnRemove->setEnabled(false);
        m_ui->bnEditPalette->setEnabled(false);
        m_ui->bnSavePalette->setEnabled(false);
        m_ui->bnLock->setEnabled(false);

        m_ui->paletteView->setAllowModification(false);
    }

    //m_ui->bnUndo->setVisible(false);
    //m_ui->bnRedo->setVisible(false);


    KoResourceServer<KoColorSet> *srv = KoResourceServerProvider::instance()->paletteServer();
    srv->addObserver(this);
}

PaletteDockerDock::~PaletteDockerDock()
{
    if (m_paletteEditor->isModified()) {
        m_paletteEditor->saveNewPaletteVersion();
    }
    KoResourceServer<KoColorSet> *srv = KoResourceServerProvider::instance()->paletteServer();
    srv->removeObserver(this);
}

void PaletteDockerDock::setViewManager(KisViewManager* kisview)
{
    m_view = kisview;
    m_resourceProvider = kisview->canvasResourceProvider();
    connect(m_resourceProvider, SIGNAL(sigFGColorChanged(KoColor)),
            this, SLOT(slotFGColorResourceChanged(KoColor)));
    kisview->nodeManager()->disconnect(m_model);
}

void PaletteDockerDock::unsetResourceServer()
{
    KoResourceServer<KoColorSet> *srv = KoResourceServerProvider::instance()->paletteServer();
    srv->removeObserver(this);
}

void PaletteDockerDock::resourceAdded(QSharedPointer<KoColorSet> resource)
{
    Q_UNUSED(resource);
}

void PaletteDockerDock::removingResource(QSharedPointer<KoColorSet> resource)
{
    Q_UNUSED(resource);
}

void PaletteDockerDock::resourceChanged(QSharedPointer<KoColorSet> resource)
{
    m_model->slotExternalPaletteModified(resource);
}

void PaletteDockerDock::slotContextMenu(const QModelIndex &)
{
    if (QApplication::mouseButtons() == Qt::RightButton) {
        m_viewContextMenu.exec(QCursor::pos());
    }
}

void PaletteDockerDock::slotAddPalette()
{
    KoColorSetSP palette = m_paletteEditor->addPalette();

    if (palette) {
        m_paletteChooser->setCurrentItem(palette);
    }
}

void PaletteDockerDock::slotRemovePalette(KoColorSetSP cs)
{
    m_paletteEditor->removePalette(cs);
}

void PaletteDockerDock::slotImportPalette()
{
    KoColorSetSP palette = m_paletteEditor->importPalette();
    if (palette) {
        m_paletteChooser->setCurrentItem(palette);
    }
}

void PaletteDockerDock::slotExportPalette(KoColorSetSP palette)
{
    KoFileDialog dialog(this, KoFileDialog::SaveFile, "Save Palette");
    dialog.setCaption(i18n("Export Palette"));
    dialog.setDefaultDir(palette->filename());
    dialog.setMimeTypeFilters(QStringList() << "application/x-krita-palette");
    QString newPath;
    if ((newPath = dialog.filename()).isEmpty()) { return; }

    QFile file(newPath);
    if (!file.open(QIODevice::WriteOnly)) {
        warnKrita << "Could not open the file for writing:" << newPath;
        return;
    }
    if (palette->saveToDevice(&file)) {
        m_view->showFloatingMessage(
            i18nc("Floating message about exporting successful", "Palette exported successfully"), QIcon(),
            500, KisFloatingMessage::Low);
    } else {
        warnKrita << "Could export to the file:" << newPath;
    }
}

void PaletteDockerDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
    if (canvas) {
        KisCanvas2 *cv = qobject_cast<KisCanvas2*>(canvas);
        m_ui->paletteView->setDisplayRenderer(cv->displayColorConverter()->displayRendererInterface());
    }

    if (m_view && m_view->document()) {
        m_activeDocument = m_view->document();
        m_paletteEditor->setView(m_view);
    }

    if (!m_currentColorSet) {
        slotSetColorSet(0);
    }
}

void PaletteDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_ui->paletteView->setDisplayRenderer(0);
    m_paletteEditor->setView(0);

    if (!m_currentColorSet) {
        slotSetColorSet(0);
    }
}

void PaletteDockerDock::slotSetColorSet(KoColorSetSP colorSet)
{
    if (m_currentColorSet == colorSet) {
        slotUpdateLblPaletteName();
        return;
    }

    if (m_currentColorSet) {
        disconnect(m_currentColorSet->undoStack());
    }

    // needs to save the palette before switching to another one
    if (m_paletteEditor->isModified() && m_currentColorSet != colorSet) {
        m_paletteEditor->saveNewPaletteVersion();
    }
    if (colorSet) {

        m_model->setColorSet(colorSet);

        m_ui->bnUndo->setEnabled(colorSet->undoStack()->canUndo() && !colorSet->isLocked());
        connect(colorSet->undoStack(), SIGNAL(canUndoChanged(bool)), m_ui->bnUndo, SLOT(setEnabled(bool)));
        connect(colorSet->undoStack(), SIGNAL(undoTextChanged(QString)), this, SLOT(setUndoToolTip(QString)));

        m_ui->bnRedo->setEnabled(colorSet->undoStack()->canRedo() && colorSet->isLocked());
        connect(colorSet->undoStack(), SIGNAL(canRedoChanged(bool)), m_ui->bnRedo, SLOT(setEnabled(bool)));
        connect(colorSet->undoStack(), SIGNAL(redoTextChanged(QString)), this, SLOT(setRedoToolTip(QString)));

        connect(m_ui->bnUndo, SIGNAL(clicked()), this, SLOT(undo()));
        connect(m_ui->bnRedo, SIGNAL(clicked()), this, SLOT(redo()));

        m_ui->bnLock->setChecked(colorSet->isLocked());
    }
    bool state = (bool)colorSet;
    if (state != (bool)m_currentColorSet) {
        m_ui->bnAdd->setEnabled(state);
        m_ui->bnRename->setEnabled(state);
        m_ui->bnRemove->setEnabled(state);
        m_ui->bnEditPalette->setEnabled(state);
        m_ui->bnSavePalette->setEnabled(state);
        m_ui->paletteView->setAllowModification(state);
        m_ui->bnLock->setEnabled(state);
    }

    m_currentColorSet = colorSet;

    if (colorSet) {
        KisConfig cfg(true);
        cfg.setDefaultPalette(colorSet->name());
        m_ui->lblPaletteName->setTextElideMode(Qt::ElideMiddle);
        m_ui->lblPaletteName->setText(colorSet->name());
    }
    else {
        m_ui->lblPaletteName->setText("");
    }
    slotUpdateLblPaletteName();
}

void PaletteDockerDock::slotEditPalette()
{
    KisDlgPaletteEditor dlg(m_paletteEditor.data());
    if (!m_currentColorSet) { return; }
    dlg.initialize(m_model);
    m_paletteEditor->startEditing();
    bool applyChanges = (dlg.exec() == QDialog::Accepted);
    m_paletteEditor->endEditing(applyChanges);
}

void PaletteDockerDock::slotSavePalette()
{
    if (m_paletteEditor->isModified()) {
        m_paletteEditor->saveNewPaletteVersion();
        slotUpdateLblPaletteName();
    }
}


void PaletteDockerDock::slotAddColor()
{
    if (m_resourceProvider) {
        m_paletteEditor->addEntry(m_resourceProvider->fgColor());
    }
    slotUpdateLblPaletteName();
}

void PaletteDockerDock::slotRemoveColor()
{
    QModelIndex index = m_ui->paletteView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_paletteEditor->removeEntry(index);
    m_ui->bnRemove->setEnabled(false);
    slotUpdateLblPaletteName();
}

void PaletteDockerDock::setFGColorByPalette(const KisSwatch &entry)
{
    if (m_resourceProvider) {
        m_colorSelfUpdate = true;
        m_resourceProvider->setFGColor(entry.color());
        m_colorSelfUpdate = false;
    }
}

void PaletteDockerDock::slotUpdateLblPaletteName()
{
    if (m_currentColorSet) {
        m_ui->lblPaletteName->setTextElideMode(Qt::ElideLeft);
        QString name = m_currentColorSet->name();

        bool isGlobal = true;
        KisResourceModel model(ResourceType::Palettes);
        QModelIndex index = model.indexForResource(m_currentColorSet);
        if (index.isValid()) {
            bool ok;
            int storageId = model.data(index, Qt::UserRole + KisAllResourcesModel::StorageId).toInt(&ok);
            if (ok) {
                KisStorageModel storageModel;
                KisResourceStorageSP storage = storageModel.storageForId(storageId);
                isGlobal = storage->type() != KisResourceStorage::StorageType::Memory;
            }
        }
        m_actSavePalette.data()->setEnabled(isGlobal);
        if (isGlobal) {
            m_actSavePalette.data()->setToolTip(i18nc("@tooltip", "Save palette explicitly, will also happen automatically on exiting Krita."));
        }
        else {
            m_actSavePalette.data()->setToolTip(i18nc("@tooltip", "Saving for document palettes is done by saving the document."));
        }
        // if the palette is not global, then let's not indicate that the changes has been made
        // (it's easier than tracking whether the document has been saved or maybe exported etc.)
        if (m_paletteEditor->isModified() && isGlobal) {
            name = "* " + name;
            QFont font = m_ui->lblPaletteName->font();
            font.setItalic(true);
            m_ui->lblPaletteName->setFont(font);
        }
        else {
            QFont font = m_ui->lblPaletteName->font();
            font.setItalic(false);
            m_ui->lblPaletteName->setFont(font);
        }

        m_ui->lblPaletteName->setText(name);
    }
    else {
        m_ui->lblPaletteName->setText("");
    }
}

void PaletteDockerDock::slotLockPalette(bool locked)
{
    m_currentColorSet->setLocked(locked);
    QIcon icon = locked ? KisIconUtils::loadIcon(koIconName("object-locked"))
                        : KisIconUtils::loadIcon(koIconName("object-unlocked"));
    m_ui->bnLock->setIcon(icon);
    m_ui->bnAdd->setEnabled(!locked);
    m_ui->bnRename->setEnabled(!locked);
    m_ui->bnRemove->setEnabled(!locked);
    m_ui->bnEditPalette->setEnabled(!locked);
    m_ui->bnSavePalette->setEnabled(!locked);
    m_ui->paletteView->setAllowModification(!locked);
}

void PaletteDockerDock::setUndoToolTip(const QString &text)
{
    m_ui->bnUndo->setToolTip(text);
}

void PaletteDockerDock::setRedoToolTip(const QString &text)
{
    m_ui->bnRedo->setToolTip(text);
}

void PaletteDockerDock::undo()
{
    m_currentColorSet->undoStack()->undo();
    slotUpdateLblPaletteName();
}

void PaletteDockerDock::redo()
{
    m_currentColorSet->undoStack()->redo();
    slotUpdateLblPaletteName();
}


void PaletteDockerDock::slotFGColorResourceChanged(const KoColor &color)
{
    if (!m_colorSelfUpdate) {
        m_ui->paletteView->slotFGColorChanged(color);
    }
}

void PaletteDockerDock::slotStoragesChanged(const QString &/*location*/)
{
    if (m_activeDocument.isNull()) {
        slotSetColorSet(0);
    }
    if (m_currentColorSet) {
        if (!m_rServer->resource(m_currentColorSet->md5Sum(), "", "")) {
            slotSetColorSet(0);
        }
    }
}

void PaletteDockerDock::slotPaletteIndexSelected(const QModelIndex &index)
{
    bool occupied = qvariant_cast<bool>(index.data(KisPaletteModel::CheckSlotRole));
    if (occupied) {
        if (!qvariant_cast<bool>(index.data(KisPaletteModel::IsGroupNameRole))) {
            m_ui->bnRemove->setEnabled(true);
            KisSwatch entry = m_model->getSwatch(index);
            setFGColorByPalette(entry);
        }
    }
    m_ui->bnRemove->setEnabled(occupied);
    slotUpdateLblPaletteName();
}

void PaletteDockerDock::slotPaletteIndexClicked(const QModelIndex &index)
{
    if (!(qvariant_cast<bool>(index.data(KisPaletteModel::CheckSlotRole)))) {
        setEntryByForeground(index);
    }
    slotUpdateLblPaletteName();
}

void PaletteDockerDock::slotPaletteIndexDoubleClicked(const QModelIndex &index)
{
    m_paletteEditor->modifyEntry(index);
    slotUpdateLblPaletteName();
}

void PaletteDockerDock::setEntryByForeground(const QModelIndex &index)
{
    m_paletteEditor->setEntry(m_resourceProvider->fgColor(), index);
    m_ui->bnRemove->setEnabled(true);
    slotUpdateLblPaletteName();
}

void PaletteDockerDock::slotEditEntry()
{
    QModelIndex index = m_ui->paletteView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_paletteEditor->modifyEntry(index);
    slotUpdateLblPaletteName();
}

void PaletteDockerDock::slotNameListSelection(const KoColor &color)
{
    m_colorSelfUpdate = true;
    m_ui->paletteView->selectClosestColor(color);
    m_resourceProvider->setFGColor(color);
    m_colorSelfUpdate = false;
}

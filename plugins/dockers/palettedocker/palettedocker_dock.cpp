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
#include <QComboBox>
#include <kis_color_button.h>
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>

#include "KisPaletteModel.h"
#include "KisColorsetChooser.h"
#include "ui_wdgpalettedock.h"
#include "kis_palette_delegate.h"
#include "kis_palette_view.h"

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
    m_wdgPaletteDock->bnAddDialog->setIcon(KisIconUtils::loadIcon("document-new"));
    m_wdgPaletteDock->bnAddDialog->setIconSize(QSize(16, 16));
    m_wdgPaletteDock->bnRemove->setIcon(KisIconUtils::loadIcon("edit-delete"));
    m_wdgPaletteDock->bnRemove->setIconSize(QSize(16, 16));
    m_wdgPaletteDock->bnAdd->setEnabled(false);
    m_wdgPaletteDock->bnRemove->setEnabled(false);

    connect(m_wdgPaletteDock->bnAdd, SIGNAL(clicked(bool)), this, SLOT(addColorForeground()));
    connect(m_wdgPaletteDock->bnAddDialog, SIGNAL(clicked(bool)), this, SLOT(addColor()));
    connect(m_wdgPaletteDock->bnRemove, SIGNAL(clicked(bool)), this, SLOT(removeColor()));

    m_model = new KisPaletteModel(this);
    m_wdgPaletteDock->paletteView->setPaletteModel(m_model);


    connect(m_wdgPaletteDock->paletteView, SIGNAL(entrySelected(KoColorSetEntry)), this, SLOT(entrySelected(KoColorSetEntry)));

    KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer(false);
    m_serverAdapter = QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoColorSet>(rServer));
    m_serverAdapter->connectToResourceServer();
    rServer->addObserver(this);

    m_colorSetChooser = new KisColorsetChooser(this);
    connect(m_colorSetChooser, SIGNAL(paletteSelected(KoColorSet*)), this, SLOT(setColorSet(KoColorSet*)));

    m_wdgPaletteDock->bnColorSets->setIcon(KisIconUtils::loadIcon("hi16-palette_library"));
    m_wdgPaletteDock->bnColorSets->setToolTip(i18n("Choose palette"));
    m_wdgPaletteDock->bnColorSets->setPopupWidget(m_colorSetChooser);

    KisConfig cfg;
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
    if (colorSet && colorSet->removable()) {
        m_wdgPaletteDock->bnAdd->setEnabled(true);
        m_wdgPaletteDock->bnRemove->setEnabled(true);
    } else {
        m_wdgPaletteDock->bnAdd->setEnabled(false);
        m_wdgPaletteDock->bnRemove->setEnabled(false);
    }
    m_currentColorSet = colorSet;
}

void PaletteDockerDock::addColorForeground()
{
    if (m_resourceProvider) {
        //setup dialog
        KoDialog *window = new KoDialog();
        window->setWindowTitle(i18n("Add a new Colorset Entry"));
        QFormLayout *editableItems = new QFormLayout();
        window->mainWidget()->setLayout(editableItems);
        QComboBox *cmbGroups = new QComboBox();
        QString defaultGroupName = i18n("Default");
        cmbGroups->addItem(defaultGroupName);
        cmbGroups->addItems(m_currentColorSet->getGroupNames());
        QLineEdit *lnIDName = new QLineEdit();
        QLineEdit *lnName = new QLineEdit();
        KisColorButton *bnColor = new KisColorButton();
        QCheckBox *chkSpot = new QCheckBox();
        editableItems->addRow(tr("Group"), cmbGroups);
        editableItems->addRow(tr("ID"), lnIDName);
        editableItems->addRow(tr("Name"), lnName);
        editableItems->addRow(tr("Color"), bnColor);
        editableItems->addRow(tr("Spot"), chkSpot);
        cmbGroups->setCurrentIndex(0);
        lnName->setText(i18n("Color ")+QString::number(m_currentColorSet->nColors()+1));
        lnIDName->setText(QString::number(m_currentColorSet->nColors()+1));
        bnColor->setColor(m_resourceProvider->fgColor());
        chkSpot->setChecked(false);

        //
        if (window->exec() == KoDialog::Accepted) {
            QString groupName = cmbGroups->currentText();
            if (groupName == defaultGroupName) {
                groupName = QString();
            }
            KoColorSetEntry newEntry;
            newEntry.color = bnColor->color();
            newEntry.name = lnName->text();
            newEntry.id = lnIDName->text();
            newEntry.spotColor = chkSpot->isChecked();
            m_currentColorSet->add(newEntry, groupName);
            m_currentColorSet->save();
            setColorSet(m_currentColorSet); // update model
        }
    }
}

void PaletteDockerDock::addColor()
{
    if (m_currentColorSet && m_resourceProvider) {

        const KoColorDisplayRendererInterface *displayRenderer =
            m_canvas->displayColorConverter()->displayRendererInterface();

        KoColor currentFgColor = m_canvas->resourceManager()->foregroundColor();
        QColor color = QColorDialog::getColor(displayRenderer->toQColor(currentFgColor));

        if (color.isValid()) {
            KoColorSetEntry newEntry;
            newEntry.color = displayRenderer->approximateFromRenderedQColor(color);
            m_currentColorSet->add(newEntry);
            m_currentColorSet->save();
            setColorSet(m_currentColorSet); // update model
        }
    }
}

void PaletteDockerDock::removeColor()
{
    QModelIndex index = m_wdgPaletteDock->paletteView->currentIndex();
    if (!index.isValid() || qVariantValue<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
        return;
    }
    QStringList entryList = qVariantValue<QStringList>(index.data(KisPaletteModel::RetrieveEntryRole));
    m_currentColorSet->removeAt(entryList.at(1).toUInt(), entryList.at(0));
    m_currentColorSet->save();
    setColorSet(m_currentColorSet); // update model
}

void PaletteDockerDock::entrySelected(KoColorSetEntry entry)
{
    quint32 index = 0;
    QString groupName = m_currentColorSet->findGroupByColorName(entry.name, &index);
    QString seperator;
    if (groupName != QString()) {
        seperator = " - ";
    }
    m_wdgPaletteDock->lblColorName->setText(groupName+seperator+entry.name);
    if (m_resourceProvider) {
        m_resourceProvider->setFGColor(entry.color);
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



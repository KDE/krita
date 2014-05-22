/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QFileDialog>
#include <QProcessEnvironment>
#include <QMessageBox>
#include <QLabel>
#include <QTimer>
#include <QMenu>

#include "ui_KoResourceManagerWidget.h"
#include "KoResourceManagerWidget.h"
#include "KoResourceManagerControl.h"
#include "KoResourceTableModel.h"
#include "KoResourceTaggingManager.h"
#include "KoDlgCreateBundle.h"
#include "KoTagChooserWidget.h"
#include <KoIcon.h>


//TODO KoResourceManagerControl constructor parameter is the number of tabs of the Resource Manager
KoResourceManagerWidget::KoResourceManagerWidget(QWidget *parent)
    : KDialog(parent)
    , m_ui(new Ui::KoResourceManagerWidget)
    , m_control(new KoResourceManagerControl(2))
    , m_tagManager(0)
    , m_firstRefresh(true)
{
    setCaption(i18n("Manage Resources"));
    setButtons(Close);
    setDefaultButton(Close);
    connect(this, SIGNAL(closeClicked()), SLOT(close()));

    m_page = new QWidget(this);
    m_ui->setupUi(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    initializeModels(true);
    initializeConnect();
    initializeTitle();
    initializeFilterMenu();
    refreshTaggingManager();

    m_ui->bnCreateSet->setIcon(koIcon("list-add"));
    m_ui->bnDelete->setIcon(koIcon("edit-delete"));
    m_ui->bnEdit->setIcon(koIcon("document-edit"));
    m_ui->bnNew->setIcon(koIcon("document-new"));
    m_ui->bnSave->setIcon(koIcon("dialog-ok"));

    m_ui->tabResourceBundles->removeTab(2);
    m_ui->tabResourceBundles->removeTab(2);
    m_ui->tabResourceBundles->removeTab(2);

    status("Welcome back ! Resource Manager is ready to use...", 1500);
}

KoResourceManagerWidget::~KoResourceManagerWidget()
{
    delete m_ui;
    delete m_control;
    delete m_resourceNameLabel;
    delete m_tagManager;
}

/*Initialize*/

void KoResourceManagerWidget::initializeFilterMenu()
{
    m_actionAll = new QAction(i18n("All"), this);
    m_actionAll->setCheckable(true);
    connect(m_actionAll, SIGNAL(toggled(bool)), this, SLOT(filterFieldSelected(bool)));

    m_actionName = new QAction(i18n("Name"), this);
    m_actionName->setCheckable(true);
    connect(m_actionName, SIGNAL(toggled(bool)), this, SLOT(filterFieldSelected(bool)));

    m_actionFile = new QAction(i18n("Filename"), this);
    m_actionFile->setCheckable(true);
    connect(m_actionFile, SIGNAL(toggled(bool)), this, SLOT(filterFieldSelected(bool)));

    QList<QAction*> liste;

    liste.append(m_actionAll);
    liste.append(m_actionName);
    liste.append(m_actionFile);

    QMenu *buttonMenu = new QMenu();
    buttonMenu->addActions(liste);
    m_ui->bnOpen->setMenu(buttonMenu);
    m_actionAll->setChecked(true);
}

void KoResourceManagerWidget::initializeTitle()
{
    QFont labelFont("Arial Black", 20, QFont::Bold);
    labelFont.setWeight(75);
    labelFont.setLetterSpacing(QFont::AbsoluteSpacing, 2);

    m_resourceNameLabel = new ClickLabel();
    m_resourceNameLabel->setText("Welcome\n to the\n Resource Manager !\n");
    m_resourceNameLabel->setFont(labelFont);
    m_resourceNameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_ui->horizontalLayout_4->insertWidget(2, m_resourceNameLabel);
    m_ui->txtName->setVisible(false);
    m_ui->scrollArea->setVisible(false);
    m_ui->label->setVisible(false);
    m_ui->bnEdit->setVisible(false);
    m_ui->bnNew->setVisible(false);
    m_ui->bnSave->setVisible(false);
}

void KoResourceManagerWidget::initializeModels(bool first)
{
    for (int i = 0; i < m_control->getNbModels(); i++) {
        QTableView* currentTableView = tableAvailable(i);
        currentTableView->setModel(m_control->getModel(i));
        currentTableView->resizeColumnToContents(1);
        currentTableView->resizeColumnToContents(0);
        connect(currentTableView, SIGNAL(pressed(QModelIndex)), m_control->getModel(i), SLOT(resourceSelected(QModelIndex)));
        connect(currentTableView->horizontalHeader(), SIGNAL(sectionPressed(int)), m_control->getModel(i), SLOT(allSelected(int)));
        connect(currentTableView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(refreshDetails(QModelIndex)));

        if (first) {
            KoResourceTableDelegate *delegate = new KoResourceTableDelegate();
            connect(delegate, SIGNAL(renameEnded(QString)), this, SLOT(rename(QString)));
            currentTableView->setItemDelegate(delegate);
        }
    }
}

void KoResourceManagerWidget::initializeConnect()
{
    connect(m_control, SIGNAL(status(QString, int)), this, SLOT(status(QString, int)));

    connect(m_ui->bnDetails, SIGNAL(clicked()), this, SLOT(showHide()));

    connect(m_ui->txtAuthor, SIGNAL(editingFinished()), this, SLOT(setMeta()));
    connect(m_ui->txtLicense, SIGNAL(editingFinished()), this, SLOT(setMeta()));
    connect(m_ui->txtWebsite, SIGNAL(editingFinished()), this, SLOT(setMeta()));
    connect(m_ui->dateUpdated, SIGNAL(editingFinished()), this, SLOT(setMeta()));
    connect(m_ui->dateCreated, SIGNAL(editingFinished()), this, SLOT(setMeta()));

    connect(m_ui->bnCreateSet, SIGNAL(clicked()), this, SLOT(createPack()));
    connect(m_ui->bnInstall, SIGNAL(clicked()), this, SLOT(installPack()));
    connect(m_ui->bnUninstall, SIGNAL(clicked()), this, SLOT(uninstallPack()));
    connect(m_ui->bnDelete, SIGNAL(clicked()), this, SLOT(deletePack()));
    connect(m_ui->bnExport, SIGNAL(clicked()), this, SLOT(exportBundle()));
    connect(m_ui->bnImport, SIGNAL(clicked()), this, SLOT(importBundle()));


    connect(m_ui->cmbResourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(filterResourceTypes(int)));

    connect(m_ui->txtName, SIGNAL(editingFinished()), this, SLOT(endRenaming()));

    connect(m_ui->bnSave, SIGNAL(clicked()), this, SLOT(saveMeta()));

    connect(m_ui->bnEdit, SIGNAL(clicked()), this, SLOT(startRenaming()));
    connect(m_ui->bnNew, SIGNAL(clicked()), this, SLOT(thumbnail()));

    connect(m_ui->bnRemoveTag, SIGNAL(clicked()), this, SLOT(removeTag()));

    connect(m_ui->bnRefresh, SIGNAL(clicked()), this, SLOT(refresh()));

    connect(m_ui->tabResourceBundles, SIGNAL(currentChanged(int)), this, SLOT(tableAvailableChanged(int)));
}

/*Tools*/

void KoResourceManagerWidget::showHide()
{
    m_ui->widget_2->setVisible(!m_ui->widget_2->isVisible());
}

void KoResourceManagerWidget::status(QString text, int timeout)
{
    m_ui->lblStatus->setText(text);
    if (!text.isEmpty()) {
        QTimer::singleShot(timeout, m_ui->lblStatus, SLOT(clear()));
    }
}

QTableView* KoResourceManagerWidget::tableAvailable(int index)
{
    return dynamic_cast<QTableView*>(m_ui->tabResourceBundles->widget(index)->layout()->itemAt(0)->widget());
}

void KoResourceManagerWidget::toBundleView(int installTab)
{
    if (m_ui->tabResourceBundles->currentIndex() != installTab) {
        m_ui->tabResourceBundles->setCurrentIndex(installTab);
    }

    if (m_ui->cmbResourceType->currentIndex() != 1) {
        m_ui->cmbResourceType->setCurrentIndex(1);
    } else {
        refresh();
    }
}

/*Functionalities*/

void KoResourceManagerWidget::createPack()
{
    if (m_control->createPack(m_ui->tabResourceBundles->currentIndex())) {
        toBundleView(0);
        status("New bundle created successfully", 3000);
    }
}

void KoResourceManagerWidget::installPack()
{
    if (m_control->install(m_ui->tabResourceBundles->currentIndex())) {
        toBundleView(1);
        status("Bundle(s) installed successfully", 3000);
    }
}

void KoResourceManagerWidget::deletePack()
{
    if (m_control->remove(m_ui->tabResourceBundles->currentIndex())) {
        refreshDetails(tableAvailable(m_ui->tabResourceBundles->currentIndex())->currentIndex());
        status("Resource(s) removed successfully", 3000);
    }
}

void KoResourceManagerWidget::uninstallPack()
{
    if (m_control->uninstall(m_ui->tabResourceBundles->currentIndex())) {
        toBundleView(0);
        status("Bundle(s) uninstalled successfully", 3000);
    }
}

//TODO Régler le pb de chgt de taille de la thumbnail
void KoResourceManagerWidget::thumbnail()
{
    QTableView* currentTableView = tableAvailable(m_ui->tabResourceBundles->currentIndex());
    QModelIndex currentIndex = currentTableView->currentIndex();
    QString fileName = QFileDialog::getOpenFileName(0,
                       tr("Import Thumbnail"), QProcessEnvironment::systemEnvironment().value("HOME").section(':', 0, 0), tr("Image Files (*.jpg)"));

    m_control->thumbnail(currentIndex, fileName, m_ui->tabResourceBundles->currentIndex());
    currentTableView->reset();
    currentTableView->setCurrentIndex(currentIndex);
}

void KoResourceManagerWidget::setMeta()
{
    QObject *emetteur = sender();

    if (emetteur->inherits("QLineEdit")) {
        int currentIndex = m_ui->tabResourceBundles->currentIndex();
        QTableView* currentTableView = tableAvailable(currentIndex);
        QLineEdit* sender = (QLineEdit*)emetteur;

        if (emetteur == m_ui->txtAuthor) {
            m_control->setMeta(currentTableView->currentIndex(), "Author", sender->text(), currentIndex);
            m_ui->txtAuthor->blockSignals(true);
            m_ui->txtAuthor->clearFocus();
            m_ui->txtAuthor->blockSignals(false);
        } else if (emetteur == m_ui->txtLicense) {
            m_control->setMeta(currentTableView->currentIndex(), "License", sender->text(), currentIndex);
            m_ui->txtLicense->blockSignals(true);
            m_ui->txtLicense->clearFocus();
            m_ui->txtLicense->blockSignals(false);
        } else if (emetteur == m_ui->txtWebsite) {
            m_control->setMeta(currentTableView->currentIndex(), "Website", sender->text(), currentIndex);
            m_ui->txtWebsite->blockSignals(true);
            m_ui->txtWebsite->clearFocus();
            m_ui->txtWebsite->blockSignals(false);
        }
    }
}

void KoResourceManagerWidget::startRenaming()
{
    status("Renaming...");
    m_ui->txtName->blockSignals(false);
    m_resourceNameLabel->setVisible(false);
    m_ui->bnEdit->setVisible(false);

    if (m_ui->bnNew->isVisible()) {
        m_ui->bnNew->setVisible(false);
    }

    m_ui->txtName->setVisible(true);
    m_ui->txtName->setFocus();
}

void KoResourceManagerWidget::endRenaming()
{
    QTableView* currentTableView = tableAvailable(m_ui->tabResourceBundles->currentIndex());
    QModelIndex currentIndex = currentTableView->currentIndex();
    m_ui->txtName->blockSignals(true);

    QString newFileName = m_ui->txtName->text();
    QString extension = m_resourceNameLabel->text().section('.', 1);
    if (!extension.isEmpty()) {
        newFileName += "." + extension;
    }

    if (m_control->rename(currentIndex, newFileName, m_ui->tabResourceBundles->currentIndex())) {
        m_resourceNameLabel->setText(newFileName);
        currentTableView->reset();
        status("Resource renamed successfully...", 3000);
    } else {
        m_ui->txtName->setText(m_resourceNameLabel->text().section('.', 0, 0));
        status("Rename cancelled...", 3000);
    }

    currentTableView->setCurrentIndex(currentIndex);
    m_resourceNameLabel->setVisible(true);
    m_ui->bnEdit->setVisible(true);
    m_ui->txtName->setVisible(false);
}

void KoResourceManagerWidget::rename(QString newName)
{
    status("Renaming...");

    QTableView* currentTableView = tableAvailable(m_ui->tabResourceBundles->currentIndex());
    QModelIndex currentIndex = currentTableView->currentIndex();

    QString newFileName = newName + "." + m_resourceNameLabel->text().section('.', 1);

    if (m_control->rename(currentIndex, newFileName, m_ui->tabResourceBundles->currentIndex())) {
        m_resourceNameLabel->setText(newFileName);
        currentTableView->reset();
        status("Resource renamed successfully...", 3000);
    } else {
        status("Rename cancelled...", 3000);
    }
}

void KoResourceManagerWidget::removeTag()
{
    if (m_ui->listTags->selectedItems().size() != 1) {
        status("No tag selected in the above list...Tag removing aborted.", 3000);
    } else {
        status("Removing tag from resource...");
        QString tagName = m_ui->listTags->selectedItems().at(0)->data(Qt::DisplayRole).toString();
        KoResourceTableModel *currentModel = m_control->getModel(m_ui->tabResourceBundles->currentIndex());
        QTableView *currentTableView = tableAvailable(m_ui->tabResourceBundles->currentIndex());
        KoResource* currentResource = currentModel->getResourceFromIndex(currentTableView->currentIndex());

        currentModel->deleteTag(currentResource, tagName);

        if (m_tagManager->currentTag() == tagName) {
            currentModel->hideResource(currentResource);
            refreshDetails(currentModel->index(-1));
        } else {
            refreshDetails(currentTableView->currentIndex());
        }

        currentTableView->reset();
        status("Tag removed successfully", 3000);
    }
}

void KoResourceManagerWidget::filterFieldSelected(bool value)
{
    status("Configuring filtering tool...");

    QAction *emetteur = (QAction*)sender();

    if (emetteur == m_actionAll) {
        if (value) {
            m_actionAll->setChecked(true);
            m_actionName->setChecked(true);
            m_actionFile->setChecked(true);
            m_control->configureFilters(0, true);
        } else {
            m_actionAll->setChecked(false);
            m_actionName->setChecked(true);
            m_actionFile->setChecked(false);
            m_control->configureFilters(0, false);
        }
        status("Filters updated...", 3000);
    } else {
        if (emetteur == m_actionName) {
            if (!m_actionFile->isChecked()) {
                m_actionName->setChecked(true);
                status("Error : must have at least one filter criterium...", 3000);
            } else {
                m_control->configureFilters(1, value);
                status("Filters updated...", 3000);
            }
        } else if (emetteur == m_actionFile) {
            if (!m_actionName->isChecked()) {
                m_actionFile->setChecked(true);
                status("Error : must have at least one filter criterium...", 3000);
            } else {
                m_control->configureFilters(2, value);
                status("Filters updated...", 3000);
            }
        }

        m_actionAll->blockSignals(true);
        m_actionAll->setChecked(m_actionName->isChecked() && m_actionFile->isChecked());
        m_actionAll->blockSignals(false);
    }
}

void KoResourceManagerWidget::filterResourceTypes(int index)
{
    int currentTab = m_ui->tabResourceBundles->currentIndex();
    KoResourceTableModel* model;

    status("Filtering...", 3000);

    m_control->filterResourceTypes(index);
    initializeModels();

    model = m_control->getModel(currentTab);

    if (model->resourcesCount() == 0) {
        m_ui->widget_2->setEnabled(false);
    } else {
        QModelIndex index = model->index(0);

        m_ui->widget_2->setEnabled(true);
        tableAvailable(currentTab)->setCurrentIndex(index);
        refreshDetails(index);
    }
    refreshTaggingManager();

    status("Resource lists updated", 3000);
}

void KoResourceManagerWidget::refreshDetails(QModelIndex newIndex)
{
    if (newIndex.row() == -1) {
        m_ui->widget_2->setEnabled(false);
        return;
    } else {
        m_ui->widget_2->setEnabled(true);
    }

    KoResource* currentResource = m_control->getModel(m_ui->tabResourceBundles->currentIndex())->currentlyVisibleResources().at(newIndex.row());
    KoResourceBundle* currentBundle = dynamic_cast<KoResourceBundle*>(currentResource);
    QString currentDate;

    if (m_firstRefresh) {
        m_ui->scrollArea->setVisible(true);
        m_ui->label->setVisible(true);
        m_ui->bnEdit->setVisible(true);

        QFont labelFont("Arial Black", 14, QFont::Bold);
        labelFont.setWeight(75);
        m_resourceNameLabel->setFont(labelFont);
        connect(m_resourceNameLabel, SIGNAL(clicked()), this, SLOT(startRenaming()));

        m_firstRefresh = false;
        m_tagManager->showTaggingBar(true, true);
        m_ui->widget_2->layout()->addWidget(m_tagManager->tagChooserWidget());
        m_ui->dateCreated->setDisplayFormat("dd/MM/yyyy");
        m_ui->dateUpdated->setDisplayFormat("dd/MM/yyyy");
    }

    //Name

    m_resourceNameLabel->setText(currentResource->shortFilename());
    m_ui->txtName->setText(currentResource->shortFilename().section('.', 0, 0));

    m_resourceNameLabel->setVisible(true);
    m_ui->txtName->setVisible(false);

    //Tags

    m_ui->listTags->clear();
    m_ui->listTags->addItems(m_control->getModel(m_ui->tabResourceBundles->currentIndex())->assignedTagsList(currentResource));
    m_ui->bnRemoveTag->setEnabled(!m_ui->listTags->count() == 0);

    //Overview

    if (currentResource->image().isNull()) {
        m_ui->label->clear();
    } else {
        m_ui->label->setPixmap(QPixmap::fromImage(currentResource->image()).scaled(1000, 150, Qt::KeepAspectRatio));
    }
    m_ui->bnNew->setVisible(false);

    if (currentBundle != 0) {
        m_ui->bnNew->setVisible(true);

        m_ui->label_3->setVisible(true);
        m_ui->label_4->setVisible(true);
        m_ui->label_5->setVisible(true);

        m_ui->txtAuthor->setText(currentBundle->getMeta("author"));
        m_ui->txtAuthor->setVisible(true);
        m_ui->txtLicense->setText(currentBundle->getMeta("license"));
        m_ui->txtLicense->setVisible(true);
        m_ui->txtWebsite->setText(currentBundle->getMeta("website"));
        m_ui->txtWebsite->setVisible(true);

        currentDate = currentBundle->getMeta("created");
        if (currentDate.isEmpty()) {
            m_ui->label_6->setVisible(false);
            m_ui->dateCreated->setVisible(false);
        } else {
            m_ui->label_6->setVisible(true);
            m_ui->dateCreated->setVisible(true);
            m_ui->dateCreated->setDate(QDate::fromString(currentDate, "dd/MM/yyyy"));
        }

        currentDate = currentBundle->getMeta("updated");
        if (currentDate.isEmpty()) {
            m_ui->label_7->setVisible(false);
            m_ui->dateUpdated->setVisible(false);
        } else {
            m_ui->label_7->setVisible(true);
            m_ui->dateUpdated->setVisible(true);
            m_ui->dateUpdated->setDate(QDate::fromString(currentDate, "dd/MM/yyyy"));
        }
        m_ui->bnSave->setVisible(true);
    } else {
        m_ui->txtAuthor->setVisible(false);
        m_ui->txtLicense->setVisible(false);
        m_ui->txtWebsite->setVisible(false);
        m_ui->dateCreated->setVisible(false);
        m_ui->dateUpdated->setVisible(false);
        m_ui->label_3->setVisible(false);
        m_ui->label_4->setVisible(false);
        m_ui->label_5->setVisible(false);
        m_ui->label_6->setVisible(false);
        m_ui->label_7->setVisible(false);
        m_ui->bnSave->setVisible(false);
    }
}

void KoResourceManagerWidget::saveMeta()
{
    int currentTabIndex = m_ui->tabResourceBundles->currentIndex();

    status("Saving metadata...", 3000);
    m_control->saveMeta(tableAvailable(currentTabIndex)->currentIndex(), currentTabIndex);
    status("Metadata saved successfully...", 3000);
}

void KoResourceManagerWidget::refreshTaggingManager(int index)
{
    if (m_tagManager) {
        if (!m_tagManager->tagChooserWidget()->selectedTagIsReadOnly()) {
            m_control->refreshTaggingManager();
            tableAvailable(index)->reset();
        }
        m_ui->widget_2->layout()->removeWidget(m_tagManager->tagChooserWidget());
        m_tagManager->showTaggingBar(true, false);
        delete m_tagManager;
    }

    m_tagManager = new KoResourceTaggingManager(m_control->getModel(index), m_ui->widget_2);
    m_tagManager->showTaggingBar(true, !m_firstRefresh);

    m_ui->gridLayout->addWidget(m_tagManager->tagFilterWidget(), 0, 1);
    m_ui->gridLayout->addWidget(m_ui->widget, 0, 2);
    m_ui->widget_2->layout()->addWidget(m_tagManager->tagChooserWidget());
}

void KoResourceManagerWidget::tableAvailableChanged(int index)
{
    refreshTaggingManager(index);

    QTableView *newView = tableAvailable(index);
    newView->setFocus();
    newView->setCurrentIndex(newView->currentIndex());
    newView->resizeColumnToContents(0);
    newView->resizeColumnToContents(1);
    refreshDetails(newView->currentIndex());

    if (index == KoResourceTableModel::Available) {
        m_ui->bnInstall->setEnabled(true);
        m_ui->bnUninstall->setEnabled(false);
    } else if (index == KoResourceTableModel::Installed) {
        m_ui->bnInstall->setEnabled(false);
        m_ui->bnUninstall->setEnabled(true);
    }
}

void KoResourceManagerWidget::exportBundle()
{
    m_control->exportBundle(m_ui->tabResourceBundles->currentIndex());
}

void KoResourceManagerWidget::importBundle()
{
    if (m_control->importBundle()) {
        toBundleView(0);
    }
}

void KoResourceManagerWidget::refresh()
{
    filterResourceTypes(m_ui->cmbResourceType->currentIndex());
}

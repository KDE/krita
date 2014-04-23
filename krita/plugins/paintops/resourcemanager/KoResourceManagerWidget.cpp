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

#include "ui_KoResourceManagerWidget.h"
#include "KoResourceManagerWidget.h"
#include "KoResourceManagerControl.h"
#include "KoResourceTableModel.h"
#include "KoResourceTaggingManager.h"
#include <QFileDialog>
#include "KoBundleCreationWidget.h"
#include <QtCore/QProcessEnvironment>
#include <QtGui/QMessageBox>
#include "KoTagChooserWidget.h"

#include <iostream>
using namespace std;

//TODO Paramètre de ManagerControl à modifier si on veut rajouter des onglets
KoResourceManagerWidget::KoResourceManagerWidget(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::KoResourceManagerWidget),control(new KoResourceManagerControl(2)),tagMan(0),firstRefresh(true)
{
    ui->setupUi(this);

    initializeModels(true);
    initializeConnect();
    initializeTitle();
    initializeFilterMenu();
    refreshTaggingManager();

    QString kritaPath=QProcessEnvironment::systemEnvironment().value("KDEDIRS").section(':',0,0);

    ui->pushButton_2->setIcon(QIcon(kritaPath+"/lib/x86_64-linux-gnu/calligra/imports/org/krita/sketch/images/svg/icon-add.svg"));
    ui->pushButton_9->setIcon(QIcon(kritaPath+"/lib/x86_64-linux-gnu/calligra/imports/org/krita/sketch/images/svg/icon-delete.svg"));
    ui->toolButton->setIcon(QIcon(kritaPath+"/lib/x86_64-linux-gnu/calligra/imports/org/krita/sketch/images/svg/icon-edit.svg"));
    ui->toolButton_2->setIcon(QIcon(kritaPath+"/lib/x86_64-linux-gnu/calligra/imports/org/krita/sketch/images/svg/icon-paint.svg"));
    ui->pushButton_12->setIcon(QIcon(kritaPath+"/lib/x86_64-linux-gnu/calligra/imports/org/krita/sketch/images/svg/icon-apply.svg"));

    ui->tabWidget->removeTab(2);
    ui->tabWidget->removeTab(2);
    ui->tabWidget->removeTab(2);

    ui->statusbar->showMessage("Welcome back ! Resource Manager is ready to use...",1500);

    /*this->model2=new MyTableModel(0);

    m_filter = new QSortFilterProxyModel(this);
    m_filter->setSourceModel(model2);
    connect(ui->lineEdit,SIGNAL(textChanged(QString)),
            m_filter,SLOT(setFilterFixedString(QString)));
    m_filter->setFilterKeyColumn(1);
    m_filter->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->tableView->setModel(m_filter);
    ui->tableView_2->setModel(m_filter);
    ui->tableView_3->setModel(m_filter);
    ui->tableView_4->setModel(m_filter);
    ui->tableView_5->setModel(m_filter);*/
}

KoResourceManagerWidget::~KoResourceManagerWidget()
{
    delete ui;
    delete control;
    delete resourceNameLabel;
    delete tagMan;
}

void KoResourceManagerWidget::initializeFilterMenu()
{
    QList<QAction*> liste;
    liste.append(ui->actionAll);
    liste.append(ui->actionName);
    liste.append(ui->actionFile);

    QMenu *buttonMenu=new QMenu();
    buttonMenu->addActions(liste);
    ui->pushButton_10->setMenu(buttonMenu);
    ui->actionAll->setChecked(true);
}

void KoResourceManagerWidget::initializeTitle()
{
    QFont labelFont("Arial Black",20,QFont::Bold);
    labelFont.setWeight(75);
    labelFont.setLetterSpacing(QFont::AbsoluteSpacing,2);

    resourceNameLabel=new ClickLabel();
    resourceNameLabel->setText("Welcome\n to the\n Resource Manager !\n");
    resourceNameLabel->setFont(labelFont);
    resourceNameLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->horizontalLayout_4->insertWidget(2,resourceNameLabel);
    ui->lineEdit_5->setVisible(false);
    ui->scrollArea->setVisible(false);
    ui->label->setVisible(false);
    ui->toolButton->setVisible(false);
    ui->toolButton_2->setVisible(false);
    ui->pushButton_12->setVisible(false);
}

void KoResourceManagerWidget::initializeModels(bool first)
{
    for (int i=0;i<control->getNbModels();i++) {
        QTableView* currentTableView=tableView(i);
        currentTableView->setModel(control->getModel(i));
        currentTableView->resizeColumnsToContents();

        connect(currentTableView,SIGNAL(pressed(QModelIndex)),control->getModel(i),SLOT(resourceSelected(QModelIndex)));
        connect(currentTableView->horizontalHeader(),SIGNAL(sectionPressed(int)),control->getModel(i),SLOT(allSelected(int)));
        connect(currentTableView->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refreshDetails(QModelIndex)));

        if (first) {
            KoResourceTableDelegate *delegate=new KoResourceTableDelegate();
            connect(delegate,SIGNAL(renameEnded(QString)),this,SLOT(rename(QString)));
            currentTableView->setItemDelegate(delegate);
        }
    }
}

void KoResourceManagerWidget::initializeConnect()
{
    connect(control,SIGNAL(status(QString,int)),this,SLOT(status(QString,int)));

    connect(ui->pushButton_6,SIGNAL(clicked()),this,SLOT(showHide()));

    connect(ui->lineEdit_2,SIGNAL(editingFinished()),this,SLOT(setMeta()));
    connect(ui->lineEdit_3,SIGNAL(editingFinished()),this,SLOT(setMeta()));
    connect(ui->lineEdit_4,SIGNAL(editingFinished()),this,SLOT(setMeta()));
    connect(ui->dateEdit_2,SIGNAL(editingFinished()),this,SLOT(setMeta()));
    connect(ui->dateEdit,SIGNAL(editingFinished()),this,SLOT(setMeta()));

    connect(ui->pushButton_2,SIGNAL(clicked()),this,SLOT(createPack()));
    connect(ui->pushButton_7,SIGNAL(clicked()),this,SLOT(installPack()));
    connect(ui->pushButton_8,SIGNAL(clicked()),this,SLOT(uninstallPack()));
    connect(ui->pushButton_9,SIGNAL(clicked()),this,SLOT(deletePack()));

    connect(ui->actionCreate_Resources_Set,SIGNAL(triggered()),this,SLOT(createPack()));
    connect(ui->actionInstall,SIGNAL(triggered()),this,SLOT(installPack()));
    connect(ui->actionUninstall,SIGNAL(triggered()),this,SLOT(uninstallPack()));
    connect(ui->actionDelete,SIGNAL(triggered()),this,SLOT(deletePack()));
    connect(ui->actionExport,SIGNAL(triggered()),this,SLOT(exportBundle()));
    connect(ui->actionImport,SIGNAL(triggered()),this,SLOT(importBundle()));

    connect(ui->actionAll,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionName,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionFile,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));

    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(filterResourceTypes(int)));

    connect(ui->actionRename,SIGNAL(triggered()),this,SLOT(startRenaming()));
    connect(ui->lineEdit_5,SIGNAL(editingFinished()),this,SLOT(endRenaming()));

    connect(ui->pushButton_12,SIGNAL(clicked()),this,SLOT(saveMeta()));

    connect(ui->actionAbout_ResManager,SIGNAL(triggered()),this,SLOT(about()));

    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(close()));
    connect(ui->pushButton_3,SIGNAL(clicked()),this,SLOT(close()));

    connect(ui->toolButton,SIGNAL(clicked()),this,SLOT(startRenaming()));
    connect(ui->toolButton_2,SIGNAL(clicked()),this,SLOT(thumbnail()));

    connect(ui->pushButton_11,SIGNAL(clicked()),this,SLOT(removeTag()));

    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(refresh()));

    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tableViewChanged(int)));
}

void KoResourceManagerWidget::showHide()
{
    ui->widget_2->setVisible(!ui->widget_2->isVisible());
}

void KoResourceManagerWidget::status(QString text,int timeout)
{
    ui->statusbar->showMessage(text,timeout);
}

QTableView* KoResourceManagerWidget::tableView(int index)
{
    return dynamic_cast<QTableView*>(ui->tabWidget->widget(index)->layout()->itemAt(0)->widget());
}

//TODO Régler le pb de chgt de taille de la thumbnail
/*Slots*/

void KoResourceManagerWidget::about()
{
    QMessageBox msgBox;
    msgBox.about(this,"About Krita Resource Manager",
                 "This software has been designed by Reload Team and KDE developers :)");
}

void KoResourceManagerWidget::createPack()
{
    control->createPack(ui->tabWidget->currentIndex());
}

void KoResourceManagerWidget::installPack()
{
    control->install(ui->tabWidget->currentIndex());
}

void KoResourceManagerWidget::deletePack()
{
    control->remove(ui->tabWidget->currentIndex());
}

void KoResourceManagerWidget::uninstallPack()
{
    control->uninstall(ui->tabWidget->currentIndex());
}




void KoResourceManagerWidget::thumbnail()
{
    QTableView* currentTableView = tableView(ui->tabWidget->currentIndex());
    QModelIndex currentIndex = currentTableView->currentIndex();
    QString fileName = QFileDialog::getOpenFileName(0,
         tr("Import Thumbnail"), QProcessEnvironment::systemEnvironment().value("HOME").section(':',0,0), tr("Image Files (*.jpg)"));

    control->thumbnail(currentIndex,fileName,ui->tabWidget->currentIndex());
    currentTableView->reset();
    currentTableView->setCurrentIndex(currentIndex);
}


void KoResourceManagerWidget::setMeta()
{
    QObject *emetteur = sender();

    if (emetteur->inherits("QLineEdit")) {
        int currentIndex=ui->tabWidget->currentIndex();
        QTableView* currentTableView=tableView(currentIndex);
        QLineEdit* sender=(QLineEdit*)emetteur;
        if (emetteur==ui->lineEdit_2) {
            control->setMeta(currentTableView->currentIndex(),"Author",sender->text(),currentIndex);
            ui->lineEdit_2->blockSignals(true);
            ui->lineEdit_2->clearFocus();
            ui->lineEdit_2->blockSignals(false);
        }
        else if (emetteur==ui->lineEdit_3) {
            control->setMeta(currentTableView->currentIndex(),"License",sender->text(),currentIndex);
            ui->lineEdit_3->blockSignals(true);
            ui->lineEdit_3->clearFocus();
            ui->lineEdit_3->blockSignals(false);
        }
        else if (emetteur==ui->lineEdit_4) {
            control->setMeta(currentTableView->currentIndex(),"Website",sender->text(),currentIndex);
            ui->lineEdit_4->blockSignals(true);
            ui->lineEdit_4->clearFocus();
            ui->lineEdit_4->blockSignals(false);
        }
    }
}

void KoResourceManagerWidget::startRenaming()
{
    ui->statusbar->showMessage("Renaming...");
    ui->lineEdit_5->blockSignals(false);
    resourceNameLabel->setVisible(false);
    ui->toolButton->setVisible(false);

    if (ui->toolButton_2->isVisible()) {
        ui->toolButton_2->setVisible(false);
    }

    ui->lineEdit_5->setVisible(true);
    ui->lineEdit_5->setFocus();
}

void KoResourceManagerWidget::endRenaming()
{
    QTableView* currentTableView=tableView(ui->tabWidget->currentIndex());
    QModelIndex currentIndex=currentTableView->currentIndex();
    ui->lineEdit_5->blockSignals(true);

    QString newFileName=ui->lineEdit_5->text();
    QString extension=resourceNameLabel->text().section('.',1);
    if (!extension.isEmpty()) {
         newFileName+="."+extension;
    }

    if (control->rename(currentIndex,newFileName,ui->tabWidget->currentIndex())) {
        resourceNameLabel->setText(newFileName);
        currentTableView->reset();
    }
    else {
        ui->lineEdit_5->setText(resourceNameLabel->text().section('.',0,0));
    }

    currentTableView->setCurrentIndex(currentIndex);
    resourceNameLabel->setVisible(true);
    ui->toolButton->setVisible(true);
    ui->lineEdit_5->setVisible(false);
    ui->statusbar->showMessage("Resource renamed successfully...",3000);
}

void KoResourceManagerWidget::rename(QString newName)
{
    ui->statusbar->showMessage("Renaming...");

    QTableView* currentTableView=tableView(ui->tabWidget->currentIndex());
    QModelIndex currentIndex=currentTableView->currentIndex();

    QString newFileName=newName+"."+resourceNameLabel->text().section('.',1);

    if (control->rename(currentIndex,newFileName,ui->tabWidget->currentIndex())) {
        resourceNameLabel->setText(newFileName);
        currentTableView->reset();
        ui->statusbar->showMessage("Resource renamed successfully...",3000);
    }
}

void KoResourceManagerWidget::removeTag(){
    if (ui->listWidget->selectedItems().size()!=1) {
        ui->statusbar->showMessage("No tag selected in the above list...Tag removing aborted.",3000);
    }
    else {
        ui->statusbar->showMessage("Removing tag from resource...");
        QString tagName=ui->listWidget->selectedItems().at(0)->data(Qt::DisplayRole).toString();
        KoResourceTableModel *currentModel=control->getModel(ui->tabWidget->currentIndex());
        QTableView *currentTableView = tableView(ui->tabWidget->currentIndex());
        KoResource* currentResource = currentModel->getResourceFromIndex(currentTableView->currentIndex());

        currentModel->deleteTag(currentResource,tagName);

        if (tagMan->currentTag()==tagName) {
            currentModel->hideResource(currentResource);
            refreshDetails(currentModel->index(-1));
        }
        else {
            refreshDetails(currentTableView->currentIndex());
        }

        currentTableView->reset();
        ui->statusbar->showMessage("Tag removed successfully",3000);
    }
}

void KoResourceManagerWidget::filterFieldSelected(bool value)
{
    ui->statusbar->showMessage("Configuring filtering tool...");

    QAction *emetteur = (QAction*)sender();

    if (emetteur==ui->actionAll) {
        if (value) {
            ui->actionAll->setChecked(true);
            ui->actionName->setChecked(true);
            ui->actionFile->setChecked(true);
            control->configureFilters(0,true);
        }
        else {
            ui->actionAll->setChecked(false);
            ui->actionName->setChecked(true);
            ui->actionFile->setChecked(false);
            control->configureFilters(0,false);
        }
        ui->statusbar->showMessage("Filters updated...",3000);
    }
    else {
        if (emetteur==ui->actionName){
            if (!ui->actionFile->isChecked()) {
                ui->actionName->setChecked(true);
                ui->statusbar->showMessage("Error : must have at least one filter criterium...",3000);
            }
            else {
                control->configureFilters(1,value);
                ui->statusbar->showMessage("Filters updated...",3000);
            }
        }
        else if (emetteur==ui->actionFile) {
            if (!ui->actionName->isChecked()) {
                ui->actionFile->setChecked(true);
                ui->statusbar->showMessage("Error : must have at least one filter criterium...",3000);
            }
            else {
                control->configureFilters(2,value);
                ui->statusbar->showMessage("Filters updated...",3000);
            }
        }

        ui->actionAll->blockSignals(true);
        ui->actionAll->setChecked(ui->actionName->isChecked() && ui->actionFile->isChecked());
        ui->actionAll->blockSignals(false);
    }
}

void KoResourceManagerWidget::filterResourceTypes(int index)
{
    int currentTab=ui->tabWidget->currentIndex();
    KoResourceTableModel* model;

    ui->statusbar->showMessage("Filtering...",3000);

    control->filterResourceTypes(index);
    initializeModels();

    model=control->getModel(currentTab);

    if (model->resourcesCount()==0) {
        ui->widget_2->setEnabled(false);
    }
    else {
        QModelIndex index=model->index(0);

        ui->widget_2->setEnabled(true);
        tableView(currentTab)->setCurrentIndex(index);
        refreshDetails(index);
    }
    refreshTaggingManager();

    ui->statusbar->showMessage("Resource lists updated",3000);
}



void KoResourceManagerWidget::refreshDetails(QModelIndex newIndex)
{
    if (newIndex.row()==-1) {
        ui->widget_2->setEnabled(false);
        return;
    }
    else {
        ui->widget_2->setEnabled(true);
    }

    KoResource* currentResource= control->getModel(ui->tabWidget->currentIndex())->currentlyVisibleResources().at(newIndex.row());
    KoResourceBundle* currentBundle=dynamic_cast<KoResourceBundle*>(currentResource);
    QString currentDate;

    if (firstRefresh) {
        ui->scrollArea->setVisible(true);
        ui->label->setVisible(true);
        ui->toolButton->setVisible(true);

        QFont labelFont("Arial Black",14,QFont::Bold);
        labelFont.setWeight(75);
        resourceNameLabel->setFont(labelFont);
        connect(resourceNameLabel,SIGNAL(clicked()),this,SLOT(startRenaming()));

        firstRefresh=false;
        tagMan->showTaggingBar(true,true);
        ui->widget_2->layout()->addWidget(tagMan->tagChooserWidget());
        ui->dateEdit->setDisplayFormat("dd/MM/yyyy");
        ui->dateEdit_2->setDisplayFormat("dd/MM/yyyy");
    }

    //Name

    resourceNameLabel->setText(currentResource->shortFilename());
    ui->lineEdit_5->setText(currentResource->shortFilename().section('.',0,0));

    resourceNameLabel->setVisible(true);
    ui->lineEdit_5->setVisible(false);

    //Tags

    ui->listWidget->clear();
    ui->listWidget->addItems(control->getModel(ui->tabWidget->currentIndex())->assignedTagsList(currentResource));
    ui->pushButton_11->setEnabled(!ui->listWidget->count()==0);

    //Overview

    if (currentResource->image().isNull()) {
        ui->label->clear();
    }
    else {
        ui->label->setPixmap(QPixmap::fromImage(currentResource->image()).scaled(1000,150,Qt::KeepAspectRatio));
    }
    ui->toolButton_2->setVisible(false);

    if (currentBundle!=0) {
        ui->toolButton_2->setVisible(true);

        ui->label_3->setVisible(true);
        ui->label_4->setVisible(true);
        ui->label_5->setVisible(true);

        ui->lineEdit_2->setText(currentBundle->getAuthor());
        ui->lineEdit_2->setVisible(true);
        ui->lineEdit_3->setText(currentBundle->getLicense());
        ui->lineEdit_3->setVisible(true);
        ui->lineEdit_4->setText(currentBundle->getWebSite());
        ui->lineEdit_4->setVisible(true);

        currentDate=currentBundle->getCreated();
        if (currentDate.isEmpty()) {
            ui->label_6->setVisible(false);
            ui->dateEdit->setVisible(false);
        }
        else {
            ui->label_6->setVisible(true);
            ui->dateEdit->setVisible(true);
            ui->dateEdit->setDate(QDate::fromString(currentDate,"dd/MM/yyyy"));
        }

        currentDate=currentBundle->getUpdated();
        if (currentDate.isEmpty()) {
            ui->label_7->setVisible(false);
            ui->dateEdit_2->setVisible(false);
        }
        else {
            ui->label_7->setVisible(true);
            ui->dateEdit_2->setVisible(true);
            ui->dateEdit_2->setDate(QDate::fromString(currentDate,"dd/MM/yyyy"));
        }
        ui->pushButton_12->setVisible(true);
    }
    else {
        ui->lineEdit_2->setVisible(false);
        ui->lineEdit_3->setVisible(false);
        ui->lineEdit_4->setVisible(false);
        ui->dateEdit->setVisible(false);
        ui->dateEdit_2->setVisible(false);
        ui->label_3->setVisible(false);
        ui->label_4->setVisible(false);
        ui->label_5->setVisible(false);
        ui->label_6->setVisible(false);
        ui->label_7->setVisible(false);
        ui->pushButton_12->setVisible(false);
    }
}

void KoResourceManagerWidget::saveMeta()
{
    ui->statusbar->showMessage("Saving metadata...",3000);
    int currentTabIndex=ui->tabWidget->currentIndex();
    control->saveMeta(tableView(currentTabIndex)->currentIndex(),currentTabIndex);
    ui->statusbar->showMessage("Metadata saved successfully...",3000);
}

void KoResourceManagerWidget::refreshTaggingManager(int index)
{
    if (tagMan) {
        if (!tagMan->tagChooserWidget()->selectedTagIsReadOnly()) {
            control->refreshTaggingManager();
            tableView(index)->reset();
        }
        ui->widget_2->layout()->removeWidget(tagMan->tagChooserWidget());
        tagMan->showTaggingBar(true,false);
        delete tagMan;
    }

    tagMan=new KoResourceTaggingManager(control->getModel(index),ui->widget_2);
    tagMan->showTaggingBar(true,!firstRefresh);

    ui->gridLayout->addWidget(tagMan->tagFilterWidget(),0,1);
    ui->gridLayout->addWidget(ui->widget,0,2);
    ui->widget_2->layout()->addWidget(tagMan->tagChooserWidget());
}

void KoResourceManagerWidget::tableViewChanged(int index)
{
    refreshTaggingManager(index);

    QTableView *newView=tableView(index);
    newView->setFocus();
    newView->setCurrentIndex(newView->currentIndex());
    refreshDetails(newView->currentIndex());

    if (index==KoResourceTableModel::Available){
        ui->pushButton_7->setEnabled(true);
        ui->pushButton_8->setEnabled(false);
    }
    else if (index==KoResourceTableModel::Installed){
        ui->pushButton_7->setEnabled(false);
        ui->pushButton_8->setEnabled(true);
    }
}

void KoResourceManagerWidget::exportBundle()
{
    control->exportBundle(ui->tabWidget->currentIndex());
}

//TODO Penser à une fonction toBundleView pour aller direct sur la vue bundle
//qd une modif est effectuée dessus
void KoResourceManagerWidget::importBundle()
{
    if(control->importBundle()) {
        if(ui->tabWidget->currentIndex()!=0) {
            ui->tabWidget->setCurrentIndex(0);
        }

        ui->comboBox->setCurrentIndex(1);
    }
}

void KoResourceManagerWidget::refresh()
{
    filterResourceTypes(ui->comboBox->currentIndex());
}

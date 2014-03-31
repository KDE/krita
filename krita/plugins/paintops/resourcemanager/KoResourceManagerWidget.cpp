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

#include <iostream>
using namespace std;

//TODO Paramètre de ManagerControl à modifier si on veut rajouter des onglets
KoResourceManagerWidget::KoResourceManagerWidget(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::KoResourceManagerWidget),control(new KoResourceManagerControl(2)),tagMan(0),firstRefresh(true)
{
    ui->setupUi(this);

    initializeModel();
    initializeConnect();
    initializeTitle();
    initializeFilterMenu();
    refreshTaggingManager();

    ui->tabWidget->removeTab(2);
    ui->tabWidget->removeTab(2);
    ui->tabWidget->removeTab(2);

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
    liste.append(ui->actionTag);
    liste.append(ui->actionAuthor);
    liste.append(ui->actionLicense);

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
    ui->horizontalLayout_4->addWidget(resourceNameLabel);
    ui->lineEdit_5->setVisible(false);
    ui->scrollArea->setVisible(false);
    ui->label->setVisible(false);
    ui->pushButton_12->setVisible(false);
}

void KoResourceManagerWidget::initializeModel()
{   
    for (int i=0;i<control->getNbModels();i++) {
        QTableView* currentTableView=tableView(i);
        currentTableView->setModel(control->getModel(i));
        currentTableView->resizeColumnsToContents();
    }
}

//TODO Lors de l'appui sur entrée changer le focus ou autre
void KoResourceManagerWidget::initializeConnect()
{
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

    connect(ui->actionAll,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionName,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionTag,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionAuthor,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionLicense,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));

    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(filterResourceTypes(int)));

    connect(ui->actionRename,SIGNAL(triggered()),this,SLOT(startRenaming()));
    connect(ui->lineEdit_5,SIGNAL(editingFinished()),this,SLOT(endRenaming()));

    connect(ui->pushButton_12,SIGNAL(clicked()),this,SLOT(saveMeta()));

    connect(ui->actionAbout_ResManager,SIGNAL(triggered()),this,SLOT(about()));

    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(close()));
    connect(ui->pushButton_3,SIGNAL(clicked()),this,SLOT(close()));

    connectTables();
}

void KoResourceManagerWidget::connectTables()
{
    for (int i=0;i<control->getNbModels();i++) {
        QTableView* currentTableView=tableView(i);
        connect(currentTableView->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refreshDetails(QModelIndex)));
        connect(currentTableView,SIGNAL(pressed(QModelIndex)),control->getModel(i),SLOT(resourceSelected(QModelIndex)));
        connect(currentTableView->horizontalHeader(),SIGNAL(sectionPressed(int)),control->getModel(i),SLOT(allSelected(int)));
        KoResourceTableDelegate *delegate=new KoResourceTableDelegate();
        connect(delegate,SIGNAL(renameEnded(QString)),this,SLOT(rename(QString)));
        currentTableView->setItemDelegate(delegate);

    }

    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tableViewChanged(int)));
}

QTableView* KoResourceManagerWidget::tableView(int index)
{
    return dynamic_cast<QTableView*>(ui->tabWidget->widget(index)->layout()->itemAt(0)->widget());
}

/*Slots*/

void KoResourceManagerWidget::about()
{
    control->about();
}

void KoResourceManagerWidget::createPack()
{
    control->createPack(ui->tabWidget->currentIndex());
}

void KoResourceManagerWidget::deletePack()
{
    control->modifySelected(2,ui->tabWidget->currentIndex());
}

void KoResourceManagerWidget::installPack()
{
    control->modifySelected(0,ui->tabWidget->currentIndex());
}

void KoResourceManagerWidget::uninstallPack()
{
    control->modifySelected(1,ui->tabWidget->currentIndex());
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
            ui->lineEdit_2->clearFocus();
        }
        else if (emetteur==ui->lineEdit_3) {
            control->setMeta(currentTableView->currentIndex(),"License",sender->text(),currentIndex);
            ui->lineEdit_3->clearFocus();
        }
        else if (emetteur==ui->lineEdit_4) {
            control->setMeta(currentTableView->currentIndex(),"Website",sender->text(),currentIndex);
            ui->lineEdit_4->clearFocus();
        }
    }
}

void KoResourceManagerWidget::startRenaming()
{
    ui->lineEdit_5->blockSignals(false);
    resourceNameLabel->setVisible(false);
    ui->lineEdit_5->setVisible(true);
    ui->lineEdit_5->setFocus();
}

void KoResourceManagerWidget::endRenaming()
{
    QTableView* currentTableView=tableView(ui->tabWidget->currentIndex());
    QModelIndex currentIndex=currentTableView->currentIndex();
    ui->lineEdit_5->blockSignals(true);

    QString newFileName=ui->lineEdit_5->text()+"."+resourceNameLabel->text().section('.',1);

    if (control->rename(currentIndex,newFileName,ui->tabWidget->currentIndex())) {
        resourceNameLabel->setText(newFileName);
        currentTableView->reset();
    }
    else {
        ui->lineEdit_5->setText(resourceNameLabel->text().section('.',0,0));
    }

    currentTableView->setCurrentIndex(currentIndex);
    resourceNameLabel->setVisible(true);
    ui->lineEdit_5->setVisible(false);
}

void KoResourceManagerWidget::rename(QString newName)
{
    QTableView* currentTableView=tableView(ui->tabWidget->currentIndex());
    QModelIndex currentIndex=currentTableView->currentIndex();

    QString newFileName=newName+"."+resourceNameLabel->text().section('.',1);

    if (control->rename(currentIndex,newFileName,ui->tabWidget->currentIndex())) {
        resourceNameLabel->setText(newFileName);
        currentTableView->reset();
    }
}

void KoResourceManagerWidget::filterFieldSelected(bool value)
{
    QAction *emetteur = (QAction*)sender();

    if (emetteur==ui->actionAll) {
        if (value) {
            ui->actionAll->setChecked(true);
            ui->actionName->setChecked(true);
            ui->actionTag->setChecked(true);
            ui->actionAuthor->setChecked(true);
            ui->actionLicense->setChecked(true);
        }
        else {
            ui->actionAll->setChecked(false);
            ui->actionName->setChecked(false);
            ui->actionTag->setChecked(false);
            ui->actionAuthor->setChecked(false);
            ui->actionLicense->setChecked(false);
        }
    }
    else if (!value) {
        ui->actionAll->blockSignals(true);
        ui->actionAll->setChecked(false);
        ui->actionAll->blockSignals(false);
    }
}

//TODO Modifier pour les modèles à partir de 2
void KoResourceManagerWidget::filterResourceTypes(int index)
{
    control->filterResourceTypes(index);

    for (int i=0;i<ui->tabWidget->count();i++){
        QTableView *currentTableView = tableView(i);
        if (i==1) {
            currentTableView->setModel(control->getModel(KoResourceTableModel::Installed));
        }
        else {
            currentTableView->setModel(control->getModel(KoResourceTableModel::Available));
        }
    }

    connectTables();
}

void KoResourceManagerWidget::showHide()
{
    ui->widget_2->setVisible(!ui->widget_2->isVisible());
}

void KoResourceManagerWidget::refreshDetails(QModelIndex newIndex)
{
    KoResource* currentResource= control->getModel(ui->tabWidget->currentIndex())->currentlyVisibleResources().at(newIndex.row());
    KoResourceBundle* currentBundle=dynamic_cast<KoResourceBundle*>(currentResource);
    QString currentDate;

    if (firstRefresh) {
        ui->scrollArea->setVisible(true);
        ui->label->setVisible(true);

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

    //Overview

    if (currentResource->image().isNull()) {
        ui->label->clear();
    }
    else {
        ui->label->setPixmap(QPixmap::fromImage(currentResource->image()).scaled(1000,150,Qt::KeepAspectRatio));
    }

    if (currentBundle!=0) {
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

//TODO Ajouter une méta donnée pour le site web ou corriger le non remplacement de la nouvelle valeur
void KoResourceManagerWidget::saveMeta()
{
    int currentIndex=ui->tabWidget->currentIndex();
    control->saveMeta(tableView(currentIndex)->currentIndex(),currentIndex);
}

void KoResourceManagerWidget::refreshTaggingManager(int index)
{
    if (tagMan) {
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

//TODO Régler le problème lié à la sélection, le changement de types et le changement d'onglet
void KoResourceManagerWidget::tableViewChanged(int index)
{
    refreshTaggingManager(index);
    refreshDetails(tableView(ui->tabWidget->currentIndex())->currentIndex());
}

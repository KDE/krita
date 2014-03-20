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

#include <iostream>
using namespace std;

KoResourceManagerWidget::KoResourceManagerWidget(QWidget *parent) :
    QMainWindow(parent),control(new KoResourceManagerControl()),ui(new Ui::KoResourceManagerWidget)
{
    ui->setupUi(this);

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

    initializeModel();
    initializeConnect();

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

    ui->tableView->resizeColumnsToContents();
    ui->tableView_2->resizeColumnsToContents();
    ui->tableView_3->resizeColumnsToContents();
    ui->tableView_4->resizeColumnsToContents();
    ui->tableView_5->resizeColumnsToContents();

    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(1);

    firstRefresh=true;

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
}

void KoResourceManagerWidget::initializeModel()
{   
    KoResourceTableModel *model=control->getModel();

    ui->tableView->setModel(model);
    ui->tableView_2->setModel(model);
    ui->tableView_3->setModel(model);
    ui->tableView_4->setModel(model);
    ui->tableView_5->setModel(model);
}

//TODO Résoudre le problème des editingFinished
void KoResourceManagerWidget::initializeConnect()
{
    connect(ui->pushButton_6,SIGNAL(clicked()),this,SLOT(showHide()));

    connect(ui->lineEdit_2,SIGNAL(editingFinished()),this,SLOT(setMeta()),Qt::DirectConnection);
    connect(ui->lineEdit_3,SIGNAL(editingFinished()),this,SLOT(setMeta()));
    connect(ui->lineEdit_4,SIGNAL(editingFinished()),this,SLOT(setMeta()));
    connect(ui->dateEdit_2,SIGNAL(editingFinished()),this,SLOT(setMeta()));
    connect(ui->dateEdit,SIGNAL(editingFinished()),this,SLOT(setMeta()));

    connect(ui->pushButton_7,SIGNAL(clicked()),this,SLOT(installPack()));
    connect(ui->pushButton_8,SIGNAL(clicked()),this,SLOT(uninstallPack()));
    connect(ui->pushButton_9,SIGNAL(clicked()),this,SLOT(deletePack()));

    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(refreshCurrentTable()));
    connect(ui->lineEdit,SIGNAL(textEdited(QString)),this,SLOT(refreshCurrentTable()));

    connect(ui->actionAll,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionName,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionTag,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionAuthor,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionLicense,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));

    connect(ui->actionInstall,SIGNAL(triggered()),this,SLOT(installPack()));
    connect(ui->actionUninstall,SIGNAL(triggered()),this,SLOT(uninstallPack()));
    connect(ui->actionDelete,SIGNAL(triggered()),this,SLOT(deletePack()));
    connect(ui->actionCreate_Resources_Set,SIGNAL(triggered()),this,SLOT(createPack()));

    connect(ui->actionRename,SIGNAL(triggered()),this,SLOT(startRenaming()));
    connect(ui->actionAbout_ResManager,SIGNAL(triggered()),this,SLOT(about()));

    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(close()));
    connect(ui->pushButton_3,SIGNAL(clicked()),this,SLOT(close()));

    connect(ui->pushButton_2,SIGNAL(clicked()),this,SLOT(createPack()));

    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(filterResourceTypes(int)));
    connect(ui->lineEdit_5,SIGNAL(editingFinished()),this,SLOT(endRenaming()));

    connectTables();
}

void KoResourceManagerWidget::connectTables()
{
    connect(ui->tableView->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refreshDetails(QModelIndex)));
    connect(ui->tableView_2->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refreshDetails(QModelIndex)));
    connect(ui->tableView_3->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refreshDetails(QModelIndex)));
    connect(ui->tableView_4->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refreshDetails(QModelIndex)));
    connect(ui->tableView_5->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refreshDetails(QModelIndex)));

    connect(ui->tableView,SIGNAL(pressed(QModelIndex)),control->getModel(),SLOT(resourceSelected(QModelIndex)),Qt::DirectConnection);
    connect(ui->tableView_2,SIGNAL(pressed(QModelIndex)),control->getModel(),SLOT(resourceSelected(QModelIndex)),Qt::DirectConnection);
    connect(ui->tableView_3,SIGNAL(pressed(QModelIndex)),control->getModel(),SLOT(resourceSelected(QModelIndex)),Qt::DirectConnection);
    connect(ui->tableView_4,SIGNAL(pressed(QModelIndex)),control->getModel(),SLOT(resourceSelected(QModelIndex)),Qt::DirectConnection);
    connect(ui->tableView_5,SIGNAL(pressed(QModelIndex)),control->getModel(),SLOT(resourceSelected(QModelIndex)),Qt::DirectConnection);

    connect(ui->tableView->horizontalHeader(),SIGNAL(sectionPressed(int)),control->getModel(),SLOT(allSelected(int)));
    connect(ui->tableView_2->horizontalHeader(),SIGNAL(sectionPressed(int)),control->getModel(),SLOT(allSelected(int)));
    connect(ui->tableView_3->horizontalHeader(),SIGNAL(sectionPressed(int)),control->getModel(),SLOT(allSelected(int)));
    connect(ui->tableView_4->horizontalHeader(),SIGNAL(sectionPressed(int)),control->getModel(),SLOT(allSelected(int)));
    connect(ui->tableView_5->horizontalHeader(),SIGNAL(sectionPressed(int)),control->getModel(),SLOT(allSelected(int)));
}

/*Slots*/

void KoResourceManagerWidget::about()
{
    control->about();
}

void KoResourceManagerWidget::createPack()
{
    control->createPack();
}

void KoResourceManagerWidget::deletePack()
{
    control->modifySelected(2);
}

void KoResourceManagerWidget::endRenaming()
{
    ui->lineEdit_5->blockSignals(true);

    QString newFileName=ui->lineEdit_5->text()+"."+resourceNameLabel->text().section('.',1);

    if (control->rename(ui->tableView->currentIndex(),newFileName)) {
        resourceNameLabel->setText(newFileName);
        ui->tableView->reset();
    }
    else {
        ui->lineEdit_5->setText(resourceNameLabel->text().section('.',0,0));
    }
    resourceNameLabel->setVisible(true);
    ui->lineEdit_5->setVisible(false);
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
        ui->actionAll->setChecked(false);
    }
}

void KoResourceManagerWidget::filterResourceTypes(int index)
{
    control->filterResourceTypes(index);
    ui->tableView->setModel(control->getModel());
    ui->tableView_2->setModel(control->getModel());
    ui->tableView_3->setModel(control->getModel());
    ui->tableView_4->setModel(control->getModel());
    ui->tableView_5->setModel(control->getModel());
    connectTables();
}

void KoResourceManagerWidget::installPack()
{
    control->modifySelected(0);
}

void KoResourceManagerWidget::refreshCurrentTable()
{

}

void KoResourceManagerWidget::refreshDetails(QModelIndex newIndex)
{
    KoResource* currentResource= control->getModel()->currentlyVisibleResources().at(newIndex.row());
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
    }

    //Name

    resourceNameLabel->setText(currentResource->shortFilename());
    ui->lineEdit_5->setText(currentResource->shortFilename().section('.',0,0));

    resourceNameLabel->setVisible(true);
    ui->lineEdit_5->setVisible(false);

    //Tags

    ui->listWidget->clear();
    ui->listWidget->addItems(control->getModel()->assignedTagsList(currentResource));

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
        //TODO Trouver pkoi ne s'affichent que les deux derniers chiffres de l'année
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
    }
}

void KoResourceManagerWidget::setMeta()
{
    QObject *emetteur = sender();

    if (emetteur->inherits("QLineEdit")) {
        QLineEdit* sender=(QLineEdit*)emetteur;
        if (emetteur==ui->lineEdit_2) {
            return control->setMeta(ui->tableView->currentIndex(),"Author",sender->text());
        }
        else if (emetteur==ui->lineEdit_3) {
            return control->setMeta(ui->tableView->currentIndex(),"License",sender->text());
        }
        else if (emetteur==ui->lineEdit_4) {
            return control->setMeta(ui->tableView->currentIndex(),"Website",sender->text());
        }
    }
}

void KoResourceManagerWidget::showHide()
{
    ui->widget_2->setVisible(!ui->widget_2->isVisible());
}

void KoResourceManagerWidget::startRenaming()
{
    ui->lineEdit_5->blockSignals(false);
    resourceNameLabel->setVisible(false);
    ui->lineEdit_5->setVisible(true);
}

void KoResourceManagerWidget::uninstallPack()
{
    control->modifySelected(1);
}



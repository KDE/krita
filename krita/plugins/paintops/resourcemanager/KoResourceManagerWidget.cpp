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

#include "KoResourceManagerWidget.h"
#include "ui_KoResourceManagerWidget.h"
#include "KoResourceServerProvider.h"
#include <QCheckBox>
#include <QWidgetAction>
#include <QSignalMapper>
#include "kis_brush_server.h"
#include "kis_resource_server_provider.h"
#include "kis_workspace_resource.h"
#include "kis_paintop_preset.h"
#include <iostream>
using namespace std;

KoResourceManagerWidget::KoResourceManagerWidget(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::KoResourceManagerWidget)
{
    control=new KoResourceManagerControl();
    ui->setupUi(this);
    initializeModel();
    initializeConnect();
    ui->actionAll->setChecked(true);

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
    delete model;
}

void KoResourceManagerWidget::initializeModel()
{
    QList<QAction*> liste;
    liste.append(ui->actionAll);
    liste.append(ui->actionName);
    liste.append(ui->actionTag);
    liste.append(ui->actionAuthor);
    liste.append(ui->actionLicense);

    buttonMenu=new QMenu();
    buttonMenu->addActions(liste);
    ui->pushButton_10->setMenu(buttonMenu);

    QList<KoAbstractResourceServerAdapter*> list2;
    list2.append(new KoResourceServerAdapter<KoAbstractGradient>(KoResourceServerProvider::instance()->gradientServer()));
    list2.append(new KoResourceServerAdapter<KoPattern>(KoResourceServerProvider::instance()->patternServer()));
    list2.append(new KoResourceServerAdapter<KisBrush>(KisBrushServer::instance()->brushServer()));
    list2.append(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer()));
    list2.append(new KoResourceServerAdapter<KisPaintOpPreset>(KisResourceServerProvider::instance()->paintOpPresetServer()));
    list2.append(new KoResourceServerAdapter<KisWorkspaceResource>(KisResourceServerProvider::instance()->workspaceServer()));

    KGlobal::mainComponent().dirs()->addResourceType("bundles", "data", "krita/bundles/");
    KoResourceServer<KoResourceBundle> *serv=new KoResourceServer<KoResourceBundle>("bundles","*.zip");
    serv->loadResources(serv->fileNames());
    list2.append(new KoResourceServerAdapter<KoResourceBundle>(serv));

    this->model=new KoResourceTableModel(list2);
    ui->tableView->setModel(model);
    ui->tableView_2->setModel(model);
    ui->tableView_3->setModel(model);
    ui->tableView_4->setModel(model);
    ui->tableView_5->setModel(model);
    ui->tableView->resizeColumnsToContents();
    ui->tableView_2->resizeColumnsToContents();
    ui->tableView_3->resizeColumnsToContents();
    ui->tableView_4->resizeColumnsToContents();
    ui->tableView_5->resizeColumnsToContents();
}

void KoResourceManagerWidget::initializeConnect()
{
    connect(ui->pushButton_6,SIGNAL(clicked()),this,SLOT(showHide()));

    connect(ui->lineEdit_2,SIGNAL(editingFinished()),this,SLOT(setMeta()));
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

    connect(ui->actionRename,SIGNAL(triggered()),this,SLOT(rename()));
    connect(ui->actionAbout_ResManager,SIGNAL(triggered()),this,SLOT(about()));

    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(close()));
    connect(ui->pushButton_3,SIGNAL(clicked()),this,SLOT(close()));

    connect(ui->pushButton_2,SIGNAL(clicked()),this,SLOT(createPack()));

    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(resourceTable(int)));

    connectTables();
    //createMiniature(*ui->label->pixmap());
}

void KoResourceManagerWidget::connectTables()
{
    connect(ui->tableView->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refresh(QModelIndex)));
    connect(ui->tableView_2->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refresh(QModelIndex)));
    connect(ui->tableView_3->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refresh(QModelIndex)));
    connect(ui->tableView_4->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refresh(QModelIndex)));
    connect(ui->tableView_5->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(refresh(QModelIndex)));

    connect(ui->tableView,SIGNAL(pressed(QModelIndex)),this,SLOT(resourceClicked(QModelIndex)),Qt::DirectConnection);
    connect(ui->tableView_2,SIGNAL(pressed(QModelIndex)),this,SLOT(resourceClicked(QModelIndex)),Qt::DirectConnection);
    connect(ui->tableView_3,SIGNAL(pressed(QModelIndex)),this,SLOT(resourceClicked(QModelIndex)),Qt::DirectConnection);
    connect(ui->tableView_4,SIGNAL(pressed(QModelIndex)),this,SLOT(resourceClicked(QModelIndex)),Qt::DirectConnection);
    connect(ui->tableView_5,SIGNAL(pressed(QModelIndex)),this,SLOT(resourceClicked(QModelIndex)),Qt::DirectConnection);

    connect(ui->tableView->horizontalHeader(),SIGNAL(sectionPressed(int)),model,SLOT(allSelected(int)));
    connect(ui->tableView_2->horizontalHeader(),SIGNAL(sectionPressed(int)),model,SLOT(allSelected(int)));
    connect(ui->tableView_3->horizontalHeader(),SIGNAL(sectionPressed(int)),model,SLOT(allSelected(int)));
    connect(ui->tableView_4->horizontalHeader(),SIGNAL(sectionPressed(int)),model,SLOT(allSelected(int)));
    connect(ui->tableView_5->horizontalHeader(),SIGNAL(sectionPressed(int)),model,SLOT(allSelected(int)));

    connect(this,SIGNAL(resourceWasSelected(QString)),model,SLOT(resourceSelected(QString)),Qt::DirectConnection);
}

void KoResourceManagerWidget::resourceTable(int index)
{
    KoResourceServer<KoResourceBundle> *serv;
    QList<KoAbstractResourceServerAdapter*> liste;

    switch (index) {
    case 0:
        liste.append(new KoResourceServerAdapter<KoAbstractGradient>(KoResourceServerProvider::instance()->gradientServer()));
        liste.append(new KoResourceServerAdapter<KoPattern>(KoResourceServerProvider::instance()->patternServer()));
        liste.append(new KoResourceServerAdapter<KisBrush>(KisBrushServer::instance()->brushServer()));
        liste.append(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer()));
        liste.append(new KoResourceServerAdapter<KisPaintOpPreset>(KisResourceServerProvider::instance()->paintOpPresetServer()));
        liste.append(new KoResourceServerAdapter<KisWorkspaceResource>(KisResourceServerProvider::instance()->workspaceServer()));
        serv=new KoResourceServer<KoResourceBundle>("bundles","*.zip");
        serv->loadResources(serv->fileNames());
        liste.append(new KoResourceServerAdapter<KoResourceBundle>(serv));
        this->model=new KoResourceTableModel(liste);
        ui->tableView->setModel(model);
        ui->tableView_2->setModel(model);
        ui->tableView_3->setModel(model);
        ui->tableView_4->setModel(model);
        ui->tableView_5->setModel(model);
        break;
    case 1:
        serv=new KoResourceServer<KoResourceBundle>("bundles","*.zip");
        serv->loadResources(serv->fileNames());
        liste.append(new KoResourceServerAdapter<KoResourceBundle>(serv));
        this->model=new KoResourceTableModel(liste);
        ui->tableView->setModel(model);
        ui->tableView_2->setModel(model);
        ui->tableView_3->setModel(model);
        ui->tableView_4->setModel(model);
        ui->tableView_5->setModel(model);
        break;
    case 2:
        //TODO A Vérifier et Corriger
        liste.append(new KoResourceServerAdapter<KisBrush>(KisBrushServer::instance()->brushServer()));
        this->model=new KoResourceTableModel(liste);
        ui->tableView->setModel(model);
        ui->tableView_2->setModel(model);
        ui->tableView_3->setModel(model);
        ui->tableView_4->setModel(model);
        ui->tableView_5->setModel(model);
        break;
    case 3:
        liste.append(new KoResourceServerAdapter<KoAbstractGradient>(KoResourceServerProvider::instance()->gradientServer()));
        this->model=new KoResourceTableModel(liste);
        ui->tableView->setModel(model);
        ui->tableView_2->setModel(model);
        ui->tableView_3->setModel(model);
        ui->tableView_4->setModel(model);
        ui->tableView_5->setModel(model);
        break;
    case 4:
        liste.append(new KoResourceServerAdapter<KisPaintOpPreset>(KisResourceServerProvider::instance()->paintOpPresetServer()));
        this->model=new KoResourceTableModel(liste);
        ui->tableView->setModel(model);
        ui->tableView_2->setModel(model);
        ui->tableView_3->setModel(model);
        ui->tableView_4->setModel(model);
        ui->tableView_5->setModel(model);
        break;
    case 5:
        liste.append(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer()));
        this->model=new KoResourceTableModel(liste);
        ui->tableView->setModel(model);
        ui->tableView_2->setModel(model);
        ui->tableView_3->setModel(model);
        ui->tableView_4->setModel(model);
        ui->tableView_5->setModel(model);
        break;
    case 6:
        liste.append(new KoResourceServerAdapter<KoPattern>(KoResourceServerProvider::instance()->patternServer()));
        this->model=new KoResourceTableModel(liste);
        ui->tableView->setModel(model);
        ui->tableView_2->setModel(model);
        ui->tableView_3->setModel(model);
        ui->tableView_4->setModel(model);
        ui->tableView_5->setModel(model);
        break;
    case 7:
        liste.append(new KoResourceServerAdapter<KisWorkspaceResource>(KisResourceServerProvider::instance()->workspaceServer()));
        this->model=new KoResourceTableModel(liste);
        ui->tableView->setModel(model);
        ui->tableView_2->setModel(model);
        ui->tableView_3->setModel(model);
        ui->tableView_4->setModel(model);
        ui->tableView_5->setModel(model);
        break;
    }
    model->clearSelected();
    connectTables();
}

void KoResourceManagerWidget::resourceClicked(QModelIndex targetIndex)
{
    if (targetIndex.column()==0) {
        emit resourceWasSelected(model->currentlyVisibleResources().at(targetIndex.row())->filename());
    }
}

void KoResourceManagerWidget::refresh(QModelIndex newIndex)
{
    KoResource* current=model->currentlyVisibleResources().at(newIndex.row());
    KoResourceBundle* prov=dynamic_cast<KoResourceBundle*>(current);

    //Name

    ui->label_2->setText(current->shortFilename());

    //Tags

    ui->listWidget->clear();
    ui->listWidget->addItems(model->assignedTagsList(current));

    //Overview

    if (current->image().isNull()) {
        ui->label->clear();
    }
    else {
        ui->label->setPixmap(QPixmap::fromImage(current->image()).scaled(1000,150,Qt::KeepAspectRatio));
    }

    if (prov!=0) {
        ui->label_3->setVisible(true);
        ui->label_4->setVisible(true);
        ui->label_5->setVisible(true);
        ui->label_6->setVisible(true);
        ui->label_7->setVisible(true);
        ui->lineEdit_2->setText(prov->getAuthor());
        ui->lineEdit_2->setVisible(true);
        ui->lineEdit_3->setText(prov->getLicense());
        ui->lineEdit_3->setVisible(true);
        ui->lineEdit_4->setText(prov->getWebSite());
        ui->lineEdit_4->setVisible(true);
        ui->dateEdit->setDate(QDate::fromString(prov->getCreated(),"dd/MM/yyyy"));
        ui->dateEdit->setVisible(true);
        ui->dateEdit_2->setDate(QDate::fromString(prov->getUpdated(),"dd/MM/yyyy"));
        ui->dateEdit_2->setVisible(true);
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

void KoResourceManagerWidget::showHide()
{
    ui->widget_2->setVisible(!ui->widget_2->isVisible());
}

void KoResourceManagerWidget::createMiniature(QPixmap pix)
{
    QLabel *label = new QLabel;
    label->setPixmap(pix.scaled(50,50,Qt::KeepAspectRatio));
}

void KoResourceManagerWidget::createPack()
{
    QList<QString> liste = model->getSelectedResource();
    if(!liste.empty()) control->createPack(liste);
}

void KoResourceManagerWidget::installPack()
{
    control->installPack(ui->label_2->text());

}

void KoResourceManagerWidget::uninstallPack()
{
    control->uninstallPack(ui->label_2->text());
}

void KoResourceManagerWidget::deletePack()
{
    control->deletePack(ui->label_2->text());
}

void KoResourceManagerWidget::refreshCurrentTable()
{
    control->refreshCurrentTable();
}

//TODO Décider comment renommer un paquet (Fenetre, tableau ...)
void KoResourceManagerWidget::rename()
{
    //TODO Récupérer les/la valeur(s) correspondante(s)
    control->rename(ui->label->text(),"New_Value");
}

void KoResourceManagerWidget::about()
{
    //TODO Vérifier si control doit intervenir
    control->about();
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

void KoResourceManagerWidget::setMeta()
{
    QObject *emetteur = sender();
    if(emetteur->inherits("QLineEdit")){
        QLineEdit* test=(QLineEdit*)emetteur;
        if(emetteur==ui->lineEdit_2)
            return control->setMeta(ui->label_2->text(),"Author",test->text());
        else if (emetteur==ui->lineEdit_3)
            return control->setMeta(ui->label_2->text(),"License",test->text());
        else if (emetteur==ui->lineEdit_4)
            return control->setMeta(ui->label_2->text(),"Website",test->text());
    }
    else if(emetteur->inherits("QDateEdit")){
        QDateEdit* test=(QDateEdit*)emetteur;
        if(emetteur==ui->dateEdit_2 && test->text()!=QString("01/01/00"))
            return control->setMeta(ui->label_2->text(),"Updated",test->text());
        else if (emetteur==ui->dateEdit && test->text()!=QString("01/01/00"))
            return control->setMeta(ui->label_2->text(),"Created",test->text());
    }
}


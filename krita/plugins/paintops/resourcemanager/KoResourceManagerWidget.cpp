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
#include <QCheckBox>
#include <QWidgetAction>

#include <KoResourceModel.h>

KoResourceManagerWidget::KoResourceManagerWidget(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::Manager)
{
    ui->setupUi(this);
    initializeModel();
    initializeConnect();
    ui->actionAll->setChecked(true);
}

KoResourceManagerWidget::~KoResourceManagerWidget()
{
    delete ui;
    delete model;
}

void KoResourceManagerWidget::initializeModel()
{
    KoResourceModel *resourceModel = new KoResourceModel(0);


    QList<QAction*> liste;
    liste.append(ui->actionAll);
    liste.append(ui->actionName);
    liste.append(ui->actionTag);
    liste.append(ui->actionAuthor);
    liste.append(ui->actionLicense);

    buttonMenu=new QMenu();
    buttonMenu->addActions(liste);
    ui->pushButton_10->setMenu(buttonMenu);

    this->model=new MyTableModel(0);
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

    connect(ui->comboBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(refreshCurrentTable()));
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(refreshCurrentTable()));
    connect(ui->lineEdit,SIGNAL(textEdited(QString)),this,SLOT(refreshCurrentTable()));

    connect(ui->actionAll,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionName,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionTag,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionAuthor,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));
    connect(ui->actionLicense,SIGNAL(toggled(bool)),this,SLOT(filterFieldSelected(bool)));

    connect(ui->actionRename,SIGNAL(triggered()),this,SLOT(rename()));
    connect(ui->actionAbout_ResManager,SIGNAL(triggered()),this,SLOT(about()));

    connect(ui->actionQuit,SIGNAL(toggled(bool)),this,SLOT(close()));
    connect(ui->pushButton_3,SIGNAL(clicked()),this,SLOT(close()));

    createMiniature(*ui->label->pixmap());
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
        if(emetteur==ui->dateEdit_2)
            return control->setMeta(ui->label_2->text(),"Updated",test->text());
        else if (emetteur==ui->dateEdit)
            return control->setMeta(ui->label_2->text(),"Created",test->text());
    }
}


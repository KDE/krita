#include "KoBundleCreationWidget.h"
#include "ui_KoBundleCreationWidget.h"

#include "KoXmlResourceBundleMeta.h"

#include <QProcessEnvironment>
#include <QFileInfo>

#include <iostream>
using namespace std;

KoBundleCreationWidget::KoBundleCreationWidget(KoXmlResourceBundleMeta* newMeta, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KoBundleCreationWidget)
{
    ui->setupUi(this);

    this->newMeta = newMeta;
    this->kritaPath = QProcessEnvironment::systemEnvironment().value("KDEDIRS").section(':',0,0)
                        +QString("/share/apps/krita/");

    initializeUI();
}

KoBundleCreationWidget::~KoBundleCreationWidget()
{
    delete ui;
}

void KoBundleCreationWidget::initializeUI()
{
    QString detailsStyleSheet=QString("QCheckBox { spacing: 5px; } QCheckBox::indicator { width: 13px; height: 13px; } ")
            +QString("QCheckBox::indicator:unchecked { image: url(")+kritaPath+QString("pics/arrow-down.png); } ")
            +QString("QCheckBox::indicator:checked { image: url(")+kritaPath+QString("pics/arrow-right.png); } ");

    ui->detailsBox->setStyleSheet(detailsStyleSheet);

    this->resize(450,10);
    ui->metaData->setVisible(false);
    ui->detailsBox->setVisible(true);

    connect(ui->detailsBox,SIGNAL(clicked()),this,SLOT(showHide()));
    connect(ui->okButton,SIGNAL(clicked()),this,SLOT(createBundle()));
    connect(ui->cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
}


void KoBundleCreationWidget::showHide()
{
    ui->metaData->setVisible(!ui->metaData->isVisible());
    if (!ui->metaData->isVisible()) {
        this->adjustSize();
        this->resize(450,this->sizeHint().height());
    }
    else {
        this->resize(450,200);
    }
}

//TODO Vérifier la présence de caractères invalides dans le nom du paquet (exemple : *"')
//Même s'ils semblent acceptés par le système
void KoBundleCreationWidget::createBundle()
{
    QString name = ui->editBundleName->text().remove(" ");

    if (name.isEmpty()) {
        ui->editBundleName->setStyleSheet(QString(" border: 1px solid red"));
        emit status("Empty bundle name...");
    }
    else {
        QFileInfo fileInfo(kritaPath+"bundles/"+name+".zip");

        if (fileInfo.exists()) {
            ui->editBundleName->setStyleSheet("border: 1px solid red");
            emit status("Bundle already exists : choose another name...");
        }
        else {
            newMeta->setMeta(name,ui->editAuthor->text(),ui->editLicense->text(),
                                ui->editWebsite->text(),ui->editDescription->text());
            accept();
        }
    }
}



#include "KoBundleCreationWidget.h"
#include "ui_KoBundleCreationWidget.h"

#include "KoXmlResourceBundleMeta.h"

#include <QProcessEnvironment>
#include <QFileInfo>

#include <iostream>
using namespace std;

KoBundleCreationWidget::KoBundleCreationWidget(KoXmlResourceBundleMeta* newMeta, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::KoBundleCreationWidget)
{
    m_ui->setupUi(this);

    this->m_newMeta = newMeta;
    this->m_kritaPath = QProcessEnvironment::systemEnvironment().value("KDEDIRS").section(':',0,0)
                        +QString("/share/apps/krita/");

    initializeUI();
}

KoBundleCreationWidget::~KoBundleCreationWidget()
{
    delete m_ui;
}

void KoBundleCreationWidget::initializeUI()
{
    QString detailsStyleSheet=QString("QCheckBox { spacing: 5px; } QCheckBox::indicator { width: 13px; height: 13px; } ")
            +QString("QCheckBox::indicator:unchecked { image: url(")+m_kritaPath+QString("pics/arrow-down.png); } ")
            +QString("QCheckBox::indicator:checked { image: url(")+m_kritaPath+QString("pics/arrow-right.png); } ");

    m_ui->detailsBox->setStyleSheet(detailsStyleSheet);

    this->resize(450,10);
    m_ui->metaData->setVisible(false);
    m_ui->detailsBox->setVisible(true);

    connect(m_ui->detailsBox,SIGNAL(clicked()),this,SLOT(showHide()));
    connect(m_ui->okButton,SIGNAL(clicked()),this,SLOT(createBundle()));
    connect(m_ui->cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
}


void KoBundleCreationWidget::showHide()
{
    m_ui->metaData->setVisible(!m_ui->metaData->isVisible());
    if (!m_ui->metaData->isVisible()) {
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
    QString name = m_ui->editBundleName->text().remove(" ");

    if (name.isEmpty()) {
        m_ui->editBundleName->setStyleSheet(QString(" border: 1px solid red"));
        emit status("Empty bundle name...");
    }
    else {
        QFileInfo fileInfo(m_kritaPath+"bundles/"+name+".zip");

        if (fileInfo.exists()) {
            m_ui->editBundleName->setStyleSheet("border: 1px solid red");
            emit status("Bundle already exists : choose another name...");
        }
        else {
            m_newMeta->setMeta(name,m_ui->editAuthor->text(),m_ui->editLicense->text(),
                                m_ui->editWebsite->text(),m_ui->editDescription->text());
            accept();
        }
    }
}



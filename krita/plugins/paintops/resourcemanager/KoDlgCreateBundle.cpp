#include "KoDlgCreateBundle.h"

#include "ui_wdg_dlgcreatebundle.h"

#include "KoXmlResourceBundleMeta.h"
#include <KoDocumentInfo.h>

#include <QProcessEnvironment>
#include <QFileInfo>
#include <QMessageBox>

#include <kis_config.h>

#include <resourcemanager.h>

KoDlgCreateBundle::KoDlgCreateBundle(QWidget *parent)
    : KDialog(parent)
    , m_ui(new Ui::WdgDlgCreateBundle)
{
    m_page = new QWidget();
    m_ui->setupUi(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    KoDocumentInfo info;
    info.updateParameters();

    KisConfig cfg;

    m_ui->editAuthor->setText(cfg.readEntry<QString>("BundleAuthorName", info.authorInfo("creator")));
    m_ui->editEmail->setText(cfg.readEntry<QString>("BundleAuthorEmail", info.authorInfo("email")));
    m_ui->editWebsite->setText(cfg.readEntry<QString>("BundleWebsite", "http://"));
    m_ui->editLicense->setText(cfg.readEntry<QString>("BunleLicense", "CC-BY-SA"));
}

KoDlgCreateBundle::~KoDlgCreateBundle()
{
    delete m_ui;
}

QString KoDlgCreateBundle::bundleName() const
{
    return m_ui->editBundleName->text().replace(" ", "_");
}

QString KoDlgCreateBundle::authorName() const
{
    return m_ui->editAuthor->text();
}

QString KoDlgCreateBundle::email() const
{
    return m_ui->editEmail->text();
}

QString KoDlgCreateBundle::website() const
{
    return m_ui->editWebsite->text();
}

QString KoDlgCreateBundle::license() const
{
    return m_ui->editLicense->text();
}

QString KoDlgCreateBundle::description() const
{
    return m_ui->editDescription->document()->toPlainText();
}


//TODO Vérifier la présence de caractères invalides dans le nom du paquet (exemple : *"')
//Même s'ils semblent acceptés par le système
void KoDlgCreateBundle::accept()
{
    QString name = m_ui->editBundleName->text().remove(" ");

    if (name.isEmpty()) {
        m_ui->editBundleName->setStyleSheet(QString(" border: 1px solid red"));
        QMessageBox::warning(this, "Krita", i18n("The resource bundle name cannot be empty."));
        return;
    }
    else {

        KoResourceServer<KoResourceBundle> *rServer = ResourceBundleServerProvider::instance()->resourceBundleServer();
        QFileInfo fileInfo(rServer->saveLocation() + name + ".bundle");

        if (fileInfo.exists()) {
            m_ui->editBundleName->setStyleSheet("border: 1px solid red");
            QMessageBox::warning(this, "Krita", i18n("A bundle with this name already exists."));
            return;
        }
        else {
            KisConfig cfg;
            cfg.writeEntry<QString>("BundleAuthorName", m_ui->editAuthor->text());
            cfg.writeEntry<QString>("BundleAuthorEmail", m_ui->editEmail->text());
            cfg.writeEntry<QString>("BundleWebsite", m_ui->editWebsite->text());
            cfg.writeEntry<QString>("BunleLicense", m_ui->editLicense->text());
            KDialog::accept();
        }
    }
}



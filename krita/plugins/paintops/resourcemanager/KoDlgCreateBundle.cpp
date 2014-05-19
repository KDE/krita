#include "KoDlgCreateBundle.h"

#include "ui_wdg_dlgcreatebundle.h"

#include <QProcessEnvironment>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices>
#include <QGridLayout>
#include <QTableWidget>
#include <QPainter>

#include <KoFilterManager.h>
#include <KoDocumentInfo.h>
#include <KoFileDialog.h>
#include <KoIcon.h>
#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <kis_resource_server_provider.h>
#include <kis_workspace_resource.h>
#include <kis_paintop_preset.h>
#include <kis_brush_server.h>

#include <kis_config.h>

#define ICON_SIZE 48

KoDlgCreateBundle::KoDlgCreateBundle(QWidget *parent)
    : KDialog(parent)
    , m_ui(new Ui::WdgDlgCreateBundle)
{
    setCaption(i18n("Create Resource Bundle"));

    m_page = new QWidget();
    m_ui->setupUi(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    connect(m_ui->bnSelectSaveLocation, SIGNAL(clicked()), SLOT(selectSaveLocation()));

    KoDocumentInfo info;
    info.updateParameters();

    KisConfig cfg;

    m_ui->editAuthor->setText(cfg.readEntry<QString>("BundleAuthorName", info.authorInfo("creator")));
    m_ui->editEmail->setText(cfg.readEntry<QString>("BundleAuthorEmail", info.authorInfo("email")));
    m_ui->editWebsite->setText(cfg.readEntry<QString>("BundleWebsite", "http://"));
    m_ui->editLicense->setText(cfg.readEntry<QString>("BundleLicense", "CC-BY-SA"));
    m_ui->lblSaveLocation->setText(cfg.readEntry<QString>("BundleExportLocation", QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));

    m_ui->bnAdd->setIcon(koIcon("arrow-right"));
    connect(m_ui->bnAdd, SIGNAL(clicked()), SLOT(addSelected()));

    m_ui->bnRemove->setIcon(koIcon("arrow-left"));
    connect(m_ui->bnRemove, SIGNAL(clicked()), SLOT(removeSelected()));

    m_ui->cmbResourceTypes->addItem(i18n("Brushes"), QString("brushes"));
    m_ui->cmbResourceTypes->addItem(i18n("Brush Presets"), QString("presets"));
    m_ui->cmbResourceTypes->addItem(i18n("Gradients"), QString("gradients"));
    m_ui->cmbResourceTypes->addItem(i18n("Patterns"), QString("patterns"));
    m_ui->cmbResourceTypes->addItem(i18n("Palettes"), QString("palettes"));
    m_ui->cmbResourceTypes->addItem(i18n("Workspaces"), QString("workspaces"));
    connect(m_ui->cmbResourceTypes, SIGNAL(activated(int)), SLOT(resourceTypeSelected(int)));

    m_ui->tableAvailable->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_ui->tableAvailable->setSelectionMode(QAbstractItemView::MultiSelection);
    m_ui->tableSelected->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_ui->tableSelected->setSelectionMode(QAbstractItemView::MultiSelection);

    connect(m_ui->bnGetPreview, SIGNAL(clicked()), SLOT(getPreviewImage()));

    resourceTypeSelected(0);
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

QString KoDlgCreateBundle::saveLocation() const
{
    return m_ui->lblSaveLocation->text();
}

QString KoDlgCreateBundle::previewImage() const
{
    return m_previewImage;
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
        QFileInfo fileInfo(m_ui->lblSaveLocation->text() + "/" + name + ".bundle");

        if (fileInfo.exists()) {
            m_ui->editBundleName->setStyleSheet("border: 1px solid red");
            QMessageBox::warning(this, "Krita", i18n("A bundle with this name already exists."));
            return;
        }
        else {
            KisConfig cfg;
            cfg.writeEntry<QString>("BunleExportLocation", m_ui->lblSaveLocation->text());
            cfg.writeEntry<QString>("BundleAuthorName", m_ui->editAuthor->text());
            cfg.writeEntry<QString>("BundleAuthorEmail", m_ui->editEmail->text());
            cfg.writeEntry<QString>("BundleWebsite", m_ui->editWebsite->text());
            cfg.writeEntry<QString>("BundleLicense", m_ui->editLicense->text());
            KDialog::accept();
        }
    }
}

void KoDlgCreateBundle::selectSaveLocation()
{
    KoFileDialog dlg(this, KoFileDialog::OpenDirectory, "resourcebundlesavelocation");
    dlg.setDefaultDir(m_ui->lblSaveLocation->text());
    dlg.setCaption(i18n("Select a directory to save the bundle"));
    QString location = dlg.url();
    m_ui->lblSaveLocation->setText(location);
}

void KoDlgCreateBundle::addSelected()
{
    foreach(QListWidgetItem *item, m_ui->tableAvailable->selectedItems()) {
        m_ui->tableSelected->addItem(m_ui->tableAvailable->takeItem(m_ui->tableAvailable->row(item)));
        QString resourceType = m_ui->cmbResourceTypes->itemData(m_ui->cmbResourceTypes->currentIndex()).toString();
        if (resourceType == "brushes") {
            m_selectedBrushes.append(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "presets") {
            m_selectedPresets.append(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "gradients") {
            m_selectedGradients.append(item->data(Qt::UserRole).toString());

        }
        else if (resourceType == "patterns") {
            m_selectedPatterns.append(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "palettes") {
            m_selectedPalettes.append(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "workspaces") {
            m_selectedWorkspaces.append(item->data(Qt::UserRole).toString());
        }
    }
}

void KoDlgCreateBundle::removeSelected()
{
    foreach(QListWidgetItem *item, m_ui->tableSelected->selectedItems()) {
        m_ui->tableAvailable->addItem(m_ui->tableSelected->takeItem(m_ui->tableSelected->row(item)));
        QString resourceType = m_ui->cmbResourceTypes->itemData(m_ui->cmbResourceTypes->currentIndex()).toString();
        if (resourceType == "brushes") {
            m_selectedBrushes.removeAll(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "presets") {
            m_selectedPresets.removeAll(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "gradients") {
            m_selectedGradients.removeAll(item->data(Qt::UserRole).toString());

        }
        else if (resourceType == "patterns") {
            m_selectedPatterns.removeAll(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "palettes") {
            m_selectedPalettes.removeAll(item->data(Qt::UserRole).toString());
        }
        else if (resourceType == "workspaces") {
            m_selectedWorkspaces.removeAll(item->data(Qt::UserRole).toString());
        }
    }
}

QPixmap imageToIcon(const QImage &img) {
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    pixmap.fill();
    QImage scaled = img.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    int x = (ICON_SIZE - scaled.width()) / 2;
    int y = (ICON_SIZE - scaled.height()) / 2;
    QPainter gc(&pixmap);
    gc.drawImage(x, y, scaled);
    gc.end();
    return pixmap;
}

void KoDlgCreateBundle::resourceTypeSelected(int idx)
{
    QString resourceType = m_ui->cmbResourceTypes->itemData(idx).toString();

    m_ui->tableAvailable->clear();
    m_ui->tableSelected->clear();

    if (resourceType == "brushes") {
        KoResourceServer<KisBrush>* server = KisBrushServer::instance()->brushServer();
        foreach(KoResource *res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedBrushes.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == "presets") {
        KoResourceServer<KisPaintOpPreset>* server = KisResourceServerProvider::instance()->paintOpPresetServer();
        foreach(KoResource *res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedBrushes.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == "gradients") {
        KoResourceServer<KoAbstractGradient>* server = KoResourceServerProvider::instance()->gradientServer();
        foreach(KoResource *res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedBrushes.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == "patterns") {
        KoResourceServer<KoPattern>* server = KoResourceServerProvider::instance()->patternServer();
        foreach(KoResource *res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedBrushes.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == "palettes") {
        KoResourceServer<KoColorSet>* server = KoResourceServerProvider::instance()->paletteServer();
        foreach(KoResource *res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedBrushes.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
    else if (resourceType == "workspaces") {
        KoResourceServer<KoColorSet>* server = KoResourceServerProvider::instance()->paletteServer();
        foreach(KoResource *res, server->resources()) {
            QListWidgetItem *item = new QListWidgetItem(imageToIcon(res->image()), res->name());
            item->setData(Qt::UserRole, res->shortFilename());

            if (m_selectedBrushes.contains(res->shortFilename())) {
                m_ui->tableSelected->addItem(item);
            }
            else {
                m_ui->tableAvailable->addItem(item);
            }
        }
    }
}

void KoDlgCreateBundle::getPreviewImage()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "BundlePreviewImage");
    dialog.setCaption(i18n("Select file to use as dynamic file layer."));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setMimeTypeFilters(KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Import));
    m_previewImage = dialog.url();
    QImage img(m_previewImage);
    img = img.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_ui->lblPreview->setPixmap(QPixmap::fromImage(img));
}



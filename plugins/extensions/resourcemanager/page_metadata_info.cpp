#include "page_metadata_info.h"
#include "ui_pagemetadatainfo.h"
#include <kis_config.h>
#include <KoDocumentInfo.h>
#include <KisImportExportManager.h>
#include <KoFileDialog.h>


PageMetadataInfo::PageMetadataInfo(KoResourceBundleSP bundle, QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageMetadataInfo)
    , m_bundle(bundle)
{
    m_ui->setupUi(this);
    KoDocumentInfo info;
    info.updateParameters();

    if (bundle) {
    } else {
        KisConfig cfg(true);

        m_ui->editAuthor->setText(cfg.readEntry<QString>("BundleAuthorName", info.authorInfo("creator")));
        m_ui->editEmail->setText(cfg.readEntry<QString>("BundleAuthorEmail", info.authorInfo("email")));
        m_ui->editWebsite->setText(cfg.readEntry<QString>("BundleWebsite", "http://"));
        m_ui->editLicense->setText(cfg.readEntry<QString>("BundleLicense", "CC-BY-SA"));
        m_ui->editBundleName->setText(cfg.readEntry<QString>("BundleName", "New Bundle"));
        m_ui->editDescription->document()->setPlainText(cfg.readEntry<QString>("BundleDescription", "New Bundle"));
        m_previewImage = cfg.readEntry<QString>("BundleImage", "");
        if (!m_previewImage.isEmpty()) {
            QImage img(m_previewImage);
            img = img.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_ui->lblPreview->setPixmap(QPixmap::fromImage(img));
        }
    }

    connect(m_ui->bnGetPreview, SIGNAL(clicked()), SLOT(getPreviewImage()));
}

PageMetadataInfo::~PageMetadataInfo()
{
    delete m_ui;
}

QString PageMetadataInfo::bundleName() const
{
    return m_ui->editBundleName->text();
}

QString PageMetadataInfo::authorName() const
{
    return m_ui->editAuthor->text();
}

QString PageMetadataInfo::email() const
{
    return m_ui->editEmail->text();
}

QString PageMetadataInfo::website() const
{
    return m_ui->editWebsite->text();
}

QString PageMetadataInfo::license() const
{
    return m_ui->editLicense->text();
}

QString PageMetadataInfo::description() const
{
    return m_ui->editDescription->document()->toPlainText();
}


QString PageMetadataInfo::previewImage() const
{
    return m_previewImage;
}

void PageMetadataInfo::getPreviewImage()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "BundlePreviewImage");
    dialog.setCaption(i18n("Select file to use as bundle icon"));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    m_previewImage = dialog.filename();
    QImage img(m_previewImage);
    img = img.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_ui->lblPreview->setPixmap(QPixmap::fromImage(img));
}

void PageMetadataInfo::showWarning()
{
     m_ui->editBundleName->setStyleSheet(QString(" border: 1px solid red"));
}

void PageMetadataInfo::removeWarning()
{
     m_ui->editBundleName->setStyleSheet(QString(""));
}


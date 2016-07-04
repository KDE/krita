/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "DlgAnimationRenderer.h"

#include <QStandardPaths>
#include <QPluginLoader>
#include <QJsonObject>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <kis_properties_configuration.h>
#include <kis_debug.h>
#include <KisMimeDatabase.h>
#include <KoJsonTrader.h>
#include <KisImportExportFilter.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_time_range.h>
#include <KisImportExportManager.h>
#include <kis_config_widget.h>
#include <KisDocument.h>
#include <QHBoxLayout>
#include <KisImportExportFilter.h>

DlgAnimationRenderer::DlgAnimationRenderer(KisImageWSP image, QWidget *parent)
    : KoDialog(parent)
    , m_image(image)
{
    setCaption(i18n("Render Animation"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgAnimaterionRenderer(this);
    m_page->layout()->setMargin(0);
    m_page->dirRequester->setMode(KoFileDialog::OpenDirectory);
    m_page->dirRequester->setFileName(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));

    m_page->intStart->setMinimum(image->animationInterface()->fullClipRange().start());
    m_page->intStart->setMaximum(image->animationInterface()->fullClipRange().end());
    m_page->intStart->setValue(image->animationInterface()->playbackRange().start());

    m_page->intEnd->setMinimum(image->animationInterface()->fullClipRange().start());
    m_page->intEnd->setMaximum(image->animationInterface()->fullClipRange().end());
    m_page->intEnd->setValue(image->animationInterface()->playbackRange().end());

    m_sequenceConfigLayout = new QHBoxLayout(m_page->grpExportOptions);
    m_encoderConfigLayout = new QHBoxLayout(m_page->grpRenderOptions);

    QStringList mimes = KisImportExportManager::mimeFilter(KisImportExportManager::Export);
    mimes.sort();
    Q_FOREACH(const QString &mime, mimes) {
        QString description = KisMimeDatabase::descriptionForMimeType(mime);
        if (description.isEmpty()) {
            description = mime;
        }
        m_page->cmbMimetype->addItem(description, mime);
        if (mime == "image/png") {
            m_page->cmbMimetype->setCurrentIndex(m_page->cmbMimetype->count() - 1);
        }

    }

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    KoJsonTrader trader;
    QList<QPluginLoader *>list = trader.query("Krita/AnimationExporter", "");
    Q_FOREACH(QPluginLoader *loader, list) {
        QJsonObject json = loader->metaData().value("MetaData").toObject();
        Q_FOREACH(const QString &mime, json.value("X-KDE-Export").toString().split(",")) {

            KLibFactory *factory = qobject_cast<KLibFactory *>(loader->instance());
            if (!factory) {
                warnUI << loader->errorString();
                continue;
            }

            QObject* obj = factory->create<KisImportExportFilter>(0);
            if (!obj || !obj->inherits("KisImportExportFilter")) {
                delete obj;
                continue;
            }

            QSharedPointer<KisImportExportFilter>filter(static_cast<KisImportExportFilter*>(obj));
            if (!filter) {
                delete obj;
                continue;
            }

            m_renderFilters.append(filter);
            QString description = KisMimeDatabase::descriptionForMimeType(mime);
            if (description.isEmpty()) {
                description = mime;
            }
            m_page->cmbRenderType->addItem(description, mime);
        }
    }
    qDeleteAll(list);
    connect(m_page->cmbRenderType, SIGNAL(activated(int)), this, SLOT(selectRenderType(int)));
    selectRenderType(m_page->cmbRenderType->currentIndex());
    connect(m_page->grpRender, SIGNAL(toggled(bool)), this, SLOT(toggleSequenceType(bool)));
    connect(m_page->cmbMimetype, SIGNAL(activated(int)), this, SLOT(sequenceMimeTypeSelected(int)));
    sequenceMimeTypeSelected(m_page->cmbMimetype->currentIndex());
}

DlgAnimationRenderer::~DlgAnimationRenderer()
{
    m_encoderConfigWidget->setParent(0);
    m_encoderConfigWidget->deleteLater();
    m_frameExportConfigWidget->setParent(0);
    m_frameExportConfigWidget->deleteLater();

    delete m_page;

}

KisPropertiesConfigurationSP DlgAnimationRenderer::getSequenceConfiguration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("basename", m_page->txtBasename->text());
    cfg->setProperty("directory", m_page->dirRequester->fileName());
    cfg->setProperty("first_frame", m_page->intStart->value());
    cfg->setProperty("last_frame", m_page->intEnd->value());
    cfg->setProperty("sequence_start", m_page->sequenceStart->value());
    cfg->setProperty("mimetype", m_page->cmbMimetype->currentData().toString());
    return cfg;
}

void DlgAnimationRenderer::setSequenceConfiguration(KisPropertiesConfigurationSP cfg)
{
    m_page->txtBasename->setText(cfg->getString("basename", "frame"));
    m_page->dirRequester->setFileName(cfg->getString("directory", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)));
    m_page->intStart->setValue(cfg->getInt("first_frame", m_image->animationInterface()->playbackRange().start()));
    m_page->intEnd->setValue(cfg->getInt("last_frame", m_image->animationInterface()->playbackRange().end()));
    m_page->sequenceStart->setValue(cfg->getInt("sequence_start", m_image->animationInterface()->playbackRange().start()));
    QString mimetype = cfg->getString("mimetype");
    for (int i = 0; i < m_page->cmbMimetype->count(); ++i) {
        if (m_page->cmbMimetype->itemData(i).toString() == mimetype) {
            m_page->cmbMimetype->setCurrentIndex(i);
            sequenceMimeTypeSelected(i);
            break;
        }
    }
}

KisPropertiesConfigurationSP DlgAnimationRenderer::getFrameExportConfiguration() const
{
    if (m_frameExportConfigWidget) {
        return m_frameExportConfigWidget->configuration();
    }
    return 0;
}

bool DlgAnimationRenderer::renderToVideo() const
{
    return m_page->grpRender->isChecked();
}

KisPropertiesConfigurationSP DlgAnimationRenderer::getVideoConfiguration() const
{
    if (!m_page->grpRender->isChecked()) {
        return 0;
    }
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("filename", m_page->videoFilename->fileName());
    cfg->setProperty("delete_sequence", m_page->chkDeleteSequence->isChecked());
    return cfg;
}

void DlgAnimationRenderer::setVideoConfiguration(KisPropertiesConfigurationSP cfg)
{

}

KisPropertiesConfigurationSP DlgAnimationRenderer::getEncoderConfiguration() const
{
    if (!m_page->grpRender->isChecked()) {
        return 0;
    }
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    return cfg;
}

void DlgAnimationRenderer::setEncoderConfiguration(KisPropertiesConfigurationSP cfg)
{

}

void DlgAnimationRenderer::selectRenderType(int index)
{
    if (m_encoderConfigWidget) {
        m_encoderConfigLayout->removeWidget(m_encoderConfigWidget);
        m_encoderConfigWidget->hide();
        m_encoderConfigWidget->setParent(0);
        m_encoderConfigWidget->deleteLater();
        m_encoderConfigWidget = 0;
    }

    if (index >= m_renderFilters.size()) return;

    QSharedPointer<KisImportExportFilter> filter = m_renderFilters[index];
    QString mimetype = m_page->cmbMimetype->itemData(index).toString();

    qDebug() << ">>>>>>>>>" << mimetype;

    if (filter) {
        m_encoderConfigWidget = filter->createConfigurationWidget(m_page->grpExportOptions, KisDocument::nativeFormatMimeType(), mimetype.toLatin1());
        if (m_encoderConfigWidget) {
            m_encoderConfigLayout->addWidget(m_encoderConfigWidget);
            m_encoderConfigWidget->setConfiguration(filter->lastSavedConfiguration());
            m_encoderConfigWidget->show();
            resize(sizeHint());
        }
        else {
            m_encoderConfigWidget = 0;
        }
    }
}

void DlgAnimationRenderer::toggleSequenceType(bool toggle)
{
    m_page->cmbMimetype->setEnabled(!toggle);
    for (int i = 0; i < m_page->cmbMimetype->count(); ++i) {
        if (m_page->cmbMimetype->itemData(i).toString() == "image/png") {
            m_page->cmbMimetype->setCurrentIndex(i);
            break;
        }
    }
}

void DlgAnimationRenderer::sequenceMimeTypeSelected(int index)
{
    if (m_frameExportConfigWidget) {
        m_sequenceConfigLayout->removeWidget(m_frameExportConfigWidget);
        m_frameExportConfigWidget->hide();
        m_frameExportConfigWidget->setParent(0);
        m_frameExportConfigWidget->deleteLater();
        m_frameExportConfigWidget = 0;
    }
    QString mimetype = m_page->cmbMimetype->itemData(index).toString();
    KisImportExportFilter *filter = KisImportExportManager::filterForMimeType(mimetype, KisImportExportManager::Export);
    if (filter) {
        m_frameExportConfigWidget = filter->createConfigurationWidget(m_page->grpExportOptions, KisDocument::nativeFormatMimeType(), mimetype.toLatin1());
        if (m_frameExportConfigWidget) {
            m_sequenceConfigLayout->addWidget(m_frameExportConfigWidget);
            m_frameExportConfigWidget->setConfiguration(filter->lastSavedConfiguration());
            m_frameExportConfigWidget->show();
            resize(sizeHint());
        }
        else {
            m_frameExportConfigWidget = 0;
        }
        delete filter;
    }
}


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
#include <QMessageBox>
#include <QStringList>
#include <QProcess>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoResourcePaths.h>
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
#include <kis_config.h>
#include <kis_file_name_requester.h>
#include <KoDialog.h>
#include "kis_slider_spin_box.h"
#include "kis_acyclic_signal_connector.h"

DlgAnimationRenderer::DlgAnimationRenderer(KisDocument *doc, QWidget *parent)
    : KoDialog(parent)
    , m_image(doc->image())
    , m_doc(doc)
    , m_defaultFileName(QFileInfo(doc->url().toLocalFile()).completeBaseName())
{
    KisConfig cfg;

    setCaption(i18n("Render Animation"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    if (m_defaultFileName.isEmpty()) {
        m_defaultFileName = i18n("Untitled");
    }

    m_page = new WdgAnimationRenderer(this);
    m_page->layout()->setMargin(0);

    m_page->dirRequester->setMode(KoFileDialog::OpenDirectory);
    QString lastLocation = cfg.readEntry<QString>("AnimationRenderer/last_sequence_export_location", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    m_page->dirRequester->setFileName(lastLocation);

    m_page->intStart->setMinimum(doc->image()->animationInterface()->fullClipRange().start());
    m_page->intStart->setMaximum(doc->image()->animationInterface()->fullClipRange().end());
    m_page->intStart->setValue(doc->image()->animationInterface()->playbackRange().start());

    m_page->intEnd->setMinimum(doc->image()->animationInterface()->fullClipRange().start());
   //m_page->intEnd->setMaximum(doc->image()->animationInterface()->fullClipRange().end()); // animators sometimes want to export after end frame
    m_page->intEnd->setValue(doc->image()->animationInterface()->playbackRange().end());



    m_page->intHeight->setMinimum(1);
    m_page->intHeight->setMaximum(10000);
    m_page->intHeight->setValue(doc->image()->height());

    m_page->intWidth->setMinimum(1);
    m_page->intWidth->setMaximum(10000);
    m_page->intWidth->setValue(doc->image()->width());


    // try to lock the width and height being updated

    KisAcyclicSignalConnector *constrainsConnector = new KisAcyclicSignalConnector(this);
    constrainsConnector->createCoordinatedConnector()->connectBackwardInt(m_page->intWidth, SIGNAL(valueChanged(int)), this, SLOT(slotLockAspectRatioDimensionsWidth(int)));
    constrainsConnector->createCoordinatedConnector()->connectForwardInt(m_page->intHeight, SIGNAL(valueChanged(int)), this, SLOT(slotLockAspectRatioDimensionsHeight(int)));

    m_page->intFramesPerSecond->setValue(doc->image()->animationInterface()->framerate());

    QFileInfo audioFileInfo(doc->image()->animationInterface()->audioChannelFileName());
    const bool hasAudio = audioFileInfo.exists();
    m_page->chkIncludeAudio->setEnabled(hasAudio);
    m_page->chkIncludeAudio->setChecked(hasAudio && !doc->image()->animationInterface()->isAudioMuted());

    QStringList mimes = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);
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

    QList<QPluginLoader *>list = KoJsonTrader::instance()->query("Krita/AnimationExporter", "");
    Q_FOREACH(QPluginLoader *loader, list) {
        QJsonObject json = loader->metaData().value("MetaData").toObject();
        QStringList mimetypes = json.value("X-KDE-Export").toString().split(",");
        Q_FOREACH(const QString &mime, mimetypes) {

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
    m_page->videoFilename->setMode(KoFileDialog::SaveFile);
    m_page->videoFilename->setStartDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));

    qDeleteAll(list);

    connect(m_page->bnExportOptions, SIGNAL(clicked()), this, SLOT(sequenceMimeTypeSelected()));
    connect(m_page->bnRenderOptions, SIGNAL(clicked()), this, SLOT(selectRenderOptions()));

    m_page->ffmpegLocation->setFileName(findFFMpeg());
    m_page->ffmpegLocation->setMode(KoFileDialog::OpenFile);
    connect(m_page->ffmpegLocation, SIGNAL(fileSelected(QString)), this, SLOT(ffmpegLocationChanged(QString)));

    m_page->cmbRenderType->setCurrentIndex(cfg.readEntry<int>("AnimationRenderer/render_type", 0));


    connect(m_page->shouldExportOnlyImageSequence, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));
    connect(m_page->shouldExportOnlyVideo, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));
    connect(m_page->shouldExportAll, SIGNAL(toggled(bool)), this, SLOT(slotExportTypeChanged()));

    updateExportUIOptions();

    // connect and cold init
    connect(m_page->cmbRenderType, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRenderType(int)));
    selectRenderType(m_page->cmbRenderType->currentIndex());

    resize(m_page->sizeHint());
}

DlgAnimationRenderer::~DlgAnimationRenderer()
{
    KisConfig cfg;

    cfg.writeEntry<QString>("AnimationRenderer/last_sequence_export_location", m_page->dirRequester->fileName());
    cfg.writeEntry<int>("AnimationRenderer/render_type", m_page->cmbRenderType->currentIndex());
    cfg.setCustomFFMpegPath(m_page->ffmpegLocation->fileName());

    if (m_encoderConfigWidget)  {
        m_encoderConfigWidget->setParent(0);
        m_encoderConfigWidget->deleteLater();
    }
    if (m_frameExportConfigWidget) {
        m_frameExportConfigWidget->setParent(0);
        m_frameExportConfigWidget->deleteLater();
    }

    delete m_page;

}

QString DlgAnimationRenderer::fetchRenderingDirectory() const
{
    QString result = m_page->dirRequester->fileName();

    if (m_page->shouldExportOnlyVideo->isChecked()) {
        const QFileInfo info(fetchRenderingFileName());

        if (info.isAbsolute()) {
            result = info.absolutePath();
        }
    }

    return result;
}

QString DlgAnimationRenderer::fetchRenderingFileName() const
{
    QString filename = m_page->videoFilename->fileName();

    if (QFileInfo(filename).completeSuffix().isEmpty()) {
        QString mimetype = m_page->cmbRenderType->itemData(m_page->cmbRenderType->currentIndex()).toString();
        filename += "." + KisMimeDatabase::suffixesForMimeType(mimetype).first();
    }

    if (QFileInfo(filename).isRelative()) {
        QDir baseDir(m_page->dirRequester->fileName());

        if (m_page->shouldExportOnlyVideo->isChecked()) {
            QString documentDir = QFileInfo(m_doc->url().toLocalFile()).absolutePath();
            if (!documentDir.isEmpty()) {
                baseDir = documentDir;
            }
        }

        filename = baseDir.absoluteFilePath(filename);
    }

    return filename;
}

KisPropertiesConfigurationSP DlgAnimationRenderer::getSequenceConfiguration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("basename", m_page->txtBasename->text());
    cfg->setProperty("last_document_path", m_doc->localFilePath());
    cfg->setProperty("directory", fetchRenderingDirectory());
    cfg->setProperty("first_frame", m_page->intStart->value());
    cfg->setProperty("last_frame", m_page->intEnd->value());
    cfg->setProperty("sequence_start", m_page->sequenceStart->value());
    cfg->setProperty("mimetype", m_page->cmbMimetype->currentData().toString());
    return cfg;
}

void DlgAnimationRenderer::setSequenceConfiguration(KisPropertiesConfigurationSP cfg)
{
    m_page->txtBasename->setText(cfg->getString("basename", "frame"));

    if (cfg->getString("last_document_path") != m_doc->localFilePath()) {
        cfg->removeProperty("first_frame");
        cfg->removeProperty("last_frame");
        cfg->removeProperty("sequence_start");
    }

    m_page->dirRequester->setFileName(cfg->getString("directory", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)));
    m_page->intStart->setValue(cfg->getInt("first_frame", m_image->animationInterface()->playbackRange().start()));
    m_page->intEnd->setValue(cfg->getInt("last_frame", m_image->animationInterface()->playbackRange().end()));
    m_page->sequenceStart->setValue(cfg->getInt("sequence_start", m_image->animationInterface()->playbackRange().start()));

    QString mimetype = cfg->getString("mimetype");
    for (int i = 0; i < m_page->cmbMimetype->count(); ++i) {
        if (m_page->cmbMimetype->itemData(i).toString() == mimetype) {
            m_page->cmbMimetype->setCurrentIndex(i);
            break;
        }
    }
}

KisPropertiesConfigurationSP DlgAnimationRenderer::getFrameExportConfiguration() const
{
    if (m_frameExportConfigWidget) {
        KisPropertiesConfigurationSP cfg = m_frameExportConfigWidget->configuration();
        cfg->setProperty("basename", m_page->txtBasename->text());
        cfg->setProperty("directory", fetchRenderingDirectory());
        cfg->setProperty("first_frame", m_page->intStart->value());
        cfg->setProperty("last_frame", m_page->intEnd->value());
        cfg->setProperty("sequence_start", m_page->sequenceStart->value());
        cfg->setProperty("ffmpeg_path", m_page->ffmpegLocation->fileName());

        return m_frameExportConfigWidget->configuration();
    }
    return 0;
}

KisPropertiesConfigurationSP DlgAnimationRenderer::getVideoConfiguration() const
{
    // don't continue if we are only exporting image sequence
    if (m_page->shouldExportOnlyImageSequence->isChecked()) {
        return 0;
    }

    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("filename", fetchRenderingFileName());
    cfg->setProperty("first_frame", m_page->intStart->value());
    cfg->setProperty("last_frame", m_page->intEnd->value());
    cfg->setProperty("sequence_start", m_page->sequenceStart->value());

    // delete image sequence if we are only exporting out video
    cfg->setProperty("delete_sequence", m_page->shouldExportOnlyVideo->isChecked());


    return cfg;
}

void DlgAnimationRenderer::setVideoConfiguration(KisPropertiesConfigurationSP /*cfg*/)
{
}

KisPropertiesConfigurationSP DlgAnimationRenderer::getEncoderConfiguration() const
{
    // don't continue if we are only exporting image sequence
    if (m_page->shouldExportOnlyImageSequence->isChecked()) {
        return 0;
    }

    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    if (m_encoderConfigWidget) {
        cfg = m_encoderConfigWidget->configuration();
    }
    cfg->setProperty("mimetype", m_page->cmbRenderType->currentData().toString());
    cfg->setProperty("directory", fetchRenderingDirectory());
    cfg->setProperty("first_frame", m_page->intStart->value());
    cfg->setProperty("last_frame", m_page->intEnd->value());
    cfg->setProperty("framerate", m_page->intFramesPerSecond->value());
    cfg->setProperty("height", m_page->intHeight->value());
    cfg->setProperty("width", m_page->intWidth->value());
    cfg->setProperty("sequence_start", m_page->sequenceStart->value());
    cfg->setProperty("include_audio", m_page->chkIncludeAudio->isChecked());

    return cfg;
}

void DlgAnimationRenderer::setEncoderConfiguration(KisPropertiesConfigurationSP cfg)
{
    m_page->intHeight->setValue(cfg->getInt("height", int(m_image->height())));
    m_page->intWidth->setValue(cfg->getInt("width", int(m_image->width())));
    m_page->intFramesPerSecond->setValue(cfg->getInt("framerate", int(m_image->animationInterface()->framerate())));

    if (m_encoderConfigWidget) {
        m_encoderConfigWidget->setConfiguration(cfg);
    }
}

QSharedPointer<KisImportExportFilter> DlgAnimationRenderer::encoderFilter() const
{
    if (m_page->cmbRenderType->currentIndex() < m_renderFilters.size()) {
        return m_renderFilters[m_page->cmbRenderType->currentIndex()];
    }
    return QSharedPointer<KisImportExportFilter>(0);
}

void DlgAnimationRenderer::selectRenderType(int index)
{
    if (index >= m_renderFilters.size()) return;
    QString mimetype = m_page->cmbRenderType->itemData(index).toString();

    if (!m_page->videoFilename->fileName().isEmpty() && QFileInfo(m_page->videoFilename->fileName()).completeBaseName() != m_defaultFileName) {
        m_defaultFileName = QFileInfo(m_page->videoFilename->fileName()).completeBaseName();
    }
    m_page->videoFilename->setMimeTypeFilters(QStringList() << mimetype, mimetype);

    m_page->videoFilename->setFileName(m_defaultFileName + "." + KisMimeDatabase::suffixesForMimeType(mimetype).first());
}

void DlgAnimationRenderer::selectRenderOptions()
{
    int index = m_page->cmbRenderType->currentIndex();

    if (m_encoderConfigWidget) {
        m_encoderConfigWidget->deleteLater();
        m_encoderConfigWidget = 0;
    }

    if (index >= m_renderFilters.size()) return;

    QSharedPointer<KisImportExportFilter> filter = m_renderFilters[index];
    QString mimetype = m_page->cmbRenderType->itemData(index).toString();
    if (filter) {
        m_encoderConfigWidget = filter->createConfigurationWidget(0, KisDocument::nativeFormatMimeType(), mimetype.toLatin1());
        if (m_encoderConfigWidget) {
            m_encoderConfigWidget->setConfiguration(filter->lastSavedConfiguration("", mimetype.toLatin1()));
            KoDialog dlg(this);
            dlg.setMainWidget(m_encoderConfigWidget);
            dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
            if (!dlg.exec()) {
                m_encoderConfigWidget->setConfiguration(filter->lastSavedConfiguration());
            } else {
                KisConfig().setExportConfiguration(mimetype.toLatin1(), m_encoderConfigWidget->configuration());
            }
            dlg.setMainWidget(0);
            m_encoderConfigWidget->hide();
            m_encoderConfigWidget->setParent(0);
        }
    }
    else {
        m_encoderConfigWidget = 0;
    }

}

void DlgAnimationRenderer::sequenceMimeTypeSelected()
{
    int index = m_page->cmbMimetype->currentIndex();

    if (m_frameExportConfigWidget) {
        m_frameExportConfigWidget->deleteLater();
        m_frameExportConfigWidget = 0;
    }

    QString mimetype = m_page->cmbMimetype->itemData(index).toString();
    QSharedPointer<KisImportExportFilter> filter(KisImportExportManager::filterForMimeType(mimetype, KisImportExportManager::Export));
    if (filter) {
        m_frameExportConfigWidget = filter->createConfigurationWidget(0, KisDocument::nativeFormatMimeType(), mimetype.toLatin1());
        if (m_frameExportConfigWidget) {
            m_frameExportConfigWidget->setConfiguration(filter->lastSavedConfiguration("", mimetype.toLatin1()));
            KoDialog dlg(this);
            dlg.setMainWidget(m_frameExportConfigWidget);
            dlg.setButtons(KoDialog::Ok | KoDialog::Cancel);
            if (!dlg.exec()) {
                m_frameExportConfigWidget->setConfiguration(filter->lastSavedConfiguration());
            }
            m_frameExportConfigWidget->hide();
            m_frameExportConfigWidget->setParent(0);
            dlg.setMainWidget(0);
        }
    }
}

void DlgAnimationRenderer::ffmpegLocationChanged(const QString &s)
{
    KisConfig cfg;
    cfg.setCustomFFMpegPath(s);
}

void DlgAnimationRenderer::updateExportUIOptions() {

    KisConfig cfg;

    // read in what type to export to. Defaults to image sequence only
    QString exportType = cfg.readEntry<QString>("AnimationRenderer/export_type", "ImageSequence");
    if (exportType == "ImageSequence") {
        m_page->shouldExportOnlyImageSequence->setChecked(true);
    } else if (exportType == "Video") {
        m_page->shouldExportOnlyVideo->setChecked(true);
    } else {
        m_page->shouldExportAll->setChecked(true); // export to both
    }
}

void DlgAnimationRenderer::slotButtonClicked(int button)
{
    if (button == KoDialog::Ok && !m_page->shouldExportOnlyImageSequence->isChecked()) {
        QString ffmpeg = m_page->ffmpegLocation->fileName();
        if (m_page->videoFilename->fileName().isEmpty()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter a file name to render to."));
            return;
        }
        else if (ffmpeg.isEmpty()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The location of FFmpeg is unknown. Please install FFmpeg first: Krita cannot render animations without FFmpeg. (<a href=\"https://www.ffmpeg.org\">www.ffmpeg.org</a>)"));
            return;
        }
        else {
            QFileInfo fi(ffmpeg);
            if (!fi.exists()) {
                QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The location of FFmpeg is invalid. Please select the correct location of the FFmpeg executable on your system."));
                return;
            }
        }
    }
    KoDialog::slotButtonClicked(button);
}

QString DlgAnimationRenderer::findFFMpeg()
{
    QString result;

    QStringList proposedPaths;

    QString customPath = KisConfig().customFFMpegPath();
    if (!customPath.isEmpty()) {
        proposedPaths << customPath;
        proposedPaths << customPath + QDir::separator() + "ffmpeg";
    }

#ifndef Q_OS_WIN
    proposedPaths << QDir::homePath() + "/bin/ffmpeg";
    proposedPaths << "/usr/bin/ffmpeg";
    proposedPaths << "/usr/local/bin/ffmpeg";
#endif
    proposedPaths << KoResourcePaths::getApplicationRoot() +
        QDir::separator() + "bin" + QDir::separator() + "ffmpeg";

    Q_FOREACH (QString path, proposedPaths) {
        if (path.isEmpty()) continue;

#ifdef Q_OS_WIN
        path = QDir::toNativeSeparators(QDir::cleanPath(path));
        if (path.endsWith(QDir::separator())) {
            continue;
        }
        if (!path.endsWith(".exe")) {
            if (!QFile::exists(path)) {
                path += ".exe";
                if (!QFile::exists(path)) {
                    continue;
                }
            }
        }
#endif
        QProcess testProcess;
        testProcess.start(path, QStringList() << "-version");
        if (testProcess.waitForStarted(1000)) {
            testProcess.waitForFinished(1000);
        }

        const bool successfulStart =
            testProcess.state() == QProcess::NotRunning &&
            testProcess.error() == QProcess::UnknownError;

        if (successfulStart) {
            result = path;
            break;
        }
    }

    return result;
}

void DlgAnimationRenderer::slotExportTypeChanged()
{
    KisConfig cfg;

    bool willEncodeVideo =
        m_page->shouldExportAll->isChecked() || m_page->shouldExportOnlyVideo->isChecked();

    // if a video format needs to be outputted
    if (willEncodeVideo) {
         // videos always uses PNG for creating video, so disable the ability to change the format
         m_page->cmbMimetype->setEnabled(false);
         for (int i = 0; i < m_page->cmbMimetype->count(); ++i) {
             if (m_page->cmbMimetype->itemData(i).toString() == "image/png") {
                 m_page->cmbMimetype->setCurrentIndex(i);
                 break;
             }
         }
    }

    m_page->intWidth->setVisible(willEncodeVideo);
    m_page->intHeight->setVisible(willEncodeVideo);
    m_page->intFramesPerSecond->setVisible(willEncodeVideo);
    m_page->fpsLabel->setVisible(willEncodeVideo);
    m_page->lblWidth->setVisible(willEncodeVideo);
    m_page->lblHeight->setVisible(willEncodeVideo);

    // if only exporting video
    if (m_page->shouldExportOnlyVideo->isChecked()) {
        m_page->cmbMimetype->setEnabled(false); // allow to change image format
        m_page->imageSequenceOptionsGroup->setVisible(false);
        m_page->videoOptionsGroup->setVisible(false); //shrinks the horizontal space temporarily to help resize() work
        m_page->videoOptionsGroup->setVisible(true);

        cfg.writeEntry<QString>("AnimationRenderer/export_type", "Video");
    }


    // if only an image sequence needs to be output
    if (m_page->shouldExportOnlyImageSequence->isChecked()) {
        m_page->cmbMimetype->setEnabled(true); // allow to change image format
        m_page->videoOptionsGroup->setVisible(false);
        m_page->imageSequenceOptionsGroup->setVisible(false);
        m_page->imageSequenceOptionsGroup->setVisible(true);

        cfg.writeEntry<QString>("AnimationRenderer/export_type", "ImageSequence");
    }

    // show all options
     if (m_page->shouldExportAll->isChecked() ) {
         m_page->imageSequenceOptionsGroup->setVisible(true);
         m_page->videoOptionsGroup->setVisible(true);

         cfg.writeEntry<QString>("AnimationRenderer/export_type", "VideoAndImageSequence");
     }


     // for the resize to work as expected, try to hide elements first before displaying other ones.
     // if the widget gets bigger at any point, the resize will use that, even if elements are hidden later to make it smaller
     resize(m_page->sizeHint());
}

void DlgAnimationRenderer::slotLockAspectRatioDimensionsWidth(int width)
{
    Q_UNUSED(width);

    float aspectRatio = (float)m_image->width() / (float)m_image->height();

    // update height here
    float newHeight = m_page->intWidth->value() / aspectRatio  ;

    m_page->intHeight->setValue(newHeight);

}

void DlgAnimationRenderer::slotLockAspectRatioDimensionsHeight(int height)
{
    Q_UNUSED(height);

    float aspectRatio = (float)m_image->width() / (float)m_image->height();

    // update width here
     float newWidth = aspectRatio *  m_page->intHeight->value();

     m_page->intWidth->setValue(newWidth);
}

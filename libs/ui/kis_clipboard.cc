/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_clipboard.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QScreen>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QScopedPointer>
#include <QTemporaryFile>

// kritaglobal
#include <algorithm>
#include <kis_assert.h>
#include <kis_debug.h>

// kritastore
#include <KoStore.h>

// kritaimage
#include <kis_annotation.h>
#include <kis_default_bounds.h>
#include <kis_image.h>
#include <kis_layer_utils.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_time_span.h>

// local
#include "KisDocument.h"
#include "KisImportExportManager.h"
#include "KisMainWindow.h"
#include "KisMimeDatabase.h"
#include "KisPart.h"
#include "KisRemoteFileFetcher.h"
#include "dialogs/kis_dlg_missing_color_profile.h"
#include "dialogs/kis_dlg_paste_format.h"
#include "kis_mimedata.h"
#include "kis_store_paintdevice_writer.h"
#include "KisDisplayConfig.h"


Q_GLOBAL_STATIC(KisClipboard, s_instance)

struct ClipboardImageFormat {
    QSet<QString> mimeTypes;
    QString format;
};

class Q_DECL_HIDDEN KisClipboardPrivate
{
public:
    KisClipboardPrivate()
        : clipboard(QApplication::clipboard())
    {
    }

    bool hasClip{};
    QClipboard *clipboard;
};

KisClipboard::KisClipboard()
    : d(new KisClipboardPrivate)
{
    // Check that we don't already have a clip ready
    clipboardDataChanged();

    // Make sure we are notified when clipboard changes
    connect(d->clipboard, &QClipboard::dataChanged, this, &KisClipboard::clipboardDataChanged, Qt::UniqueConnection);
}

KisClipboard::~KisClipboard()
{
    dbgRegistry << "deleting KisClipBoard";
    delete d;
}

KisClipboard *KisClipboard::instance()
{
    return s_instance;
}

KisPaintDeviceSP KisClipboard::clip(KisDefaultBoundsBaseSP defaultBounds,
                                    bool showPopup,
                                    int overridePasteBehaviour) const
{
    const QMimeData *cbData = d->clipboard->mimeData();

    if (!cbData) {
        return nullptr;
    }

    dbgUI << Q_FUNC_INFO;
    dbgUI << "\tFormats: " << cbData->formats();
    dbgUI << "\tUrls: " << cbData->urls();
    dbgUI << "\tHas images: " << cbData->hasImage();

    KisPaintDeviceSP clip = clipFromKritaLayers(nullptr, defaultBounds);

    if (!clip) {
        KisImageSP clipImage = clipImageFromBoardContents(showPopup, overridePasteBehaviour);
        if (clipImage) {
            clip = new KisPaintDevice(*clipImage->projection());
            clip->setDefaultBounds(defaultBounds);
        }
    }

    return clip;
}

KisImageSP KisClipboard::clipImage(bool showPopup, int overridePasteBehaviour) const
{
    const QMimeData *cbData = d->clipboard->mimeData();

    if (!cbData) {
        return nullptr;
    }

    dbgUI << Q_FUNC_INFO;
    dbgUI << "\tFormats: " << cbData->formats();
    dbgUI << "\tUrls: " << cbData->urls();
    dbgUI << "\tHas images: " << cbData->hasImage();

    KisImageSP clipImage = clipImageFromKritaLayers(nullptr);

    if (!clipImage) {
        clipImage = clipImageFromBoardContents(showPopup, overridePasteBehaviour);
    }

    return clipImage;
}

KisImageSP KisClipboard::clipImageFromBoardContents(bool showPopup, int overridePasteBehaviour, QPair<bool, PasteFormatBehaviour> source) const
{
    const QMimeData *cbData = d->clipboard->mimeData();

    if (!cbData) {
        return nullptr;
    }

    // On wayland opening a dialog invalidates the clipboard data so we cache all the data beforehand
    const auto &urls = cbData->urls();
    const QImage qimage = getImageWithFallback(cbData, true);

    return clipFromBoardContentsWithData(qimage, urls, showPopup, overridePasteBehaviour, source);
}

KisPaintDeviceSP KisClipboard::clipFromKritaLayers(const KoColorSpace *cs, KisDefaultBoundsBaseSP defaultBounds) const
{
    KisPaintDeviceSP result;

    const QMimeData *data = KisClipboard::instance()->layersMimeData();
    if (!data) return result;

    const KisMimeData *mimedata = qobject_cast<const KisMimeData *>(data);
    if (!mimedata) return result;

    return mimedata->clipFromKritaLayers(cs, defaultBounds);
}

KisImageSP KisClipboard::clipImageFromKritaLayers(const KoColorSpace *cs) const
{
    KisImageSP resultImage;

    const QMimeData *data = KisClipboard::instance()->layersMimeData();
    if (!data) return resultImage;

    const KisMimeData *mimedata = qobject_cast<const KisMimeData *>(data);
    if (!mimedata) return resultImage;

    return mimedata->clipImageFromKritaLayers(cs);
}

QPair<bool, KisClipboard::PasteFormatBehaviour>
KisClipboard::askUserForSource(const QMimeData *cbData, bool useClipboardFallback) const
{
    if (!cbData) {
        return {false, PASTE_FORMAT_ASK};
    }
    const QImage qimage = getImageWithFallback(cbData, useClipboardFallback);
    QList<QUrl> urls = cbData->urls();

    return askUserForSourceWithData(qimage, urls, useClipboardFallback);
}

QPair<bool, KisClipboard::PasteFormatBehaviour>
KisClipboard::askUserForSourceWithData(QImage qimage, const QList<QUrl> urls, bool useClipboardFallback) const
{
    KisConfig cfg(true);

    bool saveSourceSetting = false;

    auto choice = (PasteFormatBehaviour)cfg.pasteFormat(false);

    if (!qimage.isNull() || !urls.isEmpty()) {
        bool local = false;
        bool remote = false;
        bool isURI = false;

        std::for_each(urls.constBegin(), urls.constEnd(), [&](const QUrl &url) {
            local |= url.isLocalFile();
            remote |= !url.isLocalFile();
            isURI |= url.scheme() == "data";
        });

        const bool hasMultipleFormatsAvailable = (remote && local)
                || (remote && !qimage.isNull()) || (local && !qimage.isNull());

        const bool defaultOptionUnavailable =
                (!remote && choice == PASTE_FORMAT_DOWNLOAD)
                || (!local && choice == PASTE_FORMAT_LOCAL)
                || (qimage.isNull() && choice == PASTE_FORMAT_CLIP);

        dbgUI << "Incoming paste event:";
        dbgUI << "\tHas attached bitmap:" << !qimage.isNull();
        dbgUI << "\tHas local images:" << local;
        dbgUI << "\tHas remote images:" << remote;
        dbgUI << "\tHas multiple formats:" << hasMultipleFormatsAvailable;
        dbgUI << "\tDefault source preference" << choice;
        dbgUI << "\tDefault source available:" << !defaultOptionUnavailable;
        dbgUI << "\tIs data URI:" << isURI;

        if (hasMultipleFormatsAvailable && choice == PASTE_FORMAT_ASK && !isURI) {
            KisDlgPasteFormat dlg(qApp->activeWindow());

            dlg.setSourceAvailable(PASTE_FORMAT_DOWNLOAD, remote);
            dlg.setSourceAvailable(PASTE_FORMAT_LOCAL, local);
            dlg.setSourceAvailable(PASTE_FORMAT_CLIP, !qimage.isNull());

            if (dlg.exec() != KoDialog::Accepted) {
                return {false, PASTE_FORMAT_ASK};
            };

            choice = dlg.source();

            saveSourceSetting = dlg.remember();
        } else if (defaultOptionUnavailable || choice == PASTE_FORMAT_ASK) {
            if (remote) {
                choice = PASTE_FORMAT_DOWNLOAD;
            } else if (local) {
                choice = PASTE_FORMAT_LOCAL;
            } else if (!qimage.isNull()) {
                choice = PASTE_FORMAT_CLIP;
            } else {
                return {false, PASTE_FORMAT_ASK};
            }
        } else if (isURI) {
            choice = PASTE_FORMAT_DOWNLOAD;
        }
    }

    if (saveSourceSetting) {
        cfg.setPasteFormat(choice);
    }

    dbgUI << "Selected source for the paste:" << choice;

    return {true, choice};
}

KisImageSP KisClipboard::clipFromBoardContentsWithData(
    QImage qimage,
    const QList<QUrl> urls,
    bool showPopup,
    int overridePasteBehaviour,
    QPair<bool, PasteFormatBehaviour> source) const
{
    KisImageSP clipImage;
    PasteFormatBehaviour choice = PASTE_FORMAT_ASK;

    if (!source.first) {
        choice = askUserForSourceWithData(qimage , urls).second;
    } else {
        choice = source.second;
    }

    if (choice == PASTE_FORMAT_CLIP) {
        KIS_SAFE_ASSERT_RECOVER(!qimage.isNull())
        {
            warnKrita << "Clipboard was cleared before loading image";
            return nullptr;
        }

        int behaviour = overridePasteBehaviour;
        bool saveColorSetting = false;

        KisConfig cfg(true);

        if (overridePasteBehaviour == -1) {
            behaviour = cfg.pasteBehaviour();
        }

        if (behaviour == PASTE_ASK && showPopup) {
            // Ask user each time.
            KisDlgMissingColorProfile dlg(qApp->activeWindow());

            if (dlg.exec() != QDialog::Accepted) {
                return nullptr;
            }

            behaviour = dlg.source();

            saveColorSetting = dlg.remember(); // should we save this option to the config for next time?
        }

        const KoColorSpace *cs = nullptr;
        const KoColorProfile *profile = nullptr;
        if (!profile && behaviour == PASTE_ASSUME_MONITOR)
            profile = KisMimeData::displayConfigForMimePastes().profile;

        cs = KoColorSpaceRegistry::instance()->rgb8(profile);
        if (!cs) {
            cs = KoColorSpaceRegistry::instance()->rgb8();
            profile = cs->profile();
        }

        KisImageSP image = new KisImage(nullptr, qimage.width(), qimage.height(), cs, "clip");
        if (qimage.dotsPerMeterX() > 0 && qimage.dotsPerMeterY() > 0) {
            image->setResolution(qimage.dotsPerMeterX() / 72.0 * 0.0254, qimage.dotsPerMeterY() / 72.0 * 0.0254);
        }
        KisPaintLayerSP layer = new KisPaintLayer(image, "clip", OPACITY_OPAQUE_U8);
        image->addNode(layer);
        layer->paintDevice()->convertFromQImage(qimage, profile);

        clipImage = image;

        // save the user's selection to the configuration if the option is checked
        if (saveColorSetting) {
            cfg.setPasteBehaviour(behaviour);
        }
    } else {
        const auto url = std::find_if(urls.constBegin(), urls.constEnd(), [&](const QUrl &url) {
            if (choice == PASTE_FORMAT_DOWNLOAD) {
                return !url.isLocalFile();
            } else if (choice == PASTE_FORMAT_LOCAL) {
                return url.isLocalFile();
            } else {
                return false;
            }
        });

        if (url != urls.constEnd()) {
            clipImage = fetchImageByURL(*url);
        }
    }

    return clipImage;
}

void KisClipboard::clipboardDataChanged()
{
    d->hasClip = d->clipboard->mimeData()->hasImage();
    Q_EMIT clipChanged();
}

QImage KisClipboard::getImageWithFallback(const QMimeData *cbData, bool useClipboardFallback) const
{
    QImage qimage = getImageFromMimeData(cbData);

    if (qimage.isNull() && useClipboardFallback) {
        qimage = d->clipboard->image();
    }

    return qimage;
}

bool KisClipboard::hasClip() const
{
    return d->hasClip;
}

void KisClipboard::setLayers(KisNodeList nodes, KisImageSP image, const QRect referenceRect, bool forceCopy)
{
    /**
     * See a comment in KisMimeData::deepCopyNodes()
     */
    QMimeData *data = KisMimeData::mimeForLayersDeepCopy(nodes, image, referenceRect, forceCopy);
    if (!data)
        return;

    d->clipboard->setMimeData(data);
}

bool KisClipboard::hasLayers() const
{
    const QByteArray mimeType = QByteArrayLiteral("application/x-krita-node-internal-pointer");
    return d->clipboard->mimeData()->hasFormat(mimeType);
}

bool KisClipboard::hasLayerStyles() const
{
    // NOTE: please don't disable the paste action based on the
    //       result of this function, because we allow pasting
    //       of the layer styles as 'text/plain'

    return d->clipboard->mimeData()->hasFormat("application/x-krita-layer-style");
}

const QMimeData *KisClipboard::layersMimeData() const
{
    const QMimeData *cbData = d->clipboard->mimeData();
    return cbData->hasFormat("application/x-krita-node-internal-pointer") ? cbData : 0;
}

bool KisClipboard::hasUrls() const
{
    return d->clipboard->mimeData()->hasUrls();
}


bool KisClipboard::hasImage() const
{
    return d->clipboard->mimeData()->hasImage();
}

QImage KisClipboard::getImageFromMimeData(const QMimeData *cbData) const
{
    static const QList<ClipboardImageFormat> supportedFormats = {
        {{"image/png"}, "PNG"},
        {{"image/tiff"}, "TIFF"},
        {{"image/bmp", "image/x-bmp", "image/x-MS-bmp", "image/x-win-bitmap"}, "BMP"}};

    QImage image;
    QSet<QString> clipboardMimeTypes;

    Q_FOREACH (const QString &format, cbData->formats()) {
        clipboardMimeTypes << format;
    }

    Q_FOREACH (const ClipboardImageFormat &item, supportedFormats) {
        const QSet<QString> &intersection = item.mimeTypes & clipboardMimeTypes;
        if (intersection.isEmpty()) {
            continue;
        }

        const QString &format = *intersection.constBegin();
        const QByteArray &imageData = cbData->data(format);
        if (imageData.isEmpty()) {
            continue;
        }

        if (image.loadFromData(imageData, item.format.toLatin1())) {
            break;
        }
    }

    if (image.isNull() && cbData->hasImage()) {
        image = qvariant_cast<QImage>(cbData->imageData());
    }

    return image;
}

KisImageSP KisClipboard::fetchImageByURL(const QUrl &originalUrl) const
{
    KisImageSP clipImage;
    KisPaintDeviceSP result;
    QUrl url(originalUrl);
    QScopedPointer<QTemporaryFile> tmp;

    if (!originalUrl.isLocalFile()) {
        tmp.reset(new QTemporaryFile());
        tmp->setAutoRemove(true);

        // download the file and substitute the url
        KisRemoteFileFetcher fetcher;

        if (!fetcher.fetchFile(originalUrl, tmp.data())) {
            qWarning() << "Fetching" << originalUrl << "failed";
            return clipImage;
        }
        url = QUrl::fromLocalFile(tmp->fileName());
    }

    if (url.isLocalFile()) {
        QFileInfo fileInfo(url.toLocalFile());

        QString type = KisMimeDatabase::mimeTypeForFile(url.toLocalFile());
        QStringList mimes = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import);

        if (!mimes.contains(type)) {
            QString msg = KisImportExportErrorCode(ImportExportCodes::FileFormatNotSupported).errorMessage();
            QMessageBox::warning(KisPart::instance()->currentMainwindow(),
                                 i18nc("@title:window", "Krita"),
                                 i18n("Could not open %2.\nReason: %1.", msg, url.toDisplayString()));
            return clipImage;
        }

        QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

        if (doc->openPath(url.toLocalFile(), KisDocument::DontAddToRecent)) {
            // Wait for required updates, if any. BUG: 448256
            KisLayerUtils::forceAllDelayedNodesUpdate(doc->image()->root());
            doc->image()->waitForDone();

            KisPaintDeviceSP clonedProjection(new KisPaintDevice(*doc->image()->projection()));

            KisImageSP image = new KisImage(nullptr, doc->image()->width(), doc->image()->height(), doc->image()->projection()->colorSpace(), "clip");
            image->setResolution(doc->image()->xRes(), doc->image()->yRes());

            KisPaintLayerSP layer = new KisPaintLayer(image, "clip", OPACITY_OPAQUE_U8, clonedProjection);
            image->addNode(layer);

            clipImage = image;
        } else {
            qWarning() << "Failed to import file" << url.toLocalFile();
        }
    }

    return clipImage;
}

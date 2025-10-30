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
#include <kis_layer_utils.h>
#include <kis_paint_device.h>
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
    bool pushedClipboard{};
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

void KisClipboard::setClip(KisPaintDeviceSP dev, const QPoint &topLeft, const KisTimeSpan &range)
{
    if (!dev)
        return;

    d->hasClip = true;

    // We'll create a store (ZIP format) in memory
    QBuffer buffer;
    const auto mimeType = QByteArrayLiteral("application/x-krita-selection");
    QScopedPointer<KoStore> store(KoStore::createStore(&buffer, KoStore::Write, mimeType));
    KisStorePaintDeviceWriter writer(store.data());
    Q_ASSERT(store);
    Q_ASSERT(!store->bad());

    // Layer data
    if (store->open("layerdata")) {
        if (!dev->write(writer)) {
            dev->disconnect();
            store->close();
            return;
        }
        store->close();
    }

    // copied frame time limits
    if (range.isValid() && store->open("timeRange")) {
        store->write(QString("%1 %2").arg(range.start()).arg(range.end()).toLatin1());
        store->close();
    }

    // Coordinates
    if (store->open("topLeft")) {
        store->write(QString("%1 %2").arg(topLeft.x()).arg(topLeft.y()).toLatin1());
        store->close();
    }
    // ColorSpace id of layer data
    if (store->open("colormodel")) {
        QString csName = dev->colorSpace()->colorModelId().id();
        store->write(csName.toLatin1());
        store->close();
    }
    if (store->open("colordepth")) {
        QString csName = dev->colorSpace()->colorDepthId().id();
        store->write(csName.toLatin1());
        store->close();
    }

    if (dev->colorSpace()->profile()) {
        const KoColorProfile *profile = dev->colorSpace()->profile();
        KisAnnotationSP annotation;

        if (profile && profile->type() == "icc" && !profile->rawData().isEmpty()) {
            annotation = new KisAnnotation("icc", profile->name(), profile->rawData());

            if (annotation) {
                // save layer profile
                if (store->open("profile.icc")) {
                    store->write(annotation->annotation());
                    store->close();
                }
            }
        }
    }

    QMimeData *mimeData = new QMimeData;
    Q_CHECK_PTR(mimeData);

    if (mimeData) {
        mimeData->setData(mimeType, buffer.buffer());
    }

    // We also create a QImage so we can interchange with other applications
    QImage qimage;
    const KisDisplayConfig displayConfig = KisMimeData::displayConfigForMimePastes();

    qimage = dev->convertToQImage(displayConfig.profile,
                                  displayConfig.intent,
                                  displayConfig.conversionFlags);
    if (!qimage.isNull() && mimeData) {
        mimeData->setImageData(qimage);
    }

    if (mimeData) {
        d->pushedClipboard = true;
        d->clipboard->setMimeData(mimeData);
    }
}

void KisClipboard::setClip(KisPaintDeviceSP dev, const QPoint &topLeft)
{
    setClip(dev, topLeft, KisTimeSpan());
}

KisPaintDeviceSP KisClipboard::clip(const QRect &imageBounds, bool showPopup, int overridePasteBehaviour, KisTimeSpan *clipRange) const
{
    const QMimeData *cbData = d->clipboard->mimeData();

    if (!cbData) {
        return nullptr;
    }

    dbgUI << Q_FUNC_INFO;
    dbgUI << "\tFormats: " << cbData->formats();
    dbgUI << "\tUrls: " << cbData->urls();
    dbgUI << "\tHas images: " << cbData->hasImage();

    return clipFromMimeData(cbData, imageBounds, showPopup, overridePasteBehaviour, clipRange, true);
}

KisPaintDeviceSP KisClipboard::clipFromMimeData(const QMimeData *cbData,
                                                const QRect &imageBounds,
                                                bool showPopup,
                                                int overridePasteBehaviour,
                                                KisTimeSpan *clipRange,
                                                bool useClipboardFallback) const
{
    if (clipRange) {
        *clipRange = KisTimeSpan();
    }

    KisPaintDeviceSP clip = clipFromKritaSelection(cbData, imageBounds, clipRange);

    if (!clip) {
        clip = clipFromBoardContents(cbData, imageBounds, showPopup, overridePasteBehaviour, useClipboardFallback);
    }

    return clip;
}

KisPaintDeviceSP KisClipboard::clipFromKritaSelection(const QMimeData *cbData, const QRect &imageBounds, KisTimeSpan *clipRange) const
{
    const QByteArray mimeType = QByteArrayLiteral("application/x-krita-selection");

    KisPaintDeviceSP clip;

    if (!cbData) {
        return nullptr;
    }

    if (cbData->hasFormat(mimeType)) {
        QByteArray encodedData = cbData->data(mimeType);
        QBuffer buffer(&encodedData);
        QScopedPointer<KoStore> store(KoStore::createStore(&buffer, KoStore::Read, mimeType));

        const KoColorProfile *profile = 0;

        QString csDepth, csModel;

        // ColorSpace id of layer data
        if (store->hasFile("colormodel")) {
            store->open("colormodel");
            csModel = QString(store->read(store->size()));
            store->close();
        }

        if (store->hasFile("colordepth")) {
            store->open("colordepth");
            csDepth = QString(store->read(store->size()));
            store->close();
        }

        if (store->hasFile("profile.icc")) {
            QByteArray data;
            store->open("profile.icc");
            data = store->read(store->size());
            store->close();
            profile = KoColorSpaceRegistry::instance()->createColorProfile(csModel, csDepth, data);
        }

        const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(csModel, csDepth, profile);
        if (cs) {
            clip = new KisPaintDevice(cs);

            if (store->hasFile("layerdata")) {
                store->open("layerdata");
                if (!clip->read(store->device())) {
                    clip = 0;
                }
                store->close();
            }

            if (clip && !imageBounds.isEmpty()) {
                // load topLeft
                if (store->hasFile("topLeft")) {
                    store->open("topLeft");
                    QString str = store->read(store->size());
                    store->close();
                    QStringList list = str.split(' ');
                    if (list.size() == 2) {
                        QPoint topLeft(list[0].toInt(), list[1].toInt());
                        clip->setX(topLeft.x());
                        clip->setY(topLeft.y());
                    }
                }

                QRect clipBounds = clip->exactBounds();

                if (!imageBounds.contains(clipBounds) && !imageBounds.intersects(clipBounds)) {
                    QPoint diff = imageBounds.center() - clipBounds.center();
                    clip->setX(clip->x() + diff.x());
                    clip->setY(clip->y() + diff.y());
                }

                if (store->hasFile("timeRange") && clipRange) {
                    store->open("timeRange");
                    QString str = store->read(store->size());
                    store->close();
                    QStringList list = str.split(' ');
                    if (list.size() == 2) {
                        KisTimeSpan range = KisTimeSpan::fromTimeToTime(list[0].toInt(), list[1].toInt());
                        *clipRange = range;
                        dbgUI << "Pasted time range" << range;
                    }
                }
            }
        }
    }

    return clip;
}

KisPaintDeviceSP KisClipboard::clipFromKritaLayers(const KoColorSpace *cs) const
{
    const QMimeData *data = KisClipboard::instance()->layersMimeData();

    if (!data) {
        return nullptr;
    }

    const KisMimeData *mimedata = qobject_cast<const KisMimeData *>(data);
    if (!mimedata) return 0;

    KisNodeList nodes = mimedata->nodes();

    if (nodes.size() > 1) {
        // we explicitly include point (0,0) into the bounds since that
        // is a requirement for the image
        QRect bounds = QRect(0, 0, 1, 1);
        Q_FOREACH (KisNodeSP node, nodes) {
            bounds |= node->exactBounds();
        }

        KisImageSP tempImage = new KisImage(nullptr,
                                            bounds.width(),
                                            bounds.height(),
                                            cs,
                                            "ClipImage");
        for (KisNodeSP node : nodes) {
            tempImage->addNode(node, tempImage->root());
        }
        tempImage->refreshGraphAsync();
        KisLayerUtils::refreshHiddenAreaAsync(tempImage, tempImage->root(), bounds);
        tempImage->waitForDone();

        return tempImage->projection();
    } else if (!nodes.isEmpty()) {
        return nodes.first()->projection();
    }

    return nullptr;
}

QPair<bool, KisClipboard::PasteFormatBehaviour>
KisClipboard::askUserForSource(const QMimeData *cbData,
                               bool useClipboardFallback) const
{
    if (!cbData) {
        return {false, PASTE_FORMAT_ASK};
    }

    KisConfig cfg(true);

    bool saveSourceSetting = false;

    auto choice = (PasteFormatBehaviour)cfg.pasteFormat(false);

    const QImage qimage = [&]() {
        QImage qimage = getImageFromMimeData(cbData);

        if (qimage.isNull() && useClipboardFallback) {
            qimage = d->clipboard->image();
        }

        return qimage;
    }();

    if (!qimage.isNull() || cbData->hasUrls()) {
        const auto &urls = cbData->urls();

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
        dbgUI << "\tHas attached bitmap:" << cbData->hasImage();
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

KisPaintDeviceSP KisClipboard::clipFromBoardContents(const QMimeData *cbData,
                                                     const QRect &imageBounds,
                                                     bool showPopup,
                                                     int pasteBehaviourOverride,
                                                     bool useClipboardFallback,
                                                     QPair<bool, PasteFormatBehaviour> source) const
{
    if (!cbData) {
        return nullptr;
    }

    KisPaintDeviceSP clip;

    PasteFormatBehaviour choice = PASTE_FORMAT_ASK;

    if (!source.first) {
        choice = askUserForSource(cbData).second;
    } else {
        choice = source.second;
    }

    if (choice == PASTE_FORMAT_CLIP) {
        const QImage qimage = [&]() {
            QImage qimage = getImageFromMimeData(cbData);

            if (qimage.isNull() && useClipboardFallback) {
                qimage = d->clipboard->image();
            }

            return qimage;
        }();

        KIS_SAFE_ASSERT_RECOVER(!qimage.isNull())
        {
            warnKrita << "Clipboard was cleared before loading image";
            return nullptr;
        }

        int behaviour = pasteBehaviourOverride;
        bool saveColorSetting = false;

        KisConfig cfg(true);

        if (pasteBehaviourOverride == -1) {
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

        clip = new KisPaintDevice(cs);
        Q_CHECK_PTR(clip);
        clip->convertFromQImage(qimage, profile);

        // save the user's selection to the configuration if the option is checked
        if (saveColorSetting) {
            cfg.setPasteBehaviour(behaviour);
        }
    } else {
        const auto &urls = cbData->urls();
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
            clip = fetchImageByURL(*url);
        }
    }

    if (clip && !imageBounds.isEmpty()) {
        QRect clipBounds = clip->exactBounds();
        QPoint diff = imageBounds.center() - clipBounds.center();
        clip->setX(diff.x());
        clip->setY(diff.y());
    }

    return clip;
}

void KisClipboard::clipboardDataChanged()
{
    if (!d->pushedClipboard) {
        const QMimeData *cbData = d->clipboard->mimeData();
        d->hasClip = d->clipboard->mimeData()->hasImage()
                || (cbData && cbData->hasFormat("application/x-krita-selection"));
    }
    d->pushedClipboard = false;
    Q_EMIT clipChanged();
}

bool KisClipboard::hasClip() const
{
    return d->hasClip;
}

QSize KisClipboard::clipSize() const
{
    const auto mimeType = QByteArrayLiteral("application/x-krita-selection");
    const QMimeData *cbData = d->clipboard->mimeData();

    KisPaintDeviceSP clip;

    if (cbData && cbData->hasFormat(mimeType)) {
        QByteArray encodedData = cbData->data(mimeType);
        QBuffer buffer(&encodedData);
        QScopedPointer<KoStore> store(KoStore::createStore(&buffer, KoStore::Read, mimeType));
        const KoColorProfile *profile = 0;
        QString csDepth, csModel;

        // ColorSpace id of layer data
        if (store->hasFile("colormodel")) {
            store->open("colormodel");
            csModel = QString(store->read(store->size()));
            store->close();
        }

        if (store->hasFile("colordepth")) {
            store->open("colordepth");
            csDepth = QString(store->read(store->size()));
            store->close();
        }

        if (store->hasFile("profile.icc")) {
            QByteArray data;
            store->open("profile.icc");
            data = store->read(store->size());
            store->close();
            profile = KoColorSpaceRegistry::instance()->createColorProfile(csModel, csDepth, data);
        }

        const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(csModel, csDepth, profile);
        if (!cs) {
            cs = KoColorSpaceRegistry::instance()->rgb8();
        }
        clip = new KisPaintDevice(cs);

        if (store->hasFile("layerdata")) {
            store->open("layerdata");
            clip->read(store->device());
            store->close();
        }

        return clip->exactBounds().size();
    } else {
        if (d->clipboard->mimeData()->hasImage()) {
            QImage qimage = d->clipboard->image();
            return qimage.size();
        }
    }
    return QSize();
}

void KisClipboard::setLayers(KisNodeList nodes, KisImageSP image, bool forceCopy)
{
    /**
     * See a comment in KisMimeData::deepCopyNodes()
     */
    QMimeData *data = KisMimeData::mimeForLayersDeepCopy(nodes, image, forceCopy);
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

KisPaintDeviceSP KisClipboard::fetchImageByURL(const QUrl &originalUrl) const
{
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
            return result;
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
            return result;
        }

        QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

        if (doc->openPath(url.toLocalFile(), KisDocument::DontAddToRecent)) {
            // Wait for required updates, if any. BUG: 448256
            KisLayerUtils::forceAllDelayedNodesUpdate(doc->image()->root());
            doc->image()->waitForDone();
            result = new KisPaintDevice(*doc->image()->projection());
        } else {
            qWarning() << "Failed to import file" << url.toLocalFile();
        }
    }

    return result;
}

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
#include <QDesktopWidget>
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
    connect(d->clipboard, &QClipboard::dataChanged, this, &KisClipboard::clipboardDataChanged);
    connect(d->clipboard, &QClipboard::selectionChanged, this, &KisClipboard::clipboardDataChanged);
    connect(d->clipboard, &QClipboard::changed, this, &KisClipboard::clipboardDataChanged);
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
    KisConfig cfg(true);
    const KoColorProfile *monitorProfile =
        cfg.displayProfile(QApplication::desktop()->screenNumber(qApp->activeWindow()));
    qimage = dev->convertToQImage(monitorProfile,
                                  KoColorConversionTransformation::internalRenderingIntent(),
                                  KoColorConversionTransformation::internalConversionFlags());
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

KisPaintDeviceSP
KisClipboard::clip(const QRect &imageBounds, bool showPopup, int overridePasteBehaviour, KisTimeSpan *clipRange) const
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

KisPaintDeviceSP
KisClipboard::clipFromKritaSelection(const QMimeData *cbData, const QRect &imageBounds, KisTimeSpan *clipRange) const
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

KisPaintDeviceSP KisClipboard::clipFromBoardContents(const QMimeData *cbData,
                                                     const QRect &imageBounds,
                                                     bool showPopup,
                                                     int pasteBehaviourOverride,
                                                     bool useClipboardFallback) const
{
    KisPaintDeviceSP clip;

    if (!cbData) {
        return nullptr;
    }

    KisConfig cfg(true);

    bool saveSourceSetting = false;

    auto choice = (PasteFormatBehaviour)cfg.pasteFormat(false);

    if ((cbData->hasImage() || cbData->hasUrls())) {
        const auto &urls = cbData->urls();

        bool local = false;
        bool remote = false;

        std::for_each(urls.constBegin(), urls.constEnd(), [&](const QUrl &url) {
            local |= url.isLocalFile();
            remote |= !url.isLocalFile();
        });

        const bool hasMultipleFormatsAvailable =
            (remote && local) || (remote && cbData->hasImage()) || (local && cbData->hasImage());

        const bool defaultOptionUnavailable = (!remote && choice == PASTE_FORMAT_DOWNLOAD)
            || (!local && choice == PASTE_FORMAT_LOCAL) || (!cbData->hasImage() && choice == PASTE_FORMAT_CLIP);

        dbgUI << "Incoming paste event:";
        dbgUI << "\tHas attached bitmap:" << cbData->hasImage();
        dbgUI << "\tHas local images:" << local;
        dbgUI << "\tHas remote images:" << remote;
        dbgUI << "\tHas multiple formats:" << hasMultipleFormatsAvailable;
        dbgUI << "\tDefault source preference" << choice;
        dbgUI << "\tDefault source available:" << !defaultOptionUnavailable;

        if (hasMultipleFormatsAvailable && choice == PASTE_FORMAT_ASK) {
            KisDlgPasteFormat dlg(qApp->activeWindow());

            dlg.setSourceAvailable(PASTE_FORMAT_DOWNLOAD, remote);
            dlg.setSourceAvailable(PASTE_FORMAT_LOCAL, local);
            dlg.setSourceAvailable(PASTE_FORMAT_CLIP, cbData->hasImage());

            if (dlg.exec() != KoDialog::Accepted) {
                return nullptr;
            };

            choice = dlg.source();

            saveSourceSetting = dlg.remember();
        } else if (defaultOptionUnavailable || choice == PASTE_FORMAT_ASK) {
            if (remote) {
                choice = PASTE_FORMAT_DOWNLOAD;
            } else if (local) {
                choice = PASTE_FORMAT_LOCAL;
            } else if (cbData->hasImage()) {
                choice = PASTE_FORMAT_CLIP;
            } else {
                return nullptr;
            }
        }
    }

    if (saveSourceSetting) {
        cfg.setPasteFormat(choice);
    }

    dbgUI << "Selected source for the paste:" << choice;

    if (choice == PASTE_FORMAT_CLIP) {
        QImage qimage = getImageFromMimeData(cbData);

        if (qimage.isNull() && useClipboardFallback) {
            qimage = d->clipboard->image();
        }

        KIS_ASSERT(!qimage.isNull());

        int behaviour = pasteBehaviourOverride;
        bool saveColorSetting = false;

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
            profile = cfg.displayProfile(QApplication::desktop()->screenNumber(qApp->activeWindow()));

        cs = KoColorSpaceRegistry::instance()->rgb8(profile);
        if (!cs) {
            cs = KoColorSpaceRegistry::instance()->rgb8();
            profile = cs->profile();
        }

        clip = new KisPaintDevice(cs);
        Q_CHECK_PTR(clip);
        clip->convertFromQImage(qimage, profile);

        // save the persion's selection to the configuration if the option is checked
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
        d->hasClip = false;

        if (d->clipboard->mimeData()->hasImage()) {
            QImage qimage = d->clipboard->image();
            if (!qimage.isNull())
                d->hasClip = true;

            const QMimeData *cbData = d->clipboard->mimeData();
            const auto mimeType = QByteArrayLiteral("application/x-krita-selection");
            if (cbData && cbData->hasFormat(mimeType))
                d->hasClip = true;
        }
    }
    if (d->hasClip) {
        emit clipCreated();
    }
    d->pushedClipboard = false;
    emit clipChanged();
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
    return d->clipboard->mimeData()->hasFormat("application/x-krita-node");
}

bool KisClipboard::hasLayerStyles() const
{
    // NOTE: please don't disable the pacte action based on the
    //       result of this function, because we allow pasting
    //       of the layer styles as 'text/plain'

    return d->clipboard->mimeData()->hasFormat("application/x-krita-layer-style");
}

const QMimeData *KisClipboard::layersMimeData() const
{
    const QMimeData *cbData = d->clipboard->mimeData();
    return cbData->hasFormat("application/x-krita-node") ? cbData : 0;
}

QImage KisClipboard::getPreview() const
{
    const QMimeData *cbData = d->clipboard->mimeData();

    QImage img;

    for (QUrl &url : cbData->urls()) {
        if (url.isLocalFile()) {
            img.load(url.path());

            if (!img.isNull())
                break;
        }
    }

    if (img.isNull() && cbData->hasImage()) {
        img = d->clipboard->image();
    }

    return img;
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

        if (doc->importDocument(url.toLocalFile())) {
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

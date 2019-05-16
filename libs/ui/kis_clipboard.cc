/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_clipboard.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QMimeData>
#include <QObject>
#include <QImage>
#include <QMessageBox>
#include <QCheckBox>
#include <QBuffer>
#include <QGlobalStatic>

#include <klocalizedstring.h>

#include "KoColorSpace.h"
#include "KoStore.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>

#include <KisMimeDatabase.h>

// kritaimage
#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_debug.h>
#include <kis_annotation.h>
#include <kis_node.h>
#include <kis_image.h>
#include <kis_time_range.h>

// local
#include "kis_config.h"
#include "kis_store_paintdevice_writer.h"
#include "kis_mimedata.h"

Q_GLOBAL_STATIC(KisClipboard, s_instance)

KisClipboard::KisClipboard()
{
    m_pushedClipboard = false;
    m_hasClip = false;

    // Check that we don't already have a clip ready
    clipboardDataChanged();

    // Make sure we are notified when clipboard changes
    connect(QApplication::clipboard(), SIGNAL(dataChanged()),
            this, SLOT(clipboardDataChanged()));


}

KisClipboard::~KisClipboard()
{
    dbgRegistry << "deleting KisClipBoard";
}

KisClipboard* KisClipboard::instance()
{
    return s_instance;
}

void KisClipboard::setClip(KisPaintDeviceSP dev, const QPoint& topLeft, const KisTimeRange &range)
{
    if (!dev)
        return;

    m_hasClip = true;

    // We'll create a store (ZIP format) in memory
    QBuffer buffer;
    QByteArray mimeType("application/x-krita-selection");
    KoStore* store = KoStore::createStore(&buffer, KoStore::Write, mimeType);
    KisStorePaintDeviceWriter writer(store);
    Q_ASSERT(store);
    Q_ASSERT(!store->bad());

    // Layer data
    if (store->open("layerdata")) {
        if (!dev->write(writer)) {
            dev->disconnect();
            store->close();
            delete store;
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
            annotation = new  KisAnnotation("icc", profile->name(), profile->rawData());

            if (annotation) {
                // save layer profile
                if (store->open("profile.icc")) {
                    store->write(annotation->annotation());
                    store->close();
                }
            }
        }
    }

    delete store;

    QMimeData *mimeData = new QMimeData;
    Q_CHECK_PTR(mimeData);

    if (mimeData) {
        mimeData->setData(mimeType, buffer.buffer());
    }

    // We also create a QImage so we can interchange with other applications
    QImage qimage;
    KisConfig cfg(true);
    const KoColorProfile *monitorProfile = cfg.displayProfile(QApplication::desktop()->screenNumber(qApp->activeWindow()));
    qimage = dev->convertToQImage(monitorProfile, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    if (!qimage.isNull() && mimeData) {
        mimeData->setImageData(qimage);
    }

    if (mimeData) {
        m_pushedClipboard = true;
        QClipboard *cb = QApplication::clipboard();
        cb->setMimeData(mimeData);
    }

}

void KisClipboard::setClip(KisPaintDeviceSP dev, const QPoint& topLeft)
{
    setClip(dev, topLeft, KisTimeRange());
}

KisPaintDeviceSP KisClipboard::clip(const QRect &imageBounds, bool showPopup, KisTimeRange *clipRange)
{
    QByteArray mimeType("application/x-krita-selection");

    if (clipRange) {
        *clipRange = KisTimeRange();
    }

    QClipboard *cb = QApplication::clipboard();
    const QMimeData *cbData = cb->mimeData();

    KisPaintDeviceSP clip;

    if (cbData && cbData->hasFormat(mimeType)) {
        QByteArray encodedData = cbData->data(mimeType);
        QBuffer buffer(&encodedData);
        KoStore* store = KoStore::createStore(&buffer, KoStore::Read, mimeType);

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

                if (!imageBounds.contains(clipBounds) &&
                    !imageBounds.intersects(clipBounds)) {

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
                        KisTimeRange range(list[0].toInt(), list[1].toInt(), true);
                        *clipRange = range;
                        qDebug() << "Pasted time range" << range;
                    }
                }
            }
        }

        delete store;
    }

    if (!clip) {

        QStringList formats = cb->mimeData()->formats();

        QMap<int, QImage> qimages;

        Q_FOREACH(const QString format, formats) {
//            qDebug() << "Format" << format;
            QImage image;
            if (format.startsWith("image")) {
                int priority = -1;

                QByteArray ba = cb->mimeData()->data(format);

//                qDebug() << "data" << ba.size() << "mime" << KisMimeDatabase::mimeTypeForData(ba);

                if (ba.isEmpty()) {
                    continue;
                }

                QString qimageioFormat;

                if (format == "image/png") {
                    qimageioFormat = "PNG";
                    priority = 0;
                }
                else if (format == "image/tiff") {
                    qimageioFormat = "TIFF";
                    priority = 1;
                }
                else if (format == "image/bmp"
                         || format == "image/x-bmp"
                         || format == "image/x-MS-bmp"
                         || format == "image/x-win-bitmap") {

                    qimageioFormat = "BMP";
                    priority = 2;
                }
                else if (format == "image/jpeg") {
                    qimageioFormat = "JPG";
                    priority = 3;
                }

                if (!qimageioFormat.isEmpty()) {
                    image.loadFromData(ba, qimageioFormat.toLatin1());
                }

                if (!image.isNull()) {
//                    qDebug() << "Loaded image succesfully" << image.format() << image.hasAlphaChannel();
                    qimages[priority] = image;
                }
//                else {
//                    qDebug() << "Could not load image of type" << format;
//                }
            }
        }

        QImage qimage = cb->image();

        for(int i = 0; i < 4; i++) {
            if (qimages.contains(i)) {
                qimage = qimages[i];
                break;
            }
        }

//        qDebug() << "Final QImage" << qimage.format() << qimage.hasAlphaChannel();

        if (qimage.isNull()) {
            return KisPaintDeviceSP(0);
        }

        KisConfig cfg(true);
        quint32 behaviour = cfg.pasteBehaviour();
        bool saveColorSetting = false;


        if (behaviour == PASTE_ASK && showPopup) {
            // Ask user each time.
            QMessageBox mb(qApp->activeWindow());
            QCheckBox dontPrompt(i18n("Remember"), &mb);

            dontPrompt.blockSignals(true);

            mb.setWindowTitle(i18nc("@title:window", "Missing Color Profile"));
            mb.setText(i18n("The image data you are trying to paste has no color profile information. How do you want to interpret these data? \n\n As Web (sRGB) -  Use standard colors that are displayed from computer monitors.  This is the most common way that images are stored. \n\nAs on Monitor - If you know a bit about color management and want to use your monitor to determine the color profile.\n\n"));

            // the order of how you add these buttons matters as it determines the index.
            mb.addButton(i18n("As &Web"), QMessageBox::AcceptRole);
            mb.addButton(i18n("As on &Monitor"), QMessageBox::AcceptRole);
            mb.addButton(i18n("Cancel"), QMessageBox::RejectRole);
            mb.addButton(&dontPrompt, QMessageBox::ActionRole);

            behaviour = mb.exec();

            if (behaviour > 1) {
                return 0;
            }

            saveColorSetting = dontPrompt.isChecked(); // should we save this option to the config for next time?


        }

        const KoColorSpace * cs;
        const KoColorProfile *profile = 0;
        if (behaviour == PASTE_ASSUME_MONITOR)
            profile = cfg.displayProfile(QApplication::desktop()->screenNumber(qApp->activeWindow()));

        cs = KoColorSpaceRegistry::instance()->rgb8(profile);
        if (!cs) {
            cs = KoColorSpaceRegistry::instance()->rgb8();
            profile = cs->profile();
        }

        clip = new KisPaintDevice(cs);
        Q_CHECK_PTR(clip);
        clip->convertFromQImage(qimage, profile);

        QRect clipBounds = clip->exactBounds();
        QPoint diff = imageBounds.center() - clipBounds.center();
        clip->setX(diff.x());
        clip->setY(diff.y());

        // save the persion's selection to the configuration if the option is checked
        if (saveColorSetting) {
            cfg.setPasteBehaviour(behaviour);
        }

    }

    return clip;
}

void KisClipboard::clipboardDataChanged()
{
    if (!m_pushedClipboard) {
        m_hasClip = false;
        QClipboard *cb = QApplication::clipboard();
        if (cb->mimeData()->hasImage()) {

            QImage qimage = cb->image();
            if (!qimage.isNull())
                m_hasClip = true;

            const QMimeData *cbData = cb->mimeData();
            QByteArray mimeType("application/x-krita-selection");
            if (cbData && cbData->hasFormat(mimeType))
                m_hasClip = true;
        }
    }
    if (m_hasClip) {
        emit clipCreated();
    }
    m_pushedClipboard = false;
    emit clipChanged();
}


bool KisClipboard::hasClip() const
{
    return m_hasClip;
}

QSize KisClipboard::clipSize() const
{

    QClipboard *cb = QApplication::clipboard();
    QByteArray mimeType("application/x-krita-selection");
    const QMimeData *cbData = cb->mimeData();

    KisPaintDeviceSP clip;

    if (cbData && cbData->hasFormat(mimeType)) {
        QByteArray encodedData = cbData->data(mimeType);
        QBuffer buffer(&encodedData);
        KoStore* store = KoStore::createStore(&buffer, KoStore::Read, mimeType);
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
        delete store;

        return clip->exactBounds().size();
    } else {
        if (cb->mimeData()->hasImage()) {
            QImage qimage = cb->image();
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
    if (!data) return;

    QClipboard *cb = QApplication::clipboard();
    cb->setMimeData(data);
}

bool KisClipboard::hasLayers() const
{
    QClipboard *cb = QApplication::clipboard();
    const QMimeData *cbData = cb->mimeData();
    return cbData->hasFormat("application/x-krita-node");
}

const QMimeData* KisClipboard::layersMimeData() const
{
    QClipboard *cb = QApplication::clipboard();
    const QMimeData *cbData = cb->mimeData();
    return cbData->hasFormat("application/x-krita-node") ? cbData : 0;
}


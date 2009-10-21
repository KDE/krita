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
#include <QMimeData>
#include <QObject>
#include <QImage>
#include <QMessageBox>
#include <QBuffer>

#include <klocale.h>
#include <kglobal.h>

#include "KoColorSpace.h"
#include "KoStore.h"
#include <KoColorSpaceRegistry.h>
#include <colorprofiles/KoIccColorProfile.h>

// kritaimage
#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_debug.h>
#include <kis_annotation.h>

// local
#include "kis_factory2.h"
#include "kis_config.h"

KisClipboard::KisClipboard()
{
    m_pushedClipboard = false;
    m_hasClip = false;
    m_clip = 0;

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
    K_GLOBAL_STATIC(KisClipboard, s_instance);
    qAddPostRoutine(s_instance.destroy); // make sure we get destroyed first.
    return s_instance;
}

void KisClipboard::setClip(KisPaintDeviceSP selection)
{
    m_clip = selection;

    if (!selection)
        return;

    m_hasClip = true;

    // We'll create a store (ZIP format) in memory
    QBuffer buffer;
    QByteArray mimeType("application/x-krita-selection");
    KoStore* store = KoStore::createStore(&buffer, KoStore::Write, mimeType);
    Q_ASSERT(store);
    Q_ASSERT(!store->bad());

    // Layer data
    if (store->open("layerdata")) {
        if (!selection->write(store)) {
            selection->disconnect();
            store->close();
            return;
        }
        store->close();
    }

    // ColorSpace id of layer data
    if (store->open("colorspace")) {
        QString csName = selection->colorSpace()->id();
        store->write(csName.toAscii(), strlen(csName.toAscii()));
        store->close();
    }

    if (selection->colorSpace()->profile()) {
        const KoColorProfile *profile = selection->colorSpace()->profile();
        KisAnnotationSP annotation;
        if (profile) {
            const KoIccColorProfile* iccprofile = dynamic_cast<const KoIccColorProfile*>(profile);
            if (iccprofile && !iccprofile->rawData().isEmpty())
                annotation = new  KisAnnotation("icc", iccprofile->name(), iccprofile->rawData());
        }
        if (annotation) {
            // save layer profile
            if (store->open("profile.icc")) {
                store->write(annotation->annotation());
                store->close();
            }
        }
    }

    delete store;

    // We also create a QImage so we can interchange with other applications
    QImage qimg;
    KisConfig cfg;
    QString monitorProfileName = cfg.monitorProfile();
    const KoColorProfile *  monitorProfile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
    qimg = selection->convertToQImage(monitorProfile);

    QMimeData *mimeData = new QMimeData;
    Q_CHECK_PTR(mimeData);

    if (mimeData) {
        if (!qimg.isNull()) {
            mimeData->setImageData(qimg);
        }

        mimeData->setData(mimeType, buffer.buffer());

        m_pushedClipboard = true;
        QClipboard *cb = QApplication::clipboard();
        cb->setMimeData(mimeData);
    }
}

KisPaintDeviceSP KisClipboard::clip()
{
    QClipboard *cb = QApplication::clipboard();
    QByteArray mimeType("application/x-krita-selection");
    const QMimeData *cbData = cb->mimeData();

    if (cbData && cbData->hasFormat(mimeType)) {
        QByteArray encodedData = cbData->data(mimeType);
        QBuffer buffer(&encodedData);
        KoStore* store = KoStore::createStore(&buffer, KoStore::Read, mimeType);
        KoColorProfile *profile = 0;

        if (store->hasFile("profile.icc")) {
            QByteArray data;
            store->open("profile.icc");
            data = store->read(store->size());
            store->close();
            profile = new KoIccColorProfile(data);
        }

        QString csName;
        // ColorSpace id of layer data
        if (store->hasFile("colorspace")) {
            store->open("colorspace");
            csName = QString(store->read(store->size()));
            store->close();
        }

        const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(csName, profile);

        m_clip = new KisPaintDevice(cs);

        if (store->hasFile("layerdata")) {
            store->open("layerdata");
            m_clip->read(store);
            store->close();
        }
        delete store;
    } else {
        QImage qimg = cb->image();

        if (qimg.isNull())
            return KisPaintDeviceSP(0);

        KisConfig cfg;

        quint32 behaviour = cfg.pasteBehaviour();

        if (behaviour == PASTE_ASK) {
            // Ask user each time
            behaviour = QMessageBox::question(0, i18n("Pasting data from simple source"), i18n("The image data you are trying to paste has no color profile information.\n\nOn the web and in simple applications the data are supposed to be in sRGB color format.\nImporting as web will show it as it is supposed to look.\nMost monitors are not perfect though so if you made the image yourself\nyou might want to import it as it looked on you monitor.\n\nHow do you want to interpret these data?"), i18n("As &Web"), i18n("As on &Monitor"));
        }

        const KoColorSpace * cs;
        QString profileName("");
        if (behaviour == PASTE_ASSUME_MONITOR)
            profileName = cfg.monitorProfile();

        cs = KoColorSpaceRegistry::instance() ->colorSpace("RGBA", profileName);
        if (!cs) {
            cs = KoColorSpaceRegistry::instance()->rgb8();
            profileName = cs->profile()->name();
        }

        m_clip = new KisPaintDevice(cs);
        Q_CHECK_PTR(m_clip);
        m_clip->convertFromQImage(qimg, profileName);
    }

    return m_clip;
}

void KisClipboard::clipboardDataChanged()
{
    if (!m_pushedClipboard) {
        m_hasClip = false;
        QClipboard *cb = QApplication::clipboard();
        QImage qimg = cb->image();
        const QMimeData *cbData = cb->mimeData();
        QByteArray mimeType("application/x-krita-selection");

        if (cbData && cbData->hasFormat(mimeType))
            m_hasClip = true;

        if (!qimg.isNull())
            m_hasClip = true;
    }

    m_pushedClipboard = false;
}


bool KisClipboard::hasClip()
{
    return m_hasClip;
}

QSize KisClipboard::clipSize()
{

    QClipboard *cb = QApplication::clipboard();
    QByteArray mimeType("application/x-krita-selection");
    const QMimeData *cbData = cb->mimeData();

    KisPaintDeviceSP clip;

    if (cbData && cbData->hasFormat(mimeType)) {
        QByteArray encodedData = cbData->data(mimeType);
        QBuffer buffer(&encodedData);
        KoStore* store = KoStore::createStore(&buffer, KoStore::Read, mimeType);
        KoColorProfile *profile = 0;

        if (store->hasFile("profile.icc")) {
            QByteArray data;
            store->open("profile.icc");
            data = store->read(store->size());
            store->close();
            profile = new KoIccColorProfile(data);
        }

        QString csName;
        // ColorSpace id of layer data
        if (store->hasFile("colorspace")) {
            store->open("colorspace");
            csName = QString(store->read(store->size()));
            store->close();
        }

        const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(csName, profile);

        clip = new KisPaintDevice(cs);

        if (store->hasFile("layerdata")) {
            store->open("layerdata");
            clip->read(store);
            store->close();
        }
        delete store;

        return clip->exactBounds().size();
    } else {
        QImage qimg = cb->image();
        return qimg.size();
    }
}

#include "kis_clipboard.moc"

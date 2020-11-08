/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#include <QApplication>
#include <QMessageBox>

#include "KisViewManager.h"
#include "kis_algebra_2d.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_image_barrier_locker.h"
#include "kis_image_interface.h"
#include "kis_input_output_mapper.h"
#include "kis_qmic_applicator.h"
#include "kis_qmic_simple_convertor.h"
#include "kis_selection.h"

#include "gmic.h"

struct KisImageInterface::Private {
    Private() = default;

    ~Private() {
        Q_FOREACH (QSharedMemory *memorySegment, m_sharedMemorySegments) {
            //        dbgPlugins << "detaching" << memorySegment->key();
            memorySegment->detach();
        }
        qDeleteAll(m_sharedMemorySegments);
        m_sharedMemorySegments.clear();
    }

    KisViewManager *m_viewManager {nullptr};
    InputLayerMode m_inputMode {ACTIVE_LAYER};
    OutputMode m_outputMode {IN_PLACE};
    QVector<QSharedMemory *> m_sharedMemorySegments {};
    KisQmicApplicator *m_gmicApplicator {nullptr};
};

KisImageInterface::KisImageInterface(KisViewManager *parent)
    : p(new Private)
{
    p->m_viewManager = parent;
    KIS_ASSERT(p->m_viewManager);

    p->m_gmicApplicator = new KisQmicApplicator();
    connect(p->m_gmicApplicator, SIGNAL(gmicFinished(bool, int, QString)), this, SLOT(slotGmicFinished(bool, int, QString)));
}

QSize KisImageInterface::gmic_qt_get_image_size()
{
    KisSelectionSP selection = p->m_viewManager->image()->globalSelection();

    if (selection) {
        QRect selectionRect = selection->selectedExactRect();
        return selectionRect.size();
    } else {
        return p->m_viewManager->image()->size();
    }
}

QByteArray KisImageInterface::gmic_qt_get_cropped_images(int inputMode, QRectF &rc)
{
    // Parse the message, create the shared memory segments, and create a new message to send back and waid for ack

    QByteArray message;

    if (!p->m_viewManager)
        return {};

    KisImageBarrierLocker locker(p->m_viewManager->image());

    p->m_inputMode = (InputLayerMode)inputMode;

    dbgPlugins << "prepareCroppedImages()" << QString::fromUtf8(message) << rc << inputMode;

    KisInputOutputMapper mapper(p->m_viewManager->image(), p->m_viewManager->activeNode());
    KisNodeListSP nodes = mapper.inputNodes(p->m_inputMode);
    if (nodes->isEmpty()) {
        return {};
    }

    for (int i = 0; i < nodes->size(); ++i) {
        KisNodeSP node = nodes->at(i);
        if (node && node->paintDevice()) {
            QRect cropRect;

            KisSelectionSP selection = p->m_viewManager->image()->globalSelection();

            if (selection) {
                cropRect = selection->selectedExactRect();
            } else {
                cropRect = p->m_viewManager->image()->bounds();
            }

            dbgPlugins << "Converting node" << node->name() << cropRect;

            const QRectF mappedRect = KisAlgebra2D::mapToRect(cropRect).mapRect(rc);
            const QRect resultRect = mappedRect.toAlignedRect();

            QSharedMemory *m = new QSharedMemory(QString("key_%1").arg(QUuid::createUuid().toString()));
            p->m_sharedMemorySegments.append(m);
            if (!m->create(resultRect.width() * resultRect.height() * 4 * sizeof(float))) { // buf.size())) {
                qWarning() << "Could not create shared memory segment" << m->error() << m->errorString();
                return {};
            }
            m->lock();

            gmic_image<float> img;
            img.assign(resultRect.width(), resultRect.height(), 1, 4);
            img._data = reinterpret_cast<float *>(m->data());

            KisQmicSimpleConvertor::convertToGmicImageFast(node->paintDevice(), &img, resultRect);

            message.append(m->key().toUtf8());

            m->unlock();

            message.append(",");
            message.append(node->name().toUtf8().toHex());
            message.append(",");
            message.append(QByteArray::number(resultRect.width()));
            message.append(",");
            message.append(QByteArray::number(resultRect.height()));

            message.append("\n");
        }
    }

    dbgPlugins << QString::fromUtf8(message);

    return message;
}

void KisImageInterface::gmic_qt_output_images(int mode, QStringList layers)
{
    // Parse the message. read the shared memory segments, fix up the current image and send an ack
    dbgPlugins << "gmic_qt_output_images";
    p->m_outputMode = (OutputMode)mode;
    if (p->m_outputMode != IN_PLACE) {
        QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Sorry, this output mode is not implemented yet."));
        p->m_outputMode = IN_PLACE;
    }
    slotStartApplicator(layers);
}

void KisImageInterface::gmic_qt_detach()
{
    for (QSharedMemory *memorySegment : p->m_sharedMemorySegments) {
        dbgPlugins << "detaching" << memorySegment->key() << memorySegment->isAttached();
        if (memorySegment->isAttached()) {
            if (!memorySegment->detach()) {
                dbgPlugins << "\t" << memorySegment->error() << memorySegment->errorString();
            }
        }
    }
    qDeleteAll(p->m_sharedMemorySegments);
    p->m_sharedMemorySegments.clear();
}

void KisImageInterface::slotStartApplicator(QStringList gmicImages)
{
    dbgPlugins << "slotStartApplicator();" << gmicImages;
    if (!p->m_viewManager)
        return;
    // Create a vector of gmic images

    QVector<gmic_image<float> *> images;

    Q_FOREACH (const QString &image, gmicImages) {
        QStringList parts = image.split(',', QString::SkipEmptyParts);
        KIS_SAFE_ASSERT_RECOVER_BREAK(parts.size() == 5);
        QString key = parts[0];
        QString layerName = QByteArray::fromHex(parts[1].toLatin1());
        int spectrum = parts[2].toInt();
        int width = parts[3].toInt();
        int height = parts[4].toInt();

        dbgPlugins << key << layerName << width << height;

        QSharedMemory m(key);
        if (!m.attach(QSharedMemory::ReadOnly)) {
            qWarning() << "Could not attach to shared memory area." << m.error() << m.errorString();
        }
        if (m.isAttached()) {
            if (!m.lock()) {
                dbgPlugins << "Could not lock memory segment" << m.error() << m.errorString();
            }
            dbgPlugins << "Memory segment" << key << m.size() << m.constData() << m.data();
            gmic_image<float> *gimg = new gmic_image<float>();
            gimg->assign(width, height, 1, spectrum);
            gimg->name = layerName;

            gimg->_data = new float[width * height * spectrum * sizeof(float)];
            dbgPlugins << "width" << width << "height" << height << "size" << width * height * spectrum * sizeof(float) << "shared memory size" << m.size();
            memcpy(gimg->_data, m.constData(), width * height * spectrum * sizeof(float));

            dbgPlugins << "created gmic image" << gimg->name << gimg->_width << gimg->_height;

            if (!m.unlock()) {
                dbgPlugins << "Could not unlock memory segment" << m.error() << m.errorString();
            }
            if (!m.detach()) {
                dbgPlugins << "Could not detach from memory segment" << m.error() << m.errorString();
            }
            images.append(gimg);
        }
    }

    dbgPlugins << "Got" << images.size() << "gmic images";

    // Start the applicator
    KUndo2MagicString actionName = kundo2_i18n("Gmic filter");
    KisNodeSP rootNode = p->m_viewManager->image()->root();
    KisInputOutputMapper mapper(p->m_viewManager->image(), p->m_viewManager->activeNode());
    KisNodeListSP layers = mapper.inputNodes(p->m_inputMode);

    p->m_gmicApplicator->setProperties(p->m_viewManager->image(), rootNode, images, actionName, layers);
    p->m_gmicApplicator->apply();
}

void KisImageInterface::slotGmicFinished(bool successfully, int milliseconds, const QString &msg)
{
    dbgPlugins << "slotGmicFinished();" << successfully << milliseconds << msg;
    if (successfully) {
        p->m_gmicApplicator->finish();
    } else {
        p->m_gmicApplicator->cancel();
        QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("G'Mic failed, reason:") + msg);
    }
}

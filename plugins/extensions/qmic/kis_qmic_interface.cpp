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
#include "kis_qmic_interface.h"
#include "kis_input_output_mapper.h"
#include "kis_qmic_applicator.h"
#include "kis_qmic_simple_convertor.h"
#include "kis_selection.h"

#include "gmic.h"

struct KisImageInterface::Private {
    Private() = default;

    KisViewManager *m_viewManager {nullptr};
    InputLayerMode m_inputMode {ACTIVE_LAYER};
    OutputMode m_outputMode {IN_PLACE};
    QVector<KisQMicImageSP> m_sharedMemorySegments {};
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

KisImageInterface::~KisImageInterface()
{
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

QVector<KisQMicImageSP> KisImageInterface::gmic_qt_get_cropped_images(int inputMode, QRectF &rc)
{
    // Create the shared memory segments, and create a new "message" to send back

    QVector<KisQMicImageSP> message;

    if (!p->m_viewManager)
        return {};

    KisImageBarrierLocker locker(p->m_viewManager->image());

    p->m_inputMode = (InputLayerMode)inputMode;

    dbgPlugins << "prepareCroppedImages()" << message << rc << inputMode;

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

            QString noParenthesisName(node->name());
            noParenthesisName.replace(QChar('('), QChar(21)).replace(QChar(')'), QChar(22));

            auto translatedMode = KisQmicSimpleConvertor::blendingModeToString(node->compositeOpId());

            QString name = QString("mode(%1),opacity(%2),pos(%3,%4),name(%5)").arg(translatedMode).arg(node->percentOpacity()).arg(cropRect.x()).arg(cropRect.y()).arg(noParenthesisName);

            auto m = KisQMicImageSP::create(node->name(), resultRect.width(), resultRect.height(), 4);
            p->m_sharedMemorySegments << m;

            {
                QMutexLocker lock(&m->m_mutex);

                gmic_image<float> img;
                img.assign(resultRect.width(), resultRect.height(), 1, 4);


                img._data = m->m_data;
                
                KisQmicSimpleConvertor::convertToGmicImageFast(node->paintDevice(), &img, resultRect);
            }

            message << m;
        }
    }

    dbgPlugins << message;

    return message;
}

void KisImageInterface::gmic_qt_output_images(int mode, QVector<KisQMicImageSP> layers)
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
    for (auto memorySegment : p->m_sharedMemorySegments) {
        dbgPlugins << "detaching" << (quintptr)memorySegment.data();
        memorySegment.clear();
    }
    p->m_sharedMemorySegments.clear();
}

void KisImageInterface::slotStartApplicator(QVector<KisQMicImageSP> gmicImages)
{
    dbgPlugins << "slotStartApplicator();" << gmicImages;
    if (!p->m_viewManager)
        return;
    // Create a vector of gmic images

    QVector<gmic_image<float> *> images;

    for (auto &image : gmicImages) {
        QString layerName = image->m_layerName;
        int spectrum = image->m_spectrum;
        int width = image->m_width;
        int height = image->m_height;

        dbgPlugins << "Received image: " << (quintptr)image.data() << layerName << width << height;

        gmic_image<float> *gimg = nullptr;

        {
            QMutexLocker lock(&image->m_mutex);

            dbgPlugins << "Memory segment" << (quintptr)image.data() << image->size() << (quintptr)image->constData() << (quintptr)image->m_data;
            gimg = new gmic_image<float>();
            gimg->assign(width, height, 1, spectrum);
            gimg->name = layerName;

            gimg->_data = new float[width * height * spectrum * sizeof(float)];
            dbgPlugins << "width" << width << "height" << height << "size" << width * height * spectrum * sizeof(float) << "shared memory size" << image->size();
            memcpy(gimg->_data, image->constData(), width * height * spectrum * sizeof(float));

            dbgPlugins << "created gmic image" << gimg->name << gimg->_width << gimg->_height;

        }
        images.append(gimg);
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

QDebug operator<<(QDebug d, const KisQMicImage &model)
{
    d << QString("0x%1,%2,%3,%4").arg((quintptr)&model).arg(QString(model.m_layerName.toUtf8().toHex())).arg(model.m_width).arg(model.m_height);
    return d;
}

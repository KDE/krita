/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_animation_exporter.h"

#include <QProgressDialog>
#include <QEventLoop>

#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_painter.h"

#include "kis_image_lock_hijacker.h"

struct KisAnimationExporter::Private
{
    Private(KisImageSP _image)
        : image(_image)
        , currentFrame(-1)
        , tmpDevice(new KisPaintDevice(image->colorSpace()))
    {
    }

    KisImageWSP image;
    int currentFrame;
    SaveFrameCallback saveFrameCallback;
    KisPaintDeviceSP tmpDevice;
};

KisAnimationExporter::KisAnimationExporter(KisImageWSP image)
    : m_d(new Private(image))
{
    connect(m_d->image->animationInterface(), SIGNAL(sigFrameReady(int)),
            this, SLOT(frameReadyToCopy(int)), Qt::DirectConnection);

    connect(this, SIGNAL(sigFrameReadyToSave()),
            this, SLOT(frameReadyToSave()), Qt::QueuedConnection);
}

KisAnimationExporter::~KisAnimationExporter()
{
}

void KisAnimationExporter::setSaveFrameCallback(SaveFrameCallback func)
{
    m_d->saveFrameCallback = func;
}

void KisAnimationExporter::startFrameRegeneration(int time)
{
    m_d->currentFrame = time;
    m_d->image->animationInterface()->requestFrameRegeneration(m_d->currentFrame, m_d->image->bounds());
}

void KisAnimationExporter::frameReadyToCopy(int time)
{
    if (time != m_d->currentFrame) return;

    QRect rc = m_d->image->bounds();
    KisPainter::copyAreaOptimized(rc.topLeft(), m_d->image->projection(), m_d->tmpDevice, rc);

    emit sigFrameReadyToSave();
}

void KisAnimationExporter::frameReadyToSave()
{
    KisImportExportFilter::ConversionStatus result =
        KisImportExportFilter::OK;

    result = m_d->saveFrameCallback(m_d->currentFrame, m_d->tmpDevice);

    if (result == KisImportExportFilter::OK) {
        emit sigFramePrepared(m_d->currentFrame);
    } else {
        emit sigFrameFailed(m_d->currentFrame, result);
    }

    m_d->currentFrame = -1;
    m_d->tmpDevice.clear();
}


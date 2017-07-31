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

#include <KoUpdater.h>
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_painter.h"

#include "kis_image_lock_hijacker.h"

struct KisAnimationExporter::Private
{
    Private(KisImageSP _image, int fromTime, int toTime, KoUpdaterPtr _updater)
        : updater(_updater)
        , image(_image)
        , firstFrame(fromTime)
        , lastFrame(toTime)
        , currentFrame(-1)
        , batchMode(!updater)
        , isCancelled(false)
        , status(KisImportExportFilter::OK)
        , tmpDevice(new KisPaintDevice(image->colorSpace()))
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(bool(updater) == !batchMode);
    }

    KoUpdaterPtr updater;
    KisImageWSP image;

    int firstFrame;
    int lastFrame;
    int currentFrame;

    bool batchMode;
    bool isCancelled;

    KisImportExportFilter::ConversionStatus status;

    SaveFrameCallback saveFrameCallback;

    KisPaintDeviceSP tmpDevice;

    QProgressDialog progress;

};

KisAnimationExporter::KisAnimationExporter(KisImageWSP image, int fromTime, int toTime, KoUpdaterPtr updater)
    : m_d(new Private(image, fromTime, toTime, updater))
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

KisImportExportFilter::ConversionStatus KisAnimationExporter::exportAnimation()
{

    if (!m_d->batchMode) {
        QString message = i18n("Preparing to export frames...");

        m_d->progress.reset();
        m_d->progress.setLabelText(message);
        m_d->progress.setWindowModality(Qt::ApplicationModal);
        m_d->progress.setCancelButton(0);
        m_d->progress.setMinimumDuration(0);
        m_d->progress.setValue(0);
        m_d->progress.setMinimum(0);
        m_d->progress.setMaximum(100);
    }

    if (m_d->updater) {
        m_d->updater->setProgress(0);
    }

    /**
     * HACK ALERT: Here we remove the image lock! We do it in a GUI
     *             thread under the barrier lock held, so it is
     *             guaranteed no other stroke will accidentally be
     *             started by this. And showing an app-modal dialog to
     *             the user will prevent him from doing anything
     *             nasty.
     */
    KisImageLockHijacker badGuy(m_d->image);
    Q_UNUSED(badGuy);

    KIS_ASSERT_RECOVER(!m_d->image->locked()) { return KisImportExportFilter::InternalError; }

    m_d->status = KisImportExportFilter::OK;
    m_d->currentFrame = m_d->firstFrame;
    m_d->image->animationInterface()->requestFrameRegeneration(m_d->currentFrame, m_d->image->bounds());

    QEventLoop loop;
    loop.connect(this, SIGNAL(sigFinished()), SLOT(quit()));
    loop.exec();

    if (!m_d->batchMode) {
        m_d->progress.reset();
    }

    if (m_d->updater) {
        m_d->updater->setProgress(100);
    }

    return m_d->status;
}

void KisAnimationExporter::cancel()
{
    m_d->isCancelled = true;
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
    KIS_ASSERT_RECOVER(m_d->saveFrameCallback) {
        m_d->status = KisImportExportFilter::InternalError;
        emit sigFinished();
        return;
    }

    // TODO: refactor to a signal!
    if (m_d->isCancelled || (m_d->updater && m_d->updater->interrupted())) {
        m_d->status = KisImportExportFilter::UserCancelled;
        emit sigFinished();
        return;
    }

    KisImportExportFilter::ConversionStatus result =
        KisImportExportFilter::OK;
    int time = m_d->currentFrame;

    result = m_d->saveFrameCallback(time, m_d->tmpDevice);

    if (m_d->updater) {
        int length = m_d->lastFrame - m_d->firstFrame;
        m_d->updater->setProgress((time - m_d->firstFrame) * 100 / length);
    }

    // TODO: make translatable!!
    QString dialogText = QString("Exporting Frame ").append(QString::number(time)).append(" of ").append(QString::number(m_d->lastFrame));
    int percentageProcessed = (float(time) / float(m_d->lastFrame) * 100);

    m_d->progress.setLabelText(dialogText);
    m_d->progress.setValue(int(percentageProcessed));

    if (result == KisImportExportFilter::OK && time < m_d->lastFrame) {
        m_d->currentFrame = time + 1;
        m_d->image->animationInterface()->requestFrameRegeneration(m_d->currentFrame, m_d->image->bounds());
    } else {
        m_d->status = result;
        emit sigFinished();
    }
}


/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "recorderdocker_dock.h"

#include <klocalizedstring.h>
#include <QLabel>
#include <QStatusBar>
#include <QVBoxLayout>

#include "encoder_queue.h"
#include "kis_canvas2.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_signal_compressor.h"
#include <KisViewManager.h>
#include <KoColorSpaceRegistry.h>
#include <kactioncollection.h>
#include <kis_icon_utils.h>
#include <kis_zoom_manager.h>
#include <klocalizedstring.h>
#include <QDir>
#include <QFileDialog>
#include <QRegExp>
#include <QRegExpValidator>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringBuilder>
#include <QtConcurrent>

RecorderDockerDock::RecorderDockerDock()
    : QDockWidget(i18n("Recorder"))
    , m_canvas(nullptr)
    , m_imageIdleWatcher(1000)
    , m_encoderQueue(nullptr)
{
    QWidget* page = new QWidget(this);
    m_layout = new QGridLayout(page);
    m_recordDirectoryLabel = new QLabel(this);
    m_recordDirectoryLabel->setText("Directory:");

    m_layout->addWidget(m_recordDirectoryLabel, 0, 0, 1, 2);

    m_recordDirectoryLineEdit = new QLineEdit(this);
    m_recordDirectoryLineEdit->setText(QDir::homePath());
    m_recordDirectoryLineEdit->setReadOnly(true);
    m_layout->addWidget(m_recordDirectoryLineEdit, 1, 0);

    m_recordDirectoryPushButton = new QPushButton(this);
    m_recordDirectoryPushButton->setIcon(KisIconUtils::loadIcon("folder"));
    m_recordDirectoryPushButton->setToolTip(i18n("Record Video"));

    m_layout->addWidget(m_recordDirectoryPushButton, 1, 1);

    m_imageNameLabel = new QLabel(this);
    m_imageNameLabel->setText("Video Name:");

    m_layout->addWidget(m_imageNameLabel, 2, 0, 1, 2);

    m_imageNameLineEdit = new QLineEdit(this);
    m_imageNameLineEdit->setText("image");

    QRegExp rx("[0-9a-zA-z_]+");
    QValidator* validator = new QRegExpValidator(rx, this);
    m_imageNameLineEdit->setValidator(validator);

    m_layout->addWidget(m_imageNameLineEdit, 3, 0);

    m_recordToggleButton = new QPushButton(this);
    m_recordToggleButton->setCheckable(true);
    m_recordToggleButton->setIcon(KisIconUtils::loadIcon("media-record"));
    m_recordToggleButton->setToolTip(i18n("Record Video"));
    m_layout->addWidget(m_recordToggleButton, 3, 1);

    m_logLabel = new QLabel(this);
    m_logLabel->setText("Recent Save:");
    m_layout->addWidget(m_logLabel, 4, 0, 1, 2);
    m_logLineEdit = new QLineEdit(this);
    m_logLineEdit->setReadOnly(true);
    m_layout->addWidget(m_logLineEdit, 5, 0, 1, 2);

    m_spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_layout->addItem(m_spacer, 6, 0, 1, 2);
    connect(m_recordDirectoryPushButton, SIGNAL(clicked()), this, SLOT(onSelectRecordFolderButtonClicked()));
    connect(m_recordToggleButton, SIGNAL(toggled(bool)), this, SLOT(onRecordButtonToggled(bool)));
    setWidget(page);
}

void RecorderDockerDock::setCanvas(KoCanvasBase* canvas)
{
    if (m_canvas == canvas)
        return;

    setEnabled(canvas != nullptr);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas) {
        m_imageIdleWatcher.setTrackedImage(m_canvas->image());

        connect(&m_imageIdleWatcher, &KisIdleWatcher::startedIdleMode, this, &RecorderDockerDock::generateThumbnail);
        connect(m_canvas->image(), SIGNAL(sigSizeChanged(QPointF, QPointF)), SLOT(startUpdateCanvasProjection()));
    }
}

void RecorderDockerDock::startUpdateCanvasProjection()
{
    m_imageIdleWatcher.startCountdown();
}

void RecorderDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = nullptr;
}

void RecorderDockerDock::onRecordButtonToggled(bool enabled)
{
    bool enabled2 = enabled;
    enableRecord(enabled2, m_recordDirectoryLineEdit->text() % "/" % m_imageNameLineEdit->text());

    if (enabled && !enabled2) {
        disconnect(m_recordToggleButton, SIGNAL(toggle(bool)), this, SLOT(onRecordButtonToggled(bool)));
        m_recordToggleButton->setChecked(false);

        connect(m_recordToggleButton, SIGNAL(toggle(bool)), this, SLOT(onRecordButtonToggled(bool)));
    }
}

void RecorderDockerDock::onSelectRecordFolderButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    QString folder = dialog.getExistingDirectory(this, tr("Select Output Folder"), m_recordDirectoryLineEdit->text(),
                                                 QFileDialog::ShowDirsOnly);
    m_recordDirectoryLineEdit->setText(folder);
}

void RecorderDockerDock::enableRecord(bool& enabled, const QString& path)
{
    if (!m_encoderQueue)
    {
        m_encoderQueue = new EncoderQueue();
    }

    m_encoderQueue->setEnable(enabled,path,m_canvas);

    if (enabled)
    {
        startUpdateCanvasProjection();
    }
    else
    {
        delete m_encoderQueue;
        m_encoderQueue = nullptr;
    }
}

void RecorderDockerDock::generateThumbnail()
{
    if (m_encoderQueue && m_encoderQueue->isRecording()) {
        if (m_canvas && (m_encoderQueue->recordingCanvas() == m_canvas)) {
            disconnect(&m_imageIdleWatcher, &KisIdleWatcher::startedIdleMode, this,
                       &RecorderDockerDock::generateThumbnail);
            if (m_encoderQueue) {
                KisImageSP image = m_canvas->image();
                image->barrierLock();
                KisPaintDeviceSP dev = image->projection();
                m_encoderQueue->pushFrame(dev, image->width(), image->height());
                image->unlock();
            }

            connect(&m_imageIdleWatcher, &KisIdleWatcher::startedIdleMode, this,
                    &RecorderDockerDock::generateThumbnail);
        }
    }
}

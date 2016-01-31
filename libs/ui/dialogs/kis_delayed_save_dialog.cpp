/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_delayed_save_dialog.h"
#include "ui_kis_delayed_save_dialog.h"

#include <QTimer>

#include "kis_debug.h"
#include "kis_image.h"
#include "kis_composite_progress_proxy.h"


struct KisDelayedSaveDialog::Private
{
    Private(KisImageSP _image) : image(_image) {}

    KisImageSP image;
    QTimer updateTimer;
};

KisDelayedSaveDialog::KisDelayedSaveDialog(KisImageSP image, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::KisDelayedSaveDialog),
      m_d(new Private(image))
{
    KIS_ASSERT_RECOVER_NOOP(image);

    ui->setupUi(this);

    connect(ui->btnCancel, SIGNAL(clicked()), SLOT(slotCancelRequested()));
    connect(&m_d->updateTimer, SIGNAL(timeout()), SLOT(slotTimerTimeout()));

    m_d->image->compositeProgressProxy()->addProxy(ui->progressBar);
}

KisDelayedSaveDialog::~KisDelayedSaveDialog()
{
    m_d->image->compositeProgressProxy()->removeProxy(ui->progressBar);
    delete ui;
}

void KisDelayedSaveDialog::blockIfImageIsBusy()
{
    if (m_d->image->isIdle()) return;

    m_d->updateTimer.start(200);
    exec();
    m_d->updateTimer.stop();
}

void KisDelayedSaveDialog::slotTimerTimeout()
{
    if (m_d->image->isIdle()) {
        accept();
    }
}

void KisDelayedSaveDialog::slotCancelRequested()
{
    m_d->image->requestStrokeCancellation();
}

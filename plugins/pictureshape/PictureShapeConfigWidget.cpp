/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "PictureShapeConfigWidget.h"
#include "PictureShape.h"

#include <KoImageData.h>
#include <KoImageCollection.h>

#include <KDebug>
#include <KFileWidget>
#include <KJob>
#include <KIO/Job>

#include <QGridLayout>

void LoadWaiter::setImageData(KJob *job)
{
    if (m_pictureShape == 0)
        return; // ugh, the shape got deleted meanwhile
        KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob*>(job);
    Q_ASSERT(transferJob);

    if (m_pictureShape->imageCollection()) {
        KoImageData *data = m_pictureShape->imageCollection()->createImageData(transferJob->data());
        if (data) {
            m_pictureShape->setUserData(data);
            // check if the shape still size of the default shape and resize in that case
            if (qFuzzyCompare(m_pictureShape->size().width(), 50.0)) {
                m_pictureShape->setSize(data->imageSize());
            }
        }
    }
    deleteLater();
}

// ---------------------------------------------------- //

PictureShapeConfigWidget::PictureShapeConfigWidget()
    : m_shape(0),
    m_fileWidget(0)
{
}

PictureShapeConfigWidget::~PictureShapeConfigWidget()
{
    delete m_fileWidget;
}

void PictureShapeConfigWidget::open(KoShape *shape)
{
    m_shape = dynamic_cast<PictureShape*>(shape);
    Q_ASSERT(m_shape);
    delete m_fileWidget;
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_fileWidget = new KFileWidget(KUrl("kfiledialog:///OpenDialog"), this);
    m_fileWidget->setOperationMode(KFileWidget::Opening);
    m_fileWidget->setFilter("image/png image/jpeg image/gif");
    layout->addWidget(m_fileWidget);
    setLayout(layout);
    connect(m_fileWidget, SIGNAL(accepted()), this, SIGNAL(accept()));
}

void PictureShapeConfigWidget::save()
{
    if (!m_shape)
        return;
    m_fileWidget->accept();
    KUrl url = m_fileWidget->selectedUrl();
    if (!url.isEmpty()) {
        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, 0);
        LoadWaiter *waiter = new LoadWaiter(m_shape);
        connect(job, SIGNAL(result(KJob*)), waiter, SLOT(setImageData(KJob*)));
    }
}

bool PictureShapeConfigWidget::showOnShapeCreate()
{
    return true;
}

bool PictureShapeConfigWidget::showOnShapeSelect()
{
    return false;
}

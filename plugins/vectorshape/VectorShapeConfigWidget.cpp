/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "VectorShapeConfigWidget.h"

#include "VectorShape.h"
// KDE
#include <KDebug>
#include <KFileWidget>
#include <KIO/Job>
// Qt
#include <QVBoxLayout>

void LoadWaiter::setImageData(KJob *job)
{
    if (m_vectorShape) {
        KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob*>(job);
        Q_ASSERT(transferJob);

        const QByteArray contents = transferJob->data();
        const VectorShape::VectorType vectorType = VectorShape::vectorType(contents);

        m_vectorShape->setCompressedContents(qCompress(contents), vectorType);
    }

    deleteLater();
}

// ---------------------------------------------------- //

VectorShapeConfigWidget::VectorShapeConfigWidget()
    : m_shape(0),
    m_fileWidget(0)
{
}

VectorShapeConfigWidget::~VectorShapeConfigWidget()
{
    delete m_fileWidget;
}

void VectorShapeConfigWidget::open(KoShape *shape)
{
    m_shape = dynamic_cast<VectorShape*>(shape);
    Q_ASSERT(m_shape);
    delete m_fileWidget;
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_fileWidget = new KFileWidget(KUrl("kfiledialog:///OpenDialog"), this);
    m_fileWidget->setOperationMode(KFileWidget::Opening);
    const QStringList mimetypes = QStringList()
        << QLatin1String("image/x-wmf")
        << QLatin1String("image/x-emf");
    m_fileWidget->setMimeFilter(mimetypes);
    layout->addWidget(m_fileWidget);
    setLayout(layout);
    connect(m_fileWidget, SIGNAL(accepted()), this, SIGNAL(accept()));
}

void VectorShapeConfigWidget::save()
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

bool VectorShapeConfigWidget::showOnShapeCreate()
{
    return true;
}

bool VectorShapeConfigWidget::showOnShapeSelect()
{
    return false;
}

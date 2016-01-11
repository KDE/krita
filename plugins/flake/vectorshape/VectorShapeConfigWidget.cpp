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
// Qt
#include <QVBoxLayout>
#include <QUrl>
#include <QPushButton>

#include <klocalizedstring.h>

#include <kis_url_requester.h>

VectorShapeConfigWidget::VectorShapeConfigWidget()
    : m_shape(0)
    , m_fileWidget(0)
{
}

VectorShapeConfigWidget::~VectorShapeConfigWidget()
{
    delete m_fileWidget;
}

void VectorShapeConfigWidget::open(KoShape *shape)
{
    m_shape = dynamic_cast<VectorShape *>(shape);
    Q_ASSERT(m_shape);
    delete m_fileWidget;
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_fileWidget = new KisUrlRequester(this);
    m_fileWidget->setMode(KoFileDialog::OpenFile);
    const QStringList mimetypes = QStringList()
                                  << QLatin1String("image/x-wmf")
                                  << QLatin1String("image/x-emf")
                                  << QLatin1String("image/x-svm")
                                  << QLatin1String("image/svg+xml");
    m_fileWidget->setMimeTypeFilters(mimetypes);
    layout->addWidget(m_fileWidget);
    setLayout(layout);
    QPushButton *bn = new QPushButton(this);
    bn->setText(i18n("Replace Image"));
    layout->addWidget(bn);
    connect(bn, SIGNAL(clicked()), this, SIGNAL(accept()));
}

void VectorShapeConfigWidget::save()
{
    if (!m_shape) {
        return;
    }
    QString fn = m_fileWidget->url().toLocalFile();
    if (!fn.isEmpty()) {
        QFile f(fn);
        if (f.exists()) {
            f.open(QFile::ReadOnly);
            QByteArray ba = f.readAll();
            f.close();
            if (!ba.isEmpty()) {
                const VectorShape::VectorType vectorType = VectorShape::vectorType(ba);
                m_shape->setCompressedContents(qCompress(ba), vectorType);
            }
        }
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

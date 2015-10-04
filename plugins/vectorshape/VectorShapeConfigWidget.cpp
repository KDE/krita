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
#include <kfilewidget.h>
// Qt
#include <QVBoxLayout>
#include <QUrl>

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
    m_fileWidget = new KFileWidget(QUrl("kfiledialog:///OpenDialog"), this);
    m_fileWidget->setOperationMode(KFileWidget::Opening);
    const QStringList mimetypes = QStringList()
        << QLatin1String("image/x-wmf")
        << QLatin1String("image/x-emf")
        << QLatin1String("image/x-svm")
        << QLatin1String("image/svg+xml");
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
    QString fn = m_fileWidget->selectedFile();
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

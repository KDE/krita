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
#include <KoImageSelectionWidget.h>

#include <KDebug>
#include <QGridLayout>

PictureShapeConfigWidget::PictureShapeConfigWidget()
    : m_shape(0),
    m_selectionWidget(0)
{
}

PictureShapeConfigWidget::~PictureShapeConfigWidget()
{
}

void PictureShapeConfigWidget::open(KoShape *shape)
{
    m_shape = dynamic_cast<PictureShape*>(shape);
    Q_ASSERT(m_shape);
    delete m_selectionWidget;
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_selectionWidget = new KoImageSelectionWidget(m_shape->imageCollection(), this);
    layout->addWidget(m_selectionWidget);
    setLayout(layout);
}

void PictureShapeConfigWidget::save()
{
    if (!m_shape)
        return;
    KoImageData *data = m_selectionWidget->imageData();
    if (data) {
        m_shape->setUserData(data);
        m_shape->setSize(data->imageSize());
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

#include <PictureShapeConfigWidget.moc>

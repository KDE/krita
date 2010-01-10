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

#include "VideoShapeConfigWidget.h"
#include "VideoShape.h"

#include <VideoData.h>

#include <kfilewidget.h>
#include <KDebug>
#include <QGridLayout>
#include <phonon/backendcapabilities.h>

VideoShapeConfigWidget::VideoShapeConfigWidget()
    : KoShapeConfigWidgetBase()
    ,m_shape(0),
    m_fileWidget(0)
{
}

VideoShapeConfigWidget::~VideoShapeConfigWidget()
{
    delete m_fileWidget;
}

void VideoShapeConfigWidget::open(KoShape *shape)
{
    m_shape = dynamic_cast<VideoShape*>(shape);
    Q_ASSERT(m_shape);
    if (!m_fileWidget) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        m_fileWidget = new KFileWidget(KUrl("kfiledialog:///OpenVideoDialog"), this);
        m_fileWidget->setOperationMode(KFileWidget::Opening);
        m_fileWidget->setMimeFilter(Phonon::BackendCapabilities::availableMimeTypes());
        layout->addWidget(m_fileWidget);
        setLayout(layout);
    }
}

void VideoShapeConfigWidget::save()
{
    if (!m_shape)
        return;
    VideoData *data = new VideoData();
    if (data) {
        data->setExternalVideo(m_fileWidget->selectedUrl(), m_shape->videoCollection());
        m_shape->setUserData(data);
    }
}

bool VideoShapeConfigWidget::showOnShapeCreate()
{
    return true;
}

bool VideoShapeConfigWidget::showOnShapeSelect()
{
    return false;
}

#include "VideoShapeConfigWidget.moc"

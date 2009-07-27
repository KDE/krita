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

#include <kfilewidget.h>
#include <kimagefilepreview.h>
#include <KLocale>
#include <KDialog>
#include <KDebug>
#include <QGridLayout>
#include <QtGui/QStackedWidget>

PictureShapeConfigWidget::PictureShapeConfigWidget()
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setSpacing(KDialog::spacingHint());
    layout->setMargin(0);

    m_stack = new QStackedWidget(this);

    m_fileWidget = new KFileWidget(KUrl("kfiledialog:///OpenDialog"), this);
    m_fileWidget->setOperationMode(KFileWidget::Opening);
    m_fileWidget->setFilter( "image/png image/jpeg image/gif" );
    m_fileWidget->setMinimumSize( QSize(640,480) );
    m_filePreview = new KImageFilePreview(this);

    m_stack->addWidget( m_fileWidget );
    m_stack->addWidget( m_filePreview );

    layout->addWidget(m_stack, 0, 0, 1, -1);
    layout->setColumnStretch(0, 10);

    connect(m_fileWidget, SIGNAL(accepted()),
            this, SLOT(onAccepted()));
}

PictureShapeConfigWidget::~PictureShapeConfigWidget()
{
}

void PictureShapeConfigWidget::onAccepted()
{
    m_fileWidget->accept();
    m_stack->setCurrentIndex(1);
    m_filePreview->showPreview( m_fileWidget->selectedUrl() );
    emit propertyChanged();
}

void PictureShapeConfigWidget::open(KoShape *shape)
{
    m_shape = dynamic_cast<PictureShape*>(shape);
    m_filePreview->clearPreview();
    if( ! m_shape->userData() )
        return;
}
 
void PictureShapeConfigWidget::save()
{
    if (!m_shape)
        return;
/*
    KUrl url = m_fileWidget->selectedUrl();
    if (!url.isEmpty())
        m_shape->loadFromUrl(url);

    kDebug(31000) << "image url =" << url;
    m_shape->setKeepAspectRatio(true);
*/
}

bool PictureShapeConfigWidget::showOnShapeCreate()
{
    return true;
}

bool PictureShapeConfigWidget::showOnShapeSelect()
{
    return false;
}

#include "PictureShapeConfigWidget.moc"

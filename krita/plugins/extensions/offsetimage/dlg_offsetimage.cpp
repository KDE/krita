/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "dlg_offsetimage.h"

#include <klocale.h>
#include <kis_debug.h>

DlgOffsetImage::DlgOffsetImage(QWidget *  parent, const char * name, QSize imageSize)
        :   KDialog(parent),
            m_offsetSize(imageSize)
{
    setCaption(i18n("Offset Image"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);


    m_lock = false;

    m_page = new WdgOffsetImage(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("offset_image");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()),this, SLOT(okClicked()));
    connect(m_page->middleOffsetBtn, SIGNAL(clicked()), this, SLOT(slotMiddleOffset()));
    connect(m_page->offsetXspinBox, SIGNAL(valueChanged(int)), this, SLOT(slotOffsetXChanged(int)));
    connect(m_page->offsetYspinBox, SIGNAL(valueChanged(int)), this, SLOT(slotOffsetYChanged(int)));

    slotMiddleOffset();
}

DlgOffsetImage::~DlgOffsetImage()
{
    delete m_page;
}

void DlgOffsetImage::slotOffsetXChanged(int newOffsetX)
{
    m_offsetX = newOffsetX;
}

void DlgOffsetImage::slotOffsetYChanged(int newOffsetY)
{
    m_offsetY = newOffsetY;
}

void DlgOffsetImage::slotMiddleOffset()
{
    int offsetX = m_offsetSize.width() / 2;
    int offsetY = m_offsetSize.height() / 2;
    m_page->offsetXspinBox->setValue(offsetX);
    m_page->offsetYspinBox->setValue(offsetY);
}

void DlgOffsetImage::okClicked()
{
    accept();
}


#include "dlg_offsetimage.moc"

/*
 *  dlg_shearimage.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include <config.h>

#include <math.h>

#include <iostream>

using namespace std;

#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include "dlg_shearimage.h"

DlgShearImage::DlgShearImage( QWidget *  parent,
                const char * name)
    : super (parent)
{
    setCaption( i18n("Shear Image") );
    setButtons(  Ok | Cancel);
    setDefaultButton( Ok );
    setObjectName(name);

    m_lock = false;

    m_page = new WdgShearImage(this);
    Q_CHECK_PTR(m_page);
    m_page->layout()->setMargin(0);
    m_page->setObjectName("shear_image");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));

}

DlgShearImage::~DlgShearImage()
{
    delete m_page;
}

void DlgShearImage::setAngleX(quint32 angle)
{
    m_page->shearAngleX->setValue(angle);
    m_oldAngle = angle;

}

void DlgShearImage::setAngleY(quint32 angle)
{
    m_page->shearAngleY->setValue(angle);
    m_oldAngle = angle;

}

qint32 DlgShearImage::angleX()
{
    return (qint32)qRound(m_page->shearAngleX->value());
}

qint32 DlgShearImage::angleY()
{
    return (qint32)qRound(m_page->shearAngleY->value());
}

// SLOTS

void DlgShearImage::okClicked()
{
    accept();
}

#include "dlg_shearimage.moc"

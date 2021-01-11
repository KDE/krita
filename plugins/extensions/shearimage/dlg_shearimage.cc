/*
 *  dlg_shearimage.cc - part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dlg_shearimage.h"

#include <math.h>

#include <klocalizedstring.h>
#include <kis_debug.h>

DlgShearImage::DlgShearImage(QWidget *  parent,
                             const char * name)
        : KoDialog(parent)
{
    setCaption(i18n("Shear Image"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);

    m_lock = false;

    m_page = new WdgShearImage(this);
    Q_CHECK_PTR(m_page);
    m_page->layout()->setMargin(0);
    m_page->setObjectName("shear_image");
    
    m_page->shearAngleX->setDecimals(0);
    m_page->shearAngleX->setRange(-45, 45);
    m_page->shearAngleX->setWrapping(false);
    m_page->shearAngleX->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_NoFlipOptions);
    m_page->shearAngleY->setDecimals(0);
    m_page->shearAngleY->setRange(-45, 45);
    m_page->shearAngleY->setWrapping(false);
    m_page->shearAngleY->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_NoFlipOptions);

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
    m_page->shearAngleX->setAngle(angle);
    m_oldAngle = angle;

}

void DlgShearImage::setAngleY(quint32 angle)
{
    m_page->shearAngleY->setAngle(angle);
    m_oldAngle = angle;

}

qint32 DlgShearImage::angleX()
{
    return static_cast<qint32>(m_page->shearAngleX->angle());
}

qint32 DlgShearImage::angleY()
{
    return static_cast<qint32>(m_page->shearAngleY->angle());
}

// SLOTS

void DlgShearImage::okClicked()
{
    accept();
}


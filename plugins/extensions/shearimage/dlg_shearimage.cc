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
#include <KisDialogStateSaver.h>

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
    KisDialogStateSaver::restoreState(m_page, "DlgShearImage");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));

}

DlgShearImage::~DlgShearImage()
{
    KisDialogStateSaver::saveState(m_page, "DlgShearImage");
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
    return m_page->shearAngleX->value();
}

qint32 DlgShearImage::angleY()
{
    return m_page->shearAngleY->value();
}

// SLOTS

void DlgShearImage::okClicked()
{
    accept();
}


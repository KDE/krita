/*
 *  dlg_rotateimage.cc - part of KimageShop^WKrayon^WKrita
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

#include <q3groupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include "dlg_rotateimage.h"
#include "wdg_rotateimage.h"


DlgRotateImage::DlgRotateImage( QWidget *  parent,
                const char * name)
    : super (parent, name, true, i18n("Rotate Image"), Ok | Cancel, Ok)
{
    m_lock = false;

    m_page = new WdgRotateImage(this, "rotate_image");
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));
        connect( m_page->intCustom, SIGNAL( valueChanged ( int ) ),
                 this, SLOT( slotAngleValueChanged( int ) ) );

}

DlgRotateImage::~DlgRotateImage()
{
    delete m_page;
}

void DlgRotateImage::slotAngleValueChanged( int )
{
    m_page->radioCustom->setChecked(true);
}

void DlgRotateImage::setAngle(quint32 angle)
{
    if (angle == 90) {
        m_page->radio90->setChecked(true);
    }
    else if (angle == 180) {
        m_page->radio180->setChecked(true);
    }
    else if (angle == 270) {
        m_page->radio270->setChecked(true);
    }
    else {
        m_page->radioCustom->setChecked(true);
        m_page->intCustom->setValue(angle);
    }

    if (m_oldAngle != angle)
        resetPreview();

    m_oldAngle = angle;

}

qint32 DlgRotateImage::angle()
{
    double angle = 0;
    if (m_page->radio90->isChecked()) {
        angle = 90;
    }
    else if (m_page->radio180->isChecked()) {
        angle = 180;
    }
    else if (m_page->radio270->isChecked()) {
        angle = 270;
    }
    else {
        angle = qRound(m_page->intCustom->value());
    }
    if (m_page->radioCW->isChecked()) {
        return qint32(angle);
    }
    else {
        return qint32(-angle);
    }
}

void DlgRotateImage::setDirection (enumRotationDirection direction)
{
    if (direction == CLOCKWISE) {
        m_page->radioCW->setChecked(true);
    }
    else if (direction== COUNTERCLOCKWISE) {
        m_page->radioCCW->setChecked(true);
    }
}

enumRotationDirection DlgRotateImage::direction()
{
    if (m_page->radioCCW->isChecked()) {
        return COUNTERCLOCKWISE;
    }
    else {
        return CLOCKWISE;
    }
}

void DlgRotateImage::okClicked()
{
    accept();
}

void DlgRotateImage::resetPreview()
{
    // Code to update preview here.
}

#include "dlg_rotateimage.moc"

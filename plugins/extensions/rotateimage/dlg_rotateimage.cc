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

#include "dlg_rotateimage.h"

#include <math.h>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kis_icon.h>
#include <KisDialogStateSaver.h>

DlgRotateImage::DlgRotateImage(QWidget *  parent,
                               const char * name)
        : KoDialog(parent)
{
    setCaption(i18n("Rotate Image"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);

    m_lock = false;

    m_page = new WdgRotateImage(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("rotate_image");
    KisDialogStateSaver::restoreState(m_page, "DlgRotateImage");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_page->doubleCustom->setSuffix(QChar(Qt::Key_degree));

    m_page->radioCCW->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_rotateCCW"));
    m_page->radioCW->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_rotate"));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));
    connect(m_page->doubleCustom, SIGNAL(valueChanged(double)),
            this, SLOT(slotAngleValueChanged(double)));

}

DlgRotateImage::~DlgRotateImage()
{
    KisDialogStateSaver::saveState(m_page, "DlgRotateImage");
    delete m_page;
}

void DlgRotateImage::slotAngleValueChanged(double)
{
    m_page->radioCustom->setChecked(true);
}

void DlgRotateImage::setAngle(quint32 angle)
{
    if (angle == 90) {
        m_page->radio90->setChecked(true);
    } else if (angle == 180) {
        m_page->radio180->setChecked(true);
    } else if (angle == 270) {
        m_page->radio270->setChecked(true);
    } else {
        m_page->radioCustom->setChecked(true);
        m_page->doubleCustom->setValue(angle);
    }

    if (m_oldAngle != angle)
        resetPreview();

    m_oldAngle = angle;

}

double DlgRotateImage::angle()
{
    double angle = 0;
    if (m_page->radio90->isChecked()) {
        angle = 90;
    } else if (m_page->radio180->isChecked()) {
        angle = 180;
    } else if (m_page->radio270->isChecked()) {
        angle = 270;
    } else {
        angle = m_page->doubleCustom->value();
    }
    if (m_page->radioCW->isChecked()) {
        return angle;
    } else {
        return -angle;
    }
}

void DlgRotateImage::setDirection(enumRotationDirection direction)
{
    if (direction == CLOCKWISE) {
        m_page->radioCW->setChecked(true);
    } else if (direction == COUNTERCLOCKWISE) {
        m_page->radioCCW->setChecked(true);
    }
}

enumRotationDirection DlgRotateImage::direction()
{
    if (m_page->radioCCW->isChecked()) {
        return COUNTERCLOCKWISE;
    } else {
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


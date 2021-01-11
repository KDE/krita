/*
 *  dlg_rotateimage.cc - part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dlg_rotateimage.h"

#include <math.h>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kis_icon.h>

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

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_page->angleSelectorCustom->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

    m_page->radioCCW->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_rotateCCW"));
    m_page->radioCW->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_rotate"));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));
    connect(m_page->angleSelectorCustom, SIGNAL(angleChanged(double)),
            this, SLOT(slotAngleValueChanged(double)));
    connect(m_page->radioCCW, SIGNAL(toggled(bool)), SLOT(slotRadioCCWToggled(bool)));
    connect(m_page->radioCW, SIGNAL(toggled(bool)), SLOT(slotRadioCWToggled(bool)));
}

DlgRotateImage::~DlgRotateImage()
{
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
        m_page->angleSelectorCustom->setAngle(angle);
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
        angle = m_page->angleSelectorCustom->angle();
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

void DlgRotateImage::slotRadioCCWToggled(bool toggled)
{
    if (!toggled) {
        return;
    }
    m_page->angleSelectorCustom->setIncreasingDirection(KisAngleGauge::IncreasingDirection_CounterClockwise);
}

void DlgRotateImage::slotRadioCWToggled(bool toggled)
{
    if (!toggled) {
        return;
    }
    m_page->angleSelectorCustom->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
}

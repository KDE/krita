/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_cmb_contour.h"

#include <klocalizedstring.h>

KisCmbContour::KisCmbContour(QWidget *parent) :
    QComboBox(parent)
{
    addItem(i18n("Not Implemented Yet"));
    setEnabled(false);
}

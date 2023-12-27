/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef kis_multipliers_double_slider_spinbox_p_h
#define kis_multipliers_double_slider_spinbox_p_h

#include <QComboBox>
#include <kis_slider_spin_box.h>

#include "kis_debug.h"

struct KisMultipliersDoubleSliderSpinBox::Private {
    qreal currentMultiplier();
    /// Update the range of the slider depending on the currentMultiplier
    void updateRange();
    
    QComboBox *cmbMultiplier {0};
    KisDoubleSliderSpinBox *slider{0};
    qreal min, max;
    int decimals;
};

#endif

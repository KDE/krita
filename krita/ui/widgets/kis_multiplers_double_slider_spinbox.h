/* This file is part of the KDE project
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_MULTIPLIERS_DOUBLE_SLIDER_SPINBOX_H_
#define _KIS_MULTIPLIERS_DOUBLE_SLIDER_SPINBOX_H_

#include <QWidget>

class KisDoubleSliderSpinBox;

class KisMultipliersDoubleSliderSpinBox : public QWidget {
public:
    KisMultipliersDoubleSliderSpinBox(QWidget* _parent);
    ~KisMultipliersDoubleSliderSpinBox();
    
    KisDoubleSliderSpinBox* spinBox();
    
    void addMultiplier(double v);
    
private:
    struct Private;
    Private* const d;
};


#endif

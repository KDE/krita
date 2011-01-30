/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 or later of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_multi_sensors_selector.h"

#include "ui_wdgmultipliersdoublesliderspinbox.h"

struct KisMultiSensorsSelector::Private
{
  Ui_WdgMultipliersDoubleSliderSpinBox form;
};

KisMultiSensorsSelector::KisMultiSensorsSelector(QWidget* parent) : d(new Private)
{
  d->form.setupUi(this);
}

KisMultiSensorsSelector::~KisMultiSensorsSelector()
{
  delete d;
}


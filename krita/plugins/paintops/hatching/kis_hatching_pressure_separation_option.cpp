 
 /*
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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
 
 #include "kis_hatching_pressure_separation_option.h"
 #include <kis_pressure_opacity_option.h>
 
 #include <klocale.h>
 #include <kis_painter.h>
 #include <KoColor.h>
 #include <KoColorSpace.h>
 
 
 KisHatchingPressureSeparationOption::KisHatchingPressureSeparationOption()
 : KisCurveOption(i18n("Separation"), "Separation", KisPaintOpOption::brushCategory(), true )
 {
 }
 
 double KisHatchingPressureSeparationOption::apply(const KisPaintInformation & info) const
 {
     if (!isChecked()) return 0.5;
     return computeValue(info);
 }

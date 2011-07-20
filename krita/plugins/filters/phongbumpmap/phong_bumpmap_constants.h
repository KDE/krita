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

#ifndef CONSTANTS_H
#define CONSTANTS_H

/*
* Illuminants were renamed to Light Sources in the GUI
*/
const quint8 PHONG_TOTAL_ILLUMINANTS = 4;
const QString PHONG_HEIGHT_CHANNEL = "heightChannel";
const QString PHONG_ILLUMINANT_IS_ENABLED[] =
{"illuminantIsEnabled0",
"illuminantIsEnabled1",
"illuminantIsEnabled2",
"illuminantIsEnabled3"};
const QString PHONG_ILLUMINANT_COLOR[] =
{"illuminantColor0",
"illuminantColor1",
"illuminantColor2",
"illuminantColor3"};
const QString PHONG_ILLUMINANT_AZIMUTH[] = 
{"Azimuth0",
"Azimuth1",
"Azimuth2",
"Azimuth3"};
const QString PHONG_ILLUMINANT_INCLINATION[] =
{"Inclination0",
"Inclination1",
"Inclination2",
"Inclination3"};
const QString PHONG_AMBIENT_REFLECTIVITY = "ambientReflectivity";
const QString PHONG_DIFFUSE_REFLECTIVITY = "diffuseReflectivity";
const QString PHONG_SPECULAR_REFLECTIVITY = "specularReflectivity";
const QString PHONG_SHINYNESS_EXPONENT = "shinynessExponent";
const QString PHONG_DIFFUSE_REFLECTIVITY_IS_ENABLED = "diffuseReflectivityIsEnabled";
const QString PHONG_SPECULAR_REFLECTIVITY_IS_ENABLED = "specularReflectivityIsEnabled";
//const QString PHONG_SHINYNESS_EXPONENT_IS_ENABLED = "shinynessExponentIsEnabled";

#endif

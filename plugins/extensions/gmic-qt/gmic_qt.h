/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file gmic_qt.h
 *
 *  Copyright 2017 Sébastien Fourey
 *
 *  This file is part of "gmic_qt", a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  gmic_qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gmic_qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gmic_qt.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _GMIC_QT_GMIC_QT_H_
#define _GMIC_QT_GMIC_QT_H_

#define GMIC_QT_ORGANISATION_NAME "GREYC"
#define GMIC_QT_ORGANISATION_DOMAIN "greyc.fr"
#define GMIC_QT_APPLICATION_NAME "gmic_qt"

int launchPlugin();

int launchPluginHeadlessUsingLastParameters();

namespace GmicQt {
enum InputMode {
  NoInput,
  Active,
  All,
  ActiveAndBelow,
  ActiveAndAbove,
  AllVisibles,
  AllInvisibles,
  AllVisiblesDesc,
  AllInvisiblesDesc,
  AllDesc
};

enum OutputMode {
  InPlace,
  NewLayers,
  NewActiveLayers,
  NewImage
};

enum OutputMessageMode {
  Quiet,
  VerboseLayerName,
  VerboseConsole,
  VerboseLogFile,
  VeryVerboseConsole,
  VeryVerboseLogFile,
  DebugConsole,
  DebugLogFile
};

enum PreviewMode {
  FirstOutput,
  SecondOutput,
  ThirdOutput,
  FourthOutput,
  First2SecondOutput,
  First2ThirdOutput,
  First2FourthOutput,
  AllOutputs
};
extern const float PreviewFactorAny;
extern const float PreviewFactorFullImage;
extern const float PreviewFactorActualSize;
}

#endif // _GMIC_QT_GMIC_QT_H_

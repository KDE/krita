/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008 Fela Winkelmolen <fela.kde@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KARBONSIMPLIFYPATH_H
#define KARBONSIMPLIFYPATH_H

#include <QtGlobal>

class KoPathShape;

void karbonSimplifyPath(KoPathShape *path, qreal error);

#endif // KARBONSIMPLIFYPATH_H

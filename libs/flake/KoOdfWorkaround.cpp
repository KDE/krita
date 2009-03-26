/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoOdfWorkaround.h"

#include "KoShapeLoadingContext.h"
#include <KoOdfLoadingContext.h>
#include <QPen>

#include <kdebug.h>

// TODO only parse the generator string once so we don't have a string compare for every loaded shape
void KoOdfWorkaround::fixPenWidth(QPen & pen, KoShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generator().startsWith("OpenOffice.org") && pen.widthF() == 0.0) {
        pen.setWidthF(0.5);
        kDebug(30003) << "Work around OO bug with pen width 0";
    }
}

/*
 *  kis_pattern.cc - part of KImageShop
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qpoint.h>
#include <qsize.h>
#include <qimage.h>
#include <qpixmap.h>

#include <kimageeffect.h>

#include "kis_krayon.h"

KisKrayon::KisKrayon() : IconItem()
{
    m_validKrayon = false;
}

KisKrayon::~KisKrayon()
{
}

void KisKrayon::setValidKrayon(bool valid)
{
    m_validKrayon = valid;
}

QPixmap& KisKrayon::pixmap() const 
{
    return *m_pPixmap;
}

QPixmap& KisKrayon::thumbPixmap() const 
{
    return *m_pThumbPixmap;
}

QImage* KisKrayon::image() const 
{
    return m_pImage;
}



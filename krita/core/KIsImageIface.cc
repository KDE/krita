/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "KIsImageIface.h"

#include "kis_image.h"

#include <dcopclient.h>

KIsImageIface::KIsImageIface( KisImage *img_ )
	: DCOPObject()
{
	m_img = img_;
}

QString KIsImageIface::name()const
{
	return m_img->name();
}

int KIsImageIface::height() const
{
	return m_img->height();
}

int KIsImageIface::width() const
{
	return m_img->width();
}

bool KIsImageIface::alpha() const
{
	return m_img->alpha();
}

bool KIsImageIface::empty() const
{
	return m_img->empty();
}

void KIsImageIface::setName(const QString& name)
{
	m_img->setName( name );
}


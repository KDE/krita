/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "kis_resource.h"


KisResource::KisResource(const QString& filename)
{
	m_filename = filename;
	m_valid = false;
	m_dirty = false;
	m_spacing = 7;
}

KisResource::~KisResource()
{
}

bool KisResource::dirty() const
{
	return m_dirty;
}

void KisResource::setDirty(bool dirt)
{
	m_dirty = dirt;
}

QString KisResource::filename() const
{
	return m_filename;
}

void KisResource::setFilename(const QString& filename)
{
	m_filename = filename;
}

QString KisResource::name() const
{
	return m_name;
}

void KisResource::setName(const QString& name)
{
	m_name = name;
}

bool KisResource::valid() const
{
	return m_valid;
}

void KisResource::setValid(bool valid)
{
	m_valid = valid;
}

Q_INT32 KisResource::width() const
{
	return m_width;
}

void KisResource::setWidth(Q_INT32 w)
{
	m_width = w;
}

Q_INT32 KisResource::height() const
{
	return m_height;
}

void KisResource::setHeight(Q_INT32 h)
{
	m_height = h;
}

#include "kis_resource.moc"


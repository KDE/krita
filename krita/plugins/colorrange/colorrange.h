/* 
 * colorrange.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef COLORRANGE_H
#define COLORRANGE_H

#include <kparts/plugin.h>

class KisView;

class ColorRange : public KParts::Plugin
{
		Q_OBJECT
	public:
		ColorRange(QObject *parent, const char *name, const QStringList &);
		virtual ~ColorRange();
	
	private slots:
		void slotActivated();

	private:
		KisView * m_view;
		KisPainter * m_painter;

};

#endif // COLORRANGE_H

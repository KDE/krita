/* This file is part of the KDE project
   Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>
   Copyright (C) 2004 Boudewijn Rempt <boud@valdyas.org>

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

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <kparts/plugin.h>
#include <qimage.h>

class KSnapshot;

class Screenshot : public KParts::Plugin
{
	Q_OBJECT
public:
	Screenshot(QObject *parent, const char *name, const QStringList &);
	virtual ~Screenshot();
	
private slots:
	void slotScreenshot();
	void slotScreenGrabbed();

private:
	KSnapshot * snapshot;
};

#endif

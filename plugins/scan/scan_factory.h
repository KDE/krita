/* This file is part of the KDE project
   Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

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

#ifndef SCANFACTORY_H
#define SCANFACTORY_H

#include <kinstance.h>
#include <kaboutdata.h>
#include <klibloader.h>

#include "scan.h"

class ScanFactory : public KLibFactory
{
    Q_OBJECT
    public:
	ScanFactory(QObject *parent = 0, const char *name = 0);
	~ScanFactory();
	
	virtual QObject *createObject(QObject *parent = 0, const char * = 0, const char *classname = "QObject", const QStringList &args = QStringList());
	
	virtual KInstance *instance();

	static KInstance *pluginInstance() { return s_instance; }
	
    private:
	static KInstance *s_instance;
	static KAboutData *s_about;
	Scan *scanDialog;
};

#endif

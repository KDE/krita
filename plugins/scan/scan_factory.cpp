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

#include "scan_factory.moc"
#include "scan.h"

#include <klocale.h>
#include <kstddirs.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kdebug.h>

extern "C"
{
    void *init_libkofficescan()
    {
	KGlobal::locale()->insertCatalogue("kscan_plugin");
	return new ScanFactory();
    }
}

KInstance *ScanFactory::s_instance = 0L;
KAboutData *ScanFactory::s_about = 0L;

ScanFactory::ScanFactory(QObject *parent, const char *name) : KLibFactory(parent, name)
{
}

ScanFactory::~ScanFactory()
{
    delete s_instance;
    s_instance = 0L;
    delete s_about;
}

QObject *ScanFactory::createObject(QObject *parent, const char *, const char *classname, const QStringList &)
{
    Scan *scanDialog = new Scan(parent, classname);
    return scanDialog;
}

KInstance *ScanFactory::instance()
{
    if(!s_instance)
    {
	s_about = new KAboutData("Scanner Plugin", I18N_NOOP("KOffice"), "0.1");
	s_instance = new KInstance(s_about);
    }
    return s_instance;
}

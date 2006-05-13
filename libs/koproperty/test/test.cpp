/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>

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

#include <kmainwindow.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>

#include <QPixmap>
#include <QStringList>
#include <q3datetimeedit.h>
#include <QCursor>
#include <QApplication>
#include <QDesktopWidget>
//Added by qt3to4:
#include <Q3CString>

#include <property.h>
#include <editor.h>

#include "test.h"

using namespace KoProperty;

test::test()
    : KMainWindow( 0, "test" )
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	const bool flat = args->isSet("flat");
	const bool readOnly = args->isSet("ro");

//	setXMLFile("testui.rc");
	QFont f;
	f.setPixelSize(f.pixelSize()*2/3);
	setFont(f);

//  Simple
	m_set = new Set(this, "test");
	m_set->setReadOnly( readOnly );
	Q3CString group;
	if (!flat)
		group = "SimpleGroup";
	m_set->addProperty(new Property("Name", "Name"), group);
	(*m_set)["Name"].setAutoSync(1);
	
	m_set->addProperty(new Property("Int", 2, "Int"), group);
	m_set->addProperty(new Property("Double", 3.1415,"Double"), group);
	m_set->addProperty(new Property("Bool", QVariant(true, 4), "Bool"), group);
	m_set->addProperty(new Property("Date", QDate::currentDate(),"Date"), group);
	m_set->addProperty(new Property("Time", QTime::currentTime(),"Time"), group);
	m_set->addProperty(new Property("DateTime", QDateTime::currentDateTime(),"DateTime"), group);

	QStringList list;//keys
	list.append("myitem");
	list.append("otheritem");
	list.append("3rditem");
	QStringList name_list; //strings
	name_list.append("My Item");
	name_list.append("Other Item");
	name_list.append("Third Item");
	m_set->addProperty(new Property("List", list, name_list, "otheritem", "List"), group);

//  Complex
	group = flat ? "" : "ComplexGroup";
	m_set->addProperty(new Property("Rect", this->geometry(),"Rect"), group);
	m_set->addProperty(new Property("Point", QPoint(3,4), "Point"), group);
	m_set->addProperty(new Property("Size", QPoint(3,4), "Size"), group);

//  Appearance
	group = flat ? "" : "AppearanceGroup";
	m_set->addProperty(new Property("Color", this->paletteBackgroundColor(),"Color"), group);
	QPixmap pm(DesktopIcon("network"));
	m_set->addProperty(new Property("Pixmap", pm,"Pixmap"), group);
	m_set->addProperty(new Property("Font", this->font(),"Font"), group);
	m_set->addProperty(new Property("Cursor", QCursor(Qt::WaitCursor),"Cursor"), group);
	m_set->addProperty(new Property("LineStyle", 3, "LineStyle", "", LineStyle), group);
	m_set->addProperty(new Property("SizePolicy", sizePolicy(), "SizePolicy"), group);

	Editor *edit = new Editor(this,true/*autosync*/);
	setCentralWidget(edit);
	edit->changeSet(m_set);
	resize(400,qApp->desktop()->height()-200);
	move(x(),5);
	edit->setFocus();
}

test::~test()
{
}

#include "test.moc"

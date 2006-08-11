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

#include <property.h>
#include <editor.h>

#include "test.h"

using namespace KoProperty;

Test::Test()
 : KMainWindow(0,"koproperty_test")
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	const bool flat = args->isSet("flat");
	const bool readOnly = args->isSet("ro");

//	setXMLFile("testui.rc");
	QFont f;
	f.setPixelSize(f.pixelSize()*2/3);
	setFont(f);

/*  First, create the Set which will hold the properties.  */
	Property *p = 0;
	m_set = new Set(this, "test");
	m_set->setReadOnly(readOnly);
	QByteArray group;
	if (!flat) {
		group = "SimpleGroup";
		m_set->setGroupDescription(group, "Simple Group");
	}
	m_set->addProperty(new Property("Name", "Name"), group);
	(*m_set)["Name"].setAutoSync(1);
	
	m_set->addProperty(new Property("Int", 2, "Int"), group);
	m_set->addProperty(new Property("Double", 3.1415,"Double"), group);
	m_set->addProperty(new Property("Bool", QVariant(true), "Bool"), group);
	m_set->addProperty(p = new Property("3 States", QVariant(), "3 States", "", Boolean), group);
	p->setOption("3rdState", "None");
	m_set->addProperty(p = new Property("Date", QDate::currentDate(),"Date"), group);
	p->setIcon("date");
	m_set->addProperty(new Property("Time", QTime::currentTime(),"Time"), group);
	m_set->addProperty(new Property("DateTime", QDateTime::currentDateTime(),"Date/Time"), group);

	QStringList list;//keys
	list << "myitem" << "otheritem" << "3rditem";
	QStringList name_list; //strings
	name_list << "My Item" << "Other Item" << "Third Item";
	m_set->addProperty(new Property("List", list, name_list, "otheritem", "List"), group);

	// A valueFromList property matching strings with ints (could be any type supported by QVariant)
	QList<QVariant> keys;
	keys.append(1);
	keys.append(2);
	keys.append(3);
	Property::ListData *listData = new Property::ListData(keys, name_list);
	m_set->addProperty(new Property("List2", listData, "otheritem", "List 2"), group);

//  Complex
	if (!flat) {
		group = "ComplexGroup";
		m_set->setGroupDescription(group, "Complex Group");
	}
	m_set->addProperty(new Property("Rect", this->geometry(),"Rect"), group);
	m_set->addProperty(new Property("Point", QPoint(3,4), "Point"), group);
	m_set->addProperty(new Property("Size", QPoint(3,4), "Size"), group);

//  Appearance
	if (!flat) {
		group = "Appearance Group";
		m_set->setGroupDescription(group, "Appearance Group");
		m_set->setGroupIcon(group, "appearance");
	}
	m_set->addProperty(new Property("Color", palette().color(QPalette::Active, QPalette::Background),"Color"), group);
	QPixmap pm(DesktopIcon("network"));
	m_set->addProperty(p = new Property("Pixmap", pm,"Pixmap"), group);
	p->setIcon("kpaint");
	m_set->addProperty(p = new Property("Font", this->font(),"Font"), group);
	p->setIcon("fonts");
	m_set->addProperty(new Property("Cursor", QCursor(Qt::WaitCursor),"Cursor"), group);
	m_set->addProperty(new Property("LineStyle", 3, "Line Style", "", LineStyle), group);
	m_set->addProperty(new Property("SizePolicy", sizePolicy(), "Size Policy"), group);

//	kdDebug() << m_set->groupNames() << endl;

	Editor *edit = new Editor(this,true/*autosync*/);
	setCentralWidget(edit);
	edit->changeSet(m_set);
	resize(400,qApp->desktop()->height()-200);
	move(x(),5);
	edit->setFocus();
}

Test::~Test()
{
}

#include "test.moc"

/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2008-2009 Jarosław Staniek <staniek@kde.org>

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

#include "test.h"

#include "Property.h"
#include "EditorView.h"

#include <KoIcon.h>

#include <KLocale>
#include <KDebug>
#include <KCmdLineArgs>

#include <QDate>
#include <QDateTime>
#include <QPixmap>
#include <QStringList>
#include <QCursor>
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>

using namespace KoProperty;

TestWindow::TestWindow()
        : QWidget()
        , m_set(this, "test")
{
    setObjectName("koproperty_test");
    setWindowIcon(koIcon("document-properties"));

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    const bool flat = args->isSet("flat");
    const bool readOnly = args->isSet("ro");
    const QString singleProperty = args->getOption("property");

    /*  First, create the Set which will hold the properties.  */
    Property *p = 0;
    m_set.setReadOnly(readOnly);
    QByteArray group;
    if (!flat) {
        group = "SimpleGroup";
        m_set.setGroupDescription(group, "Simple Group");
    }
    if (singleProperty.isEmpty() || singleProperty=="Name") {
        m_set.addProperty(new Property("Name", "Name"), group);
        m_set["Name"].setAutoSync(1);
    }

    if (singleProperty.isEmpty() || singleProperty=="Int") {
        m_set.addProperty(new Property("Int", 2, "Int"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="Double") {
        m_set.addProperty(new Property("Double", 3.1415, "Double"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="Bool") {
        m_set.addProperty(new Property("Bool", QVariant(true), "Bool"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="3-State") {
        m_set.addProperty(p = new Property("3-State", QVariant(), "3 States", QString(), Bool), group);
        p->setOption("3State", true);
    }
    if (singleProperty.isEmpty() || singleProperty=="Date") {
        m_set.addProperty(p = new Property("Date", QDate::currentDate(), "Date"), group);
        p->setIcon("date");
    }
    if (singleProperty.isEmpty() || singleProperty=="Time") {
        m_set.addProperty(new Property("Time", QTime::currentTime(), "Time"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="DateTime") {
        m_set.addProperty(new Property("DateTime", QDateTime::currentDateTime(), "Date/Time"), group);
    }

    QStringList name_list; //strings
    name_list << "My Item" << "Other Item" << "Third Item";
    if (singleProperty.isEmpty() || singleProperty=="List") {
        QStringList list;//keys
        list << "myitem" << "otheritem" << "3rditem";
        m_set.addProperty(new Property("List", list, name_list, "otheritem", "List"), group);
    }

    if (singleProperty.isEmpty() || singleProperty=="List2") {
        // A valueFromList property matching strings with ints (could be any type supported by QVariant)
        QList<QVariant> keys;
        keys.append(1);
        keys.append(2);
        keys.append(3);
        Property::ListData *listData = new Property::ListData(keys, name_list);
        m_set.addProperty(new Property("List2", listData, 3, "List 2"), group);
    }

//  Complex
    if (!flat) {
        group = "ComplexGroup";
        m_set.setGroupDescription(group, "Complex Group");
    }
    if (singleProperty.isEmpty() || singleProperty=="Rect") {
        m_set.addProperty(new Property("Rect", QRect(5,11,100,200), "Rect"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="Point") {
        m_set.addProperty(new Property("Point", QPoint(3, 4), "Point"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="Size") {
        m_set.addProperty(new Property("Size", QSize(10, 20), "Size"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="RectF") {
        m_set.addProperty(new Property("RectF", QRectF(0.1, 0.5, 10.72, 18.21), "RectF"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="PointF") {
        m_set.addProperty(new Property("PointF", QPointF(3.14, 4.15), "PointF"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="SizeF") {
        m_set.addProperty(new Property("SizeF", QSizeF(1.1, 2.45), "SizeF"), group);
    }

//  Appearance
    if (!flat) {
        group = "Appearance Group";
        m_set.setGroupDescription(group, "Appearance Group");
        m_set.setGroupIcon(group, "appearance");
    }
    if (singleProperty.isEmpty() || singleProperty=="Color") {
        m_set.addProperty(new Property("Color", palette().color(QPalette::Active, QPalette::Background), "Color"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="Pixmap") {
        QPixmap pm(koDesktopIcon("network-wired"));
        m_set.addProperty(p = new Property("Pixmap", pm, "Pixmap"), group);
        p->setIcon("kpaint");
    }
    if (singleProperty.isEmpty() || singleProperty=="Font") {
        QFont myFont("Times", 12);
        m_set.addProperty(p = new Property("Font", myFont, "Font"), group);
        p->setIcon("fonts");
    }
    if (singleProperty.isEmpty() || singleProperty=="Cursor") {
        m_set.addProperty(new Property("Cursor", QCursor(Qt::WaitCursor), "Cursor"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="LineStyle") {
        m_set.addProperty(new Property("LineStyle", 3, "Line Style", QString(), LineStyle), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="SizePolicy") {
        QSizePolicy sp(sizePolicy());
        sp.setHorizontalStretch(1);
        sp.setVerticalStretch(2);
        m_set.addProperty(new Property("SizePolicy", sp, "Size Policy"), group);
    }
    if (singleProperty.isEmpty() || singleProperty=="Invisible") {
        m_set.addProperty(p = new Property("Invisible", "I am invisible", "Invisible"), group);
        p->setVisible(false);
    }

// kDebug(30007) << m_set.groupNames();
    QVBoxLayout *lyr = new QVBoxLayout(this);
    EditorView *editorView = new EditorView(this);
    lyr->addWidget(editorView);
    editorView->changeSet(&m_set);
//crashes.. why?: editorView->expandAll();
    resize(400, qApp->desktop()->height() - 200);
    editorView->setFocus();
    kDebug() << m_set;
}

TestWindow::~TestWindow()
{
}

#include "test.moc"

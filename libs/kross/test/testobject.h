/***************************************************************************
 * testobject.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_TEST_TESTOBJECT_H
#define KROSS_TEST_TESTOBJECT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMetaType>

#include <kdebug.h>

/// \internal
class TestObject : public QObject
{
        Q_OBJECT

        Q_PROPERTY(bool boolProperty READ boolProperty WRITE setBoolProperty)
        Q_PROPERTY(int intProperty READ intProperty WRITE setIntProperty)
        Q_PROPERTY(double doubleProperty READ doubleProperty WRITE setDoubleProperty)
        Q_PROPERTY(QString stringProperty READ stringProperty WRITE setStringProperty)
        Q_PROPERTY(QStringList stringListProperty READ stringListProperty WRITE setStringListProperty)
        Q_PROPERTY(QVariantList listProperty READ listProperty WRITE setListProperty)

    public:
        TestObject();
        TestObject(const QString& name);
        TestObject(const TestObject& obj);
        virtual ~TestObject();

    private:
        bool m_boolproperty;
        bool boolProperty() const { return m_boolproperty; }
        void setBoolProperty(bool prop) { m_boolproperty = prop; }

        int m_intproperty;
        int intProperty() const { return m_intproperty; }
        void setIntProperty(int prop) { m_intproperty = prop; }

        double m_doubleproperty;
        double doubleProperty() const { return m_doubleproperty; }
        void setDoubleProperty(double prop) { m_doubleproperty = prop; }

        QString m_stringproperty;
        QString stringProperty() const { return m_stringproperty; }
        void setStringProperty(QString prop) { m_stringproperty = prop; }

        QStringList m_stringlistproperty;
        QStringList stringListProperty() const { return m_stringlistproperty; }
        void setStringListProperty(QStringList prop) { m_stringlistproperty = prop; }

        QVariantList m_listproperty;
        QVariantList listProperty() const { return m_listproperty; }
        void setListProperty(QVariantList prop) { m_listproperty = prop; }

    signals:
        void signalVoid();
        void signalInt(int);
        void signalString(const QString&);

    public slots:

        // return the objectname
        QString name();

        // to test basic datatypes
        int func_int_int(int);
        bool func_bool_bool(bool);
        uint func_uint_uint(uint);
        double func_double_double(double);
        qlonglong func_qlonglong_qlonglong(qlonglong);
        qulonglong func_qulonglong_qulonglong(qulonglong);
        QByteArray func_qbytearray_qbytearray(QByteArray);
        QString func_qstring_qstring(const QString&);
        QStringList func_qstringlist_qstringlist(QStringList);
        QVariantList func_qvariantlist_qvariantlist(QVariantList);
        QVariantMap func_qvariantmap_qvariantmap(QVariantMap);
        QVariant func_qvariant_qvariant(const QVariant&);

        // for misc tests
        void func_void();
        void func_void_int(int);
        void func_void_qstring_int(QString,int);
        void func_void_qstringlist(QStringList);
        QString func_qstring_qstring_int(QString,int);

        // Kross::Object
        //void func_void_krossobject(Kross::Object::Ptr);
        //Kross::Object::Ptr func_krossobject_krossobject(Kross::Object::Ptr);

        // QObject
        void func_void_qobject(QObject*);
        QObject* func_qobject_qobject(QObject*);

        // TestObject
        void func_void_testobject(TestObject*);
        TestObject* func_testobject_testobject(TestObject*);

        //QObject* self() { return this; }
};

Q_DECLARE_METATYPE( TestObject* )

#endif

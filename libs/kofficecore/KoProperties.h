/*
   Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

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
   Boston, MA 02110-1301, USA.
 */
#ifndef _KO_PROPERTIES_H
#define _KO_PROPERTIES_H

#include <QString>
#include <QMap>
#include <QVariant>
#include "kofficecore_export.h"


/**
 * A KoProperties is the (de-)serializable representation of
 * a key-value map. The serialisation format is XML.
 */
class KOFFICECORE_EXPORT KoProperties {
public:

    /**
     * Create a new filter config.
     */
    KoProperties();

    /**
     * Deep copy the filter configFile
     */
    KoProperties(const KoProperties & rhs);

    ~KoProperties();

public:

    /**
     * Fill the filter configuration object from the XML encoded representation in string.
     * @param string the stored properties.
     */
    void load(const QString &string);


    /**
     * Create a serialized version of this filter config
     * @return the string version of this properties object
     */
    QString store();


    /**
     * Set the property with name to value.
     */
    void setProperty(const QString & name, const QVariant & value);

    /**
     * Set value to the value associated with property name
     * @return false if the specified property did not exist.
     */
    bool property(const QString & name, QVariant & value) const;

    /**
     * Return a property by name, wrapped in a QVariant.
     * A typical usage:
     *  @code
     *      KoProperties *props = new KoProperties();
     *      props->setProperty("name", "Marcy");
     *      props->setProperty("age", 25);
     *      QString name = props->property("name").toString();
     *      int age = props->property("age").toInt();
     *  @endcode
     * @return a property by name, wrapped in a QVariant.
     * @param name the name (or key) with which the variant was registered.
     * @see intProperty() stringProperty()
     */
    QVariant property(const QString & name) const;

    /**
     * Return an integer property by name.
     * A typical usage:
     *  @code
     *      KoProperties *props = new KoProperties();
     *      props->setProperty("age", 25);
     *      int age = props->intProperty("age");
     *  @endcode
     * @return an integer property by name
     * @param name the name (or key) with which the variant was registered.
     * @param def the default value, should there not be any property by the name this will be returned.
     * @see property() stringProperty()
     */
    int intProperty(const QString & name, int def = 0) const;
    /**
     * Return a double property by name.
     * @param name the name (or key) with which the variant was registered.
     * @param def the default value, should there not be any property by the name this will be returned.
     */
    double doubleProperty(const QString & name, double def = 0.0) const;
    /**
     * Return a boolean property by name.
     * @param name the name (or key) with which the variant was registered.
     * @param def the default value, should there not be any property by the name this will be returned.
     */
    bool boolProperty(const QString & name, bool def = false) const;
    /**
     * Return an QString property by name.
     * A typical usage:
     *  @code
     *      KoProperties *props = new KoProperties();
     *      props->setProperty("name", "Marcy");
     *      QString name = props->stringProperty("name");
     *  @endcode
     * @return an QString property by name
     * @param name the name (or key) with which the variant was registered.
     * @see property() intProperty()
     * @param def the default value, should there not be any property by the name this will be returned.
     */
    QString stringProperty(const QString & name, const QString & def = QString()) const;

private:

    class Private;
    Private * const d;
};

#endif // _KO_PROPERTIES_H

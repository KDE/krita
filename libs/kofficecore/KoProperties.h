/*
   Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 
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
#include <kdebug.h>
#include "koffice_export.h"


/**
 * A KoProperties is the serializable representation of
 * the filter parameters. Filters can subclass this class to implement
 * direct accessors to properties, but properties not in the map will
 * not be serialized.
 */
class KOFFICECORE_EXPORT KoProperties {

public:

    /**
     * Create a new filter config.
     */
    KoProperties() {}

    /**
     * Deep copy the filter configFile
     */
    KoProperties(const KoProperties & rhs);

    virtual ~KoProperties() {}

public:

    /**
     * Fill the filter configuration object from the XML encoded representation in s.
     */
    virtual void load(const QString &);


    /**
     * Create a serialized version of this filter config
     */
    virtual QString store();


    /**
     * Set the property with name to value.
     */
    virtual void setProperty(const QString & name, const QVariant & value);

    /**
     * Set value to the value associated with property name
     * @return false if the specified property did not exist.
     */
    virtual bool getProperty(const QString & name, QVariant & value);

    virtual QVariant getProperty(const QString & name);

    int getInt(const QString & name, int def = 0);
    double getDouble(const QString & name, double def = 0.0);
    bool getBool(const QString & name, bool def = false);
    QString getString(const QString & name, QString def = QString::null);

private:
    void dump();

protected:

    QMap<QString, QVariant> m_properties;

};

#endif // _KO_PROPERTIES_H

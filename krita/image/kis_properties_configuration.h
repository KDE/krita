/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _KIS_PROPERTIES_CONFIGURATION_H_
#define _KIS_PROPERTIES_CONFIGURATION_H_

#include <QString>
#include <QMap>
#include <QVariant>
#include <kdebug.h>

class QDomElement;
class QDomDocument;

#include "kis_serializable_configuration.h"
#include "krita_export.h"


/**
 * A KisSerializableConfiguration is the serializable representation of
 * parameters. This can subclass this class to implement
 * direct accessors to properties, but properties not in the map will
 * not be serialized.
 *
 * XXX: Use KoProperties here!
 */
class KRITAIMAGE_EXPORT KisPropertiesConfiguration : public KisSerializableConfiguration {

public:

    /**
     * Create a new filter config.
     */
    KisPropertiesConfiguration();

    /**
     * Deep copy the filter configFile
     */
    KisPropertiesConfiguration(const KisPropertiesConfiguration& rhs);

    virtual ~KisPropertiesConfiguration() {}

public:

    
    /**
     * Fill the filter configuration object from the XML encoded representation in s.
     * This function use the "Legacy" style XML in the 1.x kra file.
     */
    virtual void fromXML(const QString&);
    /**
     * Fill the filter configuration object from the XML encoded representation in s.
     * This function use the "Legacy" style XML in the 1.x kra file.
     */
    virtual void fromXML(const QDomElement&);
    
    /**
     * Create a serialized version of this filter config
     * This function use the "Legacy" style XML in the 1.x kra file.
     */
    virtual void toXML(QDomDocument&, QDomElement&) const;
    /**
     * Create a serialized version of this filter config
     * This function use the "Legacy" style XML in the 1.x kra file.
     */
    virtual QString toXML() const;
    
    
    /**
     * Set the property with name to value.
     */
    virtual void setProperty(const QString & name, const QVariant & value);

    /**
     * Set value to the value associated with property name
     * @return false if the specified property did not exist.
     */
    virtual bool getProperty(const QString & name, QVariant & value) const;

    virtual QVariant getProperty(const QString & name) const;

    
    
    int getInt(const QString & name, int def = 0) const;
    double getDouble(const QString & name, double def = 0.0) const;
    bool getBool(const QString & name, bool def = false) const;
    QString getString(const QString & name, const QString & def = QString::null) const;

    QMap<QString, QVariant> getProperties() const;

protected:
    /// Clear the map of properties
    void clearProperties();
private:
        void dump();
private:
    struct Private;
    Private* const d;
};

#endif // _KIS_FILTER_CONFIGURATION_H_

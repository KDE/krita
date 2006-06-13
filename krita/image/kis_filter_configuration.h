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
#ifndef _KIS_FILTER_CONFIGURATION_H_
#define _KIS_FILTER_CONFIGURATION_H_

#include <QString>
#include <QMap>
#include <QVariant>
#include <kdebug.h>
#include "krita_export.h"

class KisPreviewDialog;
class KisProgressDisplayInterface;
class KisFilterConfigWidget;
class QWidget;

/**
 * A KisFilterConfiguration is the serializable representation of
 * the filter parameters. Filters can subclass this class to implement
 * direct accessors to properties, but properties not in the map will
 * not be serialized.
 *
 * XXX: Use KoProperties here!
 */
class KRITAIMAGE_EXPORT KisFilterConfiguration {

public:

    /**
     * Create a new filter config.
     */
    KisFilterConfiguration(const QString & name, qint32 version)
        : m_name(name)
        , m_version(version) {}

    /**
     * Deep copy the filter configFile
     */
    KisFilterConfiguration(const KisFilterConfiguration & rhs);

    virtual ~KisFilterConfiguration() {}

public:

    /**
     * Fill the filter configuration object from the XML encoded representation in s.
     */
    virtual void fromXML(const QString &);

    /**
     * Create a serialized version of this filter config
     */
    virtual QString toString();

    /**
     * Get the unique, language independent name of the filter.
     */
    const QString & name() const;

    /**
     * Get the version of the filter that has created this config
     */
    qint32 version() const;

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

    QString m_name;
    qint32 m_version;
    QMap<QString, QVariant> m_properties;

};

#endif // _KIS_FILTER_CONFIGURATION_H_

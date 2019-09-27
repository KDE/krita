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
#include <kis_debug.h>
#include <kis_cubic_curve.h>
#include <KoColor.h>

class QDomElement;
class QDomDocument;

#include "kis_serializable_configuration.h"
#include "kritaimage_export.h"
#include "kis_types.h"


/**
 * KisPropertiesConfiguration is a map-based properties class that can
 * be serialized and deserialized.
 *
 * It differs from the base class KisSerializableConfiguration in that
 * it provides a number of convenience methods to get at the data and
 */
class KRITAIMAGE_EXPORT KisPropertiesConfiguration : public KisSerializableConfiguration
{

public:

    /**
     * Create a new properties  config.
     */
    KisPropertiesConfiguration();
    ~KisPropertiesConfiguration() override;

    /**
     * Deep copy the properties \p rhs
     */
    KisPropertiesConfiguration(const KisPropertiesConfiguration& rhs);

    /**
     * Deep copy the properties \p rhs
     */
    KisPropertiesConfiguration& operator=(const KisPropertiesConfiguration& rhs);

public:


    /**
     * Fill the properties  configuration object from the XML encoded representation in s.
     * This function use the "Legacy" style XML of the 1.x .kra file format.
     * @param xml the string that will be parsed as xml
     * @param clear if true, the properties map will be emptied.
     * @return true is the xml document could be parsed
     */
    bool fromXML(const QString& xml, bool clear = true) override;

    /**
     * Fill the properties  configuration object from the XML encoded representation in s.
     * This function use the "Legacy" style XML  of the 1.x .kra file format.
     *
     * Note: the existing properties will not be cleared
     */
    void fromXML(const QDomElement&) override;

    /**
     * Create a serialized version of this properties  config
     * This function use the "Legacy" style XML  of the 1.x .kra file format.
     */
    void toXML(QDomDocument&, QDomElement&) const override;

    /**
     * Create a serialized version of this properties  config
     * This function use the "Legacy" style XML  of the 1.x .kra file format.
     */
    QString toXML() const override;

    /**
     * @return true if the map contains a property with the specified name
     */
    virtual bool hasProperty(const QString& name) const;

    /**
     * Set the property with name to value.
     */
    virtual void setProperty(const QString & name, const QVariant & value);

    /**
     * Set value to the value associated with property name
     *
     * XXX: API alert: a setter that is prefixed with get?
     *
     * @return false if the specified property did not exist.
     */
    virtual bool getProperty(const QString & name, QVariant & value) const;

    virtual QVariant getProperty(const QString & name) const;

    template <typename T>
        T getPropertyLazy(const QString & name, const T &defaultValue) const {
        QVariant value = getProperty(name);
        return value.isValid() ? value.value<T>() : defaultValue;
    }

    QString getPropertyLazy(const QString & name, const char *defaultValue) const {
        return getPropertyLazy(name, QString(defaultValue));
    }

    int getInt(const QString & name, int def = 0) const;

    double getDouble(const QString & name, double def = 0.0) const;

    float getFloat(const QString& name, float def = 0.0) const;

    bool getBool(const QString & name, bool def = false) const;

    QString getString(const QString & name, const QString & def = QString()) const;

    KisCubicCurve getCubicCurve(const QString & name, const KisCubicCurve & curve = KisCubicCurve()) const;

    /**
     * @brief getColor fetch the given property as a KoColor.
     *
     * The color can be stored as
     * <ul>
     * <li>A KoColor
     * <li>A QColor
     * <li>A string that can be parsed as an XML color definition
     * <li>A string that QColor can convert to a color (see http://doc.qt.io/qt-5/qcolor.html#setNamedColor)
     * <li>An integer that QColor can convert to a color
     * </ul>
     *
     * @param name the name of the property
     * @param color the default value to be returned if the @param name does not exist.
     * @return returns the named property as a KoColor if the value can be converted to a color,
     * otherwise a empty KoColor is returned.
     */
    KoColor getColor(const QString& name, const KoColor& color = KoColor()) const;

    QMap<QString, QVariant> getProperties() const;

    /// Clear the map of properties
    void clearProperties();

    /// Marks a property that should not be saved by toXML
    void setPropertyNotSaved(const QString & name);

    void removeProperty(const QString & name);

    /**
     * Get the keys of all the properties in the object
     */
    virtual QList<QString> getPropertiesKeys() const;

    /**
     * Get a set of properties, which keys are prefixed with \p prefix. The settings object
     * \p config will have all these properties with the prefix stripped from them.
     */
    void getPrefixedProperties(const QString &prefix, KisPropertiesConfiguration *config) const;

    /**
     * A convenience override
     */
    void getPrefixedProperties(const QString &prefix, KisPropertiesConfigurationSP config) const;

    /**
     * Takes all the properties from \p config, adds \p prefix to all their keys and puths them
     * into this properties object
     */
    void setPrefixedProperties(const QString &prefix, const KisPropertiesConfiguration *config);

    /**
     * A convenience override
     */
    void setPrefixedProperties(const QString &prefix, const KisPropertiesConfigurationSP config);

    static QString escapeString(const QString &string);
    static QString unescapeString(const QString &string);

    void setProperty(const QString &name, const QStringList &value);
    QStringList getStringList(const QString &name, const QStringList &defaultValue = QStringList()) const;
    QStringList getPropertyLazy(const QString &name, const QStringList &defaultValue) const;

public:

    void dump() const;

private:

    struct Private;
    Private* const d;
};

class KRITAIMAGE_EXPORT KisPropertiesConfigurationFactory : public KisSerializableConfigurationFactory
{
public:
    KisPropertiesConfigurationFactory();
    ~KisPropertiesConfigurationFactory() override;
    KisSerializableConfigurationSP createDefault() override;
    KisSerializableConfigurationSP create(const QDomElement& e) override;
private:
    struct Private;
    Private* const d;
};

#endif

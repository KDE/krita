/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_INFOOBJECT_H
#define LIBKIS_INFOOBJECT_H

#include <QObject>
#include <kis_properties_configuration.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * InfoObject wrap a properties map. These maps can be used to set the
 * configuration for filters.
 */
class KRITALIBKIS_EXPORT InfoObject : public QObject
{
    Q_OBJECT

public:
    InfoObject(KisPropertiesConfigurationSP configuration);

    /**
     * Create a new, empty InfoObject.
     */
    explicit InfoObject(QObject *parent = 0);
    ~InfoObject() override;

    bool operator==(const InfoObject &other) const;
    bool operator!=(const InfoObject &other) const;
    /**
     * Return all properties this InfoObject manages.
     */
    QMap<QString, QVariant> properties() const;

    /**
     * Add all properties in the @p propertyMap to this InfoObject
     */
    void setProperties(QMap<QString, QVariant> propertyMap);

public Q_SLOTS:
    /**
     * set the property identified by @p key to @p value
     *
     * If you want create a property that represents a color, you can use a QColor
     * or hex string, as defined in https://doc.qt.io/qt-5/qcolor.html#setNamedColor.
     *
     */
    void setProperty(const QString &key, QVariant value);

    /**
     * return the value for the property identified by key, or None if there is no such key.
     */
    QVariant property(const QString &key);

private:

    friend class Filter;
    friend class Document;
    friend class Node;
    /**
     * @brief configuration gives access to the internal configuration object. Must
     * be used used internally in libkis
     * @return the internal configuration object.
     */
    KisPropertiesConfigurationSP configuration() const;

    struct Private;
    Private *d;

};

#endif // LIBKIS_INFOOBJECT_H

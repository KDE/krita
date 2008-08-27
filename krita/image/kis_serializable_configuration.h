/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
#ifndef _KIS_SERIALIZABLE_CONFIGURATION_H_
#define _KIS_SERIALIZABLE_CONFIGURATION_H_

class QDomElement;
class QDomDocument;

#include "krita_export.h"

/**
 * This is an interface for objects that are serializable and unserializable.
 * It can be used together with the factory in case the type of configuration object
 * is also unknown at creation time.
 */
class KRITAIMAGE_EXPORT KisSerializableConfiguration
{
public:

    virtual ~KisSerializableConfiguration() {};

    /**
     * Fill the object from the XML encoded representation in s.
     */
    virtual void fromXML(const QString&);

    /**
     * Fill the object from the XML encoded representation in s.
     */
    virtual void fromXML(const QDomElement&) = 0;

    /**
     * Create a serialized version of this object
     */
    virtual void toXML(QDomDocument&, QDomElement&) const = 0;

    /**
     * Create a serialized version of this object
     */
    virtual QString toXML() const;
};

/**
 * This is an interface for a factory of serializable configuration objects.
 */
class KRITAIMAGE_EXPORT KisSerializableConfigurationFactory
{
public:
    virtual ~KisSerializableConfigurationFactory();
    /**
     * @return an empty object with a sane default configuration
     */
    virtual KisSerializableConfiguration* createDefault() = 0;
    /**
     * @return an unserialied version of the configuration
     */
    virtual KisSerializableConfiguration* create(const QDomElement&) = 0;
};


#endif

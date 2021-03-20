/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_SERIALIZABLE_CONFIGURATION_H_
#define _KIS_SERIALIZABLE_CONFIGURATION_H_

class QDomElement;
class QDomDocument;
class QString;

#include "kritaimage_export.h"
#include "kis_shared.h"
#include "kis_shared_ptr.h"

/**
 * This is an interface for objects that are serializable and unserializable.
 * It can be used together with the factory in case the type of configuration object
 * is also unknown at creation time.
 */
class KRITAIMAGE_EXPORT KisSerializableConfiguration : public KisShared
{
public:

    KisSerializableConfiguration();

    virtual ~KisSerializableConfiguration() {}

    KisSerializableConfiguration(const KisSerializableConfiguration &rhs);

    /**
     * Fill the object from the XML encoded representation in s.
     */
    virtual bool fromXML(const QString&, bool);

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

typedef KisSharedPtr<KisSerializableConfiguration> KisSerializableConfigurationSP;

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
    virtual KisSerializableConfigurationSP createDefault() = 0;
    /**
     * @return an unserialied version of the configuration
     */
    virtual KisSerializableConfigurationSP create(const QDomElement&) = 0;
};



#endif

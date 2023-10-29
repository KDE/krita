/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007, 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_properties_configuration.h"


#include <kis_debug.h>
#include <QDomDocument>
#include <QString>

#include "kis_image.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "KoID.h"
#include "kis_types.h"
#include <KoColor.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>

struct Q_DECL_HIDDEN KisPropertiesConfiguration::Private {
    QMap<QString, QVariant> properties;
    QSet<QString> notSavedProperties;
};

KisPropertiesConfiguration::KisPropertiesConfiguration() : d(new Private)
{
}

KisPropertiesConfiguration::~KisPropertiesConfiguration()
{
    delete d;
}

KisPropertiesConfiguration::KisPropertiesConfiguration(const KisPropertiesConfiguration& rhs)
    : KisSerializableConfiguration(rhs)
    , d(new Private(*rhs.d))
{
}

KisPropertiesConfiguration &KisPropertiesConfiguration::operator=(const KisPropertiesConfiguration &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
    }

    return *this;
}

bool KisPropertiesConfiguration::fromXML(const QString & xml, bool clear)
{
    if (clear) {
        clearProperties();
    }

    QDomDocument doc;
    bool retval = doc.setContent(xml);
    if (retval) {
        QDomElement e = doc.documentElement();
        fromXML(e);
    }
    return retval;
}

void KisPropertiesConfiguration::fromXML(const QDomElement &root)
{
    QDomElement e;
    for (e = root.firstChildElement("param"); !e.isNull(); e = e.nextSiblingElement("param")) {
        QString name = e.attribute("name");
        QString value = e.text();

        // Older versions didn't have a "type" parameter,
        // so fall back to the old behavior if it's missing.
        if (!e.hasAttribute("type")) {
            d->properties[name] = QVariant(value);
        } else if (e.attribute("type") == "bytearray") {
            d->properties[name] = QVariant(QByteArray::fromBase64(value.toLatin1()));
        } else {
            d->properties[name] = value;
        }
    }
}

void KisPropertiesConfiguration::toXML(QDomDocument& doc, QDomElement& root) const
{
    QMap<QString, QVariant>::ConstIterator it;
    for (it = d->properties.constBegin(); it != d->properties.constEnd(); ++it) {
        if (d->notSavedProperties.contains(it.key())) {
            continue;
        }

        QDomElement e = doc.createElement("param");
        e.setAttribute("name", QString(it.key().toLatin1()));
        QString type = "string";
        QVariant v = it.value();
        QDomText text;
        if (v.type() == QVariant::UserType && v.userType() == qMetaTypeId<KisCubicCurve>()) {
            text = doc.createCDATASection(v.value<KisCubicCurve>().toString());
        } else if (v.type() == QVariant::UserType && v.userType() == qMetaTypeId<KoColor>()) {
            QDomDocument cdataDoc = QDomDocument("color");
            QDomElement cdataRoot = cdataDoc.createElement("color");
            cdataDoc.appendChild(cdataRoot);
            v.value<KoColor>().toXML(cdataDoc, cdataRoot);
            text = cdataDoc.createCDATASection(cdataDoc.toString());
            type = "color";
        } else if(v.type() == QVariant::String ) {
            text = doc.createCDATASection(v.toString());  // XXX: Unittest this!
            type = "string";
        } else if(v.type() == QVariant::ByteArray ) {
            text = doc.createTextNode(QString::fromLatin1(v.toByteArray().toBase64())); // Arbitrary Data
            type = "bytearray";
        } else {
            text = doc.createTextNode(v.toString());
            type = "internal";
        }
        e.setAttribute("type", type);
        e.appendChild(text);
        root.appendChild(e);
    }
}

QString KisPropertiesConfiguration::toXML() const
{
    QDomDocument doc = QDomDocument("params");
    QDomElement root = doc.createElement("params");
    doc.appendChild(root);
    toXML(doc, root);
    return doc.toString();
}


bool KisPropertiesConfiguration::hasProperty(const QString& name) const
{
    return d->properties.contains(name);
}

void KisPropertiesConfiguration::setProperty(const QString & name, const QVariant & value)
{
    if (d->properties.find(name) == d->properties.end()) {
        d->properties.insert(name, value);
    } else {
        d->properties[name] = value;
    }
}

bool KisPropertiesConfiguration::getProperty(const QString & name, QVariant & value) const
{
    if (d->properties.constFind(name) == d->properties.constEnd()) {
        return false;
    } else {
        value = d->properties.value(name);
        return true;
    }
}

QVariant KisPropertiesConfiguration::getProperty(const QString & name) const
{
    return d->properties.value(name, QVariant());
}


int KisPropertiesConfiguration::getInt(const QString & name, int def) const
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toInt();
    else
        return def;

}

double KisPropertiesConfiguration::getDouble(const QString & name, double def) const
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toDouble();
    else
        return def;
}

float KisPropertiesConfiguration::getFloat(const QString & name, float def) const
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return (float)v.toDouble();
    else
        return def;
}


bool KisPropertiesConfiguration::getBool(const QString & name, bool def) const
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toBool();
    else
        return def;
}

QString KisPropertiesConfiguration::getString(const QString & name, const QString & def) const
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toString();
    else
        return def;
}

KisCubicCurve KisPropertiesConfiguration::getCubicCurve(const QString & name, const KisCubicCurve & curve) const
{
    QVariant v = getProperty(name);
    if (v.isValid()) {
        if (v.type() == QVariant::UserType && v.userType() == qMetaTypeId<KisCubicCurve>()) {
            return v.value<KisCubicCurve>();
        } else {
            return KisCubicCurve(v.toString());
        }
    } else
        return curve;
}

KoColor KisPropertiesConfiguration::getColor(const QString& name, const KoColor& color) const
{
    QVariant v = getProperty(name);

    if (v.isValid()) {
        switch(v.type()) {
        case QVariant::UserType:
        {
            if (v.userType() == qMetaTypeId<KoColor>()) {
                return v.value<KoColor>();
            }
            break;
        }
        case QVariant::String:
        {
            QDomDocument doc;
            if (doc.setContent(v.toString())) {
                QDomElement e = doc.documentElement().firstChild().toElement();
                bool ok;
                KoColor c = KoColor::fromXML(e, Integer16BitsColorDepthID.id(), &ok);
                if (ok) {
                    return c;
                }
            }
            else {
                QColor c(v.toString());
                if (c.isValid()) {
                    KoColor kc(c, KoColorSpaceRegistry::instance()->rgb8());
                    return kc;
                }
            }
            break;
        }
        case QVariant::Color:
        {
            QColor c = v.value<QColor>();
            KoColor kc(c, KoColorSpaceRegistry::instance()->rgb8());
            return kc;
        }
        case QVariant::Int:
        {
            QColor c(v.toInt());
            if (c.isValid()) {
                KoColor kc(c, KoColorSpaceRegistry::instance()->rgb8());
                return kc;
            }
            break;
        }
        default:
            ;
        }
    }
    return color;
}

void KisPropertiesConfiguration::dump() const
{
    QMap<QString, QVariant>::ConstIterator it;
    for (it = d->properties.constBegin(); it != d->properties.constEnd(); ++it) {
        if (it->type() == QVariant::ByteArray) {
            QByteArray ba = it->toByteArray();

            if (ba.size() > 32) {
                qDebug() << it.key() << " = " << QString("...skipped total %1 bytes...").arg(ba.size()) << it.value().typeName();
            } else {
                qDebug() << it.key() << " = " << it.value() << it.value().typeName();
            }
        } else {
            qDebug() << it.key() << " = " << it.value() << it.value().typeName();
        }
    }

}

void KisPropertiesConfiguration::clearProperties()
{
    d->properties.clear();
}

void KisPropertiesConfiguration::setPropertyNotSaved(const QString& name)
{
    d->notSavedProperties.insert(name);
}

QMap<QString, QVariant> KisPropertiesConfiguration::getProperties() const
{
    return d->properties;
}

void KisPropertiesConfiguration::removeProperty(const QString & name)
{
    d->properties.remove(name);
}

QList<QString> KisPropertiesConfiguration::getPropertiesKeys() const
{
    return d->properties.keys();
}

void KisPropertiesConfiguration::getPrefixedProperties(const QString &prefix, KisPropertiesConfiguration *config) const
{
    const int prefixSize = prefix.size();

    const QList<QString> keys = getPropertiesKeys();
    Q_FOREACH (const QString &key, keys) {
        if (key.startsWith(prefix)) {
            config->setProperty(key.mid(prefixSize), getProperty(key));
        }
    }

    QString fullPrefix;
    const QString parentPrefix = getString(extractedPrefixKey());
    if (!parentPrefix.isEmpty()) {
        fullPrefix = parentPrefix + "/" + prefix;
    } else {
        fullPrefix = prefix;
    }

    config->setProperty(extractedPrefixKey(), fullPrefix);
    config->setPropertyNotSaved(extractedPrefixKey());
}

void KisPropertiesConfiguration::getPrefixedProperties(const QString &prefix, KisPropertiesConfigurationSP config) const
{
    getPrefixedProperties(prefix, config.data());
}

void KisPropertiesConfiguration::setPrefixedProperties(const QString &prefix, const KisPropertiesConfiguration *config)
{
    const QList<QString> keys = config->getPropertiesKeys();
    Q_FOREACH (const QString &key, keys) {
        this->setProperty(prefix + key, config->getProperty(key));
    }
}

void KisPropertiesConfiguration::setPrefixedProperties(const QString &prefix, const KisPropertiesConfigurationSP config)
{
    setPrefixedProperties(prefix, config.data());
}

QString KisPropertiesConfiguration::extractedPrefixKey()
{
    static const QString key = "__extractedFromPrefix";
    return key;
}

QString KisPropertiesConfiguration::escapeString(const QString &string)
{
    QString result = string;
    result.replace(";", "\\;");
    result.replace("]", "\\]");
    result.replace(">", "\\>");
    return result;
}

QString KisPropertiesConfiguration::unescapeString(const QString &string)
{
    QString result = string;
    result.replace("\\;", ";");
    result.replace("\\]", "]");
    result.replace("\\>", ">");
    return result;
}

void KisPropertiesConfiguration::setProperty(const QString &name, const QStringList &value)
{
    QStringList escapedList;
    escapedList.reserve(value.size());

    Q_FOREACH (const QString &str, value) {
        escapedList << escapeString(str);
    }

    setProperty(name, escapedList.join(';'));
}

QStringList KisPropertiesConfiguration::getStringList(const QString &name, const QStringList &defaultValue) const
{
    if (!hasProperty(name)) return defaultValue;

    const QString joined = getString(name);

    QStringList result;

    int afterLastMatch = -1;
    for (int i = 0; i < joined.size(); i++) {
        const bool lastChunk = i == joined.size() - 1;
        const bool matchedSplitter = joined[i] == ';' && (i == 0 || joined[i - 1] != '\\');

        if (lastChunk || matchedSplitter) {
            result << unescapeString(joined.mid(afterLastMatch, i - afterLastMatch + int(lastChunk && !matchedSplitter)));
            afterLastMatch = i + 1;
        }

        if (lastChunk && matchedSplitter) {
            result << QString();
        }
    }

    return result;
}

QStringList KisPropertiesConfiguration::getPropertyLazy(const QString &name, const QStringList &defaultValue) const
{
    return getStringList(name, defaultValue);
}

bool KisPropertiesConfiguration::compareTo(const KisPropertiesConfiguration* rhs) const
{
    if (rhs == nullptr)
        return false;

    for(const auto& propertyName: getPropertiesKeys()) {
        if (getProperty(propertyName) != rhs->getProperty(propertyName))
            return false;
    }

    return true;
}

// --- factory ---

struct Q_DECL_HIDDEN KisPropertiesConfigurationFactory::Private {
};

KisPropertiesConfigurationFactory::KisPropertiesConfigurationFactory() : d(new Private)
{
}

KisPropertiesConfigurationFactory::~KisPropertiesConfigurationFactory()
{
    delete d;
}

KisSerializableConfigurationSP KisPropertiesConfigurationFactory::createDefault()
{
    return new KisPropertiesConfiguration();
}

KisSerializableConfigurationSP KisPropertiesConfigurationFactory::create(const QDomElement& e)
{
    KisPropertiesConfigurationSP pc = new KisPropertiesConfiguration();
    pc->fromXML(e);
    return pc;
}


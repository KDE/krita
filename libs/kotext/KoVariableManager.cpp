/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008, 2011 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2011 Robert Mathias Marmorstein <robert@narnia.homeunix.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoVariableManager.h"

#include "KoInlineTextObjectManager.h"
#include "KoNamedVariable.h"
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>

class KoVariableManagerPrivate
{
public:
    KoVariableManagerPrivate()
            : lastId(KoInlineObject::VariableManagerStart) { }
    KoInlineTextObjectManager *inlineObjectManager;
    QHash<QString, int> variableMapping;
    QHash<int, QString> userTypes;
    QStringList variableNames;
    QStringList userVariableNames;
    int lastId;
};

KoVariableManager::KoVariableManager(KoInlineTextObjectManager *inlineObjectManager)
        : d(new KoVariableManagerPrivate)
{
    d->inlineObjectManager = inlineObjectManager;
}

KoVariableManager::~KoVariableManager()
{
    delete d;
}

void KoVariableManager::setValue(const QString &name, const QString &value, const QString &type)
{
    int key;
    // we store the mapping from name to key
    if (d->variableMapping.contains(name)) {
        key = d->variableMapping.value(name);
    } else {
        key = d->lastId++;
        d->variableMapping.insert(name, key);
        if (type.isEmpty()) {
            Q_ASSERT(!d->variableNames.contains(name));
            d->variableNames.append(name);
        } else {
            Q_ASSERT(!d->userVariableNames.contains(name));
            d->userVariableNames.append(name);
        }
    }
    if (!type.isEmpty()) {
        d->userTypes.insert(key, type);
    }
    // the variable manager stores the actual value of the variable.
    d->inlineObjectManager->setProperty(static_cast<KoInlineObject::Property>(key), value);
    emit valueChanged();
}

QString KoVariableManager::value(const QString &name) const
{
    int key = d->variableMapping.value(name);
    if (key == 0) {
        return QString();
    }
    return d->inlineObjectManager->stringProperty(static_cast<KoInlineObject::Property>(key));
}

QString KoVariableManager::userType(const QString &name) const
{
    int key = d->variableMapping.value(name);
    if (key == 0) {
        return QString();
    }
    QHash<int, QString>::const_iterator it = d->userTypes.constFind(key);
    if (it == d->userTypes.constEnd()) {
        return QString();
    }
    return it.value();
}

void KoVariableManager::remove(const QString &name)
{
    int key = d->variableMapping.value(name);
    if (key == 0) {
        return;
    }
    d->variableMapping.remove(name);
    d->userTypes.remove(key);
    d->variableNames.removeOne(name);
    d->userVariableNames.removeOne(name);
    d->inlineObjectManager->removeProperty(static_cast<KoInlineObject::Property>(key));
}

KoVariable *KoVariableManager::createVariable(const QString &name) const
{
    int key = d->variableMapping.value(name);
    if (key == 0) {
        return 0;
    }
    return new KoNamedVariable(static_cast<KoInlineObject::Property>(key), name);
}

QList<QString> KoVariableManager::variables() const
{
    return d->variableNames;
}

QList<QString> KoVariableManager::userVariables() const
{
    return d->userVariableNames;
}

#include "TextDebug.h"

void KoVariableManager::loadOdf(const KoXmlElement &bodyElement)
{
    KoXmlElement element = KoXml::namedItemNS(bodyElement, KoXmlNS::text, "user-field-decls", KoXmlTextContentPrelude);
    if (element.isNull())
        return;
    KoXmlElement e;
    forEachElement(e, element) {
        if (e.namespaceURI() != KoXmlNS::text || e.localName() != "user-field-decl")
            continue;
        const QString name = e.attributeNS(KoXmlNS::text, "name");
        QString type = e.attributeNS(KoXmlNS::office, "value-type");
        QString value;
        if (type == "string") {
            if (e.hasAttributeNS(KoXmlNS::office, "string-value"))
                value = e.attributeNS(KoXmlNS::office, "string-value");
            else // if the string-value is not present then the content defines the value
                value = e.toText().data();
        } else if (type == "boolean") {
            value = e.attributeNS(KoXmlNS::office, "boolean-value");
        } else if (type == "currency") {
            value = e.attributeNS(KoXmlNS::office, "currency");
        } else if (type == "date") {
            value = e.attributeNS(KoXmlNS::office, "date-value");
        } else if (type == "float") {
            value = e.attributeNS(KoXmlNS::office, "value");
        } else if (type == "percentage") {
            value = e.attributeNS(KoXmlNS::office, "value");
        } else if (type == "time") {
            value = e.attributeNS(KoXmlNS::office, "time-value");
        } else if (type == "void") {
            value = e.attributeNS(KoXmlNS::office, "value");
        } else if (e.hasAttributeNS(KoXmlNS::text, "formula")) {
            type = "formula";
            value = e.attributeNS(KoXmlNS::text, "formula");
        } else {
            warnText << "Unknown user-field-decl value-type=" << type;
            continue;
        }
        setValue(name, value, type);
    }
}

void KoVariableManager::saveOdf(KoXmlWriter *bodyWriter)
{
    if (userVariables().isEmpty()) {
        return;
    }
    bodyWriter->startElement("text:user-field-decls");
    foreach (const QString &name, userVariables()) {
        bodyWriter->startElement("text:user-field-decl");
        bodyWriter->addAttribute("text:name", name);
        QByteArray tag;
        QString type = userType(name);
        if (type == "formula") {
            tag = "text:formula";
        } else {
            if (type == "string") {
                tag = "office:string-value";
            } else if (type == "boolean") {
                tag = "office:boolean-value";
            } else if (type == "currency") {
                tag = "office:boolean-value";
            } else if (type == "date") {
                tag = "office:date-value";
            } else if (type == "float") {
                tag = "office:value";
            } else if (type == "percentage") {
                tag = "office:value";
            } else if (type == "time") {
                tag = "office:time-value";
            } else if (type == "void") {
                tag = "office:value";
            } else {
                tag = "office:string-value";
                type = "string";
            }
            bodyWriter->addAttribute("office:value-type", type);
        }
        bodyWriter->addAttribute(tag, value(name));
        bodyWriter->endElement();
    }
    bodyWriter->endElement();
}

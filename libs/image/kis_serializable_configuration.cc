/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_serializable_configuration.h"

#include <QDomElement>
#include <QDomDocument>
#include <QString>

KisSerializableConfiguration::KisSerializableConfiguration()
{
}

KisSerializableConfiguration::KisSerializableConfiguration(const KisSerializableConfiguration &)
    : KisShared()
{
}

bool KisSerializableConfiguration::fromXML(const QString &s, bool)
{
    QDomDocument doc;
    bool rv = doc.setContent(s);
    if (rv) {
        QDomElement e = doc.documentElement();
        fromXML(e);
    }
    return rv;
}

QString KisSerializableConfiguration::toXML() const
{
    QDomDocument doc = QDomDocument("params");
    QDomElement root = doc.createElement("params");
    doc.appendChild(root);
    toXML(doc, root);
    return doc.toString();
}

KisSerializableConfigurationFactory::~KisSerializableConfigurationFactory()
{
}

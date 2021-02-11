/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2016 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KisSwatch.h"
#include <QDomDocument>
#include <QDomElement>

KisSwatch::KisSwatch(const KoColor &color, const QString &name)
    : m_color(color)
    , m_name(name)
    , m_valid(true)
{ }

void KisSwatch::setName(const QString &name)
{
    m_name = name;
    m_valid = true;
}

void KisSwatch::setId(const QString &id)
{
    m_id = id;
    m_valid = true;
}

void KisSwatch::setColor(const KoColor &color)
{
    m_color = color;
    m_valid = true;
}

void KisSwatch::setSpotColor(bool spotColor)
{
    m_spotColor = spotColor;
    m_valid = true;
}

void KisSwatch::writeToStream(QDataStream &stream, const QString& groupName, int originalRow, int originalColumn)
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Color");
    root.setAttribute("bitdepth", color().colorSpace()->colorDepthId().id());
    doc.appendChild(root);
    color().toXML(doc, root);

    stream << name() << id() << spotColor()
           << originalRow << originalColumn
           << groupName << doc.toString();
}

KisSwatch KisSwatch::fromByteArray(QByteArray &data, QString &oldGroupName, int &originalRow, int &originalColumn)
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    KisSwatch s;
    QString name, id;
    bool spotColor;
    QString colorXml;

    while (!stream.atEnd()) {
        stream >> name >> id >> spotColor
                >> originalRow >> originalColumn
                >> oldGroupName
                >> colorXml;

        s.setName(name);
        s.setId(id);
        s.setSpotColor(spotColor);

        QDomDocument doc;
        doc.setContent(colorXml);
        QDomElement e = doc.documentElement();
        QDomElement c = e.firstChildElement();
        if (!c.isNull()) {
            QString colorDepthId = c.attribute("bitdepth", Integer8BitsColorDepthID.id());
            s.setColor(KoColor::fromXML(c, colorDepthId));
        }
    }

    return s;
}

KisSwatch KisSwatch::fromByteArray(QByteArray &data)
{
    QString s;
    int x, y;
    return fromByteArray(data, s, y, x);
}

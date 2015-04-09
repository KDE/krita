/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_asl_xml_writer.h"

#include <QDomDocument>
#include <QColor>
#include <QPointF>

#include "kis_dom_utils.h"

struct KisAslXmlWriter::Private
{
    QDomDocument document;
    QDomElement currentElement;
};


KisAslXmlWriter::KisAslXmlWriter()
    : m_d(new Private)
{
    QDomElement el = m_d->document.createElement("asl");
    m_d->document.appendChild(el);
    m_d->currentElement = el;
}

KisAslXmlWriter::~KisAslXmlWriter()
{
}

QDomDocument KisAslXmlWriter::document() const
{
    if (m_d->document.documentElement() != m_d->currentElement) {
        qWarning() << "KisAslXmlWriter::document(): unbalanced enter/leave descriptor/array";
    }

    return m_d->document;
}

void KisAslXmlWriter::enterDescriptor(const QString &key, const QString &name, const QString &classId)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Descriptor");
    el.setAttribute("name", name);
    el.setAttribute("classId", classId);

    m_d->currentElement.appendChild(el);
    m_d->currentElement = el;
}

void KisAslXmlWriter::leaveDescriptor()
{
    if (!m_d->currentElement.parentNode().toElement().isNull()) {
        m_d->currentElement = m_d->currentElement.parentNode().toElement();
    } else {
        qWarning() << "KisAslXmlWriter::leaveDescriptor(): unbalanced enter/leave descriptor";
    }
}

void KisAslXmlWriter::enterList(const QString &key)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "List");

    m_d->currentElement.appendChild(el);
    m_d->currentElement = el;
}

void KisAslXmlWriter::leaveList()
{
    if (!m_d->currentElement.parentNode().toElement().isNull()) {
        m_d->currentElement = m_d->currentElement.parentNode().toElement();
    } else {
        qWarning() << "KisAslXmlWriter::leaveList(): unbalanced enter/leave list";
    }
}

void KisAslXmlWriter::writeDouble(const QString &key, double value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Double");
    el.setAttribute("value", KisDomUtils::Private::numberToString(value));

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeInteger(const QString &key, int value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Integer");
    el.setAttribute("value", KisDomUtils::Private::numberToString(value));

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeEnum(const QString &key, const QString &typeId, const QString &value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Enum");
    el.setAttribute("typeId", typeId);
    el.setAttribute("value", value);

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeUnitFloat(const QString &key, const QString &unit, double value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "UnitFloat");
    el.setAttribute("unit", unit);
    el.setAttribute("value", KisDomUtils::Private::numberToString(value));

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeText(const QString &key, const QString &value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("value", value);

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeBoolean(const QString &key, bool value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Boolean");
    el.setAttribute("value", KisDomUtils::Private::numberToString(value));

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeColor(const QString &key, const QColor &value)
{
    enterDescriptor(key, "", "RGBC");

    writeDouble("Rd  ", value.red());
    writeDouble("Grn ", value.green());
    writeDouble("Bl  ", value.blue());

    leaveDescriptor();
}

void KisAslXmlWriter::writePoint(const QString &key, const QPointF &value)
{
    enterDescriptor(key, "", "CrPt");

    writeDouble("Hrzn", value.x());
    writeDouble("Vrtc", value.y());

    leaveDescriptor();
}

void KisAslXmlWriter::writeCurve(const QString &key, const QString &name, const QVector<QPointF> &points)
{
    enterDescriptor(key, "", "ShpC");

    writeText("Nm  ", name);

    enterList("Crv ");

    foreach (const QPointF &pt, points) {
        writePoint("", pt);
    }

    leaveList();
    leaveDescriptor();
}



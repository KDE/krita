/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_dom_utils.h"

#include <QTransform>
#include <QDebug>

#include "kis_debug.h"

namespace KisDomUtils {

void saveValue(QDomElement *parent, const QString &tag, const QSize &size)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "size");

    e.setAttribute("w", size.width());
    e.setAttribute("h", size.height());
}

void saveValue(QDomElement *parent, const QString &tag, const QRect &rc)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "rect");

    e.setAttribute("x", rc.x());
    e.setAttribute("y", rc.y());
    e.setAttribute("w", rc.width());
    e.setAttribute("h", rc.height());
}

void saveValue(QDomElement *parent, const QString &tag, const QPointF &pt)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "pointf");

    e.setAttribute("x", pt.x());
    e.setAttribute("y", pt.y());
}

void saveValue(QDomElement *parent, const QString &tag, const QVector3D &pt)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "vector3d");

    e.setAttribute("x", pt.x());
    e.setAttribute("y", pt.y());
    e.setAttribute("z", pt.z());
}

void saveValue(QDomElement *parent, const QString &tag, const QTransform &t)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "transform");

    e.setAttribute("m11", t.m11());
    e.setAttribute("m12", t.m12());
    e.setAttribute("m13", t.m13());

    e.setAttribute("m21", t.m21());
    e.setAttribute("m22", t.m22());
    e.setAttribute("m23", t.m23());

    e.setAttribute("m31", t.m31());
    e.setAttribute("m32", t.m32());
    e.setAttribute("m33", t.m33());
}

bool findOnlyElement(const QDomElement &parent, const QString &tag, QDomElement *el, QStringList *errorMessages)
{
    QDomNodeList list = parent.elementsByTagName(tag);
    if (list.size() != 1 || !list.at(0).isElement()) {
        QString msg = i18n("Could not find \"%1\" XML tag in \"%2\"", tag, parent.tagName());
        if (errorMessages) {
            *errorMessages << msg;
        } else {
            qWarning() << msg;
        }

        return false;
    }

    *el = list.at(0).toElement();
    return true;
}

namespace Private {
    bool checkType(const QDomElement &e, const QString &expectedType)
    {
        QString type = e.attribute("type", "unknown-type");
        if (type != expectedType) {
            qWarning() << i18n("Error: incorrect type (%2) for value %1. Expected %3", e.tagName(), type, expectedType);
            return false;
        }

        return true;
    }
}

bool loadValue(const QDomElement &parent, const QString &tag, QSize *size)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;
    if (!Private::checkType(e, "size")) return false;

    size->setWidth(e.attribute("w", "0").toInt());
    size->setHeight(e.attribute("h", "0").toInt());

    return true;
}

bool loadValue(const QDomElement &parent, const QString &tag, QRect *rc)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;
    if (!Private::checkType(e, "rect")) return false;

    rc->setX(e.attribute("x", "0").toInt());
    rc->setY(e.attribute("y", "0").toInt());
    rc->setWidth(e.attribute("w", "0").toInt());
    rc->setHeight(e.attribute("h", "0").toInt());

    return true;
}

bool loadValue(const QDomElement &parent, const QString &tag, QPointF *pt)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;
    if (!Private::checkType(e, "pointf")) return false;

    pt->setX(e.attribute("x", "0").toDouble());
    pt->setY(e.attribute("y", "0").toDouble());

    return true;
}

bool loadValue(const QDomElement &parent, const QString &tag, QVector3D *pt)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;
    if (!Private::checkType(e, "vector3d")) return false;

    pt->setX(e.attribute("x", "0").toDouble());
    pt->setY(e.attribute("y", "0").toDouble());
    pt->setZ(e.attribute("z", "0").toDouble());

    return true;
}

bool loadValue(const QDomElement &parent, const QString &tag, QTransform *t)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;
    if (!Private::checkType(e, "transform")) return false;

    qreal m11 = e.attribute("m11", "1.0").toDouble();
    qreal m12 = e.attribute("m12", "0.0").toDouble();
    qreal m13 = e.attribute("m13", "0.0").toDouble();

    qreal m21 = e.attribute("m21", "0.0").toDouble();
    qreal m22 = e.attribute("m22", "1.0").toDouble();
    qreal m23 = e.attribute("m23", "0.0").toDouble();

    qreal m31 = e.attribute("m31", "0.0").toDouble();
    qreal m32 = e.attribute("m32", "0.0").toDouble();
    qreal m33 = e.attribute("m33", "1.0").toDouble();

    t->setMatrix(
        m11, m12, m13,
        m21, m22, m23,
        m31, m32, m33);

    return true;
}


}

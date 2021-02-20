/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dom_utils.h"

#include <QTransform>

#include "kis_debug.h"

namespace KisDomUtils {

void saveValue(QDomElement *parent, const QString &tag, const QSize &size)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "size");

    e.setAttribute("w", toString(size.width()));
    e.setAttribute("h", toString(size.height()));
}

void saveValue(QDomElement *parent, const QString &tag, const QRect &rc)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "rect");

    e.setAttribute("x", toString(rc.x()));
    e.setAttribute("y", toString(rc.y()));
    e.setAttribute("w", toString(rc.width()));
    e.setAttribute("h", toString(rc.height()));
}

void saveValue(QDomElement *parent, const QString &tag, const QRectF &rc)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "rectf");

    e.setAttribute("x", toString(rc.x()));
    e.setAttribute("y", toString(rc.y()));
    e.setAttribute("w", toString(rc.width()));
    e.setAttribute("h", toString(rc.height()));
}

void saveValue(QDomElement *parent, const QString &tag, const QPoint &pt)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "point");

    e.setAttribute("x", toString(pt.x()));
    e.setAttribute("y", toString(pt.y()));
}

void saveValue(QDomElement *parent, const QString &tag, const QPointF &pt)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "pointf");

    e.setAttribute("x", toString(pt.x()));
    e.setAttribute("y", toString(pt.y()));
}

void saveValue(QDomElement *parent, const QString &tag, const QVector3D &pt)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "vector3d");

    e.setAttribute("x", toString(pt.x()));
    e.setAttribute("y", toString(pt.y()));
    e.setAttribute("z", toString(pt.z()));
}

void saveValue(QDomElement *parent, const QString &tag, const QTransform &t)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "transform");

    e.setAttribute("m11", toString(t.m11()));
    e.setAttribute("m12", toString(t.m12()));
    e.setAttribute("m13", toString(t.m13()));

    e.setAttribute("m21", toString(t.m21()));
    e.setAttribute("m22", toString(t.m22()));
    e.setAttribute("m23", toString(t.m23()));

    e.setAttribute("m31", toString(t.m31()));
    e.setAttribute("m32", toString(t.m32()));
    e.setAttribute("m33", toString(t.m33()));
}

void saveValue(QDomElement *parent, const QString &tag, const QColor &c)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "qcolor");
    e.setAttribute("value", c.name(QColor::HexArgb));
}

bool findOnlyElement(const QDomElement &parent, const QString &tag, QDomElement *el, QStringList *errorMessages)
{
    QDomNodeList list = parent.elementsByTagName(tag);
    if (list.size() != 1 || !list.at(0).isElement()) {
        QString msg = i18n("Could not find \"%1\" XML tag in \"%2\"", tag, parent.tagName());
        if (errorMessages) {
            *errorMessages << msg;
        } else {
            warnKrita << msg;
        }

        return false;
    }

    *el = list.at(0).toElement();
    return true;
}

bool removeElements(QDomElement &parent, const QString &tag) {
    QDomNodeList list = parent.elementsByTagName(tag);
    KIS_SAFE_ASSERT_RECOVER_NOOP(list.size() <= 1);

    for (int i = 0; i < list.size(); i++) {
        parent.removeChild(list.at(i));
    }

    return list.size() > 0;
}

namespace Private {
    bool checkType(const QDomElement &e, const QString &expectedType)
    {
        QString type = e.attribute("type", "unknown-type");
        if (type != expectedType) {
            warnKrita << i18n("Error: incorrect type (%2) for value %1. Expected %3", e.tagName(), type, expectedType);
            return false;
        }

        return true;
    }
}

bool loadValue(const QDomElement &e, float *v)
{
    if (!Private::checkType(e, "value")) return false;
    *v = toDouble(e.attribute("value", "0"));
    return true;
}

bool loadValue(const QDomElement &e, double *v)
{
    if (!Private::checkType(e, "value")) return false;
    *v = toDouble(e.attribute("value", "0"));
    return true;
}

bool loadValue(const QDomElement &e, QSize *size)
{
    if (!Private::checkType(e, "size")) return false;

    size->setWidth(toInt(e.attribute("w", "0")));
    size->setHeight(toInt(e.attribute("h", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QRect *rc)
{
    if (!Private::checkType(e, "rect")) return false;

    rc->setX(toInt(e.attribute("x", "0")));
    rc->setY(toInt(e.attribute("y", "0")));
    rc->setWidth(toInt(e.attribute("w", "0")));
    rc->setHeight(toInt(e.attribute("h", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QRectF *rc)
{
    if (!Private::checkType(e, "rectf")) return false;

    rc->setX(toInt(e.attribute("x", "0")));
    rc->setY(toInt(e.attribute("y", "0")));
    rc->setWidth(toInt(e.attribute("w", "0")));
    rc->setHeight(toInt(e.attribute("h", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QPoint *pt)
{
    if (!Private::checkType(e, "point")) return false;

    pt->setX(toInt(e.attribute("x", "0")));
    pt->setY(toInt(e.attribute("y", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QPointF *pt)
{
    if (!Private::checkType(e, "pointf")) return false;

    pt->setX(toDouble(e.attribute("x", "0")));
    pt->setY(toDouble(e.attribute("y", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QVector3D *pt)
{
    if (!Private::checkType(e, "vector3d")) return false;

    pt->setX(toDouble(e.attribute("x", "0")));
    pt->setY(toDouble(e.attribute("y", "0")));
    pt->setZ(toDouble(e.attribute("z", "0")));

    return true;
}

bool loadValue(const QDomElement &e, QTransform *t)
{
    if (!Private::checkType(e, "transform")) return false;

    qreal m11 = toDouble(e.attribute("m11", "1.0"));
    qreal m12 = toDouble(e.attribute("m12", "0.0"));
    qreal m13 = toDouble(e.attribute("m13", "0.0"));

    qreal m21 = toDouble(e.attribute("m21", "0.0"));
    qreal m22 = toDouble(e.attribute("m22", "1.0"));
    qreal m23 = toDouble(e.attribute("m23", "0.0"));

    qreal m31 = toDouble(e.attribute("m31", "0.0"));
    qreal m32 = toDouble(e.attribute("m32", "0.0"));
    qreal m33 = toDouble(e.attribute("m33", "1.0"));

    t->setMatrix(
        m11, m12, m13,
        m21, m22, m23,
        m31, m32, m33);

    return true;
}

bool loadValue(const QDomElement &e, QString *value)
{
    if (!Private::checkType(e, "value")) return false;
    *value = e.attribute("value", "no-value");
    return true;
}

bool loadValue(const QDomElement &e, QColor *value)
{
    if (!Private::checkType(e, "qcolor")) return false;
    value->setNamedColor(e.attribute("value", "#FFFF0000"));
    return true;
}

QDomElement findElementByAttibute(QDomNode parent,
                                  const QString &tag,
                                  const QString &attribute,
                                  const QString &key)
{
    QDomNode node = parent.firstChild();

    while (!node.isNull()) {
        QDomElement el = node.toElement();

        if (!el.isNull() && el.tagName() == tag) {
            QString value = el.attribute(attribute, "<undefined>");
            if (value == key) return el;
        }

        node = node.nextSibling();
    }

    return QDomElement();
}


}

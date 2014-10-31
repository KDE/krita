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

#ifndef __KIS_DOM_UTILS_H
#define __KIS_DOM_UTILS_H

#include <QPointF>
#include <QVector3D>
#include <QVector>
#include <QDomElement>
#include "klocale.h"
#include "krita_export.h"


namespace KisDomUtils {


void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QRect &rc);
void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QSize &size);
void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QPointF &pt);
void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QVector3D &pt);
void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QTransform &t);

template <typename T>
void saveValue(QDomElement *parent, const QString &tag, T value)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "value");

    e.setAttribute("value", value);
}

template <typename T>
void saveValue(QDomElement *parent, const QString &tag, const QVector<T> &array)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "array");

    int i = 0;
    foreach (const T &v, array) {
        saveValue(&e, QString("item_%1").arg(i++), v);
    }
}

bool KRITAIMAGE_EXPORT findOnlyElement(const QDomElement &parent, const QString &tag, QDomElement *el, QStringList *errorMessages = 0);

bool KRITAIMAGE_EXPORT loadValue(const QDomElement &parent, const QString &tag, QSize *size);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &parent, const QString &tag, QRect *rc);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &parent, const QString &tag, QPointF *pt);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &parent, const QString &tag, QVector3D *pt);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &parent, const QString &tag, QTransform *t);

bool KRITAIMAGE_EXPORT checkType(const QDomElement &e, const QString &expectedType);


template <typename T>
bool loadValue(const QDomElement &parent, const QString &tag, T *value)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;
    if (!checkType(e, "value")) return false;

    QVariant v(e.attribute("value", "no-value"));
    *value = v.value<T>();
    return true;
}

template <typename T>
bool loadValue(const QDomElement &parent, const QString &tag, QVector<T> *array)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;
    if (!checkType(e, "array")) return false;

    QDomElement child = e.firstChildElement();
    while (!child.isNull()) {
        T value;
        if (!loadValue(e, child.tagName(), &value)) return false;
        *array << value;
        child = child.nextSiblingElement();
    }
    return true;
}

}

#endif /* __KIS_DOM_UTILS_H */

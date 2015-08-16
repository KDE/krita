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
#include <QLocale>
#include "klocale.h"
#include "kritaimage_export.h"
#include "kis_debug.h"



namespace KisDomUtils {

    namespace Private {
        inline QString numberToString(const QString &value) {
            return value;
        }

        template<typename T>
        inline QString numberToString(T value) {
            return QString::number(value);
        }

        inline int stringToInt(const QString &str) {
            bool ok = false;
            int value = 0;

            QLocale c(QLocale::German);

            value = str.toInt(&ok);
            if (!ok) {
                value = c.toInt(str, &ok);
            }

            if (!ok) {
                qWarning() << "WARNING: KisDomUtils::stringToInt failed:" << ppVar(str);
                value = 0;
            }

            return value;
        }

        inline double stringToDouble(const QString &str) {
            bool ok = false;
            double value = 0;

            QLocale c(QLocale::German);

            /**
             * A special workaround to handle ','/'.' decimal point
             * in different locales. Added for backward compatibility,
             * because we used to save qreals directly using
             *
             * e.setAttribute("w", (qreal)value),
             *
             * which did local-aware conversion.
             */

            value = str.toDouble(&ok);
            if (!ok) {
                value = c.toDouble(str, &ok);
            }

            if (!ok) {
                qWarning() << "WARNING: KisDomUtils::stringToDouble failed:" << ppVar(str);
                value = 0;
            }

            return value;
        }
    }

/**
 * Save a value of type QRect into an XML tree. A child for \p parent
 * is created and assigned a tag \p tag.  The corresponding value can
 * be fetched from the XML using loadValue() later.
 *
 * \see loadValue()
 */
void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QRect &rc);
void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QSize &size);
void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QPointF &pt);
void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QVector3D &pt);
void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const QTransform &t);

/**
 * Save a value of a scalar type into an XML tree. A child for \p parent
 * is created and assigned a tag \p tag.  The corresponding value can
 * be fetched from the XML using loadValue() later.
 *
 * \see loadValue()
 */
template <typename T>
void saveValue(QDomElement *parent, const QString &tag, T value)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "value");
    e.setAttribute("value", Private::numberToString(value));
}

/**
 * Save a vector of values into an XML tree. A child for \p parent is
 * created and assigned a tag \p tag.  The values in the array should
 * have a type supported by saveValue() overrides. The corresponding
 * vector can be fetched from the XML using loadValue() later.
 *
 * \see loadValue()
 */
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

/**
 * Find an element with tag \p tag which is a child of \p parent. The element should
 * be the only element with the provided tag in this parent.
 *
 * \return true is the element with \p tag is found and it is unique
 */
bool KRITAIMAGE_EXPORT findOnlyElement(const QDomElement &parent, const QString &tag, QDomElement *el, QStringList *errorMessages = 0);


/**
 * Load an object from an XML element, which is a child of \p parent and has
 * a tag \p tag.
 *
 * \return true if the object is successfully loaded and is unique
 *
 * \see saveValue()
 */
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &e, float *v);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &e, double *v);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &e, QSize *size);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &e, QRect *rc);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &e, QPointF *pt);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &e, QVector3D *pt);
bool KRITAIMAGE_EXPORT loadValue(const QDomElement &e, QTransform *t);

namespace Private {
    bool KRITAIMAGE_EXPORT checkType(const QDomElement &e, const QString &expectedType);
}


/**
 * Load a scalar value from an XML element, which is a child of \p parent
 * and has a tag \p tag.
 *
 * \return true if the object is successfully loaded and is unique
 *
 * \see saveValue()
 */
template <typename T>
bool loadValue(const QDomElement &e, T *value)
{
    if (!Private::checkType(e, "value")) return false;

    QVariant v(e.attribute("value", "no-value"));
    *value = v.value<T>();
    return true;
}

/**
 * Load an array from an XML element, which is a child of \p parent
 * and has a tag \p tag.
 *
 * \return true if the object is successfully loaded and is unique
 *
 * \see saveValue()
 */
template <typename T>
bool loadValue(const QDomElement &e, QVector<T> *array)
{
    if (!Private::checkType(e, "array")) return false;

    QDomElement child = e.firstChildElement();
    while (!child.isNull()) {
        T value;
        if (!loadValue(child, &value)) return false;
        *array << value;
        child = child.nextSiblingElement();
    }
    return true;
}

template <typename T>
bool loadValue(const QDomElement &parent, const QString &tag, T *value)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;

    return loadValue(e, value);
}

KRITAIMAGE_EXPORT QDomElement findElementByAttibute(QDomNode parent,
                                                    const QString &tag,
                                                    const QString &attribute,
                                                    const QString &key);

}

#endif /* __KIS_DOM_UTILS_H */

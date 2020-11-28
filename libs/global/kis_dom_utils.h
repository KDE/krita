/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DOM_UTILS_H
#define __KIS_DOM_UTILS_H

#include <float.h>

#include <QPointF>
#include <QVector3D>
#include <QVector>
#include <QDomElement>
#include <QLocale>
#include <QColor>

#include <klocalizedstring.h>

#include "kritaglobal_export.h"
#include "kis_debug.h"
#include "krita_container_utils.h"

namespace KisDomUtils {

    inline QString toString(const QString &value) {
        return value;
    }

    template<typename T>
        inline QString toString(T value) {
        return QString::number(value);
    }

    inline QString toString(float value) {
        QString str;
        QTextStream stream;
        stream.setString(&str, QIODevice::WriteOnly);
        stream.setRealNumberPrecision(FLT_DIG);
        stream << value;
        return str;
    }

    inline QString toString(double value) {
        QString str;
        QTextStream stream;
        stream.setString(&str, QIODevice::WriteOnly);
        stream.setRealNumberPrecision(15);
        stream << value;
        return str;
    }

    inline int toInt(const QString &str, bool *ok=nullptr) {
        bool ok_locale = false;
        int value = 0;

        QLocale c(QLocale::German);

        value = str.toInt(&ok_locale);
        if (!ok_locale) {
            value = c.toInt(str, &ok_locale);
        }

        if (!ok_locale && ok == nullptr) {
            warnKrita << "WARNING: KisDomUtils::toInt failed:" << ppVar(str);
            value = 0;
        }

        if (ok != nullptr) {
            *ok = ok_locale;
        }

        return value;
    }

    inline double toDouble(const QString &str, bool *ok=nullptr) {
        bool ok_locale = false;
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

        value = str.toDouble(&ok_locale);
        if (!ok_locale) {
            value = c.toDouble(str, &ok_locale);
        }

        if (!ok_locale && ok == nullptr) {
            warnKrita << "WARNING: KisDomUtils::toDouble failed:" << ppVar(str);
            value = 0.0;
        }

        if (ok != nullptr) {
            *ok = ok_locale;
        }

        return value;
    }


    inline QString qColorToQString(QColor color)
    {
        // color channels will usually have 0-255
        QString customColor = QString::number(color.red()).append(",")
                             .append(QString::number(color.green())).append(",")
                             .append(QString::number(color.blue())).append(",")
                             .append(QString::number(color.alpha()));

        return customColor;
    }

    inline QColor qStringToQColor(QString colorString)
    {
        QStringList colorComponents = colorString.split(',');
        return QColor(colorComponents[0].toInt(), colorComponents[1].toInt(), colorComponents[2].toInt(), colorComponents[3].toInt());
    }




/**
 * Save a value of type QRect into an XML tree. A child for \p parent
 * is created and assigned a tag \p tag.  The corresponding value can
 * be fetched from the XML using loadValue() later.
 *
 * \see loadValue()
 */
void KRITAGLOBAL_EXPORT saveValue(QDomElement *parent, const QString &tag, const QRect &rc);
void KRITAGLOBAL_EXPORT saveValue(QDomElement *parent, const QString &tag, const QRectF &rc);
void KRITAGLOBAL_EXPORT saveValue(QDomElement *parent, const QString &tag, const QSize &size);
void KRITAGLOBAL_EXPORT saveValue(QDomElement *parent, const QString &tag, const QPoint &pt);
void KRITAGLOBAL_EXPORT saveValue(QDomElement *parent, const QString &tag, const QPointF &pt);
void KRITAGLOBAL_EXPORT saveValue(QDomElement *parent, const QString &tag, const QVector3D &pt);
void KRITAGLOBAL_EXPORT saveValue(QDomElement *parent, const QString &tag, const QTransform &t);
void KRITAGLOBAL_EXPORT saveValue(QDomElement *parent, const QString &tag, const QColor &t);

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
    e.setAttribute("value", toString(value));
}

/**
 * Save a vector of values into an XML tree. A child for \p parent is
 * created and assigned a tag \p tag.  The values in the array should
 * have a type supported by saveValue() overrides. The corresponding
 * vector can be fetched from the XML using loadValue() later.
 *
 * \see loadValue()
 */
template <template <class...> class Container, typename T, typename ...Args>
typename std::enable_if<KritaUtils::is_container<Container<T, Args...>>::value, void>::type
saveValue(QDomElement *parent, const QString &tag, const Container<T, Args...> &array)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "array");

    int i = 0;
    Q_FOREACH (const T &v, array) {
        saveValue(&e, QString("item_%1").arg(i++), v);
    }
}

/**
 * Find an element with tag \p tag which is a child of \p parent. The element should
 * be the only element with the provided tag in this parent.
 *
 * \return true is the element with \p tag is found and it is unique
 */
bool KRITAGLOBAL_EXPORT findOnlyElement(const QDomElement &parent, const QString &tag, QDomElement *el, QStringList *errorMessages = 0);


/**
 * Load an object from an XML element, which is a child of \p parent and has
 * a tag \p tag.
 *
 * \return true if the object is successfully loaded and is unique
 *
 * \see saveValue()
 */
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, float *v);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, double *v);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, QSize *size);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, QRect *rc);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, QRectF *rc);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, QPoint *pt);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, QPointF *pt);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, QVector3D *pt);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, QTransform *t);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, QString *value);
bool KRITAGLOBAL_EXPORT loadValue(const QDomElement &e, QColor *value);


namespace Private {
    bool KRITAGLOBAL_EXPORT checkType(const QDomElement &e, const QString &expectedType);
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
    typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
loadValue(const QDomElement &e, T *value)
{
    if (!Private::checkType(e, "value")) return false;

    QVariant v(e.attribute("value", "no-value"));
    *value = v.value<T>();
    return true;
}

/**
 * A special adapter method that makes vector- and tag-based methods
 * work with environment parameter uniformly.
 */
template <typename T, typename E>
    typename std::enable_if<std::is_empty<E>::value, bool>::type
loadValue(const QDomElement &parent, T *value, const E &/*env*/) {
    return loadValue(parent, value);
}

/**
 * Load an array from an XML element, which is a child of \p parent
 * and has a tag \p tag.
 *
 * \return true if the object is successfully loaded and is unique
 *
 * \see saveValue()
 */

template <template <class ...> class Container, typename T, typename E, typename ...Args>
typename std::enable_if<KritaUtils::is_appendable_container<Container<T, Args...>>::value, bool>::type
loadValue(const QDomElement &e, Container<T, Args...> *array, const E &env = std::tuple<>())
{
    if (!Private::checkType(e, "array")) return false;

    QDomElement child = e.firstChildElement();
    while (!child.isNull()) {
        T value;
        if (!loadValue(child, &value, env)) return false;
        array->push_back(value);
        child = child.nextSiblingElement();
    }
    return true;
}

template <template <class ...> class Container, typename T, typename E, typename F, typename ...Args>
typename std::enable_if<KritaUtils::is_appendable_container<Container<T, Args...>>::value, bool>::type
loadValue(const QDomElement &e, Container<T, Args...> *array, const E &env1, const F &env2)
{
    if (!Private::checkType(e, "array")) return false;

    QDomElement child = e.firstChildElement();
    while (!child.isNull()) {
        T value;
        if (!loadValue(child, &value, env1, env2)) return false;
        array->push_back(value);
        child = child.nextSiblingElement();
    }
    return true;
}

template <typename T, typename E = std::tuple<>>
    bool loadValue(const QDomElement &parent, const QString &tag, T *value, const E &env = E())
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;

    return loadValue(e, value, env);
}

template <typename T, typename E, typename F>
    bool loadValue(const QDomElement &parent, const QString &tag, T *value, const E &env1, const F &env2)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;

    return loadValue(e, value, env1, env2);
}


KRITAGLOBAL_EXPORT QDomElement findElementByAttibute(QDomNode parent,
                                                    const QString &tag,
                                                    const QString &attribute,
                                                    const QString &key);

KRITAGLOBAL_EXPORT bool removeElements(QDomElement &parent, const QString &tag);

}

#endif /* __KIS_DOM_UTILS_H */

/*
 *  SPDX-FileCopyrightText: 2024 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPORTINGUTILS_H
#define KISPORTINGUTILS_H

#include <QIODevice>
#include <QTextStream>
#include <QWidget>
#include <QScreen>
#include <QGuiApplication>

namespace KisPortingUtils
{

inline void setUtf8OnStream(QTextStream &stream)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    stream.setCodec("UTF-8");
#else
     stream.setEncoding(QStringConverter::Utf8);
#endif
}

inline int getScreenNumberForWidget(const QWidget *w)
{
    QList<QScreen *> screens = QGuiApplication::screens();
    if (w) {
        if (screens.contains(w->screen())) {
            return screens.indexOf(w->screen());
        }
    }

    return screens.indexOf(QGuiApplication::primaryScreen());
}

inline QString stringRemoveLast(QString str)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    return !str.isEmpty() ? str.remove(str.size() - 1, 1) : str;
#else
    return str.removeLast();
#endif
}

inline QString stringRemoveFirst(QString str)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    return !str.isEmpty() ? str.remove(0, 1) : str;
#else
    return str.removeFirst();
#endif
}

}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#define Q_UNREACHABLE_RETURN(...) Q_UNREACHABLE(); return __VA_ARGS__
#endif


#endif // KISPORTINGUTILS_H

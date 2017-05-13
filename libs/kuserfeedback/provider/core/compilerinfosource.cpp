/*
    Copyright (C) 2017 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "compilerinfosource.h"

#include <QVariant>

using namespace UserFeedback;

CompilerInfoSource::CompilerInfoSource()
    : AbstractDataSource(QStringLiteral("compiler"))
{
}

QString CompilerInfoSource::description() const
{
    return tr("The compiler used to build this application.");
}

#define STRINGIFY(x) #x
#define INT2STR(x) STRINGIFY(x)

QVariant CompilerInfoSource::data()
{
    QVariantMap m;

#ifdef Q_CC_GNU
    m.insert(QStringLiteral("type"), QStringLiteral("GCC"));
    m.insert(QStringLiteral("version"), QString::fromLatin1( "" INT2STR(__GNUC__) "." INT2STR(__GNUC_MINOR__)));
#endif

#ifdef Q_CC_CLANG
    m.insert(QStringLiteral("type"), QStringLiteral("Clang"));
    m.insert(QStringLiteral("version"), QString::fromLatin1( "" INT2STR(__clang_major__) "." INT2STR(__clang_minor__)));
#endif

#ifdef Q_CC_MSVC
    m.insert(QStringLiteral("type"), QStringLiteral("MSVC"));
    m.insert(QStringLiteral("version"), QString::fromLatin1( "" INT2STR(_MSC_VER)));
#endif

    if (m.isEmpty())
        m.insert(QStringLiteral("type"), QStringLiteral("unknown"));

    return m;
}

#undef STRINGIFY
#undef INT2STR

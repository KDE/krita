/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoVariableFactory.h"

#include <KoProperties.h>

#include <QStringList>

class KoVariableFactory::Private
{
public:
    Private(const QString & id)
            : id(id) {}

    ~Private() {
        foreach(const KoVariableTemplate & t, templates) {
            delete t.properties;
        }
    }

    const QString id;
    QList<KoVariableTemplate> templates;
    QString odfNameSpace;
    QStringList odfElementNames;
};

KoVariableFactory::KoVariableFactory(const QString & id)
        : d(new Private(id))
{
}

KoVariableFactory::~KoVariableFactory()
{
    delete d;
}

QString KoVariableFactory::id() const
{
    return d->id;
}

QList<KoVariableTemplate> KoVariableFactory::templates() const
{
    return d->templates;
}

void KoVariableFactory::addTemplate(const KoVariableTemplate &params)
{
    d->templates.append(params);
}

QStringList KoVariableFactory::odfElementNames() const
{
    return d->odfElementNames;
}

QString KoVariableFactory::odfNameSpace() const
{
    return d->odfNameSpace;
}

void KoVariableFactory::setOdfElementNames(const QString & nameSpace, const QStringList & names)
{
    d->odfNameSpace = nameSpace;
    d->odfElementNames = names;
}

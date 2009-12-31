/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
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

#include "KoShapeFactory.h"
#include "KoShape.h"
#include <KoProperties.h>

#include <kdebug.h>

class KoShapeFactory::Private
{
public:
    Private(const QString &i, const QString &n)
            : id(i),
            name(n),
            loadingPriority(0),
            hidden(false)
    {
    }

    ~Private() {
        foreach(const KoShapeTemplate & t, templates)
            delete t.properties;
        templates.clear();
    }

    QList<KoShapeTemplate> templates;
    QList<KoShapeConfigFactory*> configPanels;
    const QString id;
    const QString name;
    QString family;
    QString tooltip;
    QString iconName;
    int loadingPriority;
    QString odfNameSpace;
    QStringList odfElementNames;
    bool hidden;
};


KoShapeFactory::KoShapeFactory(QObject *parent, const QString &id, const QString &name)
        : QObject(parent),
        d(new Private(id, name))
{
}

KoShapeFactory::~KoShapeFactory()
{
    delete d;
}

KoShape * KoShapeFactory::createDefaultShapeAndInit(const QMap<QString, KoDataCenter *> & dataCenterMap) const
{
    KoShape * shape = createDefaultShape();
    Q_ASSERT(shape);
    shape->init(dataCenterMap);
    return shape;
}

KoShape * KoShapeFactory::createShapeAndInit(const KoProperties * params, const QMap<QString, KoDataCenter *> & dataCenterMap) const
{
    KoShape * shape = createShape(params);
    Q_ASSERT(shape);
    shape->init(dataCenterMap);
    return shape;
}

QString KoShapeFactory::toolTip() const
{
    return d->tooltip;
}

QString KoShapeFactory::icon() const
{
    return d->iconName;
}

QString KoShapeFactory::name() const
{
    return d->name;
}

QString KoShapeFactory::family() const
{
    return d->family;
}

int KoShapeFactory::loadingPriority() const
{
    return d->loadingPriority;
}

QStringList KoShapeFactory::odfElementNames() const
{
    return d->odfElementNames;
}

QString KoShapeFactory::odfNameSpace() const
{
    return d->odfNameSpace;
}

bool KoShapeFactory::supports(const KoXmlElement & e) const
{
    Q_UNUSED(e);
    // XXX: Remove this and replace with a pure virtual once
    // everything has implemented it.
    return false;
}

void KoShapeFactory::addTemplate(const KoShapeTemplate &params)
{
    KoShapeTemplate tmplate = params;
    tmplate.id = d->id;
    d->templates.append(tmplate);
}

void KoShapeFactory::setToolTip(const QString & tooltip)
{
    d->tooltip = tooltip;
}

void KoShapeFactory::setIcon(const QString & iconName)
{
    d->iconName = iconName;
}

void KoShapeFactory::setFamily(const QString & family)
{
    d->family = family;
}

QString KoShapeFactory::id() const
{
    return d->id;
}

void KoShapeFactory::setOptionPanels(const QList<KoShapeConfigFactory*> &panelFactories)
{
    d->configPanels = panelFactories;
}

QList<KoShapeConfigFactory*> KoShapeFactory::panelFactories()
{
    return d->configPanels;
}

QList<KoShapeTemplate> KoShapeFactory::templates() const
{
    return d->templates;
}

void KoShapeFactory::setLoadingPriority(int priority)
{
    d->loadingPriority = priority;
}

void KoShapeFactory::setOdfElementNames(const QString & nameSpace, const QStringList & names)
{
    d->odfNameSpace = nameSpace;
    d->odfElementNames = names;
}

bool KoShapeFactory::hidden() const
{
    return d->hidden;
}

void KoShapeFactory::setHidden(bool hidden)
{
    d->hidden = hidden;
}

#include <KoShapeFactory.moc>

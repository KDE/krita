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

#include "KoShapeFactoryBase.h"
#include "KoShape.h"
#include <KoProperties.h>

#include <kdebug.h>

class KoShapeFactoryBase::Private
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
    QList<KoShapeConfigFactoryBase*> configPanels;
    const QString id;
    const QString name;
    QString family;
    QString tooltip;
    QString iconName;
    int loadingPriority;
    QList<QPair<QString, QStringList> > odfElements; // odf name space -> odf element names
    bool hidden;
};


KoShapeFactoryBase::KoShapeFactoryBase(const QString &id, const QString &name)
        : d(new Private(id, name))
{
}

KoShapeFactoryBase::~KoShapeFactoryBase()
{
    delete d;
}

QString KoShapeFactoryBase::toolTip() const
{
    return d->tooltip;
}

QString KoShapeFactoryBase::icon() const
{
    return d->iconName;
}

QString KoShapeFactoryBase::name() const
{
    return d->name;
}

QString KoShapeFactoryBase::family() const
{
    return d->family;
}

int KoShapeFactoryBase::loadingPriority() const
{
    return d->loadingPriority;
}

QList<QPair<QString, QStringList> > KoShapeFactoryBase::odfElements() const
{
    return d->odfElements;
}

bool KoShapeFactoryBase::supports(const KoXmlElement & e) const
{
    Q_UNUSED(e);
    // XXX: Remove this and replace with a pure virtual once
    // everything has implemented it.
    return false;
}

void KoShapeFactoryBase::addTemplate(const KoShapeTemplate &params)
{
    KoShapeTemplate tmplate = params;
    tmplate.id = d->id;
    d->templates.append(tmplate);
}

void KoShapeFactoryBase::setToolTip(const QString & tooltip)
{
    d->tooltip = tooltip;
}

void KoShapeFactoryBase::setIcon(const QString & iconName)
{
    d->iconName = iconName;
}

void KoShapeFactoryBase::setFamily(const QString & family)
{
    d->family = family;
}

QString KoShapeFactoryBase::id() const
{
    return d->id;
}

void KoShapeFactoryBase::setOptionPanels(const QList<KoShapeConfigFactoryBase*> &panelFactories)
{
    d->configPanels = panelFactories;
}

QList<KoShapeConfigFactoryBase*> KoShapeFactoryBase::panelFactories()
{
    return d->configPanels;
}

QList<KoShapeTemplate> KoShapeFactoryBase::templates() const
{
    return d->templates;
}

void KoShapeFactoryBase::setLoadingPriority(int priority)
{
    d->loadingPriority = priority;
}

void KoShapeFactoryBase::setOdfElementNames(const QString & nameSpace, const QStringList & names)
{
    d->odfElements.clear();
    d->odfElements.append(QPair<QString, QStringList>(nameSpace, names));
}

void KoShapeFactoryBase::setOdfElements(const QList<QPair<QString, QStringList> > &elementNamesList)
{
    d->odfElements = elementNamesList;
}

bool KoShapeFactoryBase::hidden() const
{
    return d->hidden;
}

void KoShapeFactoryBase::setHidden(bool hidden)
{
    d->hidden = hidden;
}

void KoShapeFactoryBase::newDocumentResourceManager(KoResourceManager *manager)
{
    Q_UNUSED(manager);
}

KoShape *KoShapeFactoryBase::createShape(const KoProperties*, KoResourceManager *documentResources) const
{
    return createDefaultShape(documentResources);
}

/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#include <KoProperties.h>

class KoShapeFactory::Private {
public:
    Private(const QString &i, const QString &n) : id(i), name(n) {}
    ~Private() {
        foreach(KoShapeTemplate t, templates)
            delete t.properties;
        templates.clear();
    }
    QList<KoShapeTemplate> templates;
    QList<KoShapeConfigFactory*> configPanels;
    const QString id, name;
    QString tooltip;
    QString iconName;
};


KoShapeFactory::KoShapeFactory(QObject *parent, const QString &id, const QString &name)
    : QObject(parent),
    d(new Private(id, name))
{
}

KoShapeFactory::~KoShapeFactory() {
    delete d;
}

QString KoShapeFactory::toolTip() const {
    return d->tooltip;
}

QString KoShapeFactory::icon() const {
    return d->iconName;
}

QString KoShapeFactory::name() const {
    return d->name;
}

void KoShapeFactory::addTemplate(KoShapeTemplate &params) {
    params.id = d->id;
    d->templates.append(params);
}

void KoShapeFactory::setToolTip(const QString & tooltip) {
    d->tooltip = tooltip;
}

void KoShapeFactory::setIcon(const QString & iconName) {
    d->iconName = iconName;
}

QString KoShapeFactory::id() const {
    return d->id;
}

void KoShapeFactory::setOptionPanels(QList<KoShapeConfigFactory*> &panelFactories) {
    d->configPanels = panelFactories;
}

const QList<KoShapeConfigFactory*> &KoShapeFactory::panelFactories() {
    return d->configPanels;
}

const QList<KoShapeTemplate> KoShapeFactory::templates() const {
    return d->templates;
}

#include "KoShapeFactory.moc"

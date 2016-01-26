/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _KIS_GMIC_BLACKLISTER_H_
#define _KIS_GMIC_BLACKLISTER_H_
#include <QHash>
#include <QSet>
#include <QQueue>
#include <QString>
#include <QVector>

class QDomDocument;
class Command;
class Component;

class ComponentIterator
{
public:
    ComponentIterator(const Component * c);
    ~ComponentIterator();
    bool hasNext() const;
    const Component * next();

private:
    QQueue<const Component *> m_queue;
};

class KisGmicBlacklister
{
public:
    KisGmicBlacklister(const QString& filePath);
    ~KisGmicBlacklister();

    bool parseBlacklist();
    bool isBlacklisted(const QString &filterName, const QString &filterCategory);

    void dump();
    static QString toPlainText(const QString& htmlText);
    // finds the first filter that matches the filter category and name
    static Command* findFilter(const Component* rootNode, const QString& filterCategory, const QString& filterName);
    static Component* findFilterByPath(const Component* rootNode, const Component * path);


    static QList<Command* > findFilterByParamName(const Component* rootNode, const QString& paramName, const QString& paramType = QString());
    static QVector<Command* > filtersByName(const Component* rootNode, const QString& filterName);

    static QDomDocument dumpFiltersToXML(const Component* rootNode);

private:
    QString m_fileName;
    QHash<QString, QSet<QString> > m_categoryNameBlacklist;


};

#endif

/*
 * This file is part of the KDE project
 *
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_gmic_blacklister.h"
#include "kis_debug.h"

#include <QFile>
#include <QDomDocument>
#include <qdom.h>
#include <QTextDocument>


KisGmicBlacklister::KisGmicBlacklister(const QString& filePath):m_fileName(filePath)
{
    parseBlacklist();
}

KisGmicBlacklister::~KisGmicBlacklister()
{

}

bool KisGmicBlacklister::parseBlacklist()
{

    QDomDocument doc("mydocument");
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot open file";
        return false;
    }

    if (!doc.setContent(&file))
    {
        file.close();
        qDebug() << "Cannot set content";
        return false;
    }

    QDomElement docElem = doc.documentElement();
    if (docElem.tagName() != "blacklist")
    {
        // weird xml
        return false;
    }

    // iterate categories
    QDomNodeList nodeList = docElem.elementsByTagName("category");
    for(int i = 0;i < nodeList.count(); i++)
    {
        QDomElement categoryElement = nodeList.at(i).toElement();
        QString categoryName = categoryElement.attribute("name");

        // iterate filters
        QDomNodeList filterNodeList = categoryElement.elementsByTagName("filter");
        for (int j = 0; j < filterNodeList.count(); j++)
        {
            QDomElement filterElement = filterNodeList.at(j).toElement();
            QString filterName = filterElement.attribute("name");
            m_categoryNameBlacklist[ categoryName ].insert(filterName);
        }
    }

    return true;
}


void KisGmicBlacklister::dump()
{

    QList<QString> keysList = m_categoryNameBlacklist.keys();
    qDebug() << "Dumping ..." << keysList.count();
    foreach (const QString& item, keysList)
    {
        QSet<QString> filters = m_categoryNameBlacklist[item];
        qDebug() << item;
        foreach (const QString filterItem, filters)
        {
            qDebug() << "\t" << filterItem;
        }
    }
}

bool KisGmicBlacklister::isBlacklisted(const QString& filterName, const QString& filterCategory)
{
    // transform input
    QString filterNamePlain = toPlainText(filterName);
    QString filterCategoryPlain = toPlainText(filterCategory);

    bool result = false;
    if (!m_categoryNameBlacklist.contains(filterCategoryPlain))
    {
        return false;
    }

    QSet<QString> filters  = m_categoryNameBlacklist[ filterCategoryPlain ];
    return filters.contains(filterNamePlain);
}

QString KisGmicBlacklister::toPlainText(const QString& htmlText)
{
    QTextDocument doc;
    doc.setHtml(htmlText);
    return doc.toPlainText();
}

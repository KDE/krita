/*
   This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef koTemplateTree_h
#define koTemplateTree_h

#include <QList>
#include <QStringList>
#include <QPixmap>
#include <kcomponentdata.h>

class KoTemplate;
class KoTemplateGroup;

class KoTemplateTree
{

public:
    KoTemplateTree(const QByteArray &templateType, const KComponentData &instance,
                   bool readTree = false);
    ~KoTemplateTree();

    QByteArray templateType() const {
        return m_templateType;
    }
    KComponentData componentData() const {
        return m_componentData;
    }
    void readTemplateTree();
    void writeTemplateTree();

    void add(KoTemplateGroup *g);
    KoTemplateGroup *find(const QString &name) const;

    KoTemplateGroup *defaultGroup() const {
        return m_defaultGroup;
    }
    KoTemplate *defaultTemplate() const {
        return m_defaultTemplate;
    }

    QList<KoTemplateGroup*> groups () const { return m_groups; }

private:
    void readGroups();
    void readTemplates();
    void writeTemplate(KoTemplate *t, KoTemplateGroup *group,
                       const QString &localDir);

    QByteArray m_templateType;
    KComponentData m_componentData;
    QList<KoTemplateGroup*> m_groups;
    KoTemplateGroup *m_defaultGroup;
    KoTemplate *m_defaultTemplate;
};

#endif

/*
   This file is part of the KDE project
   ompyright (C) 2000 Werner Trobin <trobin@kde.org>

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

#ifndef KIS_TEMPLATE_TREE_H
#define KIS_TEMPLATE_TREE_H

#include <QList>
#include <QString>
#include "kritaui_export.h"

class KisTemplate;
class KisTemplateGroup;

class KRITAUI_EXPORT KisTemplateTree
{

public:
    KisTemplateTree(const QString &templatesResourcePath,
                   bool readTree = false);
    ~KisTemplateTree();

    QString templatesResourcePath() const {
        return m_templatesResourcePath;
    }
    void readTemplateTree();
    void writeTemplateTree();

    bool add(KisTemplateGroup *g);
    KisTemplateGroup *find(const QString &name) const;

    KisTemplateGroup *defaultGroup() const {
        return m_defaultGroup;
    }
    KisTemplate *defaultTemplate() const {
        return m_defaultTemplate;
    }

    QList<KisTemplateGroup*> groups () const { return m_groups; }

private:
    void readGroups();
    void readTemplates();
    void writeTemplate(KisTemplate *t, KisTemplateGroup *group,
                       const QString &localDir);

    QString m_templatesResourcePath;
    QList<KisTemplateGroup*> m_groups;
    KisTemplateGroup *m_defaultGroup;
    KisTemplate *m_defaultTemplate;
};

#endif

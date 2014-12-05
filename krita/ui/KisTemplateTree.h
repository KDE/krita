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
#include "krita_export.h"

class KisTemplate;
class KisTemplateGroup;

class KRITAUI_EXPORT KisTemplateTree
{

public:
    KisTemplateTree(const QByteArray &templateType, const KComponentData &instance,
                   bool readTree = false);
    ~KisTemplateTree();

    QByteArray templateType() const {
        return m_templateType;
    }
    KComponentData componentData() const {
        return m_componentData;
    }
    void readTemplateTree();
    void writeTemplateTree();

    void add(KisTemplateGroup *g);
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

    QByteArray m_templateType;
    KComponentData m_componentData;
    QList<KisTemplateGroup*> m_groups;
    KisTemplateGroup *m_defaultGroup;
    KisTemplate *m_defaultTemplate;
};

#endif

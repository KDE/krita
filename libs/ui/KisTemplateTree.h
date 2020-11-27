/*
   This file is part of the KDE project
   ompyright (C) 2000 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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

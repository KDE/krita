/*
   This file is part of the KDE project
   SPDX-FileCopyrightText: 2000 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIS_TEMPLATE_GROUP_H
#define KIS_TEMPLATE_GROUP_H

#include <QList>
#include <QStringList>

#include "kritaui_export.h"

class KisTemplate;

class KRITAUI_EXPORT KisTemplateGroup
{

public:
    explicit KisTemplateGroup(const QString &name,
                             const QString &dir = QString(),
                             int _sortingWeight = 0,
                             bool touched = false);
    ~KisTemplateGroup();

    QString name() const {
        return m_name;
    }
    QStringList dirs() const {
        return m_dirs;
    }
    void addDir(const QString &dir) {
        m_dirs.append(dir); m_touched = true;
    }
    int sortingWeight() const {
        return m_sortingWeight;
    }
    void setSortingWeight(int weight) {
        m_sortingWeight = weight;
    }
    /// If all children are hidden, we are hidden too
    bool isHidden() const;
    /// if we should hide, we hide all the children
    void setHidden(bool hidden = true) const;

    QList<KisTemplate*> templates() const { return m_templates; }

    bool add(KisTemplate *t, bool force = false, bool touch = true);
    KisTemplate *find(const QString &name) const;

    bool touched() const {
        return m_touched;
    }

private:
    QString m_name;
    QStringList m_dirs;
    QList<KisTemplate*> m_templates;
    mutable bool m_touched;
    int m_sortingWeight;
};

#endif

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

#ifndef koTemplateGroup_h
#define koTemplateGroup_h

#include <QList>
#include <QStringList>
#include <QPixmap>
#include <kcomponentdata.h>

class KoTemplate;

class KoTemplateGroup
{

public:
    explicit KoTemplateGroup(const QString &name,
                             const QString &dir = QString(),
                             int _sortingWeight = 0,
                             bool touched = false);
    ~KoTemplateGroup();

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

    QList<KoTemplate*> templates() const { return m_templates; }

    bool add(KoTemplate *t, bool force = false, bool touch = true);
    KoTemplate *find(const QString &name) const;

    bool touched() const {
        return m_touched;
    }

private:
    QString m_name;
    QStringList m_dirs;
    QList<KoTemplate*> m_templates;
    mutable bool m_touched;
    int m_sortingWeight;
};

#endif

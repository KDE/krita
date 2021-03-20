/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisTemplateGroup.h"

#include <KisTemplate.h>

#include <QFile>

KisTemplateGroup::KisTemplateGroup(const QString &name, const QString &dir,
                                   int _sortingWeight, bool touched)
    : m_name(name)
    , m_touched(touched)
    , m_sortingWeight(_sortingWeight)
{
    m_dirs.append(dir);
}

KisTemplateGroup::~KisTemplateGroup()
{
    qDeleteAll(m_templates);
}

bool KisTemplateGroup::isHidden() const
{

    QList<KisTemplate*>::const_iterator it = m_templates.begin();
    bool hidden = true;
    while (it != m_templates.end() && hidden) {
        hidden = (*it)->isHidden();
        ++it;
    }
    return hidden;
}

void KisTemplateGroup::setHidden(bool hidden) const
{
    Q_FOREACH (KisTemplate* t, m_templates)
        t->setHidden(hidden);

    m_touched = true;
}

bool KisTemplateGroup::add(KisTemplate *t, bool force, bool touch)
{

    KisTemplate *myTemplate = find(t->name());
    if (myTemplate == 0) {
        m_templates.append(t);
        m_touched = touch;
        return true;
    }
    else if (myTemplate && force) {
        //dbgUI <<"removing :" << myTemplate->fileName();
        QFile::remove(myTemplate->fileName());
        QFile::remove(myTemplate->picture());
        QFile::remove(myTemplate->file());
        m_templates.removeAll(myTemplate);
        delete myTemplate;
        m_templates.append(t);
        m_touched = touch;
        return true;
    }
    return false;
}

KisTemplate *KisTemplateGroup::find(const QString &name) const
{
    QList<KisTemplate*>::const_iterator it = m_templates.begin();
    KisTemplate* ret = 0;

    while (it != m_templates.end()) {
        if ((*it)->name() == name) {
            ret = *it;
            break;
        }

        ++it;
    }

    return ret;
}


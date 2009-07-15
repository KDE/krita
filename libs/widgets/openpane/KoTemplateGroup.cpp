/* This file is part of the KDE project
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

#include "KoTemplateGroup.h"

#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QPrinter>

#include <kdesktopfile.h>
#include <kconfig.h>
#include <kdebug.h>

#include <kcomponentdata.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <stdlib.h>

#include <KoTemplate.h>

KoTemplateGroup::KoTemplateGroup(const QString &name, const QString &dir,
                                 int _sortingWeight, bool touched) :
        m_name(name), m_touched(touched), m_sortingWeight(_sortingWeight)
{
    m_dirs.append(dir);
}

KoTemplateGroup::~KoTemplateGroup()
{
    qDeleteAll(m_templates);
}

bool KoTemplateGroup::isHidden() const
{

    QList<KoTemplate*>::const_iterator it = m_templates.begin();
    bool hidden = true;
    while (it != m_templates.end() && hidden) {
        hidden = (*it)->isHidden();
        ++it;
    }
    return hidden;
}

void KoTemplateGroup::setHidden(bool hidden) const
{
    foreach (KoTemplate* t, m_templates)
        t->setHidden(hidden);

    m_touched = true;
}

bool KoTemplateGroup::add(KoTemplate *t, bool force, bool touch)
{

    KoTemplate *myTemplate = find(t->name());
    if (myTemplate == 0L) {
        m_templates.append(t);
        m_touched = touch;
        return true;
    } else if (myTemplate && force) {
        //kDebug( 30003 ) <<"removing :" << myTemplate->fileName();
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

KoTemplate *KoTemplateGroup::find(const QString &name) const
{
    QList<KoTemplate*>::const_iterator it = m_templates.begin();
    KoTemplate* ret = NULL;

    while (it != m_templates.end()) {
        if ((*it)->name() == name) {
            ret = *it;
            break;
        }

        ++it;
    }

    return ret;
}


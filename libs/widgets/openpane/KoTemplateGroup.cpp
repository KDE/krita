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
    m_templates.setAutoDelete(true);
}

bool KoTemplateGroup::isHidden() const
{

    Q3PtrListIterator<KoTemplate> it(m_templates);
    bool hidden = true;
    while (it.current() != 0L && hidden) {
        hidden = it.current()->isHidden();
        ++it;
    }
    return hidden;
}

void KoTemplateGroup::setHidden(bool hidden) const
{

    Q3PtrListIterator<KoTemplate> it(m_templates);
    for (; it.current() != 0L; ++it)
        it.current()->setHidden(hidden);
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
        m_templates.removeRef(myTemplate);
        m_templates.append(t);
        m_touched = touch;
        return true;
    }
    return false;
}

KoTemplate *KoTemplateGroup::find(const QString &name) const
{

    Q3PtrListIterator<KoTemplate> it(m_templates);
    while (it.current() && it.current()->name() != name)
        ++it;
    return it.current();
}


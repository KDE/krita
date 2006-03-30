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

#ifndef koTemplates_h
#define koTemplates_h

#include <q3ptrlist.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <koffice_export.h>

class KInstance;

/** @internal */
class KOFFICECORE_EXPORT KoTemplate {

public:
    KoTemplate(const QString &name,
               const QString &description=QString::null,
               const QString &file=QString::null,
               const QString &picture=QString::null,
               const QString &fileName=QString::null,
               const QString &_measureSystem=QString::null,
               bool hidden=false, bool touched=false);
    ~KoTemplate() {}

    QString name() const { return m_name; }
    QString description() const { return m_descr; };
    QString file() const { return m_file; }
    QString picture() const { return m_picture; }
    QString fileName() const { return m_fileName; }
    const QPixmap &loadPicture( KInstance* instance );

    bool isHidden() const { return m_hidden; }
    void setHidden(bool hidden=true) { m_hidden=hidden; m_touched=true; }

    bool touched() const { return m_touched; }

    QString measureSystem() const { return m_measureSystem; }
    void setMeasureSystem(const QString& system) { m_measureSystem = system; }

private:
    QString m_name, m_descr, m_file, m_picture, m_fileName;
    bool m_hidden;
    mutable bool m_touched;
    bool m_cached;
    QPixmap m_pixmap;
    QString m_measureSystem;
};


class KOFFICECORE_EXPORT KoTemplateGroup {

public:
    KoTemplateGroup(const QString &name,
                    const QString &dir=QString::null,
                    int _sortingWeight=0,
                    bool touched=false);
    ~KoTemplateGroup() {}

    QString name() const { return m_name; }
    QStringList dirs() const { return m_dirs; }
    void addDir(const QString &dir) { m_dirs.append(dir); m_touched=true; }
    int sortingWeight() const { return m_sortingWeight; }
    void setSortingWeight(int weight) { m_sortingWeight = weight; }
    /// If all children are hidden, we are hidden too
    bool isHidden() const;
    /// if we should hide, we hide all the children
    void setHidden(bool hidden=true) const;

    KoTemplate *first() { return m_templates.first(); }
    KoTemplate *next() { return m_templates.next(); }
    KoTemplate *last() { return m_templates.last(); }
    KoTemplate *prev() { return m_templates.prev(); }
    KoTemplate *current() { return m_templates.current(); }

    bool add(KoTemplate *t, bool force=false, bool touch=true);
    KoTemplate *find(const QString &name) const;

    bool touched() const { return m_touched; }

private:
    QString m_name;
    QStringList m_dirs;
    Q3PtrList<KoTemplate> m_templates;
    mutable bool m_touched;
    int m_sortingWeight;
};


class KoTemplateTree {

public:
    KoTemplateTree(const QByteArray &templateType, KInstance *instance,
		   bool readTree=false);
    ~KoTemplateTree() {}

    QByteArray templateType() const { return m_templateType; }
    KInstance *instance() const { return m_instance; }
    void readTemplateTree();
    void writeTemplateTree();

    KoTemplateGroup *first() { return m_groups.first(); }
    KoTemplateGroup *next() { return m_groups.next(); }
    KoTemplateGroup *last() { return m_groups.last(); }
    KoTemplateGroup *prev() { return m_groups.prev(); }
    KoTemplateGroup *current() { return m_groups.current(); }

    void add(KoTemplateGroup *g);
    KoTemplateGroup *find(const QString &name) const;

    KoTemplateGroup *defaultGroup() const { return m_defaultGroup; }
    KoTemplate *defaultTemplate() const { return m_defaultTemplate; }

private:
    void readGroups();
    void readTemplates();
    void writeTemplate(KoTemplate *t, KoTemplateGroup *group,
		       const QString &localDir);

    QByteArray m_templateType;
    KInstance *m_instance;
    Q3PtrList<KoTemplateGroup> m_groups;
    KoTemplateGroup *m_defaultGroup;
    KoTemplate *m_defaultTemplate;
};


namespace KoTemplates {
QString trimmed(const QString &string);
}

#endif

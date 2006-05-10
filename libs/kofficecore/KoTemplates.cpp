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

#include <KoTemplates.h>

#include <QDir>
#include <qimage.h>
//Added by qt3to4:
#include <QPixmap>

#include <kdesktopfile.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kinstance.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kio/netaccess.h>

#include <stdlib.h>


KoTemplate::KoTemplate(const QString &name, const QString &description, const QString &file,
                       const QString &picture, const QString &fileName, const QString &_measureSystem,
                       bool hidden, bool touched) :
    m_name(name), m_descr(description), m_file(file), m_picture(picture), m_fileName(fileName),
    m_hidden(hidden), m_touched(touched), m_cached(false), m_measureSystem(_measureSystem)
{
}

const QPixmap &KoTemplate::loadPicture( KInstance* instance ) {

    if(m_cached)
        return m_pixmap;
    m_cached=true;
    if ( m_picture[ 0 ] == '/' )
    {
        // ### TODO: use the class KoPicture instead of QImage to support non-image pictures
        QImage img( m_picture );
        if (img.isNull()) {
            kWarning() << "Couldn't find icon " << m_picture << endl;
            m_pixmap=QPixmap();
            return m_pixmap;
        }
        const int maxHeightWidth = 128; // ### TODO: some people would surely like to have 128x128
        if (img.width() > maxHeightWidth || img.height() > maxHeightWidth) {
            img = img.scaled( maxHeightWidth, maxHeightWidth, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
        }
        m_pixmap = QPixmap::fromImage(img);
        return m_pixmap;
    } else { // relative path
        m_pixmap = instance->iconLoader()->loadIcon( m_picture, K3Icon::Desktop, 128 );
        return m_pixmap;
    }
}


KoTemplateGroup::KoTemplateGroup(const QString &name, const QString &dir,
                                 int _sortingWeight, bool touched) :
    m_name(name), m_touched(touched), m_sortingWeight(_sortingWeight)
{
    m_dirs.append(dir);
    m_templates.setAutoDelete(true);
}

bool KoTemplateGroup::isHidden() const {

    Q3PtrListIterator<KoTemplate> it(m_templates);
    bool hidden=true;
    while(it.current()!=0L && hidden) {
        hidden=it.current()->isHidden();
        ++it;
    }
    return hidden;
}

void KoTemplateGroup::setHidden(bool hidden) const {

    Q3PtrListIterator<KoTemplate> it(m_templates);
    for( ; it.current()!=0L; ++it)
        it.current()->setHidden(hidden);
    m_touched=true;
}

bool KoTemplateGroup::add(KoTemplate *t, bool force, bool touch) {

    KoTemplate *myTemplate=find(t->name());
    if(myTemplate==0L) {
        m_templates.append(t);
        m_touched=touch;
        return true;
    }
    else if(myTemplate && force) {
        //kDebug() << "removing :" << myTemplate->fileName() << endl;
        QFile::remove( myTemplate->fileName()  );
        QFile::remove( myTemplate->picture() );
        QFile::remove( myTemplate->file() );
        m_templates.removeRef(myTemplate);
        m_templates.append(t);
        m_touched=touch;
        return true;
    }
    return false;
}

KoTemplate *KoTemplateGroup::find(const QString &name) const {

    Q3PtrListIterator<KoTemplate> it(m_templates);
    while(it.current() && it.current()->name()!=name)
        ++it;
    return it.current();
}


KoTemplateTree::KoTemplateTree(const QByteArray &templateType,
                               KInstance *instance, bool readTree) :
    m_templateType(templateType), m_instance(instance), m_defaultGroup(0L),
    m_defaultTemplate(0L) {

    m_groups.setAutoDelete(true);
    if(readTree)
        readTemplateTree();
}

void KoTemplateTree::readTemplateTree() {

    readGroups();
    readTemplates();
}

void KoTemplateTree::writeTemplateTree() {
    QString localDir=m_instance->dirs()->saveLocation(m_templateType);

    for(KoTemplateGroup *group=m_groups.first(); group!=0L; group=m_groups.next()) {
        //kDebug() << "---------------------------------" << endl;
        //kDebug() << "group: " << group->name() << endl;

        bool touched=false;
        for(KoTemplate *t=group->first(); t!=0L && !touched && !group->touched(); t=group->next())
            touched=t->touched();

        if(group->touched() || touched) {
            //kDebug() << "touched" << endl;
            if(!group->isHidden()) {
                //kDebug() << "not hidden" << endl;
                KStandardDirs::makeDir(localDir+group->name()); // create the local group dir
            }
            else {
                //kDebug() << "hidden" << endl;
                if(group->dirs().count()==1 && group->dirs().contains(localDir)) {
                    //kDebug() << "local only" << endl;
                    KIO::NetAccess::del(group->dirs().first(), 0);
                    //kDebug() << "removing: " << group->dirs().first() << endl;
                }
                else {
                    //kDebug() << "global" << endl;
                    KStandardDirs::makeDir(localDir+group->name());
                }
            }
        }
        for(KoTemplate *t=group->first(); t!=0L; t=group->next()) {
            if(t->touched()) {
                //kDebug() << "++template: " << t->name() << endl;
                writeTemplate(t, group, localDir);
            }
            if(t->isHidden() && t->touched() ) {
                //kDebug() << "+++ delete local template ##############" << endl;
                writeTemplate(t, group, localDir);
                QFile::remove(t->file());
                QFile::remove(t->picture());
            }
        }
    }
}

void KoTemplateTree::add(KoTemplateGroup *g) {

    KoTemplateGroup *group=find(g->name());
    if(group==0L)
        m_groups.append(g);
    else
        group->addDir(g->dirs().first()); // "...there can be only one..." (Queen)
}

KoTemplateGroup *KoTemplateTree::find(const QString &name) const {

    Q3PtrListIterator<KoTemplateGroup> it(m_groups);
    while(it.current() && it.current()->name()!=name)
        ++it;
    return it.current();
}

void KoTemplateTree::readGroups() {

    QStringList dirs = m_instance->dirs()->resourceDirs(m_templateType);
    for(QStringList::ConstIterator it=dirs.begin(); it!=dirs.end(); ++it) {
        //kDebug() << "dir: " << *it << endl;
        QDir dir(*it);
        // avoid the annoying warning
        if(!dir.exists())
            continue;
        dir.setFilter(QDir::Dirs);
        QStringList templateDirs=dir.entryList();
        for(QStringList::ConstIterator tdirIt=templateDirs.begin(); tdirIt!=templateDirs.end(); ++tdirIt) {
            if(*tdirIt=="." || *tdirIt=="..") // we don't want to check those dirs :)
                continue;
            QDir templateDir(*it+*tdirIt);
            QString name=*tdirIt;
            QString defaultTab;
            int sortingWeight = 1000;
            if(templateDir.exists(".directory")) {
                KSimpleConfig config(templateDir.absolutePath()+"/.directory", true);
                config.setDesktopGroup();
                name=config.readEntry("Name");
                defaultTab=config.readEntry("X-KDE-DefaultTab");
                sortingWeight=config.readEntry("X-KDE-SortingWeight", 1000);
                //kDebug() << "name: " << name <<endl;
            }
            KoTemplateGroup *g=new KoTemplateGroup(name, *it+*tdirIt+QChar('/'), sortingWeight);
            add(g);
            if(defaultTab=="true")
                m_defaultGroup=g;
        }
    }
}

void KoTemplateTree::readTemplates() {

    Q3PtrListIterator<KoTemplateGroup> groupIt(m_groups);
    for( ; groupIt.current()!=0L; ++groupIt) {
        QStringList dirs=groupIt.current()->dirs();
        for(QStringList::ConstIterator it=dirs.begin(); it!=dirs.end(); ++it) {
            QDir d(*it);
            if( !d.exists() )
                continue;
            QStringList files=d.entryList( QDir::Files | QDir::Readable, QDir::Name );
            for(int i = 0; i < files.count(); ++i) {
                QString filePath = *it + files[i];
                //kDebug() << "filePath: " << filePath << endl;
                QString icon;
                QString text;
                QString description;
                QString hidden_str;
                QString fileName;
                bool hidden=false;
                bool defaultTemplate = false;
                QString templatePath;
                QString measureSystem;
                // If a desktop file, then read the name from it.
                // Otherwise (or if no name in it?) use file name
                if (KDesktopFile::isDesktopFile(filePath)) {
                    KSimpleConfig config(filePath, true);
                    config.setDesktopGroup();
                    if (config.readEntry("Type")=="Link") {
                        text=config.readEntry("Name");
                        fileName=filePath;
                        description=config.readEntry("Comment");
                        //kDebug() << "name: " << text << endl;
                        icon=config.readEntry("Icon");
                        if(icon[0]!='/' && // allow absolute paths for icons
                           QFile::exists(*it+icon)) // allow icons from icontheme
                            icon=*it+icon;
                        //kDebug() << "icon2: " << icon << endl;
                        hidden=config.readEntry("X-KDE-Hidden", false);
                        defaultTemplate = config.readEntry("X-KDE-DefaultTemplate", false);
                        measureSystem=config.readEntry("X-KDE-MeasureSystem").toLower();
                        //kDebug() << "hidden: " << hidden_str << endl;
                        templatePath=config.readPathEntry("URL");
                        //kDebug() << "Link to : " << templatePath << endl;
                        if(templatePath[0]!='/') {
                            if(templatePath.left(6)=="file:/") // I doubt this will happen
                                templatePath=templatePath.right(templatePath.length()-6);
                            //else
                            //  kDebug() << "dirname=" << *it << endl;
                            templatePath=*it+templatePath;
                            //kDebug() << "templatePath: " << templatePath << endl;
                        }
                    } else
                        continue; // Invalid
                }
                // The else if and the else branch are here for compat. with the old system
                else if ( files[i].right(4) != ".png" )
                    // Ignore everything that is not a PNG file
                    continue;
                else {
                    // Found a PNG file - the template must be here in the same dir.
                    icon = filePath;
                    QFileInfo fi(filePath);
                    text = fi.baseName();
                    templatePath = filePath; // Note that we store the .png file as the template !
                    // That's the way it's always been done. Then the app replaces the extension...
                }
                KoTemplate *t=new KoTemplate(text, description, templatePath, icon, fileName,
                                             measureSystem, hidden);
                groupIt.current()->add(t, false, false); // false -> we aren't a "user", false -> don't
                                                         // "touch" the group to avoid useless
                                                         // creation of dirs in .kde/blah/...
                if ( defaultTemplate )
                    m_defaultTemplate = t;
            }
        }
    }
}

void KoTemplateTree::writeTemplate(KoTemplate *t, KoTemplateGroup *group,
                                   const QString &localDir) {
    QString fileName;
    if ( t->isHidden() )
    {
        fileName = t->fileName();
        // try to remove the file
        if ( QFile::remove(fileName) || !QFile::exists(fileName) )
        {
            QFile::remove( t->name() );
            QFile::remove( t->picture() );
            return;
        }
    }
    // be sure that the template's file name is unique so we don't overwrite an other
    QString const path = localDir + group->name() + '/';
    QString const name = KoTemplates::trimmed( t->name() );
    fileName = path + name + ".desktop";
    if ( t->isHidden() && QFile::exists(fileName) )
        return;
    QString fill;
    while ( KIO::NetAccess::exists( fileName, true, 0 ) )
    {
        fill += '_';
        fileName = path + fill + name + ".desktop";
    }

    KSimpleConfig config( fileName );
    config.setDesktopGroup();
    config.writeEntry("Type", "Link");
    config.writePathEntry("URL", t->file());
    config.writeEntry("Name", t->name());
    config.writeEntry("Icon", t->picture());
    config.writeEntry("X-KDE-Hidden", t->isHidden());
}

namespace KoTemplates {
QString trimmed(const QString &string) {

    QString ret;
    for(int i = 0; i < string.length(); ++i) {
        QChar tmp(string[i]);
        if(!tmp.isSpace())
            ret+=tmp;
    }
    return ret;
}
}

/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisTemplateTree.h"

#include <QDir>
#include <QPrinter>
#include <QUrl>

#include <kdesktopfile.h>
#include <kconfig.h>
#include <kis_debug.h>


#include <KoResourcePaths.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>

#include <KisTemplate.h>
#include <KisTemplateGroup.h>
#include <KisTemplates.h>

KisTemplateTree::KisTemplateTree(const QString &templatesResourcePath,
                                 bool readTree)
    :  m_templatesResourcePath(templatesResourcePath)
    , m_defaultGroup(0)
    , m_defaultTemplate(0)
{
    if (readTree)
        readTemplateTree();
}

KisTemplateTree::~KisTemplateTree()
{
    qDeleteAll(m_groups);
}

void KisTemplateTree::readTemplateTree()
{
    readGroups();
    readTemplates();
}

void KisTemplateTree::writeTemplateTree()
{
    QString localDir = KoResourcePaths::saveLocation("templates");

    Q_FOREACH (KisTemplateGroup *group, m_groups) {
        //dbgUI <<"---------------------------------";
        //dbgUI <<"group:" << group->name();

        bool touched = false;
        QList<KisTemplate*> templates = group->templates();
        QList<KisTemplate*>::iterator it = templates.begin();
        for (; it != templates.end() && !touched && !group->touched(); ++it)
            touched = (*it)->touched();

        if (group->touched() || touched) {
            //dbgUI <<"touched";
            if (!group->isHidden()) {
                //dbgUI <<"not hidden";
                QDir path;
                path.mkpath(localDir + group->name()); // create the local group dir
            } else {
                //dbgUI <<"hidden";
                if (group->dirs().count() == 1 && group->dirs().contains(localDir)) {
                    //dbgUI <<"local only";
                    QFile f(group->dirs().first());
                    f.remove();
                    //dbgUI <<"removing:" << group->dirs().first();
                } else {
                    //dbgUI <<"global";
                    QDir path;
                    path.mkpath(localDir + group->name());
                }
            }
        }
        Q_FOREACH (KisTemplate *t, templates) {
            if (t->touched()) {
                //dbgUI <<"++template:" << t->name();
                writeTemplate(t, group, localDir);
            }
            if (t->isHidden() && t->touched()) {
                //dbgUI <<"+++ delete local template ##############";
                writeTemplate(t, group, localDir);
                QFile::remove(t->file());
                QFile::remove(t->picture());
            }
        }
    }
}

bool KisTemplateTree::add(KisTemplateGroup *g)
{

    KisTemplateGroup *group = find(g->name());
    if (group == 0) {
        m_groups.append(g);
        return true;
    }

    group->addDir(g->dirs().first()); // "...there can be only one..." (Queen)
    delete g;
    g = 0;
    return false;
}

KisTemplateGroup *KisTemplateTree::find(const QString &name) const
{
    QList<KisTemplateGroup*>::const_iterator it = m_groups.begin();
    KisTemplateGroup* ret = 0;

    while (it != m_groups.end()) {
        if ((*it)->name() == name) {
            ret = *it;
            break;
        }

        ++it;
    }

    return ret;
}

void KisTemplateTree::readGroups()
{
    QStringList dirs = KoResourcePaths::findDirs("templates");

    Q_FOREACH (const QString & dirName, dirs) {
        if (!dirName.contains("templates")) continue; // Hack around broken KoResourcePaths
        QDir dir(dirName);
        // avoid the annoying warning
        if (!dir.exists())
            continue;
        QStringList templateDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        Q_FOREACH (const QString & templateDirName, templateDirs) {
            QDir templateDir(dirName + "/" + templateDirName);
            QString name = templateDirName;
            QString defaultTab;
            int sortingWeight = 1000;
            if (templateDir.exists(".directory")) {
                KDesktopFile config(templateDir.absoluteFilePath(".directory"));
                KConfigGroup dg = config.desktopGroup();
                name = dg.readEntry("Name");
                defaultTab = dg.readEntry("X-KDE-DefaultTab");
                sortingWeight = dg.readEntry("X-KDE-SortingWeight", 1000);
            }
            KisTemplateGroup *g = new KisTemplateGroup(name, templateDir.absolutePath() + '/', sortingWeight);
            if (add(g)) {
                if (defaultTab == "true") {
                    m_defaultGroup = g;
                }
            }
        }
    }
}

void KisTemplateTree::readTemplates()
{
    QString dontShow = "imperial";
    if ( QLocale().measurementSystem() == QLocale::ImperialSystem) {
        dontShow = "metric";
    }

    Q_FOREACH (KisTemplateGroup* group, m_groups) {
        QStringList dirs = group->dirs();
        for (QStringList::ConstIterator it = dirs.constBegin(); it != dirs.constEnd(); ++it) {
            QDir d(*it);
            if (!d.exists())
                continue;
            QStringList files = d.entryList(QDir::Files | QDir::Readable, QDir::Name);
            for (int i = 0; i < files.count(); ++i) {
                QString filePath = *it + files[i];
                //dbgUI <<"filePath:" << filePath;
                QString icon;
                QString text;
                QString description;
                QString hidden_str;
                QString fileName;
                bool hidden = false;
                bool defaultTemplate = false;
                QString templatePath;
                QString measureSystem;
                // If a desktop file, then read the name from it.
                // Otherwise (or if no name in it?) use file name
                if (KDesktopFile::isDesktopFile(filePath)) {
                    KConfig _config(filePath, KConfig::SimpleConfig);
                    KConfigGroup config(&_config, "Desktop Entry");
                    if (config.readEntry("Type") == "Link") {
                        text = config.readEntry("Name");
                        fileName = filePath;
                        description = config.readEntry("Comment");
                        //dbgUI <<"name:" << text;
                        icon = config.readEntry("Icon");
                        if (icon[0] != '/' && // allow absolute paths for icons
                                QFile::exists(*it + icon)) // allow icons from icontheme
                            icon = *it + icon;
                        //dbgUI <<"icon2:" << icon;
                        hidden = config.readEntry("X-KDE-Hidden", false);
                        defaultTemplate = config.readEntry("X-KDE-DefaultTemplate", false);
                        measureSystem = config.readEntry("X-KDE-MeasureSystem").toLower();

                        // Don't add a template that is for the wrong measure system
                        if (measureSystem == dontShow)
                            continue;

                        //dbgUI <<"hidden:" << hidden_str;
                        templatePath = config.readPathEntry("URL", QString());
                        //dbgUI <<"Link to :" << templatePath;
                        if (templatePath[0] != '/') {
                            if (templatePath.left(6) == "file:/") // I doubt this will happen
                                templatePath = templatePath.right(templatePath.length() - 6);
                            //else
                            //  dbgUI <<"dirname=" << *it;
                            templatePath = *it + templatePath;
                            //dbgUI <<"templatePath:" << templatePath;
                        }
                    } else
                        continue; // Invalid
                }
                // The else if and the else branch are here for compat. with the old system
                else if (files[i].right(4) != ".png")
                    // Ignore everything that is not a PNG file
                    continue;
                else {
                    // Found a PNG file - the template must be here in the same dir.
                    icon = filePath;
                    QFileInfo fi(filePath);
                    text = fi.completeBaseName();
                    templatePath = filePath; // Note that we store the .png file as the template !
                    // That's the way it's always been done. Then the app replaces the extension...
                }
                KisTemplate *t = new KisTemplate(text, description, templatePath, icon, fileName,
                                               measureSystem, hidden);
                group->add(t, false, false); // false -> we aren't a "user", false -> don't
                // "touch" the group to avoid useless
                // creation of dirs in .kde/blah/...
                if (defaultTemplate)
                    m_defaultTemplate = t;
            }
        }
    }
}

void KisTemplateTree::writeTemplate(KisTemplate *t, KisTemplateGroup *group,
                                   const QString &localDir)
{
    QString fileName;
    if (t->isHidden()) {
        fileName = t->fileName();
        // try to remove the file
        if (QFile::remove(fileName) || !QFile::exists(fileName)) {
            QFile::remove(t->name());
            QFile::remove(t->picture());
            return;
        }
    }
    // be sure that the template's file name is unique so we don't overwrite an other
    QString const path = localDir + group->name() + '/';
    QString const name = KisTemplates::trimmed(t->name());
    fileName = path + name + ".desktop";
    if (t->isHidden() && QFile::exists(fileName))
        return;
    QString fill;
    while (QFile(fileName).exists()) {
        fill += '_';
        fileName = path + fill + name + ".desktop";
    }

    KConfig _config(fileName, KConfig::SimpleConfig);
    KConfigGroup config(&_config, "Desktop Entry");
    config.writeEntry("Type", "Link");
    config.writePathEntry("URL", t->file());
    config.writeEntry("Name", t->name());
    config.writeEntry("Icon", t->picture());
    config.writeEntry("X-KDE-Hidden", t->isHidden());
}

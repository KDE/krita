/***************************************************************************
 * actioncollection.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "actioncollection.h"
#include "action.h"
#include "manager.h"

#include <QStringList>
#include <QPointer>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QDomElement>

//#include <kapplication.h>
//#include <klocale.h>
#include <kicon.h>
#include <kmimetype.h>
#include <kstandarddirs.h>

using namespace Kross;

namespace Kross {

    /// \internal d-pointer class.
    class ActionCollection::Private
    {
        public:
            QPointer<ActionCollection> parent;
            QHash< QString, QPointer<ActionCollection> > collections;
            QStringList collectionnames;

            Private(ActionCollection* const p) : parent(p) {}
    };

}

ActionCollection::ActionCollection(const QString& name, ActionCollection* parent)
    : KActionCollection(parent)
    , d( new Private(parent) )
{
    setObjectName(name);
    if( d->parent )
        d->parent->registerCollection(this);
}

ActionCollection::~ActionCollection()
{
    if( d->parent )
        d->parent->unregisterCollection(objectName());
    delete d;
}

ActionCollection* ActionCollection::parentCollection() const
{
    return d->parent;
}

bool ActionCollection::hasCollection(const QString& name) const
{
    return d->collections.contains(name);
}

ActionCollection* ActionCollection::collection(const QString& name) const
{
    return d->collections[name];
}

QStringList ActionCollection::collections() const
{
    return d->collectionnames;
}

void ActionCollection::registerCollection(ActionCollection* collection)
{
    const QString name = collection->objectName();
    //Q_ASSERT( !name.isNull() );
    d->collections.insert(name, collection);
    d->collectionnames.append(name);
}

void ActionCollection::unregisterCollection(const QString& name)
{
    d->collectionnames.removeAll(name);
    d->collections.remove(name);
}

/*********************************************************************
 * Unserialize from XML / QIODevice / file / resource to child
 * ActionCollection's and Action's this ActionCollection has.
 */

bool ActionCollection::readXml(const QDomElement& element, const QDir& directory)
{
    #ifdef KROSS_ACTIONCOLLECTION_DEBUG
        krossdebug( QString("ActionCollection::readXml tagName=\"%1\"").arg(element.tagName()) );
    #endif

    bool ok = true;
    QDomNodeList list = element.childNodes();
    const int size = list.size();
    for(int i = 0; i < size; ++i) {
        QDomElement elem = list.item(i).toElement();
        if( elem.isNull() ) continue;

        #ifdef KROSS_ACTIONCOLLECTION_DEBUG
            krossdebug( QString("  ActionCollection::readXml child=%1 tagName=\"%2\"").arg(i).arg(elem.tagName()) );
        #endif

        if( elem.tagName() == "collection") {
            const QString name = elem.attribute("name");
            ActionCollection* c = d->collections.contains(name) ? d->collections[name] : 0;
            if( ! c )
                c = new ActionCollection(name, this);
            if( ! c->readXml(elem) )
                ok = false;
        }
        else if( elem.tagName() == "script") {
            QString name = elem.attribute("name");

            QString file = elem.attribute("file");
            if(! QFileInfo(file).exists()) {
                QFileInfo fi(directory, file);
                if( fi.exists() ) {
                    file = fi.absoluteFilePath();
                }
                else {
                    #ifdef KROSS_ACTIONCOLLECTION_DEBUG
                        krosswarning( QString("    ActionCollection::readXml Failed to find file \"%1\" in the script-tag with name=\"%2\"").arg(file).arg(name) );
                    #endif
                    //QString resource = KGlobal::dirs()->findResource("appdata", QString("scripts/%1/%2").arg(name).arg(f));
                    //if( ! resource.isNull() ) f = resource;
                    file = QString();
                }
            }

            QString text = elem.attribute("text");
            if( text.isEmpty() )
                text = file;

            QString description = elem.attribute("comment");
            if( description.isEmpty() )
                description = text.isEmpty() ? name : text;

            QString icon = elem.attribute("icon");
            if( icon.isEmpty() )
                icon = KMimeType::iconNameForUrl( KUrl(file) );

            QString interpreter = elem.attribute("interpreter");

            Action* a = dynamic_cast< Action* >( action(name) );
            if( a ) {
                #ifdef KROSS_ACTIONCOLLECTION_DEBUG
                    krossdebug( QString("  ActionCollection::readXml Updating Action \"%1\"").arg(a->objectName()) );
                #endif
            }
            else {
                #ifdef KROSS_ACTIONCOLLECTION_DEBUG
                    krossdebug( QString("  ActionCollection::readXml Creating Action \"%1\"").arg(name) );
                #endif

                a = new Action(this, name);
                //FIXME move that functionality direct to the Action ctor?
                connect(a, SIGNAL( started(Kross::Action*) ), &Manager::self(), SIGNAL( started(Kross::Action*)) );
                connect(a, SIGNAL( finished(Kross::Action*) ), &Manager::self(), SIGNAL( finished(Kross::Action*) ));
            }

            a->setText(text);
            a->setDescription(description);
            if( ! icon.isNull() )
                a->setIcon(KIcon(icon));
            if( ! interpreter.isNull() )
                a->setInterpreter(interpreter);
            a->setFile(file);
        }
        //else if( ! fromXml(elem) ) ok = false;
    }
    return ok;
}

bool ActionCollection::readXml(QIODevice* device, const QDir& directory)
{
    QString errMsg;
    int errLine, errCol;
    QDomDocument document;
    bool ok = document.setContent(device, false, &errMsg, &errLine, &errCol);
    if( ! ok ) {
        #ifdef KROSS_ACTIONCOLLECTION_DEBUG
            krosswarning( QString("ActionCollection::readXml Error at line %1 in col %2: %3").arg(errLine).arg(errCol).arg(errMsg) );
        #endif
        return false;
    }
    return readXml(document.documentElement(), directory);
}

bool ActionCollection::readXmlFile(const QString& file)
{
    #ifdef KROSS_ACTIONCOLLECTION_DEBUG
        krossdebug( QString("ActionCollection::readXmlFile file=\"%1\"").arg(file) );
    #endif

    QFile f(file);
    if( ! f.open(QIODevice::ReadOnly) ) {
        #ifdef KROSS_ACTIONCOLLECTION_DEBUG
            krosswarning( QString("ActionCollection::readXmlFile reading file \"%1\" failed.").arg(file) );
        #endif
        return false;
    }
    bool ok = readXml(&f, QFileInfo(file).dir());
    f.close();

    #ifdef KROSS_ACTIONCOLLECTION_DEBUG
        if( ! ok )
            krosswarning( QString("ActionCollection::readXmlFile parsing XML content of file \"%1\" failed.").arg(file) );
    #endif
    return ok;
}

bool ActionCollection::readXmlResource(const QByteArray& resource, const QString& filer)
{
    //filer = KApplication::kApplication()->objectName() + "/scripts/*/*.rc";
    QStringList files = KGlobal::dirs()->findAllResources(resource, filer);
    //files.sort();
    bool ok = true;
    foreach(QString s, files)
        if( ! readXmlFile(s) )
            ok = false;
    return ok;
}

/*********************************************************************
 * Serialize from child ActionCollection's and Action's this
 * ActionCollection has to XML / QIODevice / file / resource.
 */

QDomElement ActionCollection::writeXml()
{
    #ifdef KROSS_ACTIONCOLLECTION_DEBUG
        krossdebug( QString("ActionCollection::writeXml collection.objectName=\"%1\"").arg(objectName()) );
    #endif

    QDomDocument document;
    QDomElement element = document.createElement("collection");
    if( ! objectName().isNull() )
        element.setAttribute("name", objectName());

    foreach(QString name, d->collectionnames) {
        ActionCollection* c = d->collections[name];
        if( ! c ) continue;
        QDomElement e = c->writeXml();
        if( e.isNull() ) continue;
        element.appendChild(e);
    }

    foreach(KAction* action, actions()) {
        Action* a = dynamic_cast< Action* >(action);
        if( ! a ) continue;

        #ifdef KROSS_ACTIONCOLLECTION_DEBUG
            krossdebug( QString("  ActionCollection::writeXml action.objectName=\"%1\" action.file=\"%2\"").arg(a->objectName()).arg(a->file()) );
        #endif

        QDomElement e = document.createElement("script");
        e.setAttribute("name", a->objectName());
        e.setAttribute("text", a->text());
        e.setAttribute("comment", a->description());
        //FIXME hmmm... kde4's KIcon / Qt4's QIcon does not allow to reproduce the iconname?
        //e.setAttribute("icon", a->iconName());
        e.setAttribute("interpreter", a->interpreter());
        e.setAttribute("file", a->file());
        element.appendChild(e);
    }

    return element;
}

bool ActionCollection::writeXml(QIODevice* device, int indent)
{
    QDomDocument document;
    document.documentElement().appendChild( writeXml() );
    return device->write( document.toByteArray(indent) ) != -1;
}

#if 0
bool Manager::readConfig()
{
    KConfig* config = KApplication::kApplication()->sessionConfig();
    krossdebug( QString("Manager::readConfig hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );
    if(! config->hasGroup("scripts"))
        return false;

    // we need to remember the current names, to be able to remove "expired" actions later.
    QStringList actionnames;
    foreach(KAction* a, d->actioncollection->actions())
        actionnames.append( a->objectName() );

    // iterate now through the items in the [scripts]-section
    config->setGroup("scripts");
    foreach(QString name, config->readEntry("names", QStringList())) {
        bool needsupdate = actionnames.contains( name );
        if( needsupdate )
            actionnames.removeAll( name );

        QString text = config->readEntry(QString("%1_text").arg(name).toLatin1());
        QString description = config->readEntry(QString("%1_description").arg(name).toLatin1());
        QString icon = config->readEntry(QString("%1_icon").arg(name).toLatin1());
        QString file = config->readEntry(QString("%1_file").arg(name).toLatin1());
        QString interpreter = config->readEntry(QString("%1_interpreter").arg(name).toLatin1());

        if( text.isEmpty() )
            text = file;
        if( description.isEmpty() )
            description = text.isEmpty() ? name : text;
        if( icon.isEmpty() )
            icon = KMimeType::iconNameForUrl( KUrl(file) );

        Action* action = needsupdate
            ? dynamic_cast< Action* >( d->actioncollection->action(name) )
            : new Action(d->actioncollection, name);
        Q_ASSERT(action);

        action->setText(text);
        action->setDescription(description);
        if( ! icon.isNull() )
            action->setIcon(KIcon(icon));
        if( ! interpreter.isNull() )
            action->setInterpreter(interpreter);
        action->setFile(file);

        connect(action, SIGNAL( started(Kross::Action*) ), this, SIGNAL( started(Kross::Action*)) );
        connect(action, SIGNAL( finished(Kross::Action*) ), this, SIGNAL( finished(Kross::Action*) ));
    }

    // remove actions that are not valid anymore
    foreach(QString n, actionnames) {
        KAction* a = d->actioncollection->action(n);
        Q_ASSERT(a);
        d->actioncollection->remove(a);
        delete a;
    }

    return true;
}

bool Manager::writeConfig()
{
    KConfig* config = KApplication::kApplication()->sessionConfig();
    krossdebug( QString("Manager::writeConfig hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );
    if(config->isReadOnly())
        return false;

    config->deleteGroup("scripts"); // remove old entries
    config->setGroup("scripts"); // according to the documentation it's needed to re-set the group after delete.

    QStringList names;
    foreach(KAction* a, d->actioncollection->actions(QString::null)) {
        Action* action = static_cast< Action* >(a);
        const QString name = action->objectName();
        names << name;
        config->writeEntry(QString("%1_text").arg(name).toLatin1(), action->text());
        config->writeEntry(QString("%1_description").arg(name).toLatin1(), action->description());

        //TODO hmmm... kde4's KIcon / Qt4's QIcon does not allow to reproduce the iconname?
        //config->writeEntry(QString("%1_icon").arg(name).toLatin1(), action->icon());

        config->writeEntry(QString("%1_file").arg(name).toLatin1(), action->file());
        config->writeEntry(QString("%1_interpreter").arg(name).toLatin1(), action->interpreter());
    }

    config->writeEntry("names", names);
    //config->sync();
    return true;
}
#endif

#include "actioncollection.moc"

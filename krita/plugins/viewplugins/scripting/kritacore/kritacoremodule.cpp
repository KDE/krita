/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kritacoremodule.h"

#include <kdebug.h>

//#include <api/variant.h>
#include <api/qtobject.h>
#include <main/manager.h>

#include "kis_doc.h"
#include "kis_script.h"

#include "kis_scripts_registry.h"

#include "krs_doc.h"
#include "krs_script.h"

extern "C"
{
    /**
     * Exported an loadable function as entry point to use
     * the \a KexiAppModule.
     */
    Kross::Api::Object* init_module(Kross::Api::Manager* manager)
    {
        return new Kross::KritaCore::KritaCoreModule(manager);
    }
};


using namespace Kross::KritaCore;

KritaCoreModule::KritaCoreModule(Kross::Api::Manager* manager)
    : Kross::Api::Module("kritacore") , m_manager(manager)
{

    QMap<QString, Object::Ptr> children = manager->getChildren();
    kdDebug() << " there are " << children.size() << endl;
    for(QMap<QString, Object::Ptr>::const_iterator it = children.begin(); it != children.end(); it++)
    {
        kdDebug() << it.key() << " " << it.data() << endl;
    }
    
    // Wrap doc
    Kross::Api::Object::Ptr kritadocument = ((Kross::Api::Object*)manager)->getChild("KritaDocument");
    if(kritadocument) {
        Kross::Api::QtObject* kritadocumentqt = (Kross::Api::QtObject*)( kritadocument.data() );
        if(kritadocumentqt) {
            ::KisDoc* document = (::KisDoc*)( kritadocumentqt->getObject() );
            if(document) {
                addChild( new Doc(document) );
            } else {
                throw Kross::Api::Exception::Ptr( new Kross::Api::Exception("There was no 'KritaDocument' published.") );
            }
         }
   }
   // Wrap script
   KisScriptSP script = KisScriptsRegistry::instance()->getRunningScript();
   if(script != 0)
   {
       addChild(new Script(script));
   } else {
       throw Kross::Api::Exception::Ptr( new Kross::Api::Exception("There was no 'KritaScript' executed.") );
   }
   /*Kross::Api::Object::Ptr kritaview = ((Kross::Api::Object*)manager)->getChild("KritaView");
   if(kritaview) {
       Kross::Api::QtObject* kritaviewqt = (Kross::Api::QtObject*)( kritaview.data() );
       if(kritaviewqt) {
               ::KisView* view = (::KisView*)( kritaviewqt->getObject() );
               if(view) {
                   addChild( new View(view) );
               } else {
                   throw Kross::Api::Exception::Ptr( new Kross::Api::Exception("There was no 'KritaView' published.") );
               }
       }
}*/
}

KritaCoreModule::~KritaCoreModule()
{
}


const QString KritaCoreModule::getClassName() const
{
    return "Kross::KritaCore::KritaCoreModule";
}

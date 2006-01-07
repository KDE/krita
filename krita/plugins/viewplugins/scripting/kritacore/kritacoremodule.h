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

#ifndef KRITA_KROSS_KRITACOREMODULE_H
#define KRITA_KROSS_KRITACOREMODULE_H

#include <qstring.h>
#include <qvariant.h>

#define KROSS_MAIN_EXPORT KDE_EXPORT

#include <api/module.h>
#include <api/event.h>

namespace Kross { namespace Api {
    class Manager;
}}

namespace Kross { namespace KritaCore {
    /**
     * This class contains functions use to create new Kross object in a script
     */
    class KritaCoreFactory : public Kross::Api::Event<KritaCoreFactory>
    {
        public:
            KritaCoreFactory();
        private:
            Kross::Api::Object::Ptr newRGBColor(Kross::Api::List::Ptr);
            Kross::Api::Object::Ptr newHSVColor(Kross::Api::List::Ptr);
            Kross::Api::Object::Ptr getPattern(Kross::Api::List::Ptr);
            Kross::Api::Object::Ptr getBrush(Kross::Api::List::Ptr);
            Kross::Api::Object::Ptr newCircleBrush(Kross::Api::List::Ptr);
            Kross::Api::Object::Ptr newRectBrush(Kross::Api::List::Ptr);
    };
    /**
     *
     */
    class KritaCoreModule : public Kross::Api::Module
    {
        public:
            /**
             * Constructor.
             */
            KritaCoreModule(Kross::Api::Manager* manager);

            /**
             * Destructor.
             */
            virtual ~KritaCoreModule();

            /// \see Kross::Api::Object::getClassName
            virtual const QString getClassName() const;
            virtual Kross::Api::Object::Ptr call(const QString& name, Kross::Api::List::Ptr arguments);
        private:
            Kross::Api::Manager* m_manager;
            KritaCoreFactory* m_factory;
    };
    

}}

#endif


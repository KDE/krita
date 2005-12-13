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

#include "kis_script.h"

#include "krs_script.h"

namespace Kross {

namespace KritaCore {

Script::Script(KisScript* script): Kross::Api::Class<Script>("KritaScript"), m_script(script)
{
    addFunction("setProgressTotalSteps", &Script::setProgressTotalSteps, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("setProgressTotalSteps", &Script::setProgressTotalSteps, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("setProgress", &Script::setProgress, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("incProgress", &Script::incProgress );
    addFunction("setProgressStage", &Script::setProgressStage, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String")  << Kross::Api::Argument("Kross::Api::Variant::UInt") );
}


Script::~Script()
{
}

Kross::Api::Object::Ptr Script::setProgressTotalSteps(Kross::Api::List::Ptr args)
{
    m_script->setProgressTotalSteps( Kross::Api::Variant::toUInt(args->item(0)) );
    return 0;
}

Kross::Api::Object::Ptr Script::setProgress(Kross::Api::List::Ptr args)
{
    m_script->setProgress( Kross::Api::Variant::toUInt(args->item(0)) );
    return 0;
}

Kross::Api::Object::Ptr Script::incProgress(Kross::Api::List::Ptr)
{
    m_script->incProgress();
    return 0;
}

Kross::Api::Object::Ptr Script::setProgressStage(Kross::Api::List::Ptr args)
{
    m_script->setProgressStage( Kross::Api::Variant::toString(args->item(0)), Kross::Api::Variant::toUInt(args->item(1)) );
    return 0;
}

}

}

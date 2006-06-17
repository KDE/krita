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

#include "krs_script_progress.h"

#include "kis_script_progress.h"

namespace Kross {

namespace KritaCore {

ScriptProgress::ScriptProgress(KisScriptProgress* script): Kross::Api::Class<ScriptProgress>("KritaScript"), m_script(script)
{
    addFunction("setProgressTotalSteps", &ScriptProgress::setProgressTotalSteps);
    addFunction("setProgressTotalSteps", &ScriptProgress::setProgressTotalSteps);
    addFunction("setProgress", &ScriptProgress::setProgress);
    addFunction("incProgress", &ScriptProgress::incProgress);
    addFunction("setProgressStage", &ScriptProgress::setProgressStage);
}


ScriptProgress::~ScriptProgress()
{
}

Kross::Api::Object::Ptr ScriptProgress::setProgressTotalSteps(Kross::Api::List::Ptr args)
{
    m_script->setProgressTotalSteps( Kross::Api::Variant::toUInt(args->item(0)) );
    return Kross::Api::Object::Ptr(0);
}

Kross::Api::Object::Ptr ScriptProgress::setProgress(Kross::Api::List::Ptr args)
{
    m_script->setProgress( Kross::Api::Variant::toUInt(args->item(0)) );
    return Kross::Api::Object::Ptr(0);
}

Kross::Api::Object::Ptr ScriptProgress::incProgress(Kross::Api::List::Ptr)
{
    m_script->incProgress();
    return Kross::Api::Object::Ptr(0);
}

Kross::Api::Object::Ptr ScriptProgress::setProgressStage(Kross::Api::List::Ptr args)
{
    m_script->setProgressStage( Kross::Api::Variant::toString(args->item(0)), Kross::Api::Variant::toUInt(args->item(1)) );
    return Kross::Api::Object::Ptr(0);
}

}

}

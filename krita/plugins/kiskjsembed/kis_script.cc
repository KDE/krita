/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_script.h"

// KJSEmbed
#include <kjsembed/kjsembedpart.h>

namespace Krita {
namespace Plugins {
namespace KisKJSEmbed {

Script::Script(KJSEmbed::KJSEmbedPart* jsembedpart, const QString& script) :
m_jsEmbedPart(jsembedpart), m_script(script)
{

}


Script* Script::loadFromFile(KJSEmbed::KJSEmbedPart* jsembedpart, const QString& file)
{
    QString scripttxt = jsembedpart->loadFile(file);
    return new Script(jsembedpart, scripttxt);
}

void Script::execute()
{
    KJS::Completion result = m_jsEmbedPart->completion();
    m_jsEmbedPart->execute( result, m_script, KJS::Null());
}

}; }; };


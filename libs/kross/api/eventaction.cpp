/***************************************************************************
 * eventaction.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
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

#include "eventaction.h"
#include "variant.h"

//#include <QObject>
//#include <kaction.h>

using namespace Kross::Api;

EventAction::EventAction(const QString& name, Object* parent, KAction* action)
    : Event<EventAction>(name.isEmpty() ? action->objectName() : name, parent)
    , m_action(action)
{
    addFunction("getText", &EventAction::getText);
    addFunction("setText", &EventAction::setText,
        Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String"));

    addFunction("isEnabled", &EventAction::isEnabled);
    addFunction("setEnabled", &EventAction::setEnabled,
        Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::Bool"));

    addFunction("activate", &EventAction::activate);
}

EventAction::~EventAction()
{
}

const QString EventAction::getClassName() const
{
    return "Kross::Api::EventAction";
}

/*
Object::Ptr EventAction::call(const QString& name, KSharedPtr<List> arguments)
{
    krossdebug( QString("=============> EventAction::call() name=%1 arguments=%2").arg(name).arg(arguments->toString()) );
    //TODO
    return 0;
}
*/

Object::Ptr EventAction::getText(List::Ptr)
{
    return Object::Ptr( new Variant(m_action->text()) );
}

Object::Ptr EventAction::setText(List::Ptr args)
{
    m_action->setText( Variant::toString(args->item(0)) );
    return Object::Ptr();
}

Object::Ptr EventAction::isEnabled(List::Ptr)
{
    return Object::Ptr( new Variant(m_action->isEnabled()) );
}

Object::Ptr EventAction::setEnabled(List::Ptr args)
{
    m_action->setEnabled( Variant::toBool(args->item(0)) );
    return Object::Ptr();
}

Object::Ptr EventAction::activate(List::Ptr)
{
    m_action->activate( QAction::Trigger );
    return Object::Ptr();
}


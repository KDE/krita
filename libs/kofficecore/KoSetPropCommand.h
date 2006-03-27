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

#ifndef KOGENCOMMAND_H
#define KOGENCOMMAND_H

#include <kcommand.h>

/**
 * Generic command to set a property on an object.
 * This variant is for simple types, where the setter method takes a value.
 */
template<class Property, class Object, void (Object::* Function) (Property)> class KoSetBasicPropCommand : public KNamedCommand {

public:
    KoSetBasicPropCommand(Object *object, const QString &name) : KNamedCommand(name), m_object(object) {}
    KoSetBasicPropCommand(Object *object, const QString &name,
               const Property &oldProperty, const Property &newProperty) : KNamedCommand(name),
        m_object(object), m_oldProperty(oldProperty), m_newProperty(newProperty) {}
    virtual ~KoSetBasicPropCommand() {}

    virtual void execute() { if(m_object) (m_object->*Function)(m_newProperty); }
    virtual void unexecute() { if(m_object) (m_object->*Function)(m_oldProperty); }

    void setOldProperty(const Property &oldProperty) { m_oldProperty=oldProperty; }
    const Property &oldProperty() const { return m_oldProperty; }
    void setNewProperty(const Property &newProperty) { m_newProperty=newProperty; }
    const Property &newProperty() const { return m_newProperty; }

private:
    // Don't copy or assign this stuff
    KoSetBasicPropCommand(const KoSetBasicPropCommand<Property, Object, Function> &rhs);
    KoSetBasicPropCommand &operator=(const KoSetBasicPropCommand<Property, Object, Function> &rhs);

    Object *m_object;
    Property m_oldProperty, m_newProperty;
};

/**
 * Generic command to set a property on an object.
 * This variant is for non-trivial types, where the setter method takes a const reference.
 */
template<class Property, class Object, void (Object::* Function) (const Property &)> class KoSetPropCommand : public KNamedCommand {

public:
    KoSetPropCommand(Object *object, const QString &name) : KNamedCommand(name), m_object(object) {}
    KoSetPropCommand(Object *object, const QString &name,
               const Property &oldProperty, const Property &newProperty) : KNamedCommand(name),
        m_object(object), m_oldProperty(oldProperty), m_newProperty(newProperty) {}
    virtual ~KoSetPropCommand() {}

    virtual void execute() { if(m_object) (m_object->*Function)(m_newProperty); }
    virtual void unexecute() { if(m_object) (m_object->*Function)(m_oldProperty); }

    void setOldProperty(const Property &oldProperty) { m_oldProperty=oldProperty; }
    const Property &oldProperty() const { return m_oldProperty; }
    void setNewProperty(const Property &newProperty) { m_newProperty=newProperty; }
    const Property &newProperty() const { return m_newProperty; }

private:
    // Don't copy or assign this stuff
    KoSetPropCommand(const KoSetPropCommand<Property, Object, Function> &rhs);
    KoSetPropCommand &operator=(const KoSetPropCommand<Property, Object, Function> &rhs);

    Object *m_object;
    Property m_oldProperty, m_newProperty;
};

#endif /* KOGENCOMMAND_H */


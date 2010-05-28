/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef INSERTVARIABLEACTION_H
#define INSERTVARIABLEACTION_H

#include "InsertInlineObjectActionBase_p.h"

class KoCanvasBase;
class KoProperties;
class KoInlineObjectFactoryBase;
struct KoInlineObjectTemplate;

/// \internal
class InsertVariableAction : public InsertInlineObjectActionBase
{
public:
    InsertVariableAction(KoCanvasBase *base, KoInlineObjectFactoryBase *factory, const KoInlineObjectTemplate &templ);

private:
    virtual KoInlineObject *createInlineObject();

    KoInlineObjectFactoryBase *const m_factory;
    const QString m_templateId;
    const KoProperties *const m_properties;
    QString m_templateName;
};

#endif

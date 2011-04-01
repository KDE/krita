/*
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOSTYLE_H
#define KOSTYLE_H

#include "KoGenStyles.h"
#include "KoGenStyle.h"

#include <QString>
#include <QSharedPointer>

/**
 * A \class KoStyle is the base for all of the styles used in KoOdf.
 * Allows to easily share the styles amoung different compononents.
 *
 * As all the styles it can be shared
 **/

class KOODF_EXPORT KoStyle
{
public:
    KoStyle();
    virtual ~KoStyle();

    QString saveOdf(KoGenStyles& styles) const;

    void setName(QString name);
    QString name() const;

    void setAutoStyleInStylesDotXml(bool b);
    bool autoStyleInStylesDotXml() const;

protected:
    virtual void prepareStyle(KoGenStyle& style) const =0;
    virtual QString defaultPrefix() const =0;
    virtual KoGenStyle::Type styleType() const =0;
    virtual KoGenStyle::Type automaticstyleType() const =0;
    virtual const char* styleFamilyName() const =0;

private:
    KoGenStyles::InsertionFlags insertionFlags() const;

    bool m_autoStyle;
    QString m_name;
    bool m_autoStyleInStylesDotXml;
};

#define KOSTYLE_DECLARE_SHARED_POINTER(class) \
    typedef QSharedPointer<class> Ptr; \
    static Ptr create();

#define KOSTYLE_DECLARE_SHARED_POINTER_IMPL(class) \
    class::Ptr class::create() \
    { \
        return QSharedPointer<class>(new class); \
    }

#endif

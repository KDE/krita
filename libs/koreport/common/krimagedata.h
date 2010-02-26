/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KRIMAGEDATA_H
#define KRIMAGEDATA_H
#include "krobjectdata.h"
#include <QRect>
#include <QPainter>
#include <qdom.h>
#include "krpos.h"
#include "krsize.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>

namespace Scripting
{
class Image;
}

/**
 @author
*/
class KRImageData : public KRObjectData
{
public:
    KRImageData() {
        createProperties();
    }
    KRImageData(QDomNode & element);
    ~KRImageData() {};
    virtual KRImageData * toImage();
    virtual int type() const;

    void setMode(QString);

    void setInlineImageData(QByteArray, const QString& = QString());
    void setColumn(QString);
    QString mode();
    bool isInline();
    QString inlineImageData();
    QString controlSource();

protected:

    KoProperty::Property * m_controlSource;
    KoProperty::Property* m_resizeMode;
    KoProperty::Property* m_staticImage;

private:
    static int RTTI;
    virtual void createProperties();
    friend class KoReportPreRendererPrivate;
    friend class Scripting::Image;
};

#endif

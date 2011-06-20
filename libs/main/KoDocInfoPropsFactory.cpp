/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

   $Id$

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

#include <KoDocumentInfoPropsPage.h>

#include <kpluginfactory.h>

static QObject* createDocInfoPropsPage(QWidget* w, QObject* parent, const QVariantList& args)
{
    Q_UNUSED(w);
    KPropertiesDialog* props = qobject_cast<KPropertiesDialog *>(parent);
    Q_ASSERT(props);
    return new KoDocumentInfoPropsPage(props, args);
}

K_PLUGIN_FACTORY(PropsDlgFactory, registerPlugin<KoDocumentInfoPropsPage>(QString(), createDocInfoPropsPage);)
K_EXPORT_PLUGIN(PropsDlgFactory("calligra"))

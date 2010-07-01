/* This file is part of the KDE project
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

#include "TreeFactory.h"

#include "Tree.h"
//#include "TreeConfigWidget.h"

#include <KoXmlNS.h>
#include "KoShapeControllerBase.h"

#include <klocale.h>
#include <kdebug.h>

TreeFactory::TreeFactory(QObject *parent)
    : KoShapeFactoryBase(parent, TREEID, i18n("Tree"))
{
    setToolTip(i18n("Tree for mind maps"));
    setIcon("x-shape-image");
    setLoadingPriority(2);
}

TreeFactory::~TreeFactory() {}

KoShape *TreeFactory::createDefaultShape(KoResourceManager *documentResources) const
{
    Q_UNUSED(documentResources);
    Tree * defaultShape = new Tree();
    defaultShape->setShapeId(TREEID);

    return defaultShape;
}

bool TreeFactory::supports(const KoXmlElement &e) const
{
    Q_UNUSED(e);
    return false;
}
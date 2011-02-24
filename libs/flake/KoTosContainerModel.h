/* This file is part of the KDE project
   Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>

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
#ifndef KOTOSCONTAINERMODEL_H
#define KOTOSCONTAINERMODEL_H

#include "KoShapeContainerModel.h"

class KoTosContainerModel : public KoShapeContainerModel
{
public:
    KoTosContainerModel();
    virtual ~KoTosContainerModel();

    virtual void add(KoShape *shape);
    virtual void remove(KoShape *shape);
    virtual void setClipped(const KoShape *shape, bool clipping);
    virtual bool isClipped(const KoShape *shape) const;
    virtual void setInheritsTransform(const KoShape *shape, bool inherit);
    virtual bool inheritsTransform(const KoShape *shape) const;
    virtual bool isChildLocked(const KoShape *child) const;
    virtual int count() const;
    virtual QList<KoShape*> shapes() const;
    virtual void containerChanged(KoShapeContainer *container, KoShape::ChangeType type);

private:
    KoShape *m_textShape;
};

#endif /* KOTOSCONTAINERMODEL_H */

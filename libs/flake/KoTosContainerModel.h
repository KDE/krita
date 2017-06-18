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
    ~KoTosContainerModel() override;

    void add(KoShape *shape) override;
    void remove(KoShape *shape) override;
    void setClipped(const KoShape *shape, bool clipping) override;
    bool isClipped(const KoShape *shape) const override;
    void setInheritsTransform(const KoShape *shape, bool inherit) override;
    bool inheritsTransform(const KoShape *shape) const override;
    bool isChildLocked(const KoShape *child) const override;
    int count() const override;
    QList<KoShape*> shapes() const override;
    void containerChanged(KoShapeContainer *container, KoShape::ChangeType type) override;

private:
    KoShape *m_textShape;
};

#endif /* KOTOSCONTAINERMODEL_H */

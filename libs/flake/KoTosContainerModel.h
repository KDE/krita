/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
    int count() const override;
    QList<KoShape*> shapes() const override;
    void containerChanged(KoShapeContainer *container, KoShape::ChangeType type) override;

private:
    KoShape *m_textShape;
};

#endif /* KOTOSCONTAINERMODEL_H */

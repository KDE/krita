#ifndef KOTOSCONTAINER_P_H
#define KOTOSCONTAINER_P_H

#include "KoShapeContainer_p.h"

#include "KoTosContainer.h"

class KoTosContainerPrivate : public KoShapeContainerPrivate
{
public:

    explicit KoTosContainerPrivate(KoShapeContainer *q);

    ~KoTosContainerPrivate() override;

    KoTosContainer::ResizeBehavior resizeBehavior;
    QRectF preferredTextRect;
    Qt::Alignment alignment;
};

#endif // KOTOSCONTAINER_P_H

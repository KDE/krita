#ifndef KOTOSCONTAINER_P_H
#define KOTOSCONTAINER_P_H

#include "kritaflake_export.h"
#include "KoShapeContainer_p.h"

#include "KoTosContainer.h"

class KRITAFLAKE_EXPORT KoTosContainerPrivate : public KoShapeContainerPrivate
{
public:

    explicit KoTosContainerPrivate(KoShapeContainer *q);
    explicit KoTosContainerPrivate(const KoTosContainerPrivate &rhs, KoShapeContainer *q);

    ~KoTosContainerPrivate() override;

    KoTosContainer::ResizeBehavior resizeBehavior;
    QRectF preferredTextRect;
    Qt::Alignment alignment;
};

#endif // KOTOSCONTAINER_P_H

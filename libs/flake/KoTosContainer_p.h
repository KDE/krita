#ifndef KOTOSCONTAINER_P_H
#define KOTOSCONTAINER_P_H

#include "kritaflake_export.h"
#include "KoShapeContainer_p.h"

#include "KoTosContainer.h"
#include <QSharedData>
#include <QRect>

class KoTosContainer::Private : public QSharedData
{
public:

    explicit Private();
    explicit Private(const Private &rhs);

    virtual ~Private();

    KoTosContainer::ResizeBehavior resizeBehavior;
    QRectF preferredTextRect;
    Qt::Alignment alignment;
};

#endif // KOTOSCONTAINER_P_H

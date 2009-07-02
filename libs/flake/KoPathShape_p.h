#ifndef KOPATHSHAPEPRIVATE_H
#define KOPATHSHAPEPRIVATE_H

#include "KoShape_p.h"

class KoPathShapePrivate : public KoShapePrivate
{
public:
    KoPathShapePrivate(KoPathShape *q);

    Qt::FillRule fillRule;
};

#endif

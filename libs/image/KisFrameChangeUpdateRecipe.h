#ifndef KISFRAMECHANGEUPDATERECIPE_H
#define KISFRAMECHANGEUPDATERECIPE_H

#include <QRect>
#include <kis_time_span.h>

struct KisFrameChangeUpdateRecipe
{
    KisTimeSpan affectedRange;
    QRect affectedRect;
    QRect totalDirtyRect;

    void notify(KisNode *node) const;
};

#endif // KISFRAMECHANGEUPDATERECIPE_H

#include "KisFrameChangeUpdateRecipe.h"


#include <kis_node.h>

void KisFrameChangeUpdateRecipe::notify(KisNode *node) const
{
    node->invalidateFrames(affectedRange, affectedRect);
    if (totalDirtyRect.isValid()) {
        node->setDirty(totalDirtyRect);
    }
}

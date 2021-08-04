#include "StoryboardUtils.h"

QModelIndex siblingAtRow(const QModelIndex &index, int row)
{
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
    return index.siblingAtRow(row);
#else
    return index.sibling(row, index.column());
#endif
}

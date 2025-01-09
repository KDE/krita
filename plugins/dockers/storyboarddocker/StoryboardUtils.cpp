#include "StoryboardUtils.h"

QModelIndex siblingAtRow(const QModelIndex &index, int row)
{
    return index.siblingAtRow(row);
}

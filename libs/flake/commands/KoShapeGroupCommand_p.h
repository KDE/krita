#ifndef KoShapeGroupCommandPrivate_H
#define KoShapeGroupCommandPrivate_H

#include <QMatrix>
#include <QPair>

class KoShapeGroupCommandPrivate
{
public:
    KoShapeGroupCommandPrivate(KoShapeContainer *container, const QList<KoShape *> &shapes, const QList<bool> &clipped = QList<bool>());
    void init(QUndoCommand *q);
    QRectF containerBoundingRect();

    QList<KoShape*> shapes; ///<list of shapes to be grouped
    QList<bool> clipped; ///< list of booleas to specify the shape of the same index to eb clipped
    KoShapeContainer *container; ///< the container where the grouping should be for.
    QList<KoShapeContainer*> oldParents; ///< the old parents of the shapes
    QList<bool> oldClipped; ///< if the shape was clipped in the old parent
    QList<int> oldZIndex; ///< the old z-index of the shapes

    QList<QPair<KoShape*, int> > oldAncestorsZIndex; // only used by the ungroup command
};

#endif

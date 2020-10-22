#include "KisBezierMesh.h"

#include <QDebug>

QDebug KisBezierMeshDetails::operator<<(QDebug dbg, const BaseMeshNode &n) {
    dbg.nospace() << "Node " << n.node << " "
                  << "(lC: " << n.leftControl << " "
                  << "tC: " << n.topControl << " "
                  << "rC: " << n.rightControl << " "
                  << "bC: " << n.bottomControl << ") ";
    return dbg.nospace();
}



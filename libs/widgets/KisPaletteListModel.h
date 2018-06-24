#ifndef KISPALETTELISTMODEL_H
#define KISPALETTELISTMODEL_H

#include <QAbstractListModel>
#include <KoResourceServer.h>
#include <KoColorSet.h>

class KisPaletteListModel
{
public:
    KisPaletteListModel();
    virtual ~KisPaletteListModel();
    int rowCount () { return rserver->resourceCount(); }

private:
    KoResourceServer<KoColorSet> *rserver;
};

#endif // KISPALETTELISTMODEL_H

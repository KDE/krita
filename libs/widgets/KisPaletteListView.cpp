#include <QPointer>

#include "KisPaletteListView.h"
#include "KisPaletteListModel.h"
#include "KisPaletteListDelegate.h"

struct KisPaletteListView::Private
{
    QPointer<KisPaletteListModel> model;
    QPointer<KisPaletteListDelegate> delegate;
};

KisPaletteListView::~KisPaletteListView()
{
    // delete model;
    // delete delegate;
}

KisPaletteListView::KisPaletteListView(QWidget *parent)
//    : QAbstractItemView(parent)
//    , m_d(new Private)
{
    // m_d->model = new KisPaletteListModel;
    // setModel(m_d->model);
}

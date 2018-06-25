#include <QPointer>

#include "KisPaletteListView.h"
#include "KisPaletteListModel.h"
#include "KisPaletteListDelegate.h"

class KisPaletteListView::Private
{
public:
    Private(KisPaletteListView *view)
        : model(new KisPaletteListModel(view))
        , delegate(new KisPaletteListDelegate(view))
        , m_view(view)
    { }
    ~Private()
    { }
public:
    QScopedPointer<KisPaletteListModel> model;
    QScopedPointer<KisPaletteListDelegate> delegate;
private:
    QPointer<KisPaletteListView> m_view;
};

KisPaletteListView::~KisPaletteListView()
{
    delete m_d;
}

KisPaletteListView::KisPaletteListView(QWidget *parent)
    : QAbstractItemView(parent)
    , m_d(new Private)
{
     setModel(m_d->model);
}

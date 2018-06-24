#ifndef KISPALETTELISTVIEW_H
#define KISPALETTELISTVIEW_H

#include <QListView>

class KisPaletteListView : public QListView
{
    Q_OBJECT
public:
    KisPaletteListView(QWidget *parent = Q_NULLPTR);
    virtual ~KisPaletteListView();

private:
    class Private;
    Private *m_d;
};

#endif // KISPALETTELISTVIEW_H

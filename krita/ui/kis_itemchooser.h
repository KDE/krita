/*
 *  Copyright (c) 2002 Patrick Julein <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ITEM_CHOOSER_H_
#define KIS_ITEM_CHOOSER_H_

#include <QList>
#include <QWidget>
#include <kvbox.h>

class KHBox;

class KoIconChooser;
class KoIconItem;

typedef QList<KoIconItem *> vKoIconItem;

class KisItemChooser : public QWidget {
    typedef QWidget super;
    Q_OBJECT

public:
    KisItemChooser(QWidget *parent = 0,
               const char *name = 0);
    virtual ~KisItemChooser();

    KoIconItem *currentItem();
    void setCurrent(KoIconItem *item);
    void setCurrent(int index);

public slots:
    void addItem(KoIconItem *item);
    void addItems(const vKoIconItem& items);

signals:
    void selected(KoIconItem *item);

protected:
    virtual void update(KoIconItem *item) = 0;
    QWidget *chooserWidget() const;

private slots:
    void slotItemSelected(KoIconItem *item);

private:
    KHBox *m_frame;
    KoIconChooser *m_chooser;
};

#endif // KIS_ITEM_CHOOSER_H_


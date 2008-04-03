/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KO_RESOURCE_ITEM_CHOOSER
#define KO_RESOURCE_ITEM_CHOOSER

#include <QWidget>
#include <QTableWidgetItem>
#include "koguiutils_export.h"

class QButtonGroup;
class KoResourceChooser;
class KoResource;

class KOGUIUTILS_EXPORT KoResourceItem : public QTableWidgetItem {

public:
    KoResourceItem(KoResource *resource);
    virtual ~KoResourceItem();

    KoResource *resource() const;

    virtual int compare(const QTableWidgetItem *other) const;

protected:
    QImage thumbnail( const QSize &thumbSize ) const;

private:
    KoResource *m_resource;
};

class KOGUIUTILS_EXPORT KoResourceItemChooser : public QWidget
{
  Q_OBJECT
public:
    KoResourceItemChooser( QWidget *parent = 0 );
    ~KoResourceItemChooser();

    QTableWidgetItem *currentItem();
    void setCurrent(QTableWidgetItem *item);
    void setCurrent(int index);

    void clear();
    void setIconSize(const QSize& size);
    QSize iconSize() const;
    void showButtons( bool show );
signals:
    void selected(QTableWidgetItem *item);
    void itemDoubleClicked( QTableWidgetItem* item);
    void importClicked();
    void deleteClicked();

public slots:
    void addItem(KoResourceItem *item);
    void addItems(const QList<KoResourceItem *>& items);
    void removeItem(KoResourceItem *item);

private slots:
    void slotButtonClicked( int button );
    void selectionChanged();

protected:
    QSize viewportSize();
private:
    enum Buttons { Button_Import, Button_Remove };

    KoResourceChooser* m_chooser;
    QButtonGroup* m_buttonGroup;
};

#endif // KO_RESOURCE_ITEM_CHOOSER

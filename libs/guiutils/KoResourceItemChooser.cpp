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
#include "KoResourceItemChooser.h"

#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QFileInfo>

#include <kiconloader.h>
#include <klocale.h>

#include "KoResourceChooser.h"
#include "KoResource.h"

#define THUMB_SIZE 30

KoResourceItem::KoResourceItem(KoResource *resource)
{
    m_resource = resource;
//     updatePixmaps();
    setIcon( QIcon( QPixmap::fromImage( thumbnail( QSize( THUMB_SIZE, THUMB_SIZE ) ) ) ) );
}

KoResourceItem::~KoResourceItem()
{
}

QImage KoResourceItem::thumbnail( const QSize &thumbSize ) const
{
    QSize imgSize = m_resource->img().size();

    double scaleW = static_cast<double>( thumbSize.width() ) / static_cast<double>( imgSize.width() );
    double scaleH = static_cast<double>( thumbSize.height() ) / static_cast<double>( imgSize.height() );

    double scale = qMin( scaleW, scaleH );

    int thumbW = static_cast<int>( imgSize.width() * scale );
    int thumbH = static_cast<int>( imgSize.height() * scale );

    return m_resource->img().scaled( thumbW, thumbH, Qt::IgnoreAspectRatio );
}

KoResource *KoResourceItem::resource() const
{
    return m_resource;
}

int KoResourceItem::compare(const QTableWidgetItem *o) const
{
    const KoResourceItem *other = dynamic_cast<const KoResourceItem *>(o);

    if (other != 0) {
        return m_resource->name().localeAwareCompare(other->m_resource->name());
    } else {
        return 0;
    }
}

KoResourceItemChooser::KoResourceItemChooser( QWidget *parent )
 : QWidget( parent )
{
    // only serves as beautifier for the iconchooser
    //frame = new QHBox( this );
    //frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    m_chooser = new KoResourceChooser( QSize(32,32), this);
    m_chooser->setIconSize( QSize(30,30) );
    connect( m_chooser, SIGNAL(itemClicked( QTableWidgetItem * ) ), this, SLOT( itemSelected( QTableWidgetItem * )));
    connect( m_chooser, SIGNAL(itemDoubleClicked( QTableWidgetItem * ) ), this, SIGNAL( itemDoubleClicked( QTableWidgetItem* ) ) );

    m_buttonGroup = new QButtonGroup( this );
    m_buttonGroup->setExclusive( false );

    QGridLayout* layout = new QGridLayout( this );
    layout->addWidget( m_chooser, 0, 0, 1, 3 );

    QPushButton *button = new QPushButton( this );
    button->setIcon( SmallIcon( "list-add" ) );
    button->setToolTip( i18n("Import") );
    button->setEnabled( true );
    m_buttonGroup->addButton( button, Button_Import );
    layout->addWidget( button, 1, 0 );

    button = new QPushButton( this );
    button->setIcon( SmallIcon( "list-remove" ) );
    button->setToolTip( i18n("Delete") );
    button->setEnabled( false );
    m_buttonGroup->addButton( button, Button_Remove );
    layout->addWidget( button, 1, 1 );

    connect( m_buttonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( slotButtonClicked( int ) ));

    layout->setColumnStretch( 0, 1 );
    layout->setColumnStretch( 1, 1 );
    layout->setColumnStretch( 2, 2 );
    layout->setSpacing( 0 );
    layout->setMargin( 3 );
}

KoResourceItemChooser::~KoResourceItemChooser()
{
  delete m_chooser;
  //delete frame;
}

void KoResourceItemChooser::setCurrent(QTableWidgetItem *item)
{
    m_chooser->setCurrentItem(item);
}

void KoResourceItemChooser::setCurrent(int index)
{
    setCurrent(m_chooser->itemAt(index));
}

void KoResourceItemChooser::clear()
{
    m_chooser->clear();
}

void KoResourceItemChooser::setIconSize(const QSize& size)
{
    m_chooser->setIconSize(size);
}

QTableWidgetItem* KoResourceItemChooser::currentItem()
{
    return m_chooser->currentItem();
}

void KoResourceItemChooser::itemSelected(QTableWidgetItem *item)
{
    QAbstractButton * removeButton = m_buttonGroup->button( Button_Remove );
    if( removeButton )
        removeButton->setEnabled( static_cast<KoResourceItem*>(item)->resource()->removable() );

    emit selected(currentItem());
}

void KoResourceItemChooser::addItem(KoResourceItem *item)
{
    m_chooser->addItem(item);
}

void KoResourceItemChooser::addItems(const QList<KoResourceItem *>& items)
{
    foreach (QTableWidgetItem *item, items)
        m_chooser->addItem(item);
}

void KoResourceItemChooser::removeItem(KoResourceItem *item)
{
    m_chooser->removeItem(item);
}

void KoResourceItemChooser::slotButtonClicked( int button )
{
    if( button == Button_Import )
        emit importClicked();
    else if( button == Button_Remove )
        emit deleteClicked();
}

#include "KoResourceItemChooser.moc"

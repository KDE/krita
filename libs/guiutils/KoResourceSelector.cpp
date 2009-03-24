/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoResourceSelector.h"
#include <KoResourceServerAdapter.h>
#include <KoAbstractGradient.h>
#include <KoCheckerBoardPainter.h>
#include <KoResourceChooser.h>
#include <QtGui/QPainter>
#include <QtGui/QTableView>
#include <QtGui/QHeaderView>

#include <KDebug>

/// The resource model managing the resource data
class KoResourceModel : public QAbstractTableModel
{
public:
    KoResourceModel( KoAbstractResourceServerAdapter * resourceAdapter, QObject * parent = 0 );
    virtual ~KoResourceModel() {}
    
    /// reimplemented
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    /// reimplemented
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    /// reimplemented
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    /// reimplemented
    virtual QModelIndex index ( int row, int column = 0, const QModelIndex & parent = QModelIndex() ) const;
    /// Sets the number of columns to display
    void setColumnCount( int columnCount );
private:
    KoAbstractResourceServerAdapter * m_resourceAdapter;
    int m_columnCount;
};

/// The resource item delegate for rendering the resource preview
class KoResourceItemDelegate : public QAbstractItemDelegate
{
public:
    KoResourceItemDelegate( QObject * parent = 0 );
    virtual ~KoResourceItemDelegate() {}
    /// reimplemented
    virtual void paint( QPainter *, const QStyleOptionViewItem &, const QModelIndex & ) const;
    /// reimplemented
    QSize sizeHint ( const QStyleOptionViewItem &, const QModelIndex & ) const;
private:
    KoCheckerBoardPainter m_checkerPainter;
};

/// The resource view
class KoResourceItemView : public QTableView
{
public:
    KoResourceItemView( QWidget * parent = 0 );
    virtual ~KoResourceItemView() {}
    /// reimplemented
    virtual void resizeEvent ( QResizeEvent * event );
    /// reimplemented
    virtual bool viewportEvent( QEvent * event );
private:
    KoIconToolTip m_tip;
};

class KoResourceSelector::Private
{
public:
    Private() : model(0), view(0) {}
    KoResourceModel * model;
    KoResourceItemView * view;
};

KoResourceSelector::KoResourceSelector( KoAbstractResourceServerAdapter * resourceAdapter, QWidget * parent )
    : QComboBox( parent ), d( new Private() )
{
    d->model = new KoResourceModel(resourceAdapter, this); 
    d->view = new KoResourceItemView(this); 
    connect( this, SIGNAL(currentIndexChanged(int)),
             this, SLOT(indexChanged(int)) );
    Q_ASSERT(resourceAdapter);
    setView( d->view );
    setModel( d->model );
    setItemDelegate( new KoResourceItemDelegate( this ) );
    setMouseTracking(true);

    d->view->setCurrentIndex( d->model->index( 0, 0 ) );
}

KoResourceSelector::~KoResourceSelector()
{
    delete d;
}

void KoResourceSelector::paintEvent( QPaintEvent *pe )
{
    QComboBox::paintEvent( pe );

    QStyleOptionComboBox option;
    option.initFrom( this );
    QRect r = style()->subControlRect( QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this );

    QStyleOptionViewItem viewOption;
    viewOption.initFrom( this );
    viewOption.rect = r;
    
    QPainter painter( this );
    itemDelegate()->paint( &painter, viewOption, view()->currentIndex() );
}

void KoResourceSelector::mousePressEvent( QMouseEvent * event )
{
    QStyleOptionComboBox opt;
    opt.init( this );
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_ComboBoxArrow;
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt,
                                                           mapFromGlobal(event->globalPos()),
                                                           this);
    // only clicking on combobox arrow shows popup,
    // otherwise the resourceApplied signal is send with the current resource
    if (sc == QStyle::SC_ComboBoxArrow)
        QComboBox::mousePressEvent( event );
    else {
        QModelIndex index = view()->currentIndex();
        if( ! index.isValid() )
            return;

        KoResource * resource = static_cast<KoResource*>( index.internalPointer() );
        if( resource )
            emit resourceApplied( resource );
    }
}

void KoResourceSelector::mouseMoveEvent( QMouseEvent * event )
{
    QStyleOptionComboBox option;
    option.initFrom( this );
    QRect r = style()->subControlRect( QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this );
    if (r.contains(event->pos()))
        setCursor(Qt::PointingHandCursor);
    else
        unsetCursor();
}

void KoResourceSelector::setColumnCount( int columnCount )
{
    d->model->setColumnCount( columnCount );
}

void KoResourceSelector::setRowHeight( int rowHeight )
{
    d->view->verticalHeader()->setDefaultSectionSize( rowHeight );
}

void KoResourceSelector::indexChanged( int )
{
    QModelIndex index = view()->currentIndex();
    if( ! index.isValid() )
        return;

    KoResource * resource = static_cast<KoResource*>( index.internalPointer() );
    if( resource )
        emit resourceSelected( resource );
}

//################## The Model #############################################

KoResourceModel::KoResourceModel( KoAbstractResourceServerAdapter * resourceAdapter, QObject * parent )
    : QAbstractTableModel( parent ), m_resourceAdapter(resourceAdapter), m_columnCount(4)
{
    Q_ASSERT( m_resourceAdapter );
    m_resourceAdapter->connectToResourceServer();
}

int KoResourceModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    return m_resourceAdapter->resources().count() / m_columnCount + 1;
}

int KoResourceModel::columnCount ( const QModelIndex & ) const
{
    return m_columnCount;
}

QVariant KoResourceModel::data( const QModelIndex &index, int role ) const
{
    if( ! index.isValid() )
         return QVariant();

    switch( role )
    {
        case Qt::DecorationRole:
        {
            KoResource * resource = static_cast<KoResource*>(index.internalPointer());
            if( ! resource )
                return QVariant();

            return QVariant( resource->img() );
        }
        case KoResourceChooser::LargeThumbnailRole:
        {
            KoResource * resource = static_cast<KoResource*>(index.internalPointer());
            if( ! resource )
                return QVariant();
            
            QSize imgSize = resource->img().size();
            QSize thumbSize( 100, 100 );
            if(imgSize.height() > thumbSize.height() || imgSize.width() > thumbSize.width()) {
                qreal scaleW = static_cast<qreal>( thumbSize.width() ) / static_cast<qreal>( imgSize.width() );
                qreal scaleH = static_cast<qreal>( thumbSize.height() ) / static_cast<qreal>( imgSize.height() );

                qreal scale = qMin( scaleW, scaleH );

                int thumbW = static_cast<int>( imgSize.width() * scale );
                int thumbH = static_cast<int>( imgSize.height() * scale );

                return QVariant(resource->img().scaled( thumbW, thumbH, Qt::IgnoreAspectRatio ));
            }
            else
                return QVariant(resource->img());
        }

        default:
            return QVariant();
    }
}

QModelIndex KoResourceModel::index ( int row, int column, const QModelIndex & ) const
{
    int index = row * m_columnCount + column;
    if( index >= m_resourceAdapter->resources().count() || index < 0)
        return QModelIndex();

    KoResource * resource = m_resourceAdapter->resources()[index];
    return createIndex( row, column, resource );
}

void KoResourceModel::setColumnCount( int columnCount )
{
    m_columnCount = columnCount;
}

//################## The Delegate ##########################################

KoResourceItemDelegate::KoResourceItemDelegate( QObject * parent )
    : QAbstractItemDelegate( parent ), m_checkerPainter( 4 )
{
}

void KoResourceItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if( ! index.isValid() )
        return;

    KoResource * resource = static_cast<KoResource*>( index.internalPointer() );
    if (!resource)
        return;

    painter->save();

    if (option.state & QStyle::State_Selected)
        painter->fillRect( option.rect, option.palette.highlight() );

    QRect innerRect = option.rect.adjusted( 2, 1, -2, -1 );

    KoAbstractGradient * gradient = dynamic_cast<KoAbstractGradient*>( resource );
    if (gradient) {
        QGradient * g = gradient->toQGradient();

        QLinearGradient paintGradient;
        paintGradient.setStops( g->stops() );
        paintGradient.setStart( innerRect.topLeft() );
        paintGradient.setFinalStop( innerRect.topRight() );

        m_checkerPainter.paint( *painter, innerRect );
        painter->fillRect( innerRect, QBrush( paintGradient ) );

        delete g;
    }
    else {
        QImage thumbnail = index.data( Qt::DecorationRole ).value<QImage>();

        QSize imgSize = thumbnail.size();
        
        if(imgSize.height() > innerRect.height() || imgSize.width() > innerRect.width()) {
            qreal scaleW = static_cast<qreal>( innerRect.width() ) / static_cast<qreal>( imgSize.width() );
            qreal scaleH = static_cast<qreal>( innerRect.height() ) / static_cast<qreal>( imgSize.height() );

            qreal scale = qMin( scaleW, scaleH );

            int thumbW = static_cast<int>( imgSize.width() * scale );
            int thumbH = static_cast<int>( imgSize.height() * scale );
            thumbnail = thumbnail.scaled( thumbW, thumbH, Qt::IgnoreAspectRatio );
        }
        painter->fillRect( innerRect, QBrush(thumbnail) );
    }
    painter->restore();
}

QSize KoResourceItemDelegate::sizeHint( const QStyleOptionViewItem & optionItem, const QModelIndex & ) const
{
    return optionItem.decorationSize;
}

//################## The View ##########################################

KoResourceItemView::KoResourceItemView( QWidget * parent )
    : QTableView(parent)
{
    verticalHeader()->hide();
    horizontalHeader()->hide();
    verticalHeader()->setDefaultSectionSize( 20 );
}

void KoResourceItemView::resizeEvent( QResizeEvent * event )
{
    QTableView::resizeEvent(event);

    int columnCount = model()->columnCount( QModelIndex() );
    int columnWidth = viewport()->size().width() / columnCount;
    for( int i = 0; i < columnCount; ++i ) {
        setColumnWidth( i, columnWidth );
    }
}

bool KoResourceItemView::viewportEvent( QEvent * event )
{
    if( event->type() == QEvent::ToolTip && model() )
    {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        QStyleOptionViewItem option = viewOptions();
        QModelIndex index = model()->buddy( indexAt(he->pos()));
        if( index.isValid() )
        {
            option.rect = visualRect( index );
            m_tip.showTip( this, he->pos(), option, index );
            return true;
        }
    }

    return QTableView::viewportEvent( event );
}

#include "KoResourceSelector.moc"

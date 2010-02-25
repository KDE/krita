/* This file is part of the KDE project
   Copyright (C) 2008-2009 Jarosław Staniek <staniek@kde.org>

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

#include "EditorView.h"
#include "EditorDataModel.h"
#include "Property.h"
#include "Set.h"
#include "Factory.h"

#include <QtCore/QPointer>
#include <QtGui/QItemDelegate>
#include <QtGui/QStandardItemEditorCreator>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QToolTip>
#include <QtGui/QApplication>

#include <KLocale>
#include <KIconLoader>
#include <KIconEffect>
#include <KDebug>

using namespace KoProperty;

#if 0 // not sure if we should use it, better to fix Oxygen?
#include <kexiutils/styleproxy.h>

//! Used to alter the widget's style at design time
class EditorViewStyle : public KexiUtils::StyleProxy
{
public:
    EditorViewStyle(QStyle* parentStyle) : KexiUtils::StyleProxy(parentStyle)
    {
    }

    virtual void drawPrimitive(PrimitiveElement elem, const QStyleOption* option, 
        QPainter* painter, const QWidget* widget) const
    {
/*        if (elem == PE_PanelLineEdit) {
            const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame*>(option);
            if (panel) {
                QStyleOptionFrame alteredOption(*panel);
                alteredOption.lineWidth = 0;
                KexiUtils::StyleProxy::drawPrimitive(elem, &alteredOption, 
                    painter, widget);
                return;
            }
        }*/
        KexiUtils::StyleProxy::drawPrimitive(elem, option, 
            painter, widget);
    }
};
#endif

static bool computeAutoSync(Property *property, bool defaultAutoSync)
{
    return (property->autoSync() != 0 && property->autoSync() != 1) ?
                defaultAutoSync : (property->autoSync() != 0);
}

//----------

class ItemDelegate : public QItemDelegate
{
public:
    ItemDelegate(QWidget *parent);
    virtual ~ItemDelegate();
    virtual void paint(QPainter *painter, 
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
    virtual QWidget * createEditor(QWidget *parent, 
        const QStyleOptionViewItem & option, const QModelIndex & index ) const;
//    virtual void updateEditorGeometry( QWidget * editor, 
//        const QStyleOptionViewItem & option, const QModelIndex & index ) const;
//    virtual bool editorEvent( QEvent * event, QAbstractItemModel * model,
//        const QStyleOptionViewItem & option, const QModelIndex & index );
    mutable QPointer<QWidget> m_currentEditor;
};

ItemDelegate::ItemDelegate(QWidget *parent)
: QItemDelegate(parent)
{
//moved    setItemEditorFactory( new ItemEditorFactory() );
}

ItemDelegate::~ItemDelegate()
{
}

static int getIconSize(int rowHeight)
{
    return rowHeight * 2 / 3;
}

static int typeForProperty( Property* prop )
{
    if (prop->listData())
        return KoProperty::ValueFromList;
    else
        return prop->type();
}

void ItemDelegate::paint(QPainter *painter, 
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    QStyleOptionViewItem alteredOption(option);
    alteredOption.rect.setTop(alteredOption.rect.top() + 1);
    painter->save();
    QRect r(option.rect);
    const EditorDataModel *editorModel = dynamic_cast<const EditorDataModel*>(index.model());
    bool modified = false;
    if (index.column()==0) {
        r.setWidth(r.width() - 1);
        r.setLeft(0);

        QVariant modifiedVariant( editorModel->data(index, EditorDataModel::PropertyModifiedRole) );
        if (modifiedVariant.isValid() && modifiedVariant.toBool()) {
            modified = true;
            QFont font(alteredOption.font);
            font.setBold(true);
            alteredOption.font = font;
        }
    }
    else {
        r.setLeft(r.left()-1);
    }
    const int x2 = alteredOption.rect.right();
    const int y2 = alteredOption.rect.bottom();
    const int iconSize = getIconSize( alteredOption.rect.height() );
    if (modified) {
        alteredOption.rect.setRight( alteredOption.rect.right() - iconSize * 1 );
    }

    Property *property = editorModel->propertyForItem(index);
    const int t = typeForProperty( property ); //index.data(Qt::EditRole).userType();
    bool useQItemDelegatePaint = true; // ValueDisplayInterface is used by default
    if (index.column() == 1 && FactoryManager::self()->paint(t, painter, alteredOption, index)) {
        useQItemDelegatePaint = false;
    }
    if (useQItemDelegatePaint) {
        QItemDelegate::paint(painter, alteredOption, index);
    }

    if (modified) {
        alteredOption.rect.setRight( alteredOption.rect.right() - iconSize * 3 / 2 );
//        int x1 = alteredOption.rect.right();
        int y1 = alteredOption.rect.top();
        QLinearGradient grad(x2 - iconSize * 2, y1, x2 - iconSize / 2, y1);
        QColor color(
            alteredOption.palette.color( 
                (alteredOption.state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Base ));
        color.setAlpha(0);
        grad.setColorAt(0.0, color);
        color.setAlpha(255);
        grad.setColorAt(0.5, color);
//        grad.setColorAt(1.0, color);
        QBrush gradBrush(grad);
        painter->fillRect(x2 - iconSize * 2, y1, 
            iconSize * 2, y2 - y1 + 1, gradBrush);
        QPixmap revertIcon( DesktopIcon("edit-undo", iconSize) );
//        QPixmap alphaChannel(revertIcon.size());
//        alphaChannel.fill(QColor(127, 127, 127));
//        revertIcon.setAlphaChannel(alphaChannel);
        revertIcon = KIconEffect().apply(revertIcon, KIconEffect::Colorize, 1.0, 
            alteredOption.palette.color(
                (alteredOption.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text ), false);
        painter->drawPixmap( x2 - iconSize - 2, 
            y1 + 1 + (alteredOption.rect.height() - revertIcon.height()) / 2, revertIcon);
    }

    QColor gridLineColor( dynamic_cast<EditorView*>(painter->device()) ? 
        dynamic_cast<EditorView*>(painter->device())->gridLineColor()
        : EditorView::defaultGridLineColor() );
    QPen pen(gridLineColor);
    pen.setWidth(1);
    painter->setPen(pen);
    painter->drawRect(r);
    //kDebug()<<"rect:" << r << "viewport:" << painter->viewport() << "window:"<<painter->window();
    painter->restore();
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    return QItemDelegate::sizeHint(option, index) + QSize(0, 2);
}

QWidget * ItemDelegate::createEditor(QWidget * parent, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if (!index.isValid())
        return 0;
    QStyleOptionViewItem alteredOption(option);
//    QWidget *w = QStyledItemDelegate::createEditor(parent, alteredOption, index);
//???    int t = index.data(Qt::EditRole).userType();
    const EditorDataModel *editorModel = dynamic_cast<const EditorDataModel*>(index.model());
    Property *property = editorModel->propertyForItem(index);
    int t = typeForProperty(property);
    alteredOption.rect.setHeight(alteredOption.rect.height()+3);
    QWidget *w = FactoryManager::self()->createEditor(t, parent, alteredOption, index);
    if (w) {
        if (-1 != w->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("commitData(QWidget*)"))
            && property && !property->children())
        {
//            QObject::connect(w, SIGNAL(commitData(QWidget*)),
//                this, SIGNAL(commitData(QWidget*)));
        }
    }
    else {
        w = QItemDelegate::createEditor(parent, alteredOption, index);
    }
    QObject::disconnect(w, SIGNAL(commitData(QWidget*)),
        this, SIGNAL(commitData(QWidget*)));
    if (computeAutoSync( property, static_cast<EditorView*>(this->parent())->isAutoSync() )) {
        QObject::connect(w, SIGNAL(commitData(QWidget*)),
            this, SIGNAL(commitData(QWidget*)));
    }
//    w->resize(w->size()+QSize(0,2));
    m_currentEditor = w;
    return w;
}

/*
void ItemDelegate::updateEditorGeometry( QWidget * editor, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItem alteredOption(option);
//    alteredOption.rect.setTop(alteredOption.rect.top() + 1);
    QStyledItemDelegate::updateEditorGeometry(editor, alteredOption, index);
    QRect r(editor->geometry());
//    r.setTop(r.top() + 2);
    editor->setGeometry(r);
}
*/
/*
bool ItemDelegate::editorEvent( QEvent * event, QAbstractItemModel * model,
    const QStyleOptionViewItem & option, const QModelIndex & index )
{
    if (index.column() == 0 && event->type() == QEvent::MouseButtonPress) {
        kDebug() << "!!!";
    }
    return QStyledItemDelegate::editorEvent( event, model, option, index );
}*/

//----------

class EditorView::Private
{
public:
    Private()
     : set(0)
     , model(0)
     , gridLineColor( EditorView::defaultGridLineColor() )
     , autoSync(true)
     , slotPropertyChangedEnabled(true)
    {
    }
    Set *set;
    EditorDataModel *model;
    ItemDelegate *itemDelegate;
    QColor gridLineColor;
    bool autoSync;
    bool slotPropertyChangedEnabled;
};

EditorView::EditorView(QWidget* parent)
        : QTreeView(parent)
        , d( new Private )
{
    setObjectName(QLatin1String("EditorView"));
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setAnimated(false);
    setAllColumnsShowFocus(true);
//    setEditTriggers(QAbstractItemView::AllEditTriggers);
    
    setEditTriggers(
          QAbstractItemView::CurrentChanged
        | QAbstractItemView::DoubleClicked
        //|QAbstractItemView::SelectedClicked
        | QAbstractItemView::EditKeyPressed
        | QAbstractItemView::AnyKeyPressed
        | QAbstractItemView::AllEditTriggers);

    setItemDelegate(d->itemDelegate = new ItemDelegate(this));
}

EditorView::~EditorView()
{
    delete d;
}

void EditorView::changeSet(Set *set, SetOptions options)
{
    changeSetInternal(set, options, QByteArray());
}

void EditorView::changeSet(Set *set, const QByteArray& propertyToSelect, SetOptions options)
{
    changeSetInternal(set, options, propertyToSelect);
}

void EditorView::changeSetInternal(Set *set, SetOptions options, 
    const QByteArray& propertyToSelect)
{
//! @todo port??
#if 0
    if (d->insideSlotValueChanged) {
        //changeSet() called from inside of slotValueChanged()
        //this is dangerous, because there can be pending events,
        //especially for the GUI stuff, so let's do delayed work
        d->setListLater_list = set;
        d->preservePrevSelection_preservePrevSelection = preservePrevSelection;
        d->preservePrevSelection_propertyToSelect = propertyToSelect;
        qApp->processEvents(QEventLoop::AllEvents);
        if (d->set) {
            //store prev. selection for this prop set
            if (d->currentItem)
                d->set->setPrevSelection(d->currentItem->property()->name());
            kDebug(30007) << d->set->prevSelection();
        }
        if (!d->setListLater_set) {
            d->setListLater_set = true;
            d->changeSetLaterTimer.setSingleShot(true);
            d->changeSetLaterTimer.start(10);
        }
        return;
    }
#endif

    const bool setChanged = d->set != set;
    if (d->set) {
        acceptInput();
        //store prev. selection for this prop set
        QModelIndex index = currentIndex();
        if (index.isValid()) {
#if 0 //todo
            Property *property = d->model->propertyForItem(index);
            //TODO This crashes when changing the interpreter type in the script plugin
            //if (property->isNull())
            //    kDebug() << "WTF? a NULL property?";
            //else        
                //d->set->setPreviousSelection(property->name());
#endif
        }
        else {
            d->set->setPreviousSelection(QByteArray());
        }
        if (setChanged) {
            d->set->disconnect(this);
        }
    }

    QByteArray selectedPropertyName1 = propertyToSelect;
    QByteArray selectedPropertyName2 = propertyToSelect;
    if (options & PreservePreviousSelection) {
        //try to find prev. selection:
        //1. in new list's prev. selection
        if (set)
            selectedPropertyName1 = set->previousSelection();
        //2. in prev. list's current selection
        if (d->set)
            selectedPropertyName2 = d->set->previousSelection();
    }

    if (setChanged) {
        d->set = set;
    }
    if (d->set && setChanged) {
        //receive property changes
        connect(d->set, SIGNAL(propertyChangedInternal(KoProperty::Set&, KoProperty::Property&)),
                this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));
        connect(d->set, SIGNAL(propertyReset(KoProperty::Set&, KoProperty::Property&)),
                this, SLOT(slotPropertyReset(KoProperty::Set&, KoProperty::Property&)));
//NEEDED?        connect(d->set, SIGNAL(aboutToBeCleared()), this, SLOT(slotSetWillBeCleared()));
        connect(d->set, SIGNAL(aboutToBeDeleted()), this, SLOT(slotSetWillBeDeleted()));
    }

    EditorDataModel *oldModel = d->model;
    const Set::Order setOrder
        = (options & AlphabeticalOrder) ? Set::AlphabeticalOrder : Set::InsertionOrder;
    d->model = d->set ? new EditorDataModel(*d->set, this, setOrder) : 0;
    setModel( d->model );
    delete oldModel;

    if (d->model && d->set && !d->set->isEmpty() && (options & ExpandChildItems)) {
        const int rowCount = d->model->rowCount();
        for (int row = 0; row < rowCount; row++) {
            expand( d->model->index(row, 0) );
        }
    }

    emit propertySetChanged(d->set);

    if (d->set) {
        //select prev. selected item
        QModelIndex index;
        if (!selectedPropertyName2.isEmpty()) //try other one for old prop set
            index = d->model->indexForPropertyName( selectedPropertyName2 );
        if (!index.isValid() && !selectedPropertyName1.isEmpty()) //try old one for current prop set
            index = d->model->indexForPropertyName( selectedPropertyName1 );

        if (index.isValid()) {
            setCurrentIndex(index);
            scrollTo(index);
//            QTimer::singleShot(10, this, SLOT(selectItemLater()));
            //d->doNotSetFocusOnSelection = !hasParent(this, focusWidget());
            //setSelected(item, true);
            //d->doNotSetFocusOnSelection = false;
//   ensureItemVisible(item);
        }
    }
}

void EditorView::slotSetWillBeDeleted()
{
    changeSet(0, QByteArray());
}

void EditorView::setAutoSync(bool enable)
{
    d->autoSync = enable;
}

bool EditorView::isAutoSync() const
{
    return d->autoSync;
}

void EditorView::currentChanged( const QModelIndex & current, const QModelIndex & previous )
{
    QTreeView::currentChanged( current, previous );
}

bool EditorView::edit( const QModelIndex & index, EditTrigger trigger, QEvent * event )
{
/*    Property *property = d->model->propertyForItem(index);
    if (property && property->children())
        return false;*/

    bool result = QTreeView::edit( index, trigger, event );
    if (result) {
      QLineEdit *lineEditEditor = dynamic_cast<QLineEdit*>( (QObject*)d->itemDelegate->m_currentEditor );
      if (lineEditEditor) {
        lineEditEditor->deselect();
        lineEditEditor->end(false);
      }
    }
    return result;
}

void EditorView::drawBranches( QPainter * painter, const QRect & rect, const QModelIndex & index ) const
{
    QTreeView::drawBranches( painter, rect, index );
}

QRect EditorView::revertButtonArea( const QModelIndex& index ) const
{
    if (index.column() != 0)
        return QRect();
    QVariant modifiedVariant( d->model->data(index, EditorDataModel::PropertyModifiedRole) );
    if (!modifiedVariant.isValid() || !modifiedVariant.toBool())
        return QRect();
    const int iconSize = getIconSize( rowHeight( index ) );
    int x2 = columnWidth(0);
    int x1 = x2 - iconSize - 2;
    QRect r(visualRect(index));
//    kDebug() << r;
    r.setLeft(x1);
    r.setRight(x2);
//    kDebug() << r;
    return r;
}

bool EditorView::withinRevertButtonArea( int x, const QModelIndex& index ) const
{
    QRect r(revertButtonArea( index ));
    if (!r.isValid())
        return false;
    return r.left() < x && x < r.right();
}

void EditorView::mousePressEvent ( QMouseEvent * event )
{
    QTreeView::mousePressEvent( event );
    QModelIndex index = indexAt( event->pos() );
    setCurrentIndex(index);
    if (withinRevertButtonArea( event->x(), index )) {
        undo();
    }
}

void EditorView::undo()
{
//    const EditorDataModel *editorModel = dynamic_cast<const EditorDataModel*>(model());
//    if (!d->currentWidget || !d->currentItem || (d->set && d->set->isReadOnly()) || (d->currentWidget && d->currentWidget->isReadOnly()))
    if (!d->set || d->set->isReadOnly())
        return;

    Property *property = d->model->propertyForItem(currentIndex());
    if (computeAutoSync( property, d->autoSync ))
        property->resetValue();
//    update( currentIndex() );
//??    QTreeView::edit(currentIndex());
 /*   if (d->currentWidget && d->currentItem) {//(check because current widget could be removed by resetValue())
        d->currentWidget->setValue(d->currentItem->property()->value());
        repaintItem(d->currentItem);
    }*/
}

void EditorView::acceptInput()
{
//! @todo
}

void EditorView::commitData( QWidget * editor )
{
    d->slotPropertyChangedEnabled = false;
    QAbstractItemView::commitData( editor );
    d->slotPropertyChangedEnabled = true;
}

bool EditorView::viewportEvent( QEvent * event )
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *hevent = static_cast<QHelpEvent*>(event);
        const QModelIndex index = indexAt(hevent->pos());
        if (index.column() == 0 && withinRevertButtonArea( hevent->x(), index )) {
            QRect r(revertButtonArea( index ));
            QToolTip::showText(hevent->globalPos(), i18n("Undo changes"), this, r);
        }
        else {
            QToolTip::hideText();
        }
    }
    return QTreeView::viewportEvent(event);
}

QColor EditorView::gridLineColor() const
{
    return d->gridLineColor;
}

void EditorView::setGridLineColor(const QColor& color)
{
    d->gridLineColor = color;
}

static QModelIndex findChildItem(const Property& property, const QModelIndex &parent)
{
    const EditorDataModel *editorModel = dynamic_cast<const EditorDataModel*>(parent.model());
    if (editorModel->propertyForItem(parent) == &property) {
        return parent;
    }
    int row = 0;
    while (true) {
        QModelIndex childItem = parent.child(row, 0);
        if (childItem.isValid()) {
            QModelIndex subchild = findChildItem(property, childItem);
            if (subchild.isValid()) {
                return subchild;
            }
        }
        else {
            return QModelIndex();
        }
        row++;
    }
}

void EditorView::slotPropertyChanged(Set& set, Property& property)
{
    Q_UNUSED(set);
    if (!d->slotPropertyChangedEnabled)
        return;
    d->slotPropertyChangedEnabled = false;
    Property *realProperty = &property;
    while (realProperty->parent()) { // find top-level property
        realProperty = realProperty->parent();
    }
    const QModelIndex parentIndex( d->model->indexForPropertyName(realProperty->name()) );
    if (parentIndex.isValid()) {
        QModelIndex index = findChildItem(property, parentIndex);
        if (index.isValid()) {
            update(index);
        }
        index = d->model->indexForColumn(index, 1);
        if (index.isValid()) {
            update(index);
        }
    }
    d->slotPropertyChangedEnabled = true;
}

void EditorView::slotPropertyReset(KoProperty::Set& set, KoProperty::Property& property)
{
//! @todo OK?
    slotPropertyChanged(set, property);
}

#include "EditorView.moc"

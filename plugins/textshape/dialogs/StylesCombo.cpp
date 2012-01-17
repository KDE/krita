/* This file is part of the KDE project
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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
#include "StylesCombo.h"
#include <KoStyleThumbnailer.h>

#include "StylesModel.h"
#include "StylesComboPreview.h"
#include "StylesDelegate.h"

#include <QListView>
#include <QMouseEvent>
#include <QStyleOptionViewItemV4>

#include <KDebug>

StylesCombo::StylesCombo(QWidget *parent)
    : QComboBox(parent),
      m_stylesModel(0),
      m_view(new QListView()),
      m_selectedItem(-1),
      m_originalStyle(true)
{
    setMinimumSize(50,32);

    m_view->setMinimumWidth(250);
    m_view->setMouseTracking(true);
    setView(m_view);
    view()->viewport()->installEventFilter(this);

    StylesDelegate *delegate = new StylesDelegate();
    connect(delegate, SIGNAL(needsUpdate(QModelIndex)), m_view, SLOT(update(QModelIndex)));
    connect(delegate, SIGNAL(styleManagerButtonClicked(QModelIndex)), this, SLOT(slotShowDia(QModelIndex)));
    connect(delegate, SIGNAL(deleteStyleButtonClicked(QModelIndex)), this, SLOT(slotDeleteStyle(QModelIndex)));
    connect(delegate, SIGNAL(clickedInItem(QModelIndex)), this, SLOT(slotItemClicked(QModelIndex)));
    setItemDelegate(delegate);

    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSelectionChanged(int)));

    QComboBox::setEditable(true);
    setIconSize(QSize(0,0));

    StylesComboPreview *preview = new StylesComboPreview(this);
    QComboBox::setEditable(true);
    setLineEdit(preview);
}

StylesCombo::~StylesCombo()
{
}

void StylesCombo::setStyleIsOriginal(bool original)
{
    m_originalStyle = original;
    if (!original) {
        m_preview->setAddButtonShown(true);
    }
    else {
        m_preview->setAddButtonShown(false);
    }
}

void StylesCombo::setStylesModel(StylesModel *model)
{
    m_stylesModel = model;
    setModel(model);
}

void StylesCombo::setEditable(bool editable)
{
    if (editable) {
        // Create a StylesComboPreview instead of a QLineEdit
        // Compared to QComboBox::setEditable, we might be missing the SH_ComboBox_Popup code though...
        // If a style needs this, then we'll need to call QComboBox::setEditable and then setLineEdit again
        StylesComboPreview *edit = new StylesComboPreview( this );
        setLineEdit( edit );
    } else {
        QComboBox::setEditable(editable);
    }
}

void StylesCombo::setLineEdit(QLineEdit *edit)
{
    if (!isEditable() && edit && !qstrcmp(edit->metaObject()->className(), "QLineEdit")) {
        // uic generates code that creates a read-only StylesCombo and then
        // calls combo->setEditable( true ), which causes QComboBox to set up
        // a dumb QLineEdit instead of our nice StylesComboPreview.
        // As some StylesCombo features rely on the StylesComboPreview, we reject
        // this order here.
        delete edit;
        StylesComboPreview* preview = new StylesComboPreview(this);
        edit = preview;
    }

    QComboBox::setLineEdit(edit);
    m_preview = qobject_cast<StylesComboPreview*>(edit);

    if (m_preview) {
        connect(m_preview, SIGNAL(resized()), this, SLOT(slotUpdatePreview()));
        connect(m_preview, SIGNAL(newStyleRequested(QString)), this, SIGNAL(newStyleRequested(QString)));
        connect(m_preview, SIGNAL(clicked()), this, SLOT(slotPreviewClicked()));
    }

}

void StylesCombo::slotSelectionChanged(int index)
{
    if (index != m_selectedItem || !m_originalStyle) {
        m_selectedItem = index;
        m_preview->setPreview(m_stylesModel->stylePreview(index, m_preview->availableSize()));
        update();
        emit selectionChanged(index);
    }
}

void StylesCombo::slotItemClicked(QModelIndex index)
{
    //this slot allows us to emit a selectionChanged signal in case the already selected style isn't in its original form anymore. In such case, the view does not emit currentIndexChanged, so we use the editorEvent of the delegate to send us a signal. There is a bit of redundancy if the item clicked was indeed a new selection, hence the check in both slots.
    if (index.row() != m_selectedItem || !m_originalStyle) {
        m_selectedItem = index.row();
        m_preview->setPreview(m_stylesModel->stylePreview(m_selectedItem, m_preview->availableSize()));
        update();
        emit selectionChanged(m_selectedItem);
    }
}

void StylesCombo::slotUpdatePreview()
{
    m_preview->setPreview(m_stylesModel->stylePreview(currentIndex(), m_preview->availableSize()));
    update();
}

void StylesCombo::slotPreviewClicked()
{
    if (!view()->isVisible()) {
        showPopup();
    }
}

bool StylesCombo::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease && object == view()->viewport()) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        //If what follows isn't a HACK then I have no clue what is!!!!
        //The item delegate editorEvent method is not sent MouseButtonRelease events.
        //This is because the QComboBox installs an event filter on the view and calls
        //popup->hide() on MouseButtonRelease to dismiss the view. Since we installed an event filter on the view
        //ourselves, we can prevent hiding the popup. We have to call itemDelegate->editorEvent
        //manually though.
        QModelIndex index = view()->indexAt(mouseEvent->pos());
        QModelIndex buddy = m_stylesModel->buddy(index);
        QStyleOptionViewItemV4 options;
        options.rect = view()->visualRect(buddy);
        options.widget = m_view;
        options.state |= (buddy == view()->currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
        return view()->itemDelegate()->editorEvent(mouseEvent, m_stylesModel, options, index);
    }
    return false;
}

void StylesCombo::slotShowDia(QModelIndex index)
{
    emit showStyleManager(index.row());
}

void StylesCombo::slotDeleteStyle(QModelIndex index)
{
    emit deleteStyle(index.row());
}

#include <StylesCombo.moc>


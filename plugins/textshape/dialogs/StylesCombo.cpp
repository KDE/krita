/* This file is part of the KDE project
 * Copyright (C) 2011 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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
#include "KoStyleThumbnailer.h"

#include "StylesModel.h"
#include "StylesComboView.h"
#include "StylesComboPreview.h"
#include "StylesDelegate.h"
#include <KoParagraphStyle.h>
#include <KoStyleManager.h>
#include "StyleManagerDialog.h"

#include <QApplication>
#include <QListView>
#include <QSizePolicy>
#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QStyleOptionComboBox>
#include <QStyleOptionViewItemV4>
#include <QStylePainter>

#include <KDebug>

StylesCombo::StylesCombo(QWidget *parent)
    : QComboBox(parent),
      m_stylesModel(0),
      m_view(0)//,
//      skipNextHide(false)
{
/*    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);

    setMinimumSize(50,32);
    setMaximumHeight(25);

    m_preview = new QLabel();
    m_preview->setAutoFillBackground(true);
    m_preview->setBackgroundRole(QPalette::Base);
    m_preview->setMinimumWidth(50);
    m_preview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QHBoxLayout *l = new QHBoxLayout(this);
    l->addWidget(m_preview);
    l->setMargin(0);
    setLayout(l);

    isPopupVisible = false;
*/
    setMinimumSize(50,32);
//    m_view = new StylesComboView();
    m_view = new QListView();
    m_view->setMinimumWidth(250);
    m_view->setMouseTracking(true);
//    m_view->setSizePolicy(QSizePolicy::Minimum);
    setView(m_view);
//    view()->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
//    view()->setMinimumWidth(250);
//    view()->setMaximumWidth(250);
    view()->viewport()->installEventFilter(this);
    StylesDelegate *delegate = new StylesDelegate();
    connect(delegate, SIGNAL(needsUpdate(QModelIndex)), m_view, SLOT(update(QModelIndex)));
    connect(delegate, SIGNAL(styleManagerButtonClicked(QModelIndex)), this, SLOT(showDia()));
    connect(delegate, SIGNAL(deleteStyleButtonClicked(QModelIndex)), this, SLOT(deleteStyle(QModelIndex)));
    setItemDelegate(delegate);

    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSelectionChanged(int)));

    QComboBox::setEditable(true);
    setIconSize(QSize(0,0));

    StylesComboPreview *preview = new StylesComboPreview(this);
//    preview->setAddButtonShown(true);
    connect(preview, SIGNAL(newStyleRequested(QString)), this, SIGNAL(newStyleRequested(QString)));
    QComboBox::setEditable(true);
    setLineEdit(preview);
}

StylesCombo::~StylesCombo()
{
}

void StylesCombo::setStyleIsOriginal(bool original)
{
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
/*
void StylesCombo::setStyleManager(KoStyleManager *styleManager)
{
    Q_UNUSED(styleManager)
}
*/
void StylesCombo::setEditable(bool editable)
{
    if (editable) {
        // Create a KLineEdit instead of a QLineEdit
        // Compared to QComboBox::setEditable, we might be missing the SH_ComboBox_Popup code though...
        // If a style needs this, then we'll need to call QComboBox::setEditable and then setLineEdit again
        StylesComboPreview *edit = new StylesComboPreview( this );
//        edit->setAddButtonShown( true );
        setLineEdit( edit );
    } else {
        QComboBox::setEditable(editable);
    }
}

void StylesCombo::setLineEdit(QLineEdit *edit)
{
    if ( !isEditable() && edit &&
         !qstrcmp( edit->metaObject()->className(), "QLineEdit" ) )
    {
        // uic generates code that creates a read-only KComboBox and then
        // calls combo->setEditable( true ), which causes QComboBox to set up
        // a dumb QLineEdit instead of our nice KLineEdit.
        // As some KComboBox features rely on the KLineEdit, we reject
        // this order here.
        delete edit;
        StylesComboPreview* preview = new StylesComboPreview( this );

//        if ( isEditable() ) {
//            preview->setAddButtonShown( true );
//        }

        edit = preview;
    }

    QComboBox::setLineEdit( edit );
    m_preview = qobject_cast<StylesComboPreview*>( edit );

    // Connect the returnPressed signal for both Q[K]LineEdits'
    if (edit)
        connect( edit, SIGNAL( returnPressed() ), SIGNAL( returnPressed() ));

    if ( m_preview )
    {
        // someone calling KComboBox::setEditable( false ) destroys our
        // lineedit without us noticing. And KCompletionBase::delegate would
        // be a dangling pointer then, so prevent that. Note: only do this
        // when it is a KLineEdit!
        connect(edit, SIGNAL(destroyed()), SLOT(lineEditDeleted()));

        connect(m_preview, SIGNAL(returnPressed(const QString&)), SIGNAL(returnPressed(const QString&)));
        connect(m_preview, SIGNAL(resized()), this, SLOT(previewResized()));

//        m_preview->setTrapReturnKey( d->trapReturnKey );
    }

}

void StylesCombo::slotSelectionChanged(int index)
{
    m_preview->setPreview(m_stylesModel->stylePreview(index, m_preview->availableSize()));
    update();
    emit selectionChanged(index);

/*    KoParagraphStyle *paragStyle = m_stylesModel->styleManager()->paragraphStyle(m_stylesModel->index(index).internalId());
    if (paragStyle) {
        m_preview->setPreview(m_stylesModel->thumbnailer()->thumbnail(paragStyle, m_preview->availableSize()));
        emit paragraphStyleSelected(paragStyle);
        return;
    }
    KoCharacterStyle *characterStyle =  m_stylesModel->styleManager()->characterStyle(m_stylesModel->index(index).internalId());
    if (characterStyle) {
        m_preview->setPreview(m_stylesModel->thumbnailer()->thumbnail(characterStyle, m_preview->availableSize()));
        return;
    }
    m_preview->setPreview(QPixmap());
*/
}
/*
void StylesCombo::setCurrentFormat(const QTextBlockFormat &format)
{
    if (format == m_currentBlockFormat)
        return;
    m_currentBlockFormat = format;
    int id = m_currentBlockFormat.intProperty(KoParagraphStyle::StyleId);
    bool unchanged = true;
    KoParagraphStyle *usedStyle = 0;
    if (m_stylesModel->styleManager())
        usedStyle = m_stylesModel->styleManager()->paragraphStyle(id);
    if (usedStyle) {
        foreach(int property, m_currentBlockFormat.properties().keys()) {
            if (property == QTextFormat::ObjectIndex)
                continue;
            if (property == KoParagraphStyle::ListStyleId)
                continue;
            if (m_currentBlockFormat.property(property) != usedStyle->value(property)) {
                unchanged = false;
                break;
            }
        }
    }

    setStyleIsOriginal(unchanged);


    KoParagraphStyle *paragStyle = m_stylesModel->styleManager()->paragraphStyle(id);
    if (paragStyle) {
        m_stylesModel->setCurrentParagraphStyle(id, m_preview->isAddButtonShown()); //temporary hack for the unchanged stuff. i need to decide if this resides in the combo or in the paragWidget.
        disconnect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(selectionChanged(int)));
        setCurrentIndex(m_stylesModel->indexForParagraphStyle(*paragStyle).row());
        connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(selectionChanged(int)));
        m_preview->setPreview(m_stylesModel->thumbnailer()->thumbnail(paragStyle, m_preview->availableSize()));
    }
    update();
}

void StylesCombo::setCurrentFormat(const QTextCharFormat &format)
{
    Q_UNUSED(format)
}
*/
void StylesCombo::previewResized()
{///TODO take care of charStyles too
    kDebug() << "resized: " << size();
/*    KoParagraphStyle *usedStyle = 0;
    if (m_stylesModel->styleManager())
        usedStyle = m_stylesModel->styleManager()->paragraphStyle(m_stylesModel->index(currentIndex()).internalId());
    if (usedStyle) {
        m_preview->setPreview(m_stylesModel->thumbnailer()->thumbnail(usedStyle, m_preview->availableSize()));
    }
*/
    m_preview->setPreview(m_stylesModel->stylePreview(currentIndex(), m_preview->availableSize()));
}

bool StylesCombo::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress && object == view()->viewport()){
        QMouseEvent *mEvent = static_cast<QMouseEvent*>(event);
        QMouseEvent mouseEvent(QEvent::MouseButtonPress, mEvent->pos(),
                               mEvent->button(), mEvent->buttons(), mEvent->modifiers());
//        if (!view()->visualRect(index).contains(mouseEvent->pos()))
//            skipNextHide = true;
    }
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
        kDebug() << "option.rect sent to the popup: " << options.rect;
        kDebug() << "event sent: " << mouseEvent->type();
        options.state |= (buddy == view()->currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
        return view()->itemDelegate()->editorEvent(mouseEvent, m_stylesModel, options, index);
    }
    return false;
}
/*
void StylesCombo::paintEvent(QPaintEvent *e)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // draw the icon and text
//    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);

}
*/
void StylesCombo::slotShowDia(QModelIndex index)
{
    kDebug() << "showDia slot";
    emit showStyleManager(index.row());
//TODO this shouldn't be done here, send signal instead
/*
        KoStyleManager *styleManager = m_stylesModel->styleManager();
        Q_ASSERT(styleManager);
        if (!styleManager)
            return;  //don't crash
        StyleManagerDialog *dia = new StyleManagerDialog(this);
        dia->setStyleManager(styleManager);
        dia->show();
*/
}

void StylesCombo::slotDeleteStyle(QModelIndex index)
{
    kDebug() << "delete style slot";
    emit deleteStyle(index.row());
    //TODO this should not be handled here. send a signal instead.
/*    KoStyleManager *styleManager = m_stylesModel->styleManager();
    Q_ASSERT(styleManager);
    if (!styleManager)
        return;
    styleManager->remove(styleManager->paragraphStyle(index.internalId()));
*/
}

#include <StylesCombo.moc>


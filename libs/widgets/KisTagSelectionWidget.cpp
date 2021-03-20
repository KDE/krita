/*
 *  Author 2021 Agata Cacko cacko.azh@gmail.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisTagSelectionWidget.h"

#include <QProcessEnvironment>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QGridLayout>
#include <QTableWidget>
#include <QPainter>
#include <QListWidget>
#include <QAction>
#include <QMouseEvent>
#include <QMenu>
#include <QPair>
#include <QApplication>
#include <QInputDialog>
#include <QPainterPath>

#include <KoFileDialog.h>
#include <kis_icon.h>
#include <KoID.h>

#include <kis_debug.h>
#include <kis_global.h>
#include <TagActions.h>

#include <KisWrappableHBoxLayout.h>
#include <kis_signals_blocker.h>


#include "kis_icon.h"


WdgCloseableLabel::WdgCloseableLabel(KoID tag, bool editable, bool semiSelected, QWidget *parent)
    : QWidget(parent)
    , m_editble(editable)
    , m_tag(tag)
    , m_semiSelected(semiSelected)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(1);

    m_textLabel = new QLabel(parent);
    m_textLabel->setText(tag.name());

    int minimumSpace = 8;
    layout->addSpacerItem(new QSpacerItem(minimumSpace, 0));
    layout->addWidget(m_textLabel);
    layout->insertStretch(2, 1);
    if (m_editble) {
        m_closeIconLabel = new QLabel(parent);
        QIcon icon = KisIconUtils::loadIcon("tagclose");
        QSize size = QSize(1, 1)*m_textLabel->height()*0.3;
        m_closeIconLabel->setPixmap(icon.pixmap(size));
        layout->addWidget(m_closeIconLabel);
    }
    layout->addSpacing(minimumSpace);
    setLayout(layout);



}

WdgCloseableLabel::~WdgCloseableLabel()
{

}

void WdgCloseableLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QColor backGroundColor = qApp->palette().light().color();
    QColor foregroundColor = qApp->palette().windowText().color();
    qreal r1 = 0.65;
    qreal r2 = 1 - r1;
    QColor outlineColor = QColor::fromRgb(256*(r1*backGroundColor.redF() + r2*foregroundColor.redF()),
                                          256*(r1*backGroundColor.greenF() + r2*foregroundColor.greenF()),
                                          256*(r1*backGroundColor.blueF() + r2*foregroundColor.blueF()));


    QBrush windowB = qApp->palette().window();
    QBrush windowTextB = qApp->palette().windowText();

    QWidget::paintEvent(event);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(this->rect(), 6, 6);

    // good color:
    painter.fillPath(path, qApp->palette().light());

    if (m_semiSelected) {

        QPen penwt = QPen(outlineColor, 1);
        penwt.setStyle(Qt::DashLine);
        QPen penw = QPen(windowB, 1);

        QPainterPath outlinePath;
        //outlinePath.addRoundedRect(this->rect().adjusted(2, 2, -2, -2), 4, 4); // for pen width == 2
        outlinePath.addRoundedRect(this->rect().adjusted(1, 1, -1, -1), 4, 4); // for pen width == 1

        painter.setPen(penwt);
        painter.drawPath(outlinePath);
    }

}

void WdgCloseableLabel::mousePressEvent(QMouseEvent *event)
{
    if (!m_editble) {
        return;
    }

    QRect closeRect = kisGrowRect(m_closeIconLabel->rect(), 5);

    m_closeIconLabel->pos();
    if (closeRect.contains(event->pos() - m_closeIconLabel->pos())) {
        // working, just add a signal
        emit sigRemoveTagFromSelection(m_tag);
    }
}




WdgAddTagButton::WdgAddTagButton(QWidget *parent)
    : QToolButton(parent)
    , m_compressor(10, KisSignalCompressor::FIRST_ACTIVE)
{
    setPopupMode(QToolButton::InstantPopup);
    setContentsMargins(0, 0, 0, 0);
    QSize defaultSize = QSize(1, 1)*m_size;
    setMinimumSize(defaultSize);
    setMaximumSize(defaultSize);

    connect(&m_compressor, SIGNAL(timeout()), this, SLOT(slotFinishLastAction()), Qt::UniqueConnection);
    connect(this, SIGNAL(triggered(QAction*)), SLOT(slotAddNewTag(QAction*)));

    UserInputTagAction *newTag = new UserInputTagAction(this);
    newTag->setCloseParentOnTrigger(false);

    connect(newTag, SIGNAL(triggered(QString)), this, SLOT(slotCreateNewTag(QString)), Qt::UniqueConnection);
    m_createNewTagAction = newTag;

}

WdgAddTagButton::~WdgAddTagButton()
{

}

void WdgAddTagButton::setAvailableTagsList(QList<KoID> &notSelected)
{
    QList<QAction*> actionsToRemove = actions();
    Q_FOREACH(QAction* action, actionsToRemove) {
        removeAction(action);
    }

    Q_FOREACH(KoID tag, notSelected) {
        QAction* action = new QAction(tag.name());
        action->setData(QVariant::fromValue<KoID>(tag));
        addAction(action);
    }

    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    addAction(separator);

    addAction(m_createNewTagAction);
    setDefaultAction(0);
}

void WdgAddTagButton::slotFinishLastAction()
{
    if (m_lastAction == CreateNewTag) {
        emit sigCreateNewTag(m_lastTagToCreate);
    } else {
        emit sigAddNewTag(m_lastTagToAdd);
    }


}

void WdgAddTagButton::slotAddNewTag(QAction *action)
{
    if (action == m_createNewTagAction) {
        m_lastTagToCreate = action->data().toString();
        m_lastAction = CreateNewTag;
        m_compressor.start();
        KisSignalsBlocker b(m_createNewTagAction);
        m_createNewTagAction->setText("");
    } else if (!action->data().isNull() && action->data().canConvert<KoID>()) {
        m_lastTagToAdd = action->data().value<KoID>();
        m_lastAction = AddNewTag;
        m_compressor.start();
    }


    if (this->menu()) {
        this->menu()->close();
    }
}

void WdgAddTagButton::slotCreateNewTag(QString tagName)
{
    m_lastTagToCreate = tagName;
    m_lastAction = CreateNewTag;
    m_compressor.start();
    KisSignalsBlocker b(m_createNewTagAction);
    m_createNewTagAction->setText("");


    if (this->menu()) {
        this->menu()->close();
    }
}

void WdgAddTagButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(this->rect(), 6, 6);
    painter.fillPath(path, qApp->palette().light());
    painter.setPen(QPen(qApp->palette().windowText(), painter.pen().widthF()));
    QIcon icon = KisIconUtils::loadIcon("list-add");
    QSize size = this->rect().size()*0.6;

    QSize iconSize = icon.actualSize(size);
    QPixmap pix = icon.pixmap(size);
    QSize realSize = iconSize.scaled(size, Qt::KeepAspectRatio);//pix.rect().size();
    QPoint p = this->rect().topLeft() + QPoint(this->rect().width()/2 - realSize.width()/2, this->rect().height()/2 - realSize.height()/2);
    painter.setOpacity(!isEnabled() ? 0.3 : 1.0);
    painter.drawPixmap(p, pix);
    painter.setOpacity(1.0);
}

KisTagSelectionWidget::KisTagSelectionWidget(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new KisWrappableHBoxLayout(this);
    m_addTagButton = new WdgAddTagButton(this);

    m_layout->addWidget(m_addTagButton);
    connect(m_addTagButton, SIGNAL(sigCreateNewTag(QString)), this, SIGNAL(sigCreateNewTag(QString)), Qt::UniqueConnection);
    connect(m_addTagButton, SIGNAL(sigAddNewTag(KoID)), this, SIGNAL(sigAddTagToSelection(KoID)), Qt::UniqueConnection);


    setLayout(m_layout);

}

KisTagSelectionWidget::~KisTagSelectionWidget()
{

}

void KisTagSelectionWidget::setTagList(bool editable, QList<KoID> &selected, QList<KoID> &notSelected)
{
    QList<KoID> semiSelected;
    setTagList(editable, selected, notSelected, semiSelected);
}

void KisTagSelectionWidget::setTagList(bool editable, QList<KoID> &selected, QList<KoID> &notSelected, QList<KoID> &semiSelected)
{
    m_editable = editable;
    QLayoutItem *item;

    disconnect(m_addTagButton, SIGNAL(sigCreateNewTag(QString)), this, SIGNAL(sigCreateNewTag(QString)));
    disconnect(m_addTagButton, SIGNAL(sigAddNewTag(KoID)), this, SIGNAL(sigAddTagToSelection(KoID)));

    while((item = m_layout->takeAt(0))) {
        if (item->widget()) {
            if (!dynamic_cast<WdgAddTagButton*>(item->widget())) {
                delete item->widget();
            }
        }
        delete item;
    }


    WdgAddTagButton* addTagButton = dynamic_cast<WdgAddTagButton*>(m_addTagButton);
    addTagButton->setAvailableTagsList(notSelected);

    Q_FOREACH(KoID tag, selected) {
        WdgCloseableLabel* label = new WdgCloseableLabel(tag, m_editable, false, this);
        connect(label, SIGNAL(sigRemoveTagFromSelection(KoID)), this, SLOT(slotRemoveTagFromSelection(KoID)), Qt::UniqueConnection);
        m_layout->addWidget(label);
    }

    Q_FOREACH(KoID tag, semiSelected) {
        WdgCloseableLabel* label = new WdgCloseableLabel(tag, m_editable, true, this);
        connect(label, SIGNAL(sigRemoveTagFromSelection(KoID)), this, SLOT(slotRemoveTagFromSelection(KoID)), Qt::UniqueConnection);
        m_layout->addWidget(label);
    }

    m_layout->addWidget(m_addTagButton);
    m_addTagButton->setVisible(m_editable);


    connect(m_addTagButton, SIGNAL(sigCreateNewTag(QString)), this, SIGNAL(sigCreateNewTag(QString)), Qt::UniqueConnection);
    connect(m_addTagButton, SIGNAL(sigAddNewTag(KoID)), this, SIGNAL(sigAddTagToSelection(KoID)), Qt::UniqueConnection);

    if (m_editable) {
    }

    if (layout()) {
        layout()->invalidate();
    }
}

void KisTagSelectionWidget::slotAddTagToSelection(QAction *action)
{
    if (!action) return;
    if (action->data().isNull()) {
    } else {
        KoID custom = action->data().value <KoID>();
        emit sigAddTagToSelection(custom);
    }
}

void KisTagSelectionWidget::slotRemoveTagFromSelection(KoID tag)
{
    emit sigRemoveTagFromSelection(tag);
}

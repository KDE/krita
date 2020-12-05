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

#include <KoFileDialog.h>
#include <kis_icon.h>
#include <KoID.h>

#include <kis_debug.h>

#include<KisWrappableHBoxLayout.h>


#include "kis_icon.h"


WdgCloseableLabel::WdgCloseableLabel(KoID tag, bool editable, QWidget *parent)
    : QWidget(parent)
    , m_editble(editable)
    , m_tag(tag)
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
        //m_closeIconLabel->setPixmap();
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
    //painter.setBackground(QBrush(Qt::blue));


    QBrush windowB = qApp->palette().window();
    QBrush windowTextB = qApp->palette().windowText();



    QWidget::paintEvent(event);
    QBrush brush = QBrush(Qt::red);
    //QPen pen = QPen(brush, 3);
    //painter.setPen(pen);
    //painter.setBrush(qApp->palette().background());
    //painter.setBrush(brush);
    //painter.setPen(Qt::transparent);
    QPen pen = painter.pen();
    //pen.setWidthF(2);

    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(this->rect(), 6, 6);
    //painter.fillPath(path, Qt::red);

    // good color:
    painter.fillPath(path, qApp->palette().light());


    //painter.setPen(QPen(qApp->palette().windowText()));
    QPen penwt = QPen(windowTextB, 1);
    QPen penw = QPen(windowB, 1);


    painter.setPen(penwt);
    //painter.drawPath(path);


    QPainterPath path2;
    path2.addRoundedRect(this->rect().adjusted(-2, -2, -2, -2), 7, 7);

    painter.setPen(penw);
    //painter.drawPath(path2);




    //painter.drawRect(this->rect());

    /*
    painter.setBrush(QBrush(Qt::blue));
    painter.setPen(QPen(Qt::transparent));

    QSize size = QSize(15, 0.7*this->height());

    painter.setBrush(qApp->palette().window());
    painter.drawRect(QRect(QPoint(80, 0), size));

    painter.setBrush(qApp->palette().windowText());
    painter.drawRect(QRect(QPoint(100, 0), size));

    painter.setBrush(qApp->palette().base());
    painter.drawRect(QRect(QPoint(120, 0), size));

    painter.setBrush(qApp->palette().alternateBase());
    painter.drawRect(QRect(QPoint(140, 0), size));

    painter.setBrush(qApp->palette().brightText());
    painter.drawRect(QRect(QPoint(160, 0), size));


    painter.setBrush(qApp->palette().light());
    painter.drawRect(QRect(QPoint(180, 0), size));

    */

    //painter.drawRect(m_closeIconLabel->rect());
    //painter.drawRect(m_textLabel->rect());
    //painter.setRenderHint(QPainter::Antialiasing);
    //QPainterPath path;
    //path.addRoundedRect(this->rect(), 6, 6);
    //painter.fillPath(path, Qt::red);

    // good color:
    //painter.fillPath(path, qApp->palette().light());
    //painter.drawRect(this->rect().adjusted(-3, -3, -3, -3));

}

void WdgCloseableLabel::mousePressEvent(QMouseEvent *event)
{
    if (!m_editble) {
        return;
    }

    ENTER_FUNCTION() << "pressed on something!" << ppVar(event->pos()) << ppVar(this->rect()) << ppVar(m_closeIconLabel->rect());
    //layout()->
    m_closeIconLabel->pos();
    if (m_closeIconLabel->rect().contains(event->pos() - m_closeIconLabel->pos())) {
        // working, just add a signal
        ENTER_FUNCTION() << "YES PRESSED!";
        emit sigRemoveTagFromSelection(m_tag);
    }
}

/*
QSize WdgCloseableLabel::sizeHint() const
{
    return this->rect().size();
}
*/

WdgAddTagButton::WdgAddTagButton(QWidget *parent)
    : QToolButton(parent)
{
    //setIcon();
    setPopupMode(QToolButton::InstantPopup);
    setContentsMargins(0, 0, 0, 0);
}

WdgAddTagButton::~WdgAddTagButton()
{

}

void WdgAddTagButton::setAvailableTagsList(QList<KoID> &notSelected)
{
    ENTER_FUNCTION() << notSelected.count();

    QList<QAction*> actionsToRemove = actions();
    Q_FOREACH(QAction* action, actionsToRemove) {
        removeAction(action);
    }

    Q_FOREACH(KoID tag, notSelected) {
        QAction* action = new QAction(tag.name());
        action->setData(QVariant::fromValue<KoID>(tag));
        addAction(action);
    }

    setDefaultAction(0);
}

void WdgAddTagButton::paintEvent(QPaintEvent *event)
{
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
    ENTER_FUNCTION() << ppVar(this->rect()) << ppVar(size) << ppVar(realSize) << ppVar(p);
    //QPoint p = this->rect().topLeft() + QPoint(this->rect().width()*0.05, this->rect().height()*0.05);
    painter.drawPixmap(p, pix);

    //QToolButton::paintEvent(event);
}

WdgAddTagsCategoriesButton::WdgAddTagsCategoriesButton(QWidget *parent)
    : QToolButton(parent)
{
    setPopupMode(QToolButton::InstantPopup);
    setContentsMargins(0, 0, 0, 0);
}

WdgAddTagsCategoriesButton::~WdgAddTagsCategoriesButton()
{

}

void WdgAddTagsCategoriesButton::setAvailableTagsList(QList<CustomTagsCategorySP> &notSelected)
{

    return;

    ENTER_FUNCTION() << notSelected.count();
    QList<QAction*> actionsToRemove = actions();
    Q_FOREACH(QAction* action, actionsToRemove) {
        removeAction(action);
    }

    /*
    Q_FOREACH(CustomTagSP tag, notSelected) {
        QAction* action = new QAction(tag->name);
        action->setData(tag->data);
        addAction(action);
    }
    */

    QMenu* menu = new QMenu(this);
    Q_FOREACH(CustomTagsCategorySP category, notSelected) {
        QMenu* submenu = menu->addMenu(category->categoryName);
        ENTER_FUNCTION() << "### adding a submenu " << submenu->title();

        Q_FOREACH(CustomTagSP tag, category->tags) {
            QAction* action = submenu->addAction(tag->name);
            action->setData(tag->data);
            ENTER_FUNCTION() << "### adding an action: " << action->text();
        }
        //addAction(submenu->act);
    }

    setMenu(menu);


    setDefaultAction(0);
}

void WdgAddTagsCategoriesButton::paintEvent(QPaintEvent *event)
{
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
    ENTER_FUNCTION() << ppVar(this->rect()) << ppVar(size) << ppVar(realSize) << ppVar(p);
    //QPoint p = this->rect().topLeft() + QPoint(this->rect().width()*0.05, this->rect().height()*0.05);
    painter.drawPixmap(p, pix);
    //QToolButton::paintEvent(event);
}



WdgTagSelection::WdgTagSelection(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new KisWrappableHBoxLayout(this);
    m_addTagButton = new WdgAddTagButton(this);

    m_layout->addWidget(m_addTagButton);
    connect(m_addTagButton, SIGNAL(triggered(QAction*)), this, SLOT(slotAddTagToSelection(QAction*)));

    setLayout(m_layout);

}

WdgTagSelection::~WdgTagSelection()
{

}

void WdgTagSelection::setTagList(bool editable, QList<KoID> &selected, QList<KoID> &notSelected)
{

    ENTER_FUNCTION() << "void WdgTagSelection::setTagList(bool editable, QList<CustomTagSP> &selected, QList<CustomTagsCategorySP> &notSelected)";
    ENTER_FUNCTION() << ppVar(selected.count()) << ppVar(notSelected.count());
    m_editable = editable;
    QLayoutItem *item;

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

    ENTER_FUNCTION() << ppVar(m_layout->count());

    Q_FOREACH(KoID tag, selected) {

        ENTER_FUNCTION() << "Created label for " << ppVar(tag.name());
        WdgCloseableLabel* label = new WdgCloseableLabel(tag, m_editable, this);
        connect(label, SIGNAL(sigRemoveTagFromSelection(KoID)), this, SLOT(slotRemoveTagFromSelection(KoID)));
        m_layout->addWidget(label);
    }
    ENTER_FUNCTION() << "(1)";

    m_layout->addWidget(m_addTagButton);
    ENTER_FUNCTION() << "(2)";

    m_addTagButton->setVisible(m_editable);
    ENTER_FUNCTION() << "(3)";
    if (m_editable) {
        connect(m_addTagButton, SIGNAL(triggered(QAction*)), this, SLOT(slotAddTagToSelection(QAction*)));
    }

    ENTER_FUNCTION() << "(4)";

    if (m_layout) {
        m_layout->invalidate();
    }

    ENTER_FUNCTION() << "(5)";

}

void WdgTagSelection::setTagList(bool editable, QList<KoID> &selected, QList<CustomTagsCategorySP> &notSelected)
{
    return;


    ENTER_FUNCTION() << "void WdgTagSelection::setTagList(bool editable, QList<CustomTagSP> &selected, QList<CustomTagsCategorySP> &notSelected)";
    ENTER_FUNCTION() << ppVar(this);

    ENTER_FUNCTION() << ppVar(selected.count()) << ppVar(notSelected.count());


    m_editable = editable;

    if (m_addTagButton) {
        m_layout->removeWidget(m_addTagButton);
        delete m_addTagButton;
        m_addTagButton = 0;
    }

    QLayoutItem *item;
    while((item = m_layout->takeAt(0))) {
        if (item->widget()) {
           delete item->widget();
        }
        delete item;
    }



    WdgAddTagsCategoriesButton* addTagButton = new WdgAddTagsCategoriesButton(this);
    addTagButton->setAvailableTagsList(notSelected);
    m_addTagButton = addTagButton;

    ENTER_FUNCTION() << ppVar(m_layout->count());

    Q_FOREACH(KoID tag, selected) {

        ENTER_FUNCTION() << "Creater label for " << ppVar(tag.name());
        WdgCloseableLabel* label = new WdgCloseableLabel(tag, m_editable, this);
        connect(label, SIGNAL(sigRemoveTagFromSelection(KoID)), this, SLOT(slotRemoveTagFromSelection(KoID)));
        m_layout->addWidget(label);
    }

    m_layout->addWidget(m_addTagButton);

    m_addTagButton->setVisible(m_editable);
    if (m_editable) {
        connect(m_addTagButton, SIGNAL(triggered(QAction*)), this, SLOT(slotAddTagToSelection(QAction*)));
    }

    if (m_layout) {
        m_layout->invalidate();
    }

}

void WdgTagSelection::slotAddTagToSelection(QAction *action)
{
    ENTER_FUNCTION();

    if (!action || action->data().isNull()) return;

    ENTER_FUNCTION() << "Adding tag" << ppVar(action->text());

    KoID custom = action->data().value <KoID>();

    //ENTER_FUNCTION() << ppVar(custom) << ppVar(custom.isNull()) << ppVar(custom->data.userType()) << ppVar(custom->data.typeName()) << ppVar(custom->data.isNull());
    ENTER_FUNCTION() << ppVar(custom) << ppVar(action->data().userType()) << ppVar(custom.name()) << ppVar(custom.id());


    emit sigAddTagToSelection(custom);
}

void WdgTagSelection::slotRemoveTagFromSelection(KoID tag)
{
    ENTER_FUNCTION() << "Removing tag" << ppVar(tag.name());

    emit sigRemoveTagFromSelection(tag);
}

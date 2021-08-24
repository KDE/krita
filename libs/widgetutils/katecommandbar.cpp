/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "katecommandbar.h"
#include "commandmodel.h"

#include <QAction>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QTreeView>
#include <QVBoxLayout>
#include <QDebug>

#include <kactioncollection.h>
#include <KLocalizedString>

#include <kfts_fuzzy_match.h>

class CommandBarFilterModel : public QSortFilterProxyModel
{
public:
    CommandBarFilterModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

    Q_SLOT void setFilterString(const QString &string)
    {
        beginResetModel();
        m_pattern = string;
        endResetModel();
    }

protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override
    {
        const int l = sourceLeft.data(CommandModel::Score).toInt();
        const int r = sourceRight.data(CommandModel::Score).toInt();
        return l < r;
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (m_pattern.isEmpty()) {
            return true;
        }

        int score = 0;
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        if (idx.isValid()){
            const QString row = idx.data(Qt::DisplayRole).toString();
            int pos = row.indexOf(QLatin1Char(':'));
            if (pos < 0) {
                return false;
            }
            const QString actionName = row.mid(pos + 2);
            const bool res = kfts::fuzzy_match_sequential(m_pattern, actionName, score);
            sourceModel()->setData(idx, score, CommandModel::Score);
            return res;
        }
        return false;
    }

private:
    QString m_pattern;
};

class CommandBarStyleDelegate : public QStyledItemDelegate
{
public:
    CommandBarStyleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem options = option;
        initStyleOption(&options, index);

        QTextDocument doc;

        const auto original = index.data().toString();

        const auto strs = index.data().toString().split(QLatin1Char(':'));
        QString str = strs.at(1);
        const QString nameColor = option.palette.color(QPalette::Link).name();
        kfts::to_fuzzy_matched_display_string(m_filterString, str, QString("<b style=\"color:%1;\">").arg(nameColor), QString("</b>"));

        const QString component = QString("<span style=\"color: %1;\"><b>").arg(nameColor) + strs.at(0) + QString(":</b> </span>");

        doc.setHtml(component + str);
        doc.setDocumentMargin(2);

        painter->save();

        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        // fix stuff for rtl
        // QTextDocument doesn't work with RTL text out of the box so we give it a hand here by increasing
        // the text width to our rect size. Icon displacement is also calculated here because 'translate()'
        // later will not work.
        const bool rtl = original.isRightToLeft();
        if (rtl) {
            auto r = options.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &options, options.widget);
            auto hasIcon = index.data(Qt::DecorationRole).value<QIcon>().isNull();
            if (hasIcon) {
                doc.setTextWidth(r.width() - 25);
            } else {
                doc.setTextWidth(r.width());
            }
        }

        // draw text
        painter->translate(option.rect.x(), option.rect.y());
        // leave space for icon

        if (!rtl) {
            painter->translate(25, 0);
        }

        doc.drawContents(painter);

        painter->restore();
    }

public Q_SLOTS:
    void setFilterString(const QString &text)
    {
        m_filterString = text;
    }

private:
    QString m_filterString;
};

class ShortcutStyleDelegate : public QStyledItemDelegate
{
public:
    ShortcutStyleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem options = option;
        initStyleOption(&options, index);
        painter->save();

        const auto shortcutString = index.data().toString();

        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        if (!shortcutString.isEmpty()) {
            // collect rects for each word
            QVector<QPair<QRect, QString>> btns;
            const auto list = [&shortcutString] {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                auto list = shortcutString.split(QLatin1Char('+'), QString::SkipEmptyParts);
#else
                auto list = shortcutString.split(QLatin1Char('+'), Qt::SkipEmptyParts);
#endif
                if (shortcutString.endsWith(QLatin1String("+"))) {
                    list.append(QStringLiteral("+"));
                }
                return list;
            }();
            btns.reserve(list.size());
            for (const QString &text : list) {
                QRect r = option.fontMetrics.boundingRect(text);
                r.setWidth(r.width() + 8);
                btns.append({r, text});
            }

            const auto plusRect = option.fontMetrics.boundingRect(QLatin1Char('+'));

            // draw them
            int x = option.rect.x();
            const int y = option.rect.y();
            const int plusY = option.rect.y() + plusRect.height() / 2;
            const int total = btns.size();

            // make sure our rects are nicely V-center aligned in the row
            painter->translate(QPoint(0, (option.rect.height() - btns.at(0).first.height()) / 2));

            int i = 0;
            painter->setRenderHint(QPainter::Antialiasing);
            for (const auto &btn : btns) {
                painter->setPen(Qt::NoPen);
                const QRect &rect = btn.first;

                QRect buttonRect(x, y, rect.width(), rect.height());

                // draw rounded rect shadow
                auto shadowRect = buttonRect.translated(0, 1);
                painter->setBrush(option.palette.shadow());
                painter->drawRoundedRect(shadowRect, 3, 3);

                // draw rounded rect itself
                painter->setBrush(option.palette.button());
                painter->drawRoundedRect(buttonRect, 3, 3);

                // draw text inside rounded rect
                painter->setPen(option.palette.buttonText().color());
                painter->drawText(buttonRect, Qt::AlignCenter, btn.second);

                // draw '+'
                if (i + 1 < total) {
                    x += rect.width() + 5;
                    painter->drawText(QPoint(x, plusY + (rect.height() / 2)), QString("+"));
                    x += plusRect.width() + 5;
                }
                i++;
            }
        }

        painter->restore();
    }
};

KateCommandBar::KateCommandBar(QWidget *parent)
    : QMenu(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(4, 4, 4, 4);
    setLayout(layout);

    m_lineEdit = new QLineEdit(this);
    setFocusProxy(m_lineEdit);

    layout->addWidget(m_lineEdit);

    m_treeView = new QTreeView();
    layout->addWidget(m_treeView, 1);
    m_treeView->setTextElideMode(Qt::ElideMiddle);
    m_treeView->setUniformRowHeights(true);

    m_model = new CommandModel(this);

    CommandBarStyleDelegate *delegate = new CommandBarStyleDelegate(this);
    ShortcutStyleDelegate *del = new ShortcutStyleDelegate(this);
    m_treeView->setItemDelegateForColumn(0, delegate);
    m_treeView->setItemDelegateForColumn(1, del);

    m_proxyModel = new CommandBarFilterModel(this);
    m_proxyModel->setFilterRole(Qt::DisplayRole);
    m_proxyModel->setSortRole(CommandModel::Score);
    m_proxyModel->setFilterKeyColumn(0);

    connect(m_lineEdit, &QLineEdit::returnPressed, this, &KateCommandBar::slotReturnPressed);
    connect(m_lineEdit, &QLineEdit::textChanged, m_proxyModel, &CommandBarFilterModel::setFilterString);
    connect(m_lineEdit, &QLineEdit::textChanged, delegate, &CommandBarStyleDelegate::setFilterString);
    connect(m_lineEdit, &QLineEdit::textChanged, this, [this]() {
        m_treeView->viewport()->update();
        reselectFirst();
    });
    connect(m_treeView, &QTreeView::clicked, this, &KateCommandBar::slotReturnPressed);

    m_proxyModel->setSourceModel(m_model);
    m_treeView->setSortingEnabled(true);
    m_treeView->setModel(m_proxyModel);

    m_treeView->installEventFilter(this);
    m_lineEdit->installEventFilter(this);

    m_treeView->setHeaderHidden(true);
    m_treeView->setRootIsDecorated(false);
    m_treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->setSelectionMode(QTreeView::SingleSelection);

    setHidden(true);
}

void KateCommandBar::updateBar(const QList<KActionCollection *> &actionCollections, int totalActions)
{
    qDeleteAll(m_disposableActionCollections);
    m_disposableActionCollections.clear();

    QVector<QPair<QString, QAction *>> actionList;
    actionList.reserve(totalActions);

    for (const auto collection : actionCollections) {

        if (collection->componentName().contains("disposable")) {
            m_disposableActionCollections << collection;
        }

        const QList<QAction *> collectionActions = collection->actions();
        const QString componentName = collection->componentDisplayName();
        for (const auto action : collectionActions) {
            // sanity + empty check ensures displayable actions and removes ourself
            // from the action list
            if (action && action->isEnabled() && !action->text().isEmpty()) {
                actionList.append({componentName, action});
            }
        }
    }




    m_model->refresh(std::move(actionList));
    reselectFirst();

    updateViewGeometry();
    show();
    setFocus();
}

bool KateCommandBar::eventFilter(QObject *obj, QEvent *event)
{
    // catch key presses + shortcut overrides to allow to have ESC as application wide shortcut, too, see bug 409856
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (obj == m_lineEdit) {
            const bool forward2list = (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down) || (keyEvent->key() == Qt::Key_PageUp)
                || (keyEvent->key() == Qt::Key_PageDown);
            if (forward2list) {
                QCoreApplication::sendEvent(m_treeView, event);
                return true;
            }

            if (keyEvent->key() == Qt::Key_Escape) {
                m_lineEdit->clear();
                keyEvent->accept();
                hide();
                return true;
            }
        } else {
            const bool forward2input = (keyEvent->key() != Qt::Key_Up) && (keyEvent->key() != Qt::Key_Down) && (keyEvent->key() != Qt::Key_PageUp)
                && (keyEvent->key() != Qt::Key_PageDown) && (keyEvent->key() != Qt::Key_Tab) && (keyEvent->key() != Qt::Key_Backtab);
            if (forward2input) {
                QCoreApplication::sendEvent(m_lineEdit, event);
                return true;
            }
        }
    }

    // hide on focus out, if neither input field nor list have focus!
    else if (event->type() == QEvent::FocusOut && !(m_lineEdit->hasFocus() || m_treeView->hasFocus())) {
        m_lineEdit->clear();
        hide();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

void KateCommandBar::slotReturnPressed()
{
    auto act = m_proxyModel->data(m_treeView->currentIndex(), Qt::UserRole).value<QAction *>();
    if (act) {
        // if the action is a menu, we take all its actions
        // and reload our dialog with these instead.
        if (auto menu = act->menu()) {
            auto menuActions = menu->actions();
            QVector<QPair<QString, QAction *>> list;
            list.reserve(menuActions.size());

            // if there are no actions, trigger load actions
            // this happens with some menus that are loaded on demand
            if (menuActions.size() == 0) {
                Q_EMIT menu->aboutToShow();
                menuActions = menu->actions();
            }

            for (auto menuAction : qAsConst(menuActions)) {
                if (menuAction) {
                    list.append({KLocalizedString::removeAcceleratorMarker(act->text()), menuAction});
                }
            }
            m_model->refresh(list);
            m_lineEdit->clear();
            return;
        } else {
            act->trigger();
        }
    }
    m_lineEdit->clear();
    hide();
}

void KateCommandBar::reselectFirst()
{
    QModelIndex index = m_proxyModel->index(0, 0);
    m_treeView->setCurrentIndex(index);
}

void KateCommandBar::updateViewGeometry()
{
    m_treeView->resizeColumnToContents(0);
    m_treeView->resizeColumnToContents(1);

    const QSize centralSize = parentWidget()->size();

    // width: 2.4 of editor, height: 1/2 of editor
    const QSize viewMaxSize(centralSize.width() / 2.4, centralSize.height() / 2);

    // Position should be central over window
    const int xPos = std::max(0, (centralSize.width() - viewMaxSize.width()) / 2);
    const int yPos = std::max(0, (centralSize.height() - viewMaxSize.height()) * 1 / 4);

    const QPoint p(xPos, yPos);
    move(p + parentWidget()->pos());

    this->setFixedSize(viewMaxSize);
}

/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_categorized_item_delegate.h"

#include <kdeversion.h>
#include <kcategorydrawer.h>
#include <KCategorizedSortFilterProxyModel>
#include <QPainter>
#include <QApplication>

#if KDE_IS_VERSION(4,5,0)
    class KisCategoryDrawer : public KCategoryDrawerV3
#else
    class KisCategoryDrawer : public KCategoryDrawer
#endif
{
public:
#if KDE_IS_VERSION(4,5,0)
    KisCategoryDrawer(KCategorizedView *view = 0)
        : KCategoryDrawerV3(view)
#else
        KisCategoryDrawer()
            : KCategoryDrawer()
#endif
    {
    }

    virtual void drawCategory ( const QModelIndex& index, int /*sortRole*/, const QStyleOption& option, QPainter* painter ) const
    {
        painter->setRenderHint(QPainter::Antialiasing);
        const QString category = index.model()->data(index, KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString();

        QLinearGradient gradient(option.rect.topLeft(), option.rect.bottomLeft());
        if (index.row() != 0) {
            gradient.setColorAt(0, Qt::transparent);
        }
        gradient.setColorAt(0.3, option.palette.background().color());
        gradient.setColorAt(0.8, option.palette.background().color());
        gradient.setColorAt(1, Qt::transparent);
        painter->fillRect(option.rect, gradient);

        QFont font(QApplication::font());
        font.setBold(true);
        const QFontMetrics fontMetrics = QFontMetrics(font);

        QRect textRect = option.rect.adjusted(5, 0, 0, 0);

        painter->save();
        painter->setFont(font);
        painter->setPen(option.palette.text().color());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, category);
        painter->restore();
    }
};

struct KisCategorizedItemDelegate::Private {
    QAbstractItemDelegate* fallback;
    KisCategoryDrawer* categoryDrawer;
    bool isFirstOfCategory(const QModelIndex& index);
};

bool KisCategorizedItemDelegate::Private::isFirstOfCategory(const QModelIndex& index)
{

    if (index.row() == 0) return true;
    QModelIndex idx = index.model()->index(index.row() - 1, index.column(), index.parent());
    const QString category1 = index.model()->data(index, KCategorizedSortFilterProxyModel::CategorySortRole).toString();
    const QString category2 = index.model()->data(idx, KCategorizedSortFilterProxyModel::CategorySortRole).toString();
    return category1 != category2;
}

KisCategorizedItemDelegate::KisCategorizedItemDelegate(QAbstractItemDelegate* _fallback, QObject* parent)
        : QAbstractItemDelegate(parent)
        , d(new Private)
{
    _fallback->setParent(this);
    d->fallback = _fallback;
    d->categoryDrawer = new KisCategoryDrawer;
}

KisCategorizedItemDelegate::~KisCategorizedItemDelegate()
{

    delete d->fallback;
    delete d->categoryDrawer;
    delete d;
}

QWidget * KisCategorizedItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return d->fallback->createEditor(parent, option, index);
}

bool KisCategorizedItemDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    return d->fallback->editorEvent(event, model, option, index);
}

void KisCategorizedItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & _option, const QModelIndex & index) const
{
    // We will need to edit the option to make sure the header isn't drawned as selected
    QStyleOptionViewItem* option = 0;
    if (const QStyleOptionViewItemV4 *v4 = qstyleoption_cast<const QStyleOptionViewItemV4*>(&_option)) {
        option = new QStyleOptionViewItemV4(*v4);
    } else if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3*>(&_option)) {
        option = new QStyleOptionViewItemV3(*v3);
    } else if (const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2*>(&_option)) {
        option = new QStyleOptionViewItemV2(*v2);
    } else {
        option = new QStyleOptionViewItem(_option);
    }
    Q_ASSERT(option);
    // If it's a first category then we need to draw it
    if (d->isFirstOfCategory(index)) {
        // Prepare the rectangle for drawing the category
        int h = d->categoryDrawer->categoryHeight(index, *option);
        QRect rect = option->rect;

        // Make sure the categroy isn't drawned as selected
        option->state &= (~QStyle::State_Selected);
        Q_ASSERT(!(option->state & QStyle::State_Selected));
        option->state &= (~QStyle::State_HasFocus);
        Q_ASSERT(!(option->state & QStyle::State_HasFocus));
        option->state &= (~QStyle::State_MouseOver);
        Q_ASSERT(!(option->state & QStyle::State_MouseOver));
        option->rect.setHeight(h);

        // draw the cateogry
        d->categoryDrawer->drawCategory(index, 0, *option, painter);

        // Prepare the rectangle for the item
        option->rect = rect;
        option->rect.setY(rect.y() + h);
        option->rect.setHeight(rect.height() - h);
        option->state = _option.state;
    }
    d->fallback->paint(painter, *option, index);
    delete option;
}

void KisCategorizedItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    d->fallback->setEditorData(editor, index);
}

void KisCategorizedItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    d->fallback->setModelData(editor, model, index);
}

QSize KisCategorizedItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QSize size = d->fallback->sizeHint(option, index);
    // If is first of a category, then add the space needed to paint the category
    if (d->isFirstOfCategory(index)) {
        size.setHeight(d->categoryDrawer->categoryHeight(index, option) + size.height());
    }
    return size;
}

void KisCategorizedItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    d->fallback->updateEditorGeometry(editor, option, index);

    // If it's the first category, then the editor need to be moved
    if (d->isFirstOfCategory(index)) {
        int h = d->categoryDrawer->categoryHeight(index, option);
        editor->move(editor->x(), editor->y() + h);
        editor->resize(editor->width(), editor->height() - h);
    }
}

/*
 *  Copyright (C) 2008 Jeremy Whiting <jpwhiting@kde.org>
 *  Copyright (C) 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *  Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>
 *  Copyright (c) 2017 Aniketh Girish anikethgireesh@gmail.com
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

#include "itemsgridviewdelegate_p.h"

#include <QPainter>
#include <QSortFilterProxyModel>
#include <QApplication>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QHBoxLayout>
#include <QAbstractItemView>

#include <KFormat>
#include <KLocalizedString>
#include <KRatingWidget>
#include <KSqueezedTextLabel>

#include <KNSCore/ItemsModel>

enum { DelegateTitleLabel, DelegateAuthorLabel, DelegateDownloadCounterLabel,
       DelegateGridRatingWidget
     };

ItemsGridViewDelegate::ItemsGridViewDelegate(QAbstractItemView *itemView, KNSCore::Engine *engine, QObject *parent)
    : ItemsViewBaseDelegate(itemView, engine, parent)
    , m_elementYPos(0)
{
    createOperationBar();
}

ItemsGridViewDelegate::~ItemsGridViewDelegate()
{
}

QList<QWidget *> ItemsGridViewDelegate::createItemWidgets(const QModelIndex &index) const
{
    Q_UNUSED(index)

    QList<QWidget *> m_widgetList;
    KSqueezedTextLabel *titleLabel = new KSqueezedTextLabel();
    titleLabel->setOpenExternalLinks(true);
    titleLabel->setTextElideMode(Qt::ElideRight);
    // not so nice - work around constness to install the event filter
    ItemsGridViewDelegate *delegate = const_cast<ItemsGridViewDelegate *>(this);
    titleLabel->installEventFilter(delegate);
    m_widgetList << titleLabel;

    KSqueezedTextLabel *authorLabel = new KSqueezedTextLabel();
    authorLabel->setTextElideMode(Qt::ElideRight);
    m_widgetList << authorLabel;

    KSqueezedTextLabel *downloadCounterLabel = new KSqueezedTextLabel();
    downloadCounterLabel->setTextElideMode(Qt::ElideRight);
    m_widgetList << downloadCounterLabel;

    KRatingWidget *rating = new KRatingWidget();
    rating->setMaxRating(10);
    rating->setHalfStepsEnabled(true);
    m_widgetList << rating;

    return m_widgetList;
}

void ItemsGridViewDelegate::updateItemWidgets(const QList<QWidget *> widgets,
        const QStyleOptionViewItem &option,
        const QPersistentModelIndex &index) const
{
    const KNSCore::ItemsModel *model = qobject_cast<const KNSCore::ItemsModel *>(index.model());
    if (!model) {
        return;
    }

    KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
    int elementYPos = KNSCore::PreviewHeight + ItemMargin + FrameThickness * 2;

    //setup rating widget
    KRatingWidget *rating = qobject_cast<KRatingWidget *>(widgets.at(DelegateGridRatingWidget));
    if (rating) {
        if (entry.rating() > 0) {
            rating->setToolTip(i18n("Rating: %1%", entry.rating()));
            // assume all entries come with rating 0..100 but most are in the range 20 - 80, so 20 is 0 stars, 80 is 5 stars
            rating->setRating((entry.rating() - 20) * 10 / 60);
            //make the rating widget smaller than the one at list view
            int newWidth = 68;
            QSize size(newWidth, 15);
            rating->resize(size);
            //put rating widget under image rectangle
            rating->move((ItemGridWidth - newWidth) / 2, elementYPos);
            elementYPos += rating->height();
        } else {
            //is it better to stay visible?
            rating->setVisible(false);
        }
    }
    elementYPos += ItemMargin;

    //setup title label
    QLabel *titleLabel = qobject_cast<QLabel *>(widgets.at(DelegateTitleLabel));
    if (titleLabel != nullptr) {
        titleLabel->setWordWrap(true);
        titleLabel->setAlignment(Qt::AlignHCenter);
        //titleLabel->setFrameStyle(QFrame::Panel);
        titleLabel->resize(QSize(option.rect.width() - (ItemMargin * 2), option.fontMetrics.height() * 2));
        titleLabel->move((ItemGridWidth - titleLabel->width()) / 2, elementYPos);

        QString title;
        QUrl link = qvariant_cast<QUrl>(entry.homepage());
        if (!link.isEmpty()) {
            title += "<b><a href=\"" + link.url() + "\">" + entry.name() + "</a></b>\n";
        } else {
            title += "<b>" + entry.name() + "</b>";
        }

        const auto downloadInfo = entry.downloadLinkInformationList();
        if (!downloadInfo.isEmpty() && downloadInfo.at(0).size > 0) {
            QString sizeString = KFormat().formatByteSize(downloadInfo.at(0).size * 1000);
            title += i18nc("Show the size of the file in a list", "<br />Size: %1", sizeString);
        }

        titleLabel->setText(title);
        elementYPos += titleLabel->height();
    }
    //setup author label
    QLabel *authorLabel = qobject_cast<QLabel *>(widgets.at(DelegateAuthorLabel));
    if (authorLabel != nullptr) {
        authorLabel->setWordWrap(true);
        authorLabel->setAlignment(Qt::AlignHCenter);
        authorLabel->resize(QSize(option.rect.width() - (ItemMargin * 2), option.fontMetrics.height()));
        authorLabel->move((ItemGridWidth - authorLabel->width()) / 2, elementYPos);

        QString text;
        QString authorName = entry.author().name();
        QString email = entry.author().email();
        QString authorPage = entry.author().homepage();

        if (!authorName.isEmpty()) {
            if (!authorPage.isEmpty()) {
                text += "<p>" + i18nc("Show the author of this item in a list", "By <i>%1</i>", " <a href=\"" + authorPage + "\">" + authorName + "</a>") + "</p>\n";
            } else if (!email.isEmpty()) {
                text += "<p>" + i18nc("Show the author of this item in a list", "By <i>%1</i>", authorName) + " <a href=\"mailto:" + email + "\">" + email + "</a></p>\n";
            } else {
                text += "<p>" + i18nc("Show the author of this item in a list", "By <i>%1</i>", authorName) + "</p>\n";
            }
        }
        authorLabel->setText(text);
        elementYPos += authorLabel->height();
    }
    elementYPos += ItemMargin;

    //setup download label
    QLabel *downloadLabel = qobject_cast<QLabel *>(widgets.at(DelegateDownloadCounterLabel));
    if (downloadLabel != nullptr) {
        downloadLabel->setWordWrap(true);
        downloadLabel->setAlignment(Qt::AlignHCenter);
        downloadLabel->resize(QSize(option.rect.width() - (ItemMargin * 2), option.fontMetrics.height()));
        downloadLabel->move((ItemGridWidth - downloadLabel->width()) / 2, elementYPos);

        unsigned int fans = entry.numberFans();
        unsigned int downloads = entry.downloadCount();

        QString text;
        QString fanString;
        QString downloadString;
        if (fans > 0) {
            fanString = i18ncp("fan as in supporter", "1 fan", "%1 fans", fans);
        }
        if (downloads > 0) {
            downloadString = i18np("1 download", "%1 downloads", downloads);
        }
        if (downloads > 0 || fans > 0) {
            text += "<p>" + downloadString;
            if (downloads > 0 && fans > 0) {
                text += QLatin1String(", ");
            }
            text += fanString + QLatin1String("</p>");
        }
        downloadLabel->setText(text);
        elementYPos += downloadLabel->height();
    }
    elementYPos += ItemMargin;
    m_elementYPos = elementYPos;
}

void ItemsGridViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_MouseOver) {
        QModelIndex focIndex = focusedIndex();
        if (m_oldIndex != focIndex || !m_operationBar->isVisible()) {
            ItemsGridViewDelegate *delegate = const_cast<ItemsGridViewDelegate *>(this);

            delegate->displayOperationBar(option.rect, index);
            delegate->m_oldIndex = focIndex;
        }
    } else {
        QModelIndex focindex = focusedIndex();
        if (!focindex.isValid()) {
            m_operationBar->hide();
        }
    }

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);

    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlightedText().color()));
    } else {
        painter->setPen(QPen(option.palette.text().color()));
    }

    const KNSCore::ItemsModel *realmodel = qobject_cast<const KNSCore::ItemsModel *>(index.model());

    if (realmodel->hasPreviewImages()) {
        int width = option.rect.width();

        KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
        if (entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1).isEmpty()) {
            ;
        } else {
            QPoint centralPoint(option.rect.left() + width / 2, option.rect.top() + ItemMargin + FrameThickness + KNSCore::PreviewHeight / 2);
            QImage image = entry.previewImage(KNSCore::EntryInternal::PreviewSmall1);
            if (!image.isNull()) {
                QPoint previewPoint(centralPoint.x() - image.width() / 2, centralPoint.y() - image.height() / 2);
                painter->drawImage(previewPoint, image);

                QPixmap frameImageScaled = m_frameImage.scaled(image.width() + FrameThickness * 2, image.height() + FrameThickness * 2);
                QPoint framePoint(centralPoint.x() - frameImageScaled.width() / 2, centralPoint.y() - frameImageScaled.height() / 2);
                painter->drawPixmap(framePoint, frameImageScaled);
            } else {
                QPoint thumbnailPoint(option.rect.left() + ((width - KNSCore::PreviewWidth - FrameThickness * 2) / 2), option.rect.top() + ItemMargin);
                QRect rect(thumbnailPoint, QSize(KNSCore::PreviewWidth + FrameThickness * 2, KNSCore::PreviewHeight + FrameThickness * 2));
                painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, i18n("Loading Preview"));
            }
        }
    }

    painter->restore();
}

QSize ItemsGridViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QSize size;

    size.setWidth(ItemGridWidth);
    size.setHeight(qMax(option.fontMetrics.height() * 13, ItemGridHeight)); // up to 6 lines of text, and two margins
    return size;
}

void ItemsGridViewDelegate::createOperationBar()
{
    m_operationBar = new QWidget(this->itemView()->viewport());

    m_detailsButton = new QToolButton();
    m_detailsButton->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    m_detailsButton->setPopupMode(QToolButton::InstantPopup);
    m_detailsButton->setToolTip(i18n("Details"));
    m_detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("documentinfo")));
    setBlockedEventTypes(m_detailsButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(m_detailsButton, &QToolButton::clicked,
            this, static_cast<void(ItemsGridViewDelegate::*)()>(&ItemsGridViewDelegate::slotDetailsClicked));

    m_installButton = new QToolButton();
    m_installButton->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    m_installButton->setPopupMode(QToolButton::InstantPopup);

    setBlockedEventTypes(m_installButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(m_installButton, &QAbstractButton::clicked, this, &ItemsGridViewDelegate::slotInstallClicked);
    connect(m_installButton, &QToolButton::triggered, this, &ItemsGridViewDelegate::slotInstallActionTriggered);

    if (m_installButton->menu()) {
        QMenu *buttonMenu = m_installButton->menu();
        buttonMenu->clear();
        m_installButton->setMenu(nullptr);
        buttonMenu->deleteLater();
    }

    QHBoxLayout *layout = new QHBoxLayout(m_operationBar);

    layout->setSpacing(1);
    layout->addWidget(m_installButton);
    layout->addWidget(m_detailsButton);

    m_operationBar->adjustSize();
    m_operationBar->hide();
}

void ItemsGridViewDelegate::displayOperationBar(const QRect &rect, const QModelIndex &index)
{
    KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
    if (m_installButton != nullptr) {
        if (m_installButton->menu() != nullptr) {
            QMenu *buttonMenu = m_installButton->menu();
            buttonMenu->clear();
            m_installButton->setMenu(nullptr);
            buttonMenu->deleteLater();
        }

        bool installable = false;
        bool enabled = true;
        QString text;
        QIcon icon;

        switch (entry.status()) {
        case KNS3::Entry::Installed:
            text = i18n("Uninstall");
            icon = m_iconDelete;
            break;
        case KNS3::Entry::Updateable:
            text = i18n("Update");
            icon = m_iconUpdate;
            installable = true;
            break;
        case KNS3::Entry::Installing:
            text = i18n("Installing");
            enabled = false;
            icon = m_iconUpdate;
            break;
        case KNS3::Entry::Updating:
            text = i18n("Updating");
            enabled = false;
            icon = m_iconUpdate;
            break;
        case KNS3::Entry::Downloadable:
            text = i18n("Install");
            icon = m_iconInstall;
            installable = true;
            break;
        case KNS3::Entry::Deleted:
            text = i18n("Install Again");
            icon = m_iconInstall;
            installable = true;
            break;
        default:
            text = i18n("Install");
        }
        m_installButton->setToolTip(text);
        m_installButton->setIcon(icon);
        m_installButton->setEnabled(enabled);
        if (installable && entry.downloadLinkCount() > 1) {
            QMenu *installMenu = new QMenu(m_installButton);
            foreach (const KNSCore::EntryInternal::DownloadLinkInformation &info, entry.downloadLinkInformationList()) {
                QString text = info.name;
                if (!info.distributionType.trimmed().isEmpty()) {
                    text + " (" + info.distributionType.trimmed() + ')';
                }
                QAction *installAction = installMenu->addAction(m_iconInstall, text);
                installAction->setData(QPoint(index.row(), info.id));
            }
            m_installButton->setMenu(installMenu);
        }

        m_operationBar->move(rect.left() + (ItemGridWidth - m_operationBar->width()) / 2, rect.top() + m_elementYPos);
        m_operationBar->show();
    }
}

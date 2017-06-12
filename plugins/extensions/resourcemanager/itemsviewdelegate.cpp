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

#include "itemsviewdelegate_p.h"

#include <QPainter>
#include <QSortFilterProxyModel>
#include <QApplication>
#include <QToolButton>
#include <QMenu>
#include <QProcess>

#include <kformat.h>
#include <klocalizedstring.h>
#include <kratingwidget.h>
#include <KShell>

#include <KNSCore/ItemsModel>

#include "entrydetailsdialog_p.h"

enum { DelegateLabel, DelegateInstallButton, DelegateDetailsButton,  DelegateRatingWidget };

ItemsViewDelegate::ItemsViewDelegate(QAbstractItemView *itemView, KNSCore::Engine *engine, QObject *parent)
    : ItemsViewBaseDelegate(itemView, engine, parent)
{
}

ItemsViewDelegate::~ItemsViewDelegate()
{
}

QList<QWidget *> ItemsViewDelegate::createItemWidgets(const QModelIndex &index) const
{
    Q_UNUSED(index);
    QList<QWidget *> list;

    QLabel *infoLabel = new QLabel();
    infoLabel->setOpenExternalLinks(true);
    // not so nice - work around constness to install the event filter
    ItemsViewDelegate *delegate = const_cast<ItemsViewDelegate *>(this);
    infoLabel->installEventFilter(delegate);
    list << infoLabel;

    QToolButton *installButton = new QToolButton();
    installButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    list << installButton;
    setBlockedEventTypes(installButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(installButton, &QAbstractButton::clicked, this, &ItemsViewDelegate::slotInstallClicked);
    connect(installButton, &QToolButton::triggered, this, &ItemsViewDelegate::slotInstallActionTriggered);

    QToolButton *detailsButton = new QToolButton();
    detailsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    list << detailsButton;
    setBlockedEventTypes(detailsButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(detailsButton, &QToolButton::clicked,
            this, static_cast<void(ItemsViewDelegate::*)()>(&ItemsViewDelegate::slotDetailsClicked));

    KRatingWidget *rating = new KRatingWidget();
    rating->setMaxRating(10);
    rating->setHalfStepsEnabled(true);
    list << rating;
    //connect(rating, SIGNAL(ratingChanged(uint)), this, SLOT());

    return list;
}

void ItemsViewDelegate::updateItemWidgets(const QList<QWidget *> widgets,
        const QStyleOptionViewItem &option,
        const QPersistentModelIndex &index) const
{
    const KNSCore::ItemsModel *model = qobject_cast<const KNSCore::ItemsModel *>(index.model());
    if (!model) {
        return;
    }

    const KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();

    // setup the install button
    int margin = option.fontMetrics.height() / 2;
    int right = option.rect.width();

    QToolButton *installButton = qobject_cast<QToolButton *>(widgets.at(DelegateInstallButton));
    if (installButton != nullptr) {

        if (installButton->menu()) {
            QMenu *buttonMenu = installButton->menu();
            buttonMenu->clear();
            installButton->setMenu(nullptr);
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
        installButton->setText(text);
        installButton->setEnabled(enabled);
        installButton->setIcon(icon);
        installButton->setPopupMode(QToolButton::InstantPopup);

        if (installable && entry.downloadLinkCount() > 1) {
            QMenu *installMenu = new QMenu(installButton);
            foreach (const KNSCore::EntryInternal::DownloadLinkInformation &info, entry.downloadLinkInformationList()) {
                QString text = info.name;
                if (!info.distributionType.trimmed().isEmpty()) {
                    text += " (" + info.distributionType.trimmed() + ')';
                }
                QAction *installAction = installMenu->addAction(m_iconInstall, text);
                installAction->setData(QPoint(index.row(), info.id));
            }
            installButton->setMenu(installMenu);
        } else if (entry.status() == KNS3::Entry::Installed && m_engine->hasAdoptionCommand()) {
            QMenu* m = new QMenu(installButton);
            m->addAction(i18n("Use"), m, [this, entry](){
                QStringList args = KShell::splitArgs(m_engine->adoptionCommand(entry));
                QProcess::startDetached(args.takeFirst(), args);
            });
            installButton->setPopupMode(QToolButton::MenuButtonPopup);
            installButton->setMenu(m);
        }
    }

    QToolButton *detailsButton = qobject_cast<QToolButton *>(widgets.at(DelegateDetailsButton));
    if (detailsButton) {
        detailsButton->setText(i18n("Details"));
        detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("documentinfo")));
    }

    if (installButton && detailsButton) {
        if (m_buttonSize.width() < installButton->sizeHint().width()) {
            const_cast<QSize &>(m_buttonSize) = QSize(
                                                    qMax(option.fontMetrics.height() * 7,
                                                            qMax(installButton->sizeHint().width(), detailsButton->sizeHint().width())),
                                                    installButton->sizeHint().height());
        }
        installButton->resize(m_buttonSize);
        installButton->move(right - installButton->width() - margin, option.rect.height() / 2 - installButton->height() * 1.5);
        detailsButton->resize(m_buttonSize);
        detailsButton->move(right - installButton->width() - margin, option.rect.height() / 2 - installButton->height() / 2);
    }

    QLabel *infoLabel = qobject_cast<QLabel *>(widgets.at(DelegateLabel));
    infoLabel->setWordWrap(true);
    if (infoLabel != nullptr) {
        if (model->hasPreviewImages()) {
            // move the text right by kPreviewWidth + margin pixels to fit the preview
            infoLabel->move(KNSCore::PreviewWidth + margin * 2, 0);
            infoLabel->resize(QSize(option.rect.width() - KNSCore::PreviewWidth - (margin * 6) - m_buttonSize.width(), option.fontMetrics.height() * 7));

        } else {
            infoLabel->move(margin, 0);
            infoLabel->resize(QSize(option.rect.width() - (margin * 4) - m_buttonSize.width(), option.fontMetrics.height() * 7));
        }

        QString text = QLatin1String("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                       "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; margin:0 0 0 0;}\n"
                       "</style></head><body><p><b>");

        QUrl link = qvariant_cast<QUrl>(entry.homepage());
        if (!link.isEmpty()) {
            text += "<p><a href=\"" + link.url() + "\">" + entry.name() + "</a></p>\n";
        } else {
            text += entry.name();
        }

        const auto downloadInfo = entry.downloadLinkInformationList();
        if (!downloadInfo.isEmpty() && downloadInfo.at(0).size > 0) {
            QString sizeString = KFormat().formatByteSize(downloadInfo.at(0).size * 1000);
            text += i18nc("Show the size of the file in a list", "<p>Size: %1</p>", sizeString);
        }

        text += QLatin1String("</b></p>\n");

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

        QString summary = "<p>" + option.fontMetrics.elidedText(entry.summary(),
                          Qt::ElideRight, infoLabel->width() * 3) + "</p>\n";
        text += summary;

        unsigned int downloads = entry.downloadCount();

        QString downloadString;

        if (downloads > 0) {
            downloadString = i18np("1 download", "%1 downloads", downloads);
        }
        if (downloads > 0) {
            text += "<p>" + downloadString;
            if (downloads > 0) {
                text += QLatin1String(", ");
            }
        }

        text += QLatin1String("</body></html>");
        // use simplified to get rid of newlines etc
        text = KNSCore::replaceBBCode(text).simplified();
        infoLabel->setText(text);
    }

    KRatingWidget *rating = qobject_cast<KRatingWidget *>(widgets.at(DelegateRatingWidget));
    if (rating) {
        if (entry.rating() > 0) {
            rating->setToolTip(i18n("Rating: %1%", entry.rating()));
            // assume all entries come with rating 0..100 but most are in the range 20 - 80, so 20 is 0 stars, 80 is 5 stars
            rating->setRating((entry.rating() - 20) * 10 / 60);
            // put the rating label below the install button
            rating->move(right - installButton->width() - margin, option.rect.height() / 2 + installButton->height() / 2);
            rating->resize(m_buttonSize);
        } else {
            rating->setVisible(false);
        }
    }
}

// draws the preview
void ItemsViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int margin = option.fontMetrics.height() / 2;

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
        int height = option.rect.height();
        QPoint point(option.rect.left() + margin, option.rect.top() + ((height - KNSCore::PreviewHeight) / 2));

        KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
        if (entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1).isEmpty()) {
            // paint the no preview icon
            //point.setX((PreviewWidth - m_noImage.width())/2 + 5);
            //point.setY(option.rect.top() + ((height - m_noImage.height()) / 2));
            //painter->drawPixmap(point, m_noImage);
        } else {
            QImage image = entry.previewImage(KNSCore::EntryInternal::PreviewSmall1);
            if (!image.isNull()) {
                point.setX((KNSCore::PreviewWidth - image.width()) / 2 + 5);
                point.setY(option.rect.top() + ((height - image.height()) / 2));
                painter->drawImage(point, image);

                QPoint framePoint(point.x() - 5, point.y() - 5);
                painter->drawPixmap(framePoint, m_frameImage.scaled(image.width() + 10, image.height() + 10));
            } else {
                QRect rect(point, QSize(KNSCore::PreviewWidth, KNSCore::PreviewHeight));
                painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, i18n("Loading Preview"));
            }
        }

    }
    painter->restore();
}

QSize ItemsViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QSize size;

    size.setWidth(option.fontMetrics.height() * 4);
    size.setHeight(qMax(option.fontMetrics.height() * 7, KNSCore::PreviewHeight)); // up to 6 lines of text, and two margins
    return size;
}

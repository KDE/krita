/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_splash_screen.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QCheckBox>
#include <QDebug>
#include <QFile>

#include <KisPart.h>
#include <KisApplication.h>


#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>

#include <kis_factory2.h>

KisSplashScreen::KisSplashScreen(const QString &version, const QPixmap &pixmap, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, Qt::SplashScreen | Qt::FramelessWindowHint | f)
{
    setupUi(this);
    setWindowIcon(KIcon(KGlobal::mainComponent().aboutData()->programIconName()));

    lblSplash->setPixmap(pixmap);
    bnClose->hide();
    connect(bnClose, SIGNAL(clicked()), this, SLOT(close()));
    chkShowAtStartup->hide();
    connect(chkShowAtStartup, SIGNAL(toggled(bool)), this, SLOT(toggleShowAtStartup(bool)));

    KConfigGroup cfg(KisFactory::componentData().config(), "SplashScreen");
    bool hideSplash = cfg.readEntry("HideSplashAfterStartup", false);
    chkShowAtStartup->setChecked(hideSplash);

    lblLinks->setFixedWidth(pixmap.width());
    lblLinks->setWordWrap(true);
    lblLinks->setTextFormat(Qt::RichText);
    lblLinks->setText(i18n("<html>"
                           "<head/>"
                           "<body>"

                           "<p align=\"center\"><b><a href=\"https://krita.org/kickstarter/\"><span style=\" text-decoration: underline; color:#FFFFFF;\">Support Krita's Kickstarter!</span></a></b></p><p/>"

                           "<p>During May, Krita is running a kickstarter campaign. We're crowdfunding performance improvements, animation support and a host of exciting stretch goals. Help us make Krita even better!</p>"

                           "</body>"
                           "</html>"));

    lblVersion->setText(i18n("Version: %1", version));

//    KConfigGroup cfg2(KisFactory::componentData().config(), "RecentFiles");
//    int i = 1;

//    QString recent = i18n("<html>"
//                          "<head/>"
//                          "<body>"
//                          "<p align=\"center\"><b>Recent Files</b></p>");

//    QString path;
//    QStringList recentfiles;

//    do {
//        path = cfg2.readPathEntry(QString("File%1").arg(i), QString());
//        if (!path.isEmpty()) {
//            QString name = cfg2.readPathEntry(QString("Name%1").arg(i), QString());
//            KUrl url(path);
//            if (name.isEmpty())
//                name = url.fileName();

//            if (!url.isLocalFile() || QFile::exists(url.toLocalFile())) {
//                recentfiles.insert(0, QString("<p><a href=\"%1\"><span style=\"color:#FFFFFF;\">%2</span></a></p>").arg(path).arg(name));
//            }
//        }

//        i++;
//    } while (!path.isEmpty() || i <= 8);

//    recent += recentfiles.join("\n");
//    recent += "</body>"
//            "</html>";
//    lblRecent->setText(recent);
//    lblRecent->hide();

//    connect(lblRecent, SIGNAL(linkActivated(QString)), SLOT(linkClicked(QString)));
}


void KisSplashScreen::repaint()
{
    QWidget::repaint();
    QApplication::flush();
}

void KisSplashScreen::show()
{
    QRect r(QPoint(), sizeHint());
    resize(r.size());
    move(QApplication::desktop()->availableGeometry().center() - r.center());
    if (isVisible()) {
        repaint();
    }

}

void KisSplashScreen::toggleShowAtStartup(bool toggle)
{
    KConfigGroup cfg(KGlobal::config(), "SplashScreen");
    cfg.writeEntry("HideSplashAfterStartup", toggle);
}

void KisSplashScreen::linkClicked(const QString &link)
{
    KisPart::instance()->openExistingFile(KUrl(link));
    if (isTopLevel()) {
        close();
    }
}

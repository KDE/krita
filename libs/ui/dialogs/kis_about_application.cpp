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
#include "kis_about_application.h"

#include <kis_debug.h>

#include <QStandardPaths>
#include <QTabWidget>
#include <QLabel>
#include <QTextEdit>
#include <QTextBrowser>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDate>
#include <QApplication>
#include <QFile>
#include <QDesktopServices>

#include <klocalizedstring.h>

#include "../../krita/data/splash/splash_screen.xpm"
#include "../../krita/data/splash/splash_holidays.xpm"
#include "../../krita/data/splash/splash_screen_x2.xpm"
#include "../../krita/data/splash/splash_holidays_x2.xpm"

#include "kis_splash_screen.h"

KisAboutApplication::KisAboutApplication(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("About Krita"));

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    QTabWidget *wdgTab = new QTabWidget;
    vlayout->addWidget(wdgTab);
    KisSplashScreen *splash = 0;

    QDate currentDate = QDate::currentDate();
    if (currentDate > QDate(currentDate.year(), 12, 4) ||
            currentDate < QDate(currentDate.year(), 1, 9)) {
        splash = new KisSplashScreen(qApp->applicationVersion(), QPixmap(splash_holidays_xpm), QPixmap(splash_holidays_x2_xpm));
    }
    else {
        splash = new KisSplashScreen(qApp->applicationVersion(), QPixmap(splash_screen_xpm), QPixmap(splash_screen_x2_xpm));
    }

    splash->setWindowFlags(Qt::Widget);
    splash->displayLinks(true);
    splash->setFixedSize(splash->sizeHint());

    wdgTab->addTab(splash, i18n("About"));
    setMinimumSize(wdgTab->sizeHint());


    QTextEdit *lblAuthors = new QTextEdit();
    lblAuthors->setReadOnly(true);

    QString authors = i18n("<html>"
                          "<head/>"
                          "<body>"
                          "<h1 align=\"center\">Created By</h1></p>"
                          "<p>");

    QFile fileDevelopers(":/developers.txt");
    Q_ASSERT(fileDevelopers.exists());
    fileDevelopers.open(QIODevice::ReadOnly);

    Q_FOREACH (const QByteArray &author, fileDevelopers.readAll().split('\n')) {
        authors.append(QString::fromUtf8(author));
        authors.append(", ");
    }
    authors.chop(2);
    authors.append(".</p></body></html>");
    lblAuthors->setText(authors);
    wdgTab->addTab(lblAuthors, i18nc("Heading for the list of Krita authors/developers", "Authors"));

    QTextEdit *lblKickstarter = new QTextEdit();
    lblKickstarter->setReadOnly(true);

    QString backers = i18n("<html>"
                          "<head/>"
                          "<body>"
                          "<h1 align=\"center\">Backed By</h1>"
                          "<p>");

    QFile fileBackers(":/backers.txt");
    Q_ASSERT(fileBackers.exists());
    fileBackers.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream backersText(&fileBackers);
    backersText.setCodec("UTF-8");
    backers.append(backersText.readAll().replace("\n", ", "));
    backers.chop(2);
    backers.append(i18n(".</p><p><i>Thanks! You were all <b>awesome</b>!</i></p></body></html>"));
    lblKickstarter->setText(backers);
    wdgTab->addTab(lblKickstarter, i18n("Backers"));



    QTextEdit *lblCredits = new QTextEdit();
    lblCredits->setReadOnly(true);
    QString credits = i18n("<html>"
                          "<head/>"
                          "<body>"
                          "<h1 align=\"center\">Thanks To</h1>"
                          "<p>");

    QFile fileCredits(":/credits.txt");
    Q_ASSERT(fileCredits.exists());
    fileCredits.open(QIODevice::ReadOnly);

    Q_FOREACH (const QString &credit, QString::fromUtf8(fileCredits.readAll()).split('\n', QString::SkipEmptyParts)) {
        if (credit.contains(":")) {
            QList<QString> creditSplit = credit.split(':');
            credits.append(creditSplit.at(0));
            credits.append(" (<i>" + creditSplit.at(1) + "</i>)");
            credits.append(", ");
        }
    }
    credits.chop(2);
    credits.append(i18n(".</p><p><i>For supporting Krita development with advice, icons, brush sets and more.</i></p></body></html>"));

    lblCredits->setText(credits);
    wdgTab->addTab(lblCredits, i18n("Also Thanks To"));

    QTextEdit *lblLicense = new QTextEdit();
    lblLicense->setReadOnly(true);
    QString license = i18n("<html>"
                           "<head/>"
                           "<body>"
                           "<h1 align=\"center\"><b>Your Rights</b></h1>"
                           "<p>Krita is released under the GNU General Public License (version 3 or any later version).</p>"
                           "<p>This license grants people a number of freedoms:</p>"
                           "<ul>"
                           "<li>You are free to use Krita, for any purpose</li>"
                           "<li>You are free to distribute Krita</li>"
                           "<li>You can study how Krita works and change it</li>"
                           "<li>You can distribute changed versions of Krita</li>"
                           "</ul>"
                           "<p>The Krita Foundation and its projects on krita.org are <b>committed</b> to preserving Krita as free software.</p>"
                           "<h1 align=\"center\">Your artwork</h1>"
                           "<p>What you create with Krita is your sole property. All your artwork is free for you to use as you like.</p>"
                           "<p>That means that Krita can be used commercially, for any purpose. There are no restrictions whatsoever.</p>"
                           "<p>Krita’s GNU GPL license guarantees you this freedom. Nobody is ever permitted to take it away, in contrast "
                           "to trial or educational versions of commercial software that will forbid your work in commercial situations.</p>"
                           "<br/><hr/><pre>");

    QFile licenseFile(":/LICENSE");
    Q_ASSERT(licenseFile.exists());
    licenseFile.open(QIODevice::ReadOnly);
    QByteArray ba = licenseFile.readAll();
    license.append(QString::fromUtf8(ba));
    license.append("</pre></body></html>");
    lblLicense->setText(license);

    wdgTab->addTab(lblLicense, i18n("License"));

    QTextBrowser *lblThirdParty = new QTextBrowser();
    lblThirdParty->setOpenExternalLinks(true);
    QFile thirdPartyFile(":/libraries.txt");
    if (thirdPartyFile.open(QIODevice::ReadOnly)) {
        ba = thirdPartyFile.readAll();

        QString thirdPartyHtml = i18n("<html>"
                                      "<head/>"
                                      "<body>"
                                      "<h1 align=\"center\"><b>Third-party Libraries used by Krita</b></h1>"
                                      "<p>Krita is built on the following free software libraries:</p><p><ul>");


        Q_FOREACH(const QString &lib, QString::fromUtf8(ba).split('\n')) {
            if (!lib.startsWith("#")) {
                QStringList parts = lib.split(',');
                if (parts.size() >= 3) {
                    thirdPartyHtml.append(QString("<li><a href=\"%2\">%1</a>: %3</li>").arg(parts[0], parts[1], parts[2]));
                }
            }
        }
        thirdPartyHtml.append("<ul></p></body></html>");
        lblThirdParty->setText(thirdPartyHtml);
    }
    wdgTab->addTab(lblThirdParty, i18n("Third-party libraries"));


    QPushButton *bnClose = new QPushButton(i18n("Close"));
    connect(bnClose, SIGNAL(clicked()), SLOT(close()));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(10);
    hlayout->addStretch(10);
    hlayout->addWidget(bnClose);



    vlayout->addLayout(hlayout);

}

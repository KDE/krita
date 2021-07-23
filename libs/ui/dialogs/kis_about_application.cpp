/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <kaboutdata.h>

#include <klocalizedstring.h>
#include "kis_splash_screen.h"

KisAboutApplication::KisAboutApplication(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("About Krita"));

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    QTabWidget *wdgTab = new QTabWidget;
    vlayout->addWidget(wdgTab);
    KisSplashScreen *splash = new KisSplashScreen(true);

    splash->setWindowFlags(Qt::Widget);
    splash->displayLinks(true);

    wdgTab->addTab(splash, i18n("About"));

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


    // Translators
    KAboutData aboutData(KAboutData::applicationData());
    if (aboutData.translators().isEmpty()) {
        aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"),
                                i18nc("EMAIL OF TRANSLATORS", "Your emails"));

    }

    if (!aboutData.translators().isEmpty()) {
        wdgTab->addTab(createTranslatorsWidget(aboutData.translators(), aboutData.ocsProviderUrl()),
                       i18nc("@title:tab", "Translators"));
    }


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
    bnClose->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    connect(bnClose, SIGNAL(clicked()), SLOT(close()));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(10);
    hlayout->addStretch(10);
    hlayout->addWidget(bnClose);

    vlayout->addLayout(hlayout);

    setMinimumSize(vlayout->sizeHint());
}

QWidget *KisAboutApplication::createTranslatorsWidget(const QList<KAboutPerson> &translators, const QString &ocsProviderUrl)
{
    QString aboutTranslationTeam = KAboutData::aboutTranslationTeam();

    qDebug() << aboutTranslationTeam << ocsProviderUrl;

    QTextBrowser *lblTranslators = new QTextBrowser();

    lblTranslators->setOpenExternalLinks(true);

    QString translatorHtml = i18n("<html>"
                                  "<head/>"
                                  "<body>"
                                  "<h1 align=\"center\"><b>Translators</b></h1>"
                                  "<p><ul>");


    Q_FOREACH(const KAboutPerson &person, translators) {
        translatorHtml.append(QString("<li>%1</li>").arg(person.name()));
    }

    translatorHtml.append("<ul></p>");
    translatorHtml.append(i18n("<p>KDE is translated into many languages thanks to the work of the "
                          "translation teams all over the world.</p><p>For more information on KDE "
                          "internationalization visit <a href=\"http://l10n.kde.org\">http://l10n."
                          "kde.org</a></p>"));
    translatorHtml.append("</body></html>");

    lblTranslators->setText(translatorHtml);

    return lblTranslators;
}


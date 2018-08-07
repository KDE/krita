/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbugreport.h"

#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLayout>
#include <QRadioButton>
#include <QGroupBox>
#include <QLocale>
#include <QCloseEvent>
#include <QLabel>
#include <QUrl>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QComboBox>
#include <QLineEdit>
#include <QDebug>
#include <QTextEdit>
#include <QDesktopServices>

#include <kaboutdata.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kemailsettings.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>

#include <ktitlewidget.h>

#include "systeminformation_p.h"

#include "config-xmlgui.h"

#include <kis_icon_utils.h>

class KBugReportPrivate
{
public:
    KBugReportPrivate(KBugReport *q): q(q), m_aboutData(KAboutData::applicationData()) {}

    void _k_updateUrl();

    KBugReport *q;
    QProcess *m_process;
    KAboutData m_aboutData;

    QTextEdit *m_lineedit;
    QLineEdit *m_subject;
    QLabel *m_version;
    QString m_strVersion;
    QGroupBox *m_bgSeverity;

    QComboBox *appcombo;
    QString lastError;
    QString appname;
    QString os;
    QUrl url;
    QList<QRadioButton *> severityButtons;
    int currentSeverity()
    {
        for (int i = 0; i < severityButtons.count(); i++)
            if (severityButtons[i]->isChecked()) {
                return i;
            }
        return -1;
    }
};

KBugReport::KBugReport(const KAboutData &aboutData, QWidget *_parent)
    : QDialog(_parent), d(new KBugReportPrivate(this))
{
    setWindowTitle(i18n("Submit Bug Report"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    d->m_aboutData = aboutData;
    d->m_process = 0;
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::close());

    QLabel *tmpLabel;
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);

    KTitleWidget *title = new KTitleWidget(this);
    title->setText(i18n("Submit Bug Report"));
    title->setPixmap(KisIconUtils::loadIcon(QStringLiteral("tools-report-bug")).pixmap(32));
    lay->addWidget(title);

    QGridLayout *glay = new QGridLayout();
    lay->addLayout(glay);

    int row = 0;

    // Program name
    QString qwtstr = i18n("The application for which you wish to submit a bug report - if incorrect, please use the Report Bug menu item of the correct application");
    tmpLabel = new QLabel(i18n("Application: "), this);
    glay->addWidget(tmpLabel, row, 0);
    tmpLabel->setWhatsThis(qwtstr);
    d->appcombo = new QComboBox(this);
    d->appcombo->setWhatsThis(qwtstr);

    QStringList packageList = QStringList() << "krita";
    d->appcombo->addItems(packageList);
    connect(d->appcombo, SIGNAL(activated(int)), SLOT(_k_appChanged(int)));
    d->appname = d->m_aboutData.productName();
    glay->addWidget(d->appcombo, row, 1);
    int index = 0;
    for (; index < d->appcombo->count(); index++) {
        if (d->appcombo->itemText(index) == d->appname) {
            break;
        }
    }
    if (index == d->appcombo->count()) { // not present
        d->appcombo->addItem(d->appname);
    }
    d->appcombo->setCurrentIndex(index);

    tmpLabel->setWhatsThis(qwtstr);

    // Version
    qwtstr = i18n("The version of this application - please make sure that no newer version is available before sending a bug report");
    tmpLabel = new QLabel(i18n("Version:"), this);
    glay->addWidget(tmpLabel, ++row, 0);
    tmpLabel->setWhatsThis(qwtstr);
    d->m_strVersion = d->m_aboutData.version();
    if (d->m_strVersion.isEmpty()) {
        d->m_strVersion = i18n("no version set (programmer error)");
    }
    d->m_version = new QLabel(d->m_strVersion, this);
    d->m_version->setTextInteractionFlags(Qt::TextBrowserInteraction);
    //glay->addWidget( d->m_version, row, 1 );
    glay->addWidget(d->m_version, row, 1, 1, 2);
    d->m_version->setWhatsThis(qwtstr);

    tmpLabel = new QLabel(i18n("OS:"), this);
    glay->addWidget(tmpLabel, ++row, 0);
    d->os = SystemInformation::operatingSystemVersion();

    tmpLabel = new QLabel(d->os, this);
    tmpLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    glay->addWidget(tmpLabel, row, 1, 1, 2);

    tmpLabel = new QLabel(i18n("Compiler:"), this);
    glay->addWidget(tmpLabel, ++row, 0);
    tmpLabel = new QLabel(QString::fromLatin1(XMLGUI_COMPILER_VERSION), this);
    tmpLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    glay->addWidget(tmpLabel, row, 1, 1, 2);

    // Point to the web form

    lay->addSpacing(10);
    QString text = i18n("<qt>"
                        "<p>Please read <b><a href=\"https://docs.krita.org/en/untranslatable_pages/reporting_bugs.html\">this guide</a></b> for reporting bugs first!</p>"
                        "<p>To submit a bug report, click on the button below. This will open a web browser "
                        "window on <a href=\"http://bugs.kde.org\">http://bugs.kde.org</a> where you will find "
                        "a form to fill in. The information displayed above will be transferred to that server.</p></qt>");
    QLabel *label = new QLabel(text, this);
    label->setOpenExternalLinks(true);
    label->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    label->setWordWrap(true);
    lay->addWidget(label);
    lay->addSpacing(10);

    d->appcombo->setFocus();

    d->_k_updateUrl();

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setText(i18n("&Launch Bug Report Wizard"));
    okButton->setIcon(KisIconUtils::loadIcon(QStringLiteral("tools-report-bug")));
    lay->addWidget(buttonBox);
    setMinimumHeight(sizeHint().height() + 20);   // WORKAROUND: prevent "cropped" qcombobox
}

KBugReport::~KBugReport()
{
    delete d;
}

void KBugReportPrivate::_k_updateUrl()
{
    url = QUrl(QStringLiteral("https://bugs.kde.org/enter_bug.cgi"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QLatin1String("guided"));    // use the guided form

    // the string format is product/component, where component is optional
    QStringList list = appcombo->currentText().split(QLatin1Char('/'));
    query.addQueryItem(QStringLiteral("product"), list[0]);
    if (list.size() == 2) {
        query.addQueryItem(QStringLiteral("component"), list[1]);
    }

    query.addQueryItem(QStringLiteral("version"), m_strVersion);

    // TODO: guess and fill OS(sys_os) and Platform(rep_platform) fields
#ifdef Q_OS_WIN
    query.addQueryItem(QStringLiteral("op_sys"), QStringLiteral("MS Windows"));
    query.addQueryItem(QStringLiteral("rep_platform"), QStringLiteral("MS Windows"));
#endif

    url.setQuery(query);
}

void KBugReport::accept()
{
    QDesktopServices::openUrl(d->url);
}


#include "moc_kbugreport.cpp"

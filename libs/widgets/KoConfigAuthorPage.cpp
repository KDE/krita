/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
                 2012 C. Boemann <cbo@boemann.dk>
                 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

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

#include "KoConfigAuthorPage.h"

#include "ui_KoConfigAuthorPage.h"

#include <KoGlobal.h>
#include <KoIcon.h>
#include <QDebug>

#include <klocalizedstring.h>
#include <kuser.h>
#include <kemailsettings.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <QLineEdit>
#include <QCompleter>
#include <QStackedWidget>
#include <QList>
#include <QComboBox>
#include <QGridLayout>
#include <QString>
#include <QStringList>
#include <QToolButton>
#include <QInputDialog>
#include <QTableView>
#include <QStandardItem>
#include <QLabel>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QDir>
#include <QByteArray>

class Q_DECL_HIDDEN KoConfigAuthorPage::Private
{
public:
    QList<Ui::KoConfigAuthorPage *> profileUiList;
    QStackedWidget *stack;
    QComboBox *cmbAuthorProfiles;
    QToolButton *bnDeleteUser;
    QStringList positions;
    QStringList contactModes;
    QStringList contactKeys;
    QString defaultAuthor;
};


KoConfigAuthorPage::KoConfigAuthorPage()
        : d(new Private)
{
    QGridLayout *layout = new QGridLayout;

    d->cmbAuthorProfiles = new QComboBox();
    layout->addWidget(d->cmbAuthorProfiles, 0, 0);
    QToolButton *newUser = new QToolButton();
    newUser->setIcon(koIcon("list-add"));
    newUser->setToolTip(i18n("Add new author profile (starts out as a copy of current)"));
    layout->addWidget(newUser, 0, 1);
    d->bnDeleteUser = new QToolButton();
    d->bnDeleteUser->setIcon(koIcon("trash-empty"));
    d->bnDeleteUser->setToolTip(i18n("Delete the author profile"));
    layout->addWidget(d->bnDeleteUser, 0, 2);
    QFrame *f = new QFrame();
    f->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(f, 1, 0);
    d->stack = new QStackedWidget();
    layout->addWidget(d->stack, 2, 0, 1, 3);
    setLayout(layout);

    //list of positions that we can use to provide useful autocompletion.
    d->positions << QString(i18nc("This is a list of suggestions for positions an artist can take, comma-separated","Adapter,Animator,Artist,Art Director,Author,Assistant,"
                                 "Editor,Background,Cartoonist,Colorist,Concept Artist,"
                                 "Corrector,Cover Artist,Creator,Designer,Inker,"
                                 "Letterer,Matte Painter,Painter,Penciller,Proofreader,"
                                 "Pixel Artist,Redliner,Sprite Artist,Typographer,Texture Artist,"
                                 "Translator,Writer,Other")).split(",");

    //Keep these two in sync!
    d->contactModes << i18n("Homepage") << i18n("Email") << i18n("Post Address") << i18n("Telephone") << i18n("Fax");
    d->contactKeys << "homepage" << "email" << "address" << "telephone" << "fax";
    QStringList headerlabels;
    headerlabels<< i18n("Type") << i18n("Entry");

    Ui::KoConfigAuthorPage *aUi = new Ui::KoConfigAuthorPage();
    QWidget *w = new QWidget;
    d->defaultAuthor = i18n("Anonymous");

    QStringList profilesNew;
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/authorinfo/");
    QStringList filters = QStringList() << "*.authorinfo";
    Q_FOREACH(const QString &entry, dir.entryList(filters)) {
        QFile file(dir.absoluteFilePath(entry));
        if (file.exists()) {
            file.open(QFile::ReadOnly);
            QByteArray ba = file.readAll();
            file.close();
            QDomDocument doc = QDomDocument();
            doc.setContent(ba);
            QDomElement root = doc.firstChildElement();
            aUi = new Ui::KoConfigAuthorPage();
            w = new QWidget;
            aUi->setupUi(w);
            QString profile = root.attribute("name");

            QDomElement el = root.firstChildElement("nickname");
            if (!el.isNull()) {
                aUi->leNickName->setText(el.text());
            }
            el = root.firstChildElement("givenname");
            if (!el.isNull()) {
                aUi->leFirstName->setText(el.text());
            }
            el = root.firstChildElement("middlename");
            if (!el.isNull()) {
                aUi->leInitials->setText(el.text());
            }
            el = root.firstChildElement("familyname");
            if (!el.isNull()) {
                aUi->leLastName->setText(el.text());
            }
            el = root.firstChildElement("title");
            if (!el.isNull()) {
                aUi->leTitle->setText(el.text());
            }
            el = root.firstChildElement("position");
            if (!el.isNull()) {
                aUi->lePosition->setText(el.text());
            }
            el = root.firstChildElement("company");
            if (!el.isNull()) {
                aUi->leCompany->setText(el.text());
            }

            aUi->tblContactInfo->setItemDelegate(new KoContactInfoDelegate(this, d->contactModes));
            QStandardItemModel *modes = new QStandardItemModel();
            aUi->tblContactInfo->setModel(modes);
            el = root.firstChildElement("contact");
            while (!el.isNull()) {
                QList<QStandardItem *> list;
                QString type = d->contactModes.at(d->contactKeys.indexOf(el.attribute("type")));
                list.append(new QStandardItem(type));
                list.append(new QStandardItem(el.text()));
                modes->appendRow(list);
                el = el.nextSiblingElement("contact");
            }
            modes->setHorizontalHeaderLabels(headerlabels);
            QCompleter *positionSuggestions = new QCompleter(d->positions);
            positionSuggestions->setCaseSensitivity(Qt::CaseInsensitive);
            aUi->lePosition->setCompleter(positionSuggestions);

            connect(aUi->btnAdd, SIGNAL(clicked()), this, SLOT(addContactEntry()));
            connect(aUi->btnRemove, SIGNAL(clicked()), this, SLOT(removeContactEntry()));

            d->cmbAuthorProfiles->addItem(profile);
            profilesNew.append(profile);
            d->profileUiList.append(aUi);
            d->stack->addWidget(w);
        }
    }

    // Add all the user defined profiles (old type)
    KConfig *config = KoGlobal::calligraConfig();
    KConfigGroup authorGroup(config, "Author");
    QStringList profiles = authorGroup.readEntry("profile-names", QStringList());


    foreach (const QString &profile , profiles) {
        if (!profilesNew.contains(profile)) {
            KConfigGroup cgs(&authorGroup, "Author-" + profile);
            aUi = new Ui::KoConfigAuthorPage();
            w = new QWidget;
            aUi->setupUi(w);
            aUi->leNickName->setText(cgs.readEntry("creator"));
            aUi->leFirstName->setText(cgs.readEntry("creator-first-name"));
            aUi->leLastName->setText(cgs.readEntry("creator-last-name"));
            aUi->leInitials->setText(cgs.readEntry("initial"));
            aUi->leTitle->setText(cgs.readEntry("author-title"));
            aUi->lePosition->setText(cgs.readEntry("position"));
            QCompleter *positionSuggestions = new QCompleter(d->positions);
            positionSuggestions->setCaseSensitivity(Qt::CaseInsensitive);
            aUi->lePosition->setCompleter(positionSuggestions);
            aUi->leCompany->setText(cgs.readEntry("company"));

            aUi->tblContactInfo->setItemDelegate(new KoContactInfoDelegate(this, d->contactModes));
            QStandardItemModel *modes = new QStandardItemModel();
            aUi->tblContactInfo->setModel(modes);
            if (cgs.hasKey("email")) {
                QList<QStandardItem *> list;
                QString email = d->contactModes.at(d->contactKeys.indexOf("email"));
                list.append(new QStandardItem(email));
                list.append(new QStandardItem(cgs.readEntry("email")));
                modes->appendRow(list);
            }
            if (cgs.hasKey("telephone-work")) {
                QList<QStandardItem *> list;
                QString tel = d->contactModes.at(d->contactKeys.indexOf("telephone"));
                list.append(new QStandardItem(tel));
                list.append(new QStandardItem(cgs.readEntry("telephone-work")));
                modes->appendRow(list);
            }
            if (cgs.hasKey("fax")) {
                QList<QStandardItem *> list;
                QString fax = d->contactModes.at(d->contactKeys.indexOf("fax"));
                list.append(new QStandardItem(fax));
                list.append(new QStandardItem(cgs.readEntry("fax")));
                modes->appendRow(list);
            }
            QStringList postal;
            postal << cgs.readEntry("street") << cgs.readEntry("postal-code") << cgs.readEntry("city") << cgs.readEntry("country");
            QString address;
            Q_FOREACH(QString part, postal) {
                if (!part.isEmpty()) {
                    address+= part + "\n";
                }
            }
            if (!address.isEmpty()) {
                QList<QStandardItem *> list;
                QString add = d->contactModes.at(d->contactKeys.indexOf("address"));
                list.append(new QStandardItem(add));
                list.append(new QStandardItem(address));
                modes->appendRow(list);
            }
            modes->setHorizontalHeaderLabels(headerlabels);
            connect(aUi->btnAdd, SIGNAL(clicked()), this, SLOT(addContactEntry()));
            connect(aUi->btnRemove, SIGNAL(clicked()), this, SLOT(removeContactEntry()));

            d->cmbAuthorProfiles->addItem(profile);
            d->profileUiList.append(aUi);
            d->stack->addWidget(w);
        }
    }


    // Add a default profile
    aUi = new Ui::KoConfigAuthorPage();
    w = new QWidget;
    if (!profiles.contains(d->defaultAuthor) || profilesNew.contains(d->defaultAuthor)) {
        //w->setEnabled(false);
        aUi->setupUi(w);
        w->setEnabled(false);
        d->cmbAuthorProfiles->insertItem(0, d->defaultAuthor);
        d->stack->insertWidget(0, w);
        d->profileUiList.insert(0, aUi);
    }


    // Connect slots
    connect(d->cmbAuthorProfiles, SIGNAL(currentIndexChanged(int)), this, SLOT(profileChanged(int)));
    connect(newUser, SIGNAL(clicked(bool)), this, SLOT(addUser()));
    connect(d->bnDeleteUser, SIGNAL(clicked(bool)), this, SLOT(deleteUser()));

    d->cmbAuthorProfiles->setCurrentIndex(0);
    profileChanged(0);
}

KoConfigAuthorPage::~KoConfigAuthorPage()
{
    delete d;
}

void KoConfigAuthorPage::profileChanged(int i)
{
    d->stack->setCurrentIndex(i);
    // Profile 0 should never be deleted: it's the anonymous profile.
    d->bnDeleteUser->setEnabled(i > 0);
}

void KoConfigAuthorPage::addUser()
{
    bool ok;
    QString profileName = QInputDialog::getText(this, i18n("Name of Profile"), i18n("Name (not duplicate or blank name):"), QLineEdit::Normal, "", &ok);

    if (!ok) {
        return;
    }

    Ui::KoConfigAuthorPage *curUi = d->profileUiList[d->cmbAuthorProfiles->currentIndex()];
    Ui::KoConfigAuthorPage *aUi = new Ui::KoConfigAuthorPage();
    QWidget *w = new QWidget;
    aUi->setupUi(w);

    aUi->leNickName->setText(curUi->leNickName->text());
    aUi->leInitials->setText(curUi->leInitials->text());
    aUi->leTitle->setText(curUi->leTitle->text());
    aUi->leCompany->setText(curUi->leCompany->text());
    aUi->leFirstName->setText(curUi->leFirstName->text());
    aUi->leLastName->setText(curUi->leLastName->text());
    aUi->lePosition->setText(curUi->lePosition->text());
    QCompleter *positionSuggestions = new QCompleter(d->positions);
    positionSuggestions->setCaseSensitivity(Qt::CaseInsensitive);
    aUi->lePosition->setCompleter(positionSuggestions);
    aUi->tblContactInfo->setItemDelegate(new KoContactInfoDelegate(this, d->contactModes));
    QStandardItemModel *modes = new QStandardItemModel();
    aUi->tblContactInfo->setModel(modes);

    connect(aUi->btnAdd, SIGNAL(clicked()), this, SLOT(addContactEntry()));
    connect(aUi->btnRemove, SIGNAL(clicked()), this, SLOT(removeContactEntry()));

    int index = d->cmbAuthorProfiles->currentIndex() + 1;
    d->cmbAuthorProfiles->insertItem(index, profileName);
    d->profileUiList.insert(index, aUi);
    d->stack->insertWidget(index, w);
    d->cmbAuthorProfiles->setCurrentIndex(index);
}

void KoConfigAuthorPage::deleteUser()
{
    int index = d->cmbAuthorProfiles->currentIndex();
    QWidget *w = d->stack->currentWidget();

    d->stack->removeWidget(w);
    d->profileUiList.removeAt(index);
    d->cmbAuthorProfiles->removeItem(index);
    delete w;
}

void KoConfigAuthorPage::addContactEntry()
{
    int i = d->cmbAuthorProfiles->currentIndex();
    Ui::KoConfigAuthorPage *aUi = d->profileUiList[i];
    QStandardItemModel *contact = static_cast<QStandardItemModel*>(aUi->tblContactInfo->model());
    QList<QStandardItem*>list;
    list.append(new QStandardItem(d->contactModes.at(0)));
    list.append(new QStandardItem(i18n("New Contact Info")));
    contact->appendRow(list);
    aUi->tblContactInfo->setModel(contact);
}

void KoConfigAuthorPage::removeContactEntry()
{
    int i = d->cmbAuthorProfiles->currentIndex();
    Ui::KoConfigAuthorPage *aUi = d->profileUiList[i];
    QModelIndex index = aUi->tblContactInfo->selectionModel()->currentIndex();
    aUi->tblContactInfo->model()->removeRow(index.row());
}

void KoConfigAuthorPage::apply()
{
    QString authorInfo = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/authorinfo/";
    QDir dir(authorInfo);
    if (!dir.mkpath(authorInfo)) {
        qWarning()<<"We can't make an author info directory, and therefore not save!";
        return;
    }
    for (int i = 0; i < d->profileUiList.size(); i++) {
        if (d->cmbAuthorProfiles->itemText(i)!= d->defaultAuthor) {
            QByteArray ba;
            QDomDocument doc = QDomDocument();
            Ui::KoConfigAuthorPage *aUi = d->profileUiList[i];

            QDomElement root = doc.createElement("author");
            root.setAttribute("name", d->cmbAuthorProfiles->itemText(i));

            QDomElement nickname = doc.createElement("nickname");
            nickname.appendChild(doc.createTextNode(aUi->leNickName->text()));
            root.appendChild(nickname);
            QDomElement givenname = doc.createElement("givenname");
            givenname.appendChild(doc.createTextNode(aUi->leFirstName->text()));
            root.appendChild(givenname);
            QDomElement familyname = doc.createElement("familyname");
            familyname.appendChild(doc.createTextNode(aUi->leLastName->text()));
            root.appendChild(familyname);
            QDomElement middlename = doc.createElement("middlename");
            middlename.appendChild(doc.createTextNode(aUi->leInitials->text()));
            root.appendChild(middlename);
            QDomElement title = doc.createElement("title");
            title.appendChild(doc.createTextNode(aUi->leTitle->text()));
            root.appendChild(title);
            QDomElement company = doc.createElement("company");
            company.appendChild(doc.createTextNode(aUi->leCompany->text()));
            root.appendChild(company);
            QDomElement position = doc.createElement("position");
            position.appendChild(doc.createTextNode(aUi->lePosition->text()));
            root.appendChild(position);
            if (aUi->tblContactInfo) {
                if (aUi->tblContactInfo->model()) {
                    for (int i=0; i<aUi->tblContactInfo->model()->rowCount(); i++) {
                        QModelIndex index = aUi->tblContactInfo->model()->index(i, 1);
                        QModelIndex typeIndex = aUi->tblContactInfo->model()->index(i, 0);
                        QDomElement contactEl = doc.createElement("contact");
                        QString content = QVariant(aUi->tblContactInfo->model()->data(index)).toString();
                        contactEl.appendChild(doc.createTextNode(content));
                        QString type = QVariant(aUi->tblContactInfo->model()->data(typeIndex)).toString();
                        contactEl.setAttribute("type", d->contactKeys.at(d->contactModes.indexOf(type)));
                        root.appendChild(contactEl);
                    }
                }
            }
            doc.appendChild(root);
            ba = doc.toByteArray();

            QFile f(authorInfo + d->cmbAuthorProfiles->itemText(i) +".authorinfo");
            f.open(QFile::WriteOnly);
            if (f.write(ba) < 0) {
                qWarning()<<"Writing author info went wrong:"<<f.errorString();
            }
            f.close();
        }
    }
}

KoContactInfoDelegate::KoContactInfoDelegate(QWidget *parent, QStringList contactModes): QStyledItemDelegate(parent), m_contactModes(contactModes)
{
}

KoContactInfoDelegate::~KoContactInfoDelegate()
{

}

QWidget* KoContactInfoDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{

    if (index.column() > 0) {
        return new QLineEdit(parent);
    } else {
        QComboBox *box = new QComboBox(parent);
        box->addItems(m_contactModes);
        return box;
    }
}

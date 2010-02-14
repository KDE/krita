/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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
 * Boston, MA 02110-1301, USA.
*/

#include "SemanticStylesheetsEditor.h"
#include "ui_SemanticStylesheetsEditor.h"
#include "RdfSemanticItem.h"

#include <kdebug.h>

class SemanticStylesheetsEditor::Private
{
public:
    Ui::SemanticStylesheetsEditor *m_ui;
    KoDocumentRdf *m_rdf;
    QTreeWidgetItem *m_systemSheetsParentItem;
    QTreeWidgetItem *m_userSheetsParentItem;
    QMap<QString, QTreeWidgetItem*> m_systemSheetsItems;
    QMap<QString, QTreeWidgetItem*> m_userSheetsItems;
    Private()
            : m_systemSheetsParentItem(0)
            , m_userSheetsParentItem(0) {}
};

enum {
    // TODO CamelCase
    COL_NAME = 0,
    COL_SEMOBJ = 1,
    COL_VAR = 0,
    COL_DESC = 1
};

class SemanticStylesheetWidgetItem : public QTreeWidgetItem
{
public:
    KoDocumentRdf *m_rdf;
    SemanticStylesheet *m_ss;
    RdfSemanticItem *m_si;
    SemanticStylesheetWidgetItem(KoDocumentRdf *rdf,
                                 SemanticStylesheet *ss,
                                 RdfSemanticItem *si,
                                 QTreeWidgetItem *parent,
                                 int type = Type)
            : QTreeWidgetItem(parent, type)
            , m_rdf(rdf)
            , m_ss(ss)
            , m_si(si) {
    }
    SemanticStylesheetWidgetItem(KoDocumentRdf *rdf,
                                 RdfSemanticItem *si,
                                 QTreeWidgetItem *parent,
                                 int type = Type)
            : QTreeWidgetItem(parent, type)
            , m_rdf(rdf)
            , m_ss(0)
            , m_si(si) {
    }

    virtual void setData(int column, int role, const QVariant &value) {
        if (m_ss && m_ss->isMutable() && column == COL_NAME) {
            QString newName = value.toString();
            kDebug(30015) << "update to value:" << newName;
            m_ss->name(newName);
            QVariant v = m_ss->name();
            QTreeWidgetItem::setData(column, role, v);
        } else {
            QTreeWidgetItem::setData(column, role, value);
        }
    }
};


SemanticStylesheetsEditor::SemanticStylesheetsEditor(QWidget *parent, KoDocumentRdf *rdf)
        : KDialog(parent),
        d(new Private())
{
    d->m_rdf = rdf;
    QWidget *widget = new QWidget(this);
    setMainWidget(widget);
    d->m_ui = new Ui::SemanticStylesheetsEditor();
    d->m_ui->setupUi(widget);
    d->m_ui->newStylesheet->setEnabled(false);
    d->m_ui->deleteStylesheet->setEnabled(false);
    connect(d->m_ui->newStylesheet, SIGNAL(clicked()),
            this, SLOT(newStylesheet()));
    connect(d->m_ui->deleteStylesheet, SIGNAL(clicked()),
            this, SLOT(deleteStylesheet()));
    connect(d->m_ui->stylesheets, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
    d->m_ui->stylesheets->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->m_ui->stylesheets->sortItems(COL_NAME, Qt::DescendingOrder);
    connect(d->m_ui->definition, SIGNAL(textChanged()),
            this, SLOT(definitionChanged()));
    // setup system/user parent treewidgets
    d->m_systemSheetsParentItem = new QTreeWidgetItem(d->m_ui->stylesheets);
    d->m_systemSheetsParentItem->setText(COL_NAME, i18n("System"));
    d->m_systemSheetsParentItem->setText(COL_SEMOBJ, "");
    d->m_ui->stylesheets->expandItem(d->m_systemSheetsParentItem);

    QStringList classNames;
    QTreeWidgetItem *treewidget = 0;
    treewidget = d->m_systemSheetsParentItem;
    classNames = RdfSemanticItem::classNames();
    foreach (const QString &klass, classNames) {
        RdfSemanticItem* si = RdfSemanticItem::createSemanticItem(this, rdf, klass);
        SemanticStylesheetWidgetItem *item = new SemanticStylesheetWidgetItem(rdf, si, treewidget);
        item->setText(COL_NAME, klass);
        d->m_systemSheetsItems[klass] = item;
        d->m_ui->stylesheets->expandItem(item);
    }
    d->m_userSheetsParentItem = new QTreeWidgetItem(d->m_ui->stylesheets);
    d->m_userSheetsParentItem->setText(COL_NAME, i18n("User"));
    d->m_userSheetsParentItem->setText(COL_SEMOBJ, "");
    d->m_ui->stylesheets->expandItem(d->m_userSheetsParentItem);

    treewidget = d->m_userSheetsParentItem;
    classNames = RdfSemanticItem::classNames();
    foreach (const QString &klass, classNames) {
        RdfSemanticItem* si = RdfSemanticItem::createSemanticItem(this, rdf, klass);
        SemanticStylesheetWidgetItem* item = new SemanticStylesheetWidgetItem(rdf, si, treewidget);
        item->setText(COL_NAME, klass);
        d->m_userSheetsItems[klass] = item;
        d->m_ui->stylesheets->expandItem(item);
    }

    // initialize stylesheets tree
    classNames = RdfSemanticItem::classNames();
    foreach (const QString &klass, classNames) {
        kDebug(30015) << "klass:" << klass;
        RdfSemanticItem *p = RdfSemanticItem::createSemanticItem(this, rdf, klass);
        setupStylesheetsItems(klass, p, p->stylesheets(), d->m_systemSheetsItems);
        setupStylesheetsItems(klass, p, p->userStylesheets(), d->m_userSheetsItems, true);
    }
    connect(d->m_ui->variables, SIGNAL(itemActivated(QTableWidgetItem*)),
            this, SLOT(onVariableActivated(QTableWidgetItem*)));
    d->m_ui->variables->horizontalHeader()->setResizeMode(COL_VAR, QHeaderView::ResizeToContents);
    d->m_ui->variables->horizontalHeader()->setResizeMode(COL_DESC, QHeaderView::ResizeToContents);
}

SemanticStylesheetsEditor::~SemanticStylesheetsEditor()
{
}

void SemanticStylesheetsEditor::setupStylesheetsItems(const QString& klass,
        RdfSemanticItem *si,
        const QList<SemanticStylesheet*> &ssl,
        const QMap<QString, QTreeWidgetItem*> &m,
        bool editable)
{
    foreach (SemanticStylesheet *ss, ssl) {
        kDebug(30015) << "ss:" << ss->name();
        QTreeWidgetItem *parent = m[klass];
        SemanticStylesheetWidgetItem *item = new SemanticStylesheetWidgetItem(d->m_rdf, ss, si, parent);
        item->setText(COL_NAME, ss->name());
        item->setText(COL_SEMOBJ, klass);
        if (editable) {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
    }
}

void SemanticStylesheetsEditor::slotOk()
{
}

void SemanticStylesheetsEditor::maskButtonsDependingOnCurrentItem(QTreeWidgetItem *current)
{
    bool newEnabled = true;
    bool delEnabled = true;
    for (QMap< QString, QTreeWidgetItem* >::iterator iter = d->m_userSheetsItems.begin();
            iter != d->m_userSheetsItems.end(); ++iter) {
        if (iter.value() == current) {
            delEnabled = false;
        }
    }
    if (d->m_userSheetsParentItem == current) {
        newEnabled = false;
        delEnabled = false;
    } else {
        while (current) {
            kDebug(30015) << "current:" << current->text(COL_NAME);
            if (d->m_systemSheetsParentItem == current) {
                newEnabled = false;
                delEnabled = false;
                break;
            }
            current = current->parent();
        }
    }
    d->m_ui->newStylesheet->setEnabled(newEnabled);
    d->m_ui->deleteStylesheet->setEnabled(delEnabled);
}

void SemanticStylesheetsEditor::definitionChanged()
{
    if (SemanticStylesheetWidgetItem *ssitem
            = dynamic_cast<SemanticStylesheetWidgetItem*>(d->m_ui->stylesheets->currentItem())) {
        if (!ssitem->m_ss) {
            return;
        }
        QString desc = d->m_ui->definition->toPlainText();
        kDebug(30015) << "ss:" << ssitem->m_ss->name() << " desc:" << desc;
        ssitem->m_ss->templateString(desc);
    }
}

struct SignalBlockerRAII {
    QObject *obj;
    bool oldval;
    SignalBlockerRAII(QObject *o)
            : obj(o) {
        oldval = obj->blockSignals(true);
    }
    ~SignalBlockerRAII() {
        obj->blockSignals(oldval);
    }
};


void SemanticStylesheetsEditor::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    SignalBlockerRAII _blocker1(d->m_ui->definition);
    kDebug(30015) << "current:" << current;
    kDebug(30015) << "previous:" << previous;
    maskButtonsDependingOnCurrentItem(current);
    if (previous) {
        QString desc = d->m_ui->definition->toPlainText();
        kDebug(30015) << "desc:" << desc;
        if (SemanticStylesheetWidgetItem *ssitem
                = dynamic_cast<SemanticStylesheetWidgetItem*>(previous)) {
            kDebug(30015) << "ssitem, ss?:" << ssitem->m_ss;

            if (ssitem->m_ss) {
                ssitem->m_ss->templateString(desc);
            }
        }
    }
    if (SemanticStylesheetWidgetItem *ssitem
            = dynamic_cast<SemanticStylesheetWidgetItem*>(current)) {
        d->m_ui->definition->setPlainText(QString());
        d->m_ui->variables->clear();
        if (!ssitem->m_ss) {
            return;
        }
        d->m_ui->definition->setPlainText(ssitem->m_ss->templateString());
        // update the list of available variables
        QTableWidget *v = d->m_ui->variables;
        v->clear();
        int row = 0;
        QMap<QString, QString> m;
        ssitem->m_si->setupStylesheetReplacementMapping(m);
        QMap< QString, QString >::const_iterator mi = m.constBegin();
        QMap< QString, QString >::const_iterator me = m.constEnd();
        for (; mi != me; ++mi) {
            QTableWidgetItem *item = 0;
            QString varName = mi.key();
            QString desc("t");
            v->setRowCount(row + 1);
            item = new QTableWidgetItem(varName);
            v->setItem(row, COL_VAR, item);
            item = new QTableWidgetItem(desc);
            v->setItem(row, COL_DESC, item);
            ++row;
        }
    }
}

void SemanticStylesheetsEditor::newStylesheet()
{
    kDebug(30015);
    if (SemanticStylesheetWidgetItem *ssitem
            = dynamic_cast<SemanticStylesheetWidgetItem*>(d->m_ui->stylesheets->currentItem())) {
        if (ssitem->m_ss) {
            ssitem = dynamic_cast<SemanticStylesheetWidgetItem*>(ssitem->parent());
            if (!ssitem) {
                return;
            }
        }
        kDebug(30015) << ssitem;
        static int uniqueCounter = 1; // TODO avoid static
        RdfSemanticItem *si = ssitem->m_si;
        SemanticStylesheet *ss = si->createUserStylesheet(
                                     QString("new stylesheet %1").arg(uniqueCounter++), "");
        QTreeWidgetItem *parent = ssitem;
        SemanticStylesheetWidgetItem* item =
            new SemanticStylesheetWidgetItem(d->m_rdf, ss, si, parent);
        item->setText(COL_NAME, ss->name());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        d->m_ui->stylesheets->setCurrentItem(item);
        d->m_ui->stylesheets->scrollToItem(item);
    }
}

void SemanticStylesheetsEditor::deleteStylesheet()
{
    kDebug(30015);
    if (SemanticStylesheetWidgetItem *ssitem
            = dynamic_cast<SemanticStylesheetWidgetItem*>(d->m_ui->stylesheets->currentItem())) {
        if (!ssitem->m_ss) {
            return;
        }
        SemanticStylesheet *sheet = ssitem->m_ss;
        ssitem->m_ss = 0;
        ssitem->m_si->destroyUserStylesheet(sheet);
        ssitem->parent()->removeChild(ssitem);
    }
}

void SemanticStylesheetsEditor::onVariableActivated(QTableWidgetItem* item)
{
    QTableWidgetItem *vitem = d->m_ui->variables->item(item->row(), COL_VAR);
    QString vtext = vitem->text();
    QTextCursor cursor(d->m_ui->definition->textCursor());
    cursor.insertText(vtext);
}

#include <SemanticStylesheetsEditor.moc>

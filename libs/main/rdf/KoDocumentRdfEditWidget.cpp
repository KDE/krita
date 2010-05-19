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

#include "KoDocumentRdfEditWidget.h"
#include <ui_KoDocumentRdfEditWidget.h>
#include "KoDocumentRdf.h"
#include "KoRdfPrefixMapping.h"
#include "../KoDocument.h"
#include "KoSopranoTableModelDelegate.h"
#include "KoSopranoTableModel.h"
#include "../KoGlobal.h"

#include <kdebug.h>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <KMenu>
#include <KMessageBox>


/**
 * Helper class to maintain a single row in the namespace editing
 * widget. This allows prefixes to be edited and updates the internal
 * data structures accordingly.
 */
class KoRdfPrefixMappingTreeWidgetItem : public QTreeWidgetItem
{
    KoRdfPrefixMapping *m_mapping;
    QString m_key;
public:
    enum {
        ColKey = 0,
        ColValue = 1,
        ColSize
    };
    KoRdfPrefixMappingTreeWidgetItem(KoRdfPrefixMapping *mapping, QString key, int type = Type)
            : QTreeWidgetItem(type)
            , m_mapping(mapping)
            , m_key(key) {
        setFlags(QTreeWidgetItem::flags() | Qt::ItemIsEditable);
    }
    virtual void setData(int column, int role, const QVariant &value) {
        m_mapping->dump();
        kDebug(30015) << "m_key:" << m_key << " value:" << value;
        if (column == ColKey) {
            QString url = m_mapping->prefexToURI(m_key);
            m_mapping->remove(m_key);
            m_key = value.toString();
            m_mapping->insert(m_key, url);
        } else if (column == ColValue) {
            m_mapping->remove(m_key);
            m_mapping->insert(m_key, value.toString());
        }
        QTreeWidgetItem::setData(column, role, value);
    }
    virtual QVariant data(int column, int role) const {
        if (role != Qt::DisplayRole && role != Qt::EditRole) {
            return QVariant();
        }
        if (column == ColKey) {
            return m_key;
        } else if (column == ColValue) {
            QString url = m_mapping->prefexToURI(m_key);
            return url;
        }
        return QTreeWidgetItem::data(column, role);
    }
    void removeFromMapping() {
        m_mapping->remove(m_key);
    }
};

class KoDocumentRdfEditWidget::KoDocumentRdfEditWidgetPrivate
{
public:
    QWidget *m_widget;
    KoDocumentRdf *m_rdf;
    Ui::KoDocumentRdfEditWidget *m_ui;
    KoSopranoTableModel *m_tripleModel;
    KoSopranoTableModel *m_sparqlResultModel;
    QSortFilterProxyModel *m_tripleProxyModel;
    KoRdfSemanticTree m_semanticItemsTree;

    KoDocumentRdfEditWidgetPrivate(KoDocumentRdf *m_rdf)
            : m_rdf(m_rdf) , m_tripleProxyModel(0) {
        m_ui = new Ui::KoDocumentRdfEditWidget();
        m_widget = new QWidget();
        m_ui->setupUi(m_widget);
    }
    ~KoDocumentRdfEditWidgetPrivate() {
        delete m_ui;
    }

    void setupWidget(KoDocumentRdfEditWidget *kdrew) {
        // setup triple view page
        m_tripleModel = new KoSopranoTableModel(m_rdf);
        m_tripleProxyModel = new QSortFilterProxyModel(m_widget);
        m_tripleProxyModel->setSourceModel(m_tripleModel);
        m_ui->m_tripleView->setModel(m_tripleProxyModel);
        m_ui->m_tripleView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_ui->m_tripleView->hideColumn(KoSopranoTableModel::ColIsValid);
        m_ui->m_tripleView->hideColumn(KoSopranoTableModel::ColObjXsdType);
        m_ui->m_tripleView->horizontalHeader()->setResizeMode(KoSopranoTableModel::ColSubj, QHeaderView::Stretch);
        m_ui->m_tripleView->horizontalHeader()->setResizeMode(KoSopranoTableModel::ColPred, QHeaderView::Stretch);
        m_ui->m_tripleView->horizontalHeader()->setResizeMode(KoSopranoTableModel::ColObj,  QHeaderView::Stretch);
        m_ui->m_tripleView->horizontalHeader()->setResizeMode(KoSopranoTableModel::ColCtx,  QHeaderView::Stretch);
        m_ui->m_tripleView->setItemDelegate(new KoSopranoTableModelDelegate(m_tripleProxyModel));
        // setup namespace page
        QTreeWidget *v = m_ui->m_namespaceView;
        v->setColumnCount(KoRdfPrefixMappingTreeWidgetItem::ColSize);
        v->sortItems(KoRdfPrefixMappingTreeWidgetItem::ColValue, Qt::DescendingOrder);
        v->header()->setResizeMode(KoRdfPrefixMappingTreeWidgetItem::ColKey,
                                   QHeaderView::ResizeToContents);
        KoRdfPrefixMapping *mapping = m_rdf->prefixMapping();
        const QMap<QString,QString> &m = m_rdf->prefixMapping()->mappings();
        for (QMap<QString,QString>::const_iterator mi = m.begin(); mi != m.end(); ++mi) {
            KoRdfPrefixMappingTreeWidgetItem *item =
                new KoRdfPrefixMappingTreeWidgetItem(mapping, mi.key());
            v->addTopLevelItem(item);
        }
        // setup semantic page
        v = m_ui->m_semanticView;
        m_semanticItemsTree = KoRdfSemanticTree::createTree(v);
        m_semanticItemsTree.update(m_rdf);
        // stylesheets page
        buildComboBox(m_ui->m_defaultContactsSheet,
                      KoRdfSemanticItem::createSemanticItem(kdrew, m_rdf, "Contact"));
        buildComboBox(m_ui->m_defaultEventsSheet,
                      KoRdfSemanticItem::createSemanticItem(kdrew, m_rdf, "Event"));
        buildComboBox(m_ui->m_defaultLocationsSheet,
                      KoRdfSemanticItem::createSemanticItem(kdrew, m_rdf, "Location"));
        kDebug(30015) << "format(), setting up ss page.";
        QList<KoRdfFoaF*> foaf = m_rdf->foaf();
        kDebug(30015) << "format(), setting up ss page, foaf.sz:" << foaf.size();
    }

    void buildComboBox(QComboBox *w, KoRdfSemanticItem *si) {
        if (!si) {
            return;
        }
        KoSemanticStylesheet *activeSheet = si->defaultStylesheet();
        int activeSheetIndex = 0;
        kDebug(30015) << "format(), activeSheet:" << activeSheet->name();

        foreach (KoSemanticStylesheet *ss, si->stylesheets()) {
            QVariant ud = QVariant::fromValue(ss);
            w->addItem(ss->name(), ud);
            if (activeSheet->name() == ss->name()) {
                activeSheetIndex = w->count() - 1;
            }
        }
        foreach (KoSemanticStylesheet *ss, si->userStylesheets()) {
            QVariant ud = QVariant::fromValue(ss);
            w->addItem(ss->name(), ud);
            if (activeSheet->name() == ss->name()) {
                activeSheetIndex = w->count() - 1;
            }
        }
        kDebug(30015) << "format(), active:" << activeSheetIndex;
        w->setCurrentIndex(activeSheetIndex);
    }

    void clearTriplesSelection() {
        m_ui->m_tripleView->selectionModel()->clear();
    }

    void selectTriples(int row) {
        m_ui->m_tripleView->selectRow(row);
    }

    void selectTriples(QModelIndex mi,
                       QItemSelectionModel::SelectionFlags command = QItemSelectionModel::Select) {
        m_ui->m_tripleView->selectionModel()->select(mi, command | QItemSelectionModel::Rows);
    }

    void selectTriples(QModelIndexList ml,
                       QItemSelectionModel::SelectionFlags command = QItemSelectionModel::Select) {
        Q_UNUSED(command);
        QModelIndex mi;
        foreach (mi, ml) {
            selectTriples(mi);
        }
    }

    QModelIndexList mapFromSource(QModelIndexList mil) const {
        QModelIndexList ret;
        foreach (QModelIndex idx, mil) {
            QModelIndex pidx = m_tripleProxyModel->mapFromSource(idx);
            ret << pidx;
        }
        return ret;
    }

    QModelIndex mapFromSource(QModelIndex idx) const {
        QModelIndex pidx = m_tripleProxyModel->mapFromSource(idx);
        return pidx;
    }

    QModelIndex mapToSource(const QModelIndex proxyIndex) const {
        QModelIndex sidx = m_tripleProxyModel->mapToSource(proxyIndex);
        return sidx;
    }

    QModelIndexList mapToSource(const QModelIndexList proxyList) const {
        QModelIndexList ret;
        foreach (QModelIndex idx, proxyList) {
            QModelIndex sidx = m_tripleProxyModel->mapToSource(idx);
            ret << sidx;
        }
        return ret;
    }
};


KoDocumentRdfEditWidget::KoDocumentRdfEditWidget(QWidget *parent, KoDocumentRdf *docRdf)
        : KoDocumentRdfEditWidgetBase(parent, docRdf)
        , d(new KoDocumentRdfEditWidgetPrivate(docRdf))
{
    d->setupWidget(this);
    connect(d->m_ui->newTripleButton, SIGNAL(clicked()), this, SLOT(addTriple()));
    connect(d->m_ui->copyTripleButton, SIGNAL(clicked()), this, SLOT(copyTriples()));
    connect(d->m_ui->deleteTripleButton, SIGNAL(clicked()), this, SLOT(deleteTriples()));
    connect(d->m_ui->newNamespaceButton, SIGNAL(clicked()), this, SLOT(addNamespace()));
    connect(d->m_ui->deleteNamespaceButton, SIGNAL(clicked()), this, SLOT(deleteNamespace()));
    d->m_ui->m_semanticView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d->m_ui->m_semanticView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showSemanticViewContextMenu(const QPoint &)));
    connect(d->m_ui->m_sparqlExecute, SIGNAL(clicked()), this, SLOT(sparqlExecute()));
    connect(d->m_ui->m_defaultContactsSheetButton, SIGNAL(clicked()), this, SLOT(defaultContactsSheetButton()));
    connect(d->m_ui->m_defaultEventsSheetButton, SIGNAL(clicked()), this, SLOT(defaultEventsSheetButton()));
    connect(d->m_ui->m_defaultLocationsSheetButton, SIGNAL(clicked()), this, SLOT(defaultLocationsSheetButton()));
    connect(d->m_ui->m_setAllStylesheetsButton, SIGNAL(clicked()), this, SLOT(defaultAllSheetButton()));
    connect(docRdf, SIGNAL(semanticObjectUpdated(KoRdfSemanticItem*)),
            this, SLOT(semanticObjectUpdated(KoRdfSemanticItem*)));
}

KoDocumentRdfEditWidget::~KoDocumentRdfEditWidget()
{
    if (d) {
        delete d;
    }
}


QWidget* KoDocumentRdfEditWidget::widget()
{
    return d->m_widget;
}

bool KoDocumentRdfEditWidget::shouldDialogCloseBeVetoed()
{
    bool ret = false;
    if (int invalidCount = d->m_tripleModel->invalidStatementCount()) {
        kDebug(30015) << "invalidCount:" << invalidCount;
        int dialogRet = KMessageBox::warningContinueCancel(
                            this,
                            i18nc("triple is a Rdf related term", "<qt>Partially edited Rdf will be lost if you proceed."
                                 "<p>There are %1 invalid triples in the dialog.</qt>",
                                 QString("<b>%1</b>").arg(invalidCount)),
                            i18nc("triple is a Rdf related term", "Lose invalid triples?"),
                            KStandardGuiItem::cont(),
                            KStandardGuiItem::cancel(),
                            "InvalidTriplesInDocInfoDialog");
        if (dialogRet == KMessageBox::Cancel) {
            ret = true;
            QModelIndexList invalidList = d->m_tripleModel->invalidStatementList();
            if (!invalidList.isEmpty()) {
                d->m_ui->m_tripleView->selectionModel()->clear();
                foreach (QModelIndex idx, invalidList) {
                    QModelIndex pidx = d->mapFromSource(idx);
                    d->m_ui->m_tripleView->selectionModel()->select(
                        pidx, QItemSelectionModel::Select);
                }

                QModelIndex pidx = d->mapFromSource(invalidList.first());
                d->m_ui->m_tripleView->scrollTo(pidx);
            }
        }
        kDebug(30015) << "dialogRet:" << dialogRet << " ret:" << ret;
    }
    return ret;
}

void KoDocumentRdfEditWidget::apply()
{
    KoDocumentRdf *rdf = d->m_rdf;
    if (KoRdfSemanticItem *si = KoRdfSemanticItem::createSemanticItem(0, rdf, "Contact")) {
        si->defaultStylesheet(stylesheetFromComboBox(d->m_ui->m_defaultContactsSheet));
        delete si;
    }
    if (KoRdfSemanticItem *si = KoRdfSemanticItem::createSemanticItem(0, rdf, "Event")) {
        si->defaultStylesheet(stylesheetFromComboBox(d->m_ui->m_defaultEventsSheet));
        delete si;
    }
    if (KoRdfSemanticItem *si = KoRdfSemanticItem::createSemanticItem(0, rdf, "Location")) {
        si->defaultStylesheet(stylesheetFromComboBox(d->m_ui->m_defaultLocationsSheet));
        delete si;
    }
}

void KoDocumentRdfEditWidget::semanticObjectUpdated(KoRdfSemanticItem *item)
{
    Q_UNUSED(item);
    kDebug(30015) << "updating the sem item list view";
    d->m_semanticItemsTree.update(d->m_rdf);
}

void KoDocumentRdfEditWidget::showSemanticViewContextMenu(const QPoint &position)
{
    QPointer<KMenu> menu = new KMenu(0);
    QList<KAction*> actions;
    if (QTreeWidgetItem *baseitem = d->m_ui->m_semanticView->itemAt(position)) {
        if (KoRdfSemanticTreeWidgetItem *item = dynamic_cast<KoRdfSemanticTreeWidgetItem*>(baseitem)) {
            actions = item->actions(menu);
        }
    }
    if (actions.count() > 0) {
        foreach (KAction *a, actions) {
            menu->addAction(a);
        }
        menu->exec(d->m_ui->m_semanticView->mapToGlobal(position));
    }
    delete menu;
}

void KoDocumentRdfEditWidget::addTriple()
{
    KoDocumentRdf *m_rdf = d->m_rdf;
    //
    // We have to make sure the new triple is unique, so the object
    // is set to a bnode value. Because the user most likely doesn't
    // want to create a bnode, we change it to a URI first.
    //
    Soprano::Node obj(QUrl(m_rdf->model()->createBlankNode().toString()));
    Soprano::Statement st(Soprano::Node(QUrl("http://koffice.org/new-node")),
                          Soprano::Node(QUrl("http://koffice.org/new-node")),
                          obj,
                          m_rdf->manifestRdfNode());
    int newRowNumber = d->m_tripleModel->insertStatement(st);
    QModelIndex pidx = d->mapFromSource(d->m_tripleModel->index(newRowNumber, 0));
    // select newrow
    d->clearTriplesSelection();
    d->selectTriples(pidx.row());
    d->m_ui->m_tripleView->scrollTo(pidx);
}

void KoDocumentRdfEditWidget::copyTriples()
{
    QModelIndexList col = d->m_ui->m_tripleView->selectionModel()->selectedRows();
    QModelIndexList newRowsBackend = d->m_tripleModel->copyTriples(d->mapToSource(col));
    QModelIndexList newRows = d->mapFromSource(newRowsBackend);
    d->clearTriplesSelection();
    d->selectTriples(newRows);
    if (!newRows.isEmpty()) {
        d->m_ui->m_tripleView->scrollTo(newRows.first());
    }
}

void KoDocumentRdfEditWidget::deleteTriples()
{
    QModelIndexList col = d->m_ui->m_tripleView->selectionModel()->selectedRows();
    d->m_tripleModel->deleteTriples(d->mapToSource(col));
}

void KoDocumentRdfEditWidget::addNamespace()
{
    KoDocumentRdf *m_rdf = d->m_rdf;
    QTreeWidget *v = d->m_ui->m_namespaceView;
    QString key = QString("newnamespace%1").arg(QDateTime::currentDateTime().toTime_t());
    QString value = "http://www.example.com/fixme#";
    KoRdfPrefixMapping *mapping = m_rdf->prefixMapping();
    mapping->insert(key, value);
    kDebug(30015) << "adding key:" << key << " value:" << value;
    KoRdfPrefixMappingTreeWidgetItem* item =
        new KoRdfPrefixMappingTreeWidgetItem(mapping, key);
    v->addTopLevelItem(item);
    v->setCurrentItem(item);
    v->editItem(item);
}

void KoDocumentRdfEditWidget::deleteNamespace()
{
    QTreeWidget *v = d->m_ui->m_namespaceView;
    QList<QTreeWidgetItem *> sel = v->selectedItems();
    kDebug(30015) << "selection.sz:" << sel.size();
    foreach (QTreeWidgetItem *item, sel) {
        if (KoRdfPrefixMappingTreeWidgetItem *ritem
                = dynamic_cast<KoRdfPrefixMappingTreeWidgetItem *>(item)) {
            ritem->removeFromMapping();
        }
        v->invisibleRootItem()->removeChild(item);
    }
}

void KoDocumentRdfEditWidget::sparqlExecute()
{
    d->m_ui->m_sparqlResultView->selectionModel()->clear();
    QString sparql = d->m_ui->m_sparqlQuery->toPlainText();
    Soprano::Model *m = d->m_rdf->model();
    kDebug(30015) << "running SPARQL query:" << sparql;
    Soprano::QueryResultIterator qrIter =
        m->executeQuery(sparql, Soprano::Query::QueryLanguageSparql);
    QList<Soprano::BindingSet> bindings = qrIter.allBindings();
    QTableWidget *tableWidget = d->m_ui->m_sparqlResultView;
    tableWidget->setSortingEnabled(false);
    tableWidget->setRowCount(bindings.size());
    int row = 0;
    int column = 0;
    QListIterator<Soprano::BindingSet> it(bindings);
    for (; it.hasNext(); column = 0, row++) {
        Soprano::BindingSet bs = it.next();
        QStringList bindingNames = bs.bindingNames();
        kDebug(30015) << "bindings.sz:" << bindingNames.size();
        if (row == 0) {
            tableWidget->setColumnCount(bindingNames.size());
            tableWidget->setHorizontalHeaderLabels(bindingNames);
        }
        foreach (const QString &b, bindingNames) {
            QString cellText = bs[b].toString();
            QTableWidgetItem *newItem = new QTableWidgetItem(cellText);
            tableWidget->setItem(row, column, newItem);
            kDebug(30015) << "r:" << row << " c:" << column << " data:" << cellText;
            ++column;
        }
    }
    tableWidget->setSortingEnabled(true);
}

KoSemanticStylesheet *KoDocumentRdfEditWidget::stylesheetFromComboBox(QComboBox *w) const
{
    QAbstractItemModel *m = w->model();
    QVariant ud = m->data(m->index(w->currentIndex(), 0), Qt::UserRole);
    KoSemanticStylesheet *ss = ud.value<KoSemanticStylesheet*>();
    return ss;
}

//
// build a map end-pos -> { extent, semitem*, xmlid }
// and apply in reverse order.
//
void KoDocumentRdfEditWidget::defaultContactsSheetButton()
{
    KoDocumentRdf *rdf = d->m_rdf;
    Q_ASSERT(rdf);
    rdf->ensureTextTool();
    QString stylesheetName = d->m_ui->m_defaultContactsSheet->currentText();
    kDebug(30015) << "changing contact default stylesheet to:" << stylesheetName;
    KoSemanticStylesheet *ss = stylesheetFromComboBox(d->m_ui->m_defaultContactsSheet);
    if (KoRdfSemanticItem *si = KoRdfSemanticItem::createSemanticItem(0, rdf, "Contact")) {
        si->defaultStylesheet(ss);
        delete si;
    }
    QMap<int, KoDocumentRdf::reflowItem> reflowCol;
    QList<KoRdfFoaF*> col = rdf->foaf();
    foreach (KoRdfSemanticItem* obj, col) {
        rdf->insertReflow(reflowCol, obj, ss);
    }
    rdf->applyReflow(reflowCol);
}

void KoDocumentRdfEditWidget::defaultEventsSheetButton()
{
    KoDocumentRdf *rdf = d->m_rdf;
    Q_ASSERT(rdf);
    rdf->ensureTextTool();
    QString stylesheetName = d->m_ui->m_defaultEventsSheet->currentText();
    kDebug(30015) << "changing event default stylesheet to:" << stylesheetName;
    KoSemanticStylesheet *ss = stylesheetFromComboBox(d->m_ui->m_defaultEventsSheet);
    if (KoRdfSemanticItem* si = KoRdfSemanticItem::createSemanticItem(0, rdf, "Event")) {
        si->defaultStylesheet(ss);
        delete si;
    }
    QMap<int, KoDocumentRdf::reflowItem> reflowCol;
    QList<KoRdfCalendarEvent*> col = rdf->calendarEvents();
    foreach (KoRdfSemanticItem* obj, col) {
        rdf->insertReflow(reflowCol, obj, ss);
    }
    rdf->applyReflow(reflowCol);
}

void KoDocumentRdfEditWidget::defaultLocationsSheetButton()
{
    KoDocumentRdf *rdf = d->m_rdf;
    Q_ASSERT(rdf);
    rdf->ensureTextTool();
    QString stylesheetName = d->m_ui->m_defaultLocationsSheet->currentText();
    kDebug(30015) << stylesheetName;
    KoSemanticStylesheet *ss = stylesheetFromComboBox(d->m_ui->m_defaultLocationsSheet);
    if (KoRdfSemanticItem* si = KoRdfSemanticItem::createSemanticItem(0, rdf, "Location")) {
        si->defaultStylesheet(ss);
        delete si;
    }
    QMap<int, KoDocumentRdf::reflowItem> reflowCol;
    QList<KoRdfLocation*> col = rdf->locations();
    foreach (KoRdfSemanticItem *obj, col) {
        rdf->insertReflow(reflowCol, obj, ss);
    }
    rdf->applyReflow(reflowCol);
}

void KoDocumentRdfEditWidget::defaultAllSheetButton()
{
    defaultContactsSheetButton();
    defaultEventsSheetButton();
    defaultLocationsSheetButton();
}

#include <KoDocumentRdfEditWidget.moc>

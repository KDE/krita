/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
   Copyright     2007 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kedittoolbar.h"
#include "kedittoolbar_p.h"

#include "config-xmlgui.h"

#include <QShowEvent>
#include <QAction>
#include <QDialogButtonBox>
#include <QDomDocument>
#include <QLayout>
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QToolButton>
#include <QLabel>
#include <QApplication>
#include <QGridLayout>
#include <QCheckBox>
#include <QMimeData>
#include <QPushButton>
#include <QStandardPaths>
#include <QComboBox>
#include <QLineEdit>
#include <QDebug>

#ifdef HAVE_ICONTHEMES
#include <kicondialog.h>
#endif
#include <klistwidgetsearchline.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kconfig.h>

#include "kactioncollection.h"
#include "kxmlguifactory.h"
#include "ktoolbar.h"

#include <kis_icon_utils.h>
#include "kis_action_registry.h"

static const char separatorstring[] = I18N_NOOP("--- separator ---");

#define SEPARATORSTRING i18n(separatorstring)

//static const char *const s_XmlTypeToString[] = { "Shell", "Part", "Local", "Merged" };

typedef QList<QDomElement> ToolBarList;

namespace KDEPrivate
{

/**
 * Return a list of toolbar elements given a toplevel element
 */
static ToolBarList findToolBars(const QDomElement &start)
{
    ToolBarList list;

    for (QDomElement elem = start; !elem.isNull(); elem = elem.nextSiblingElement()) {
        if (elem.tagName() == QStringLiteral("ToolBar")) {
            if (elem.attribute(QStringLiteral("noEdit")) != QLatin1String("true")) {
                list.append(elem);
            }
        } else {
            if (elem.tagName() != QStringLiteral("MenuBar")) { // there are no toolbars inside the menubar :)
                list += findToolBars(elem.firstChildElement());    // recursive
            }
        }
    }

    return list;
}

class XmlData
{
public:
    enum XmlType { Shell = 0, Part, Local, Merged };

    explicit XmlData(XmlType xmlType, const QString &xmlFile, KActionCollection *collection)
        : m_isModified(false)
        , m_xmlFile(xmlFile)
        , m_type(xmlType)
        , m_actionCollection(collection)
    {
    }

    ~XmlData()
    {
    }

    void dump() const
    {
#if 0
        qDebug() << "XmlData" << this << "xmlFile:" << m_xmlFile;
        foreach (const QDomElement &element, m_barList) {
            qDebug() << "    ToolBar:" << toolBarText(element);
        }
        //KisActionRegistry::instance()->
        if (m_actionCollection) {
            qDebug() << "    " << m_actionCollection->actions().count() << "actions in the collection.";
        } else {
            qDebug() << "    no action collection.";
        }
#endif
    }

    QString xmlFile() const
    {
        return m_xmlFile;
    }

    XmlType type() const
    {
        return m_type;
    }

    KActionCollection *actionCollection() const
    {
        return m_actionCollection;
    }

    void setDomDocument(const QDomDocument &domDoc)
    {
        m_document = domDoc.cloneNode().toDocument();
        m_barList = findToolBars(m_document.documentElement());
    }

    // Return reference, for e.g. actionPropertiesElement() to modify the document
    QDomDocument &domDocument()
    {
        return m_document;
    }

    const QDomDocument &domDocument() const
    {
        return m_document;
    }

    /**
     * Return the text (user-visible name) of a given toolbar
     */
    QString toolBarText(const QDomElement &it) const;

    bool m_isModified;

    ToolBarList &barList()
    {
        return m_barList;
    }

    const ToolBarList &barList() const
    {
        return m_barList;
    }

private:
    ToolBarList  m_barList;
    QString      m_xmlFile;
    QDomDocument m_document;
    XmlType      m_type;
    KActionCollection *m_actionCollection {0};
};

QString XmlData::toolBarText(const QDomElement &it) const
{
    const QLatin1String attrName("name");

    QString name;
    QByteArray txt(it.namedItem(QStringLiteral("text")).toElement().text().toUtf8());
    if (txt.isEmpty()) {
        txt = it.namedItem(QStringLiteral("text")).toElement().text().toUtf8();
    }
    if (txt.isEmpty()) {
        name = it.attribute(attrName);
    } else {
        QByteArray domain = it.namedItem(QStringLiteral("text")).toElement().attribute(QStringLiteral("translationDomain")).toUtf8();
        if (domain.isEmpty()) {
            domain = it.ownerDocument().documentElement().attribute(QStringLiteral("translationDomain")).toUtf8();
            if (domain.isEmpty()) {
                domain = KLocalizedString::applicationDomain();
            }
        }
        name = i18nd(domain.constData(), txt.constData());
    }

    // the name of the toolbar might depend on whether or not
    // it is in kparts
    if ((m_type == XmlData::Shell) ||
            (m_type == XmlData::Part)) {
        QString doc_name(m_document.documentElement().attribute(attrName));
        name += QStringLiteral(" <") + doc_name + QLatin1Char('>');
    }
    return name;
}

class ToolBarItem : public QListWidgetItem
{
public:
    ToolBarItem(QListWidget *parent, const QString &tag = QString(), const QString &name = QString(), const QString &statusText = QString())
        : QListWidgetItem(parent),
          m_internalTag(tag),
          m_internalName(name),
          m_statusText(statusText),
          m_isSeparator(false),
          m_isTextAlongsideIconHidden(false)
    {
        // Drop between items, not onto items
        setFlags((flags() | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
    }

    void setInternalTag(const QString &tag)
    {
        m_internalTag = tag;
    }
    void setInternalName(const QString &name)
    {
        m_internalName = name;
    }
    void setStatusText(const QString &text)
    {
        m_statusText = text;
    }
    void setSeparator(bool sep)
    {
        m_isSeparator = sep;
    }
    void setTextAlongsideIconHidden(bool hidden)
    {
        m_isTextAlongsideIconHidden = hidden;
    }
    QString internalTag() const
    {
        return m_internalTag;
    }
    QString internalName() const
    {
        return m_internalName;
    }
    QString statusText() const
    {
        return m_statusText;
    }
    bool isSeparator() const
    {
        return m_isSeparator;
    }
    bool isTextAlongsideIconHidden() const
    {
        return m_isTextAlongsideIconHidden;
    }

    int index() const
    {
        return listWidget()->row(const_cast<ToolBarItem *>(this));
    }

private:
    QString m_internalTag;
    QString m_internalName;
    QString m_statusText;
    bool m_isSeparator;
    bool m_isTextAlongsideIconHidden;
};

static QDataStream &operator<< (QDataStream &s, const ToolBarItem &item)
{
    s << item.internalTag();
    s << item.internalName();
    s << item.statusText();
    s << item.isSeparator();
    s << item.isTextAlongsideIconHidden();
    return s;
}
static QDataStream &operator>> (QDataStream &s, ToolBarItem &item)
{
    QString internalTag;
    s >> internalTag;
    item.setInternalTag(internalTag);
    QString internalName;
    s >> internalName;
    item.setInternalName(internalName);
    QString statusText;
    s >> statusText;
    item.setStatusText(statusText);
    bool sep;
    s >> sep;
    item.setSeparator(sep);
    bool hidden;
    s >> hidden;
    item.setTextAlongsideIconHidden(hidden);
    return s;
}

////

ToolBarListWidget::ToolBarListWidget(QWidget *parent)
    : QListWidget(parent),
      m_activeList(true)
{
    setDragDropMode(QAbstractItemView::DragDrop); // no internal moves
}

QMimeData *ToolBarListWidget::mimeData(const QList<QListWidgetItem *> items) const
{
    if (items.isEmpty()) {
        return 0;
    }
    QMimeData *mimedata = new QMimeData();

    QByteArray data;
    {
        QDataStream stream(&data, QIODevice::WriteOnly);
        // we only support single selection
        ToolBarItem *item = static_cast<ToolBarItem *>(items.first());
        stream << *item;
    }

    mimedata->setData(QStringLiteral("application/x-kde-action-list"), data);
    mimedata->setData(QStringLiteral("application/x-kde-source-treewidget"), m_activeList ? "active" : "inactive");

    return mimedata;
}

bool ToolBarListWidget::dropMimeData(int index, const QMimeData *mimeData, Qt::DropAction action)
{
    Q_UNUSED(action)
    const QByteArray data = mimeData->data(QStringLiteral("application/x-kde-action-list"));
    if (data.isEmpty()) {
        return false;
    }
    QDataStream stream(data);
    const bool sourceIsActiveList = mimeData->data(QStringLiteral("application/x-kde-source-treewidget")) == "active";
    ToolBarItem *item = new ToolBarItem(this); // needs parent, use this temporarily
    stream >> *item;
    emit dropped(this, index, item, sourceIsActiveList);
    return true;
}

ToolBarItem *ToolBarListWidget::currentItem() const
{
    return static_cast<ToolBarItem *>(QListWidget::currentItem());
}

IconTextEditDialog::IconTextEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Change Text"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setClearButtonEnabled(true);
    QLabel *label = new QLabel(i18n("Icon te&xt:"), this);
    label->setBuddy(m_lineEdit);
    grid->addWidget(label, 0, 0);
    grid->addWidget(m_lineEdit, 0, 1);

    m_cbHidden = new QCheckBox(i18n("&Hide text when toolbar shows text alongside icons"), this);
    grid->addWidget(m_cbHidden, 1, 1);

    layout->addLayout(grid);

    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(m_buttonBox);

    connect(m_lineEdit, SIGNAL(textChanged(QString)), SLOT(slotTextChanged(QString)));

    m_lineEdit->setFocus();
    setFixedHeight(sizeHint().height());
}

void IconTextEditDialog::setIconText(const QString &text)
{
    m_lineEdit->setText(text);
}

QString IconTextEditDialog::iconText() const
{
    return m_lineEdit->text().trimmed();
}

void IconTextEditDialog::setTextAlongsideIconHidden(bool hidden)
{
    m_cbHidden->setChecked(hidden);
}

bool IconTextEditDialog::textAlongsideIconHidden() const
{
    return m_cbHidden->isChecked();
}

void IconTextEditDialog::slotTextChanged(const QString &text)
{
    // Do not allow empty icon text
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.trimmed().isEmpty());
}

class KEditToolBarWidgetPrivate
{
public:
    /**
     *
     * @param collection In the old-style constructor, this is the collection passed
     * to the KEditToolBar constructor.
     * In the xmlguifactory-based constructor, we let KXMLGUIClient create a dummy one,
     * but it probably isn't used.
     */
    KEditToolBarWidgetPrivate(KEditToolBarWidget *widget,
                              const QString &cName, KActionCollection *collection)
        : m_collection(collection),
          m_widget(widget),
          m_factory(0),
          m_loadedOnce(false)
    {
        m_componentName = cName;
        m_isPart   = false;
        m_helpArea = 0L;
        // We want items with an icon to align with items without icon
        // So we use an empty QPixmap for that
        const int iconSize = widget->style()->pixelMetric(QStyle::PM_SmallIconSize);
        m_emptyIcon = QPixmap(iconSize, iconSize);
        m_emptyIcon.fill(Qt::transparent);
    }
    ~KEditToolBarWidgetPrivate()
    {
    }

    // private slots
    void slotToolBarSelected(int index);

    void slotInactiveSelectionChanged();
    void slotActiveSelectionChanged();

    void slotInsertButton();
    void slotRemoveButton();
    void slotUpButton();
    void slotDownButton();

    void selectActiveItem(const QString &);

    void slotDropped(ToolBarListWidget *list, int index, ToolBarItem *item, bool sourceIsActiveList);

    void setupLayout();

    void initOldStyle(const QString &file, bool global, const QString &defaultToolbar);
    void initFromFactory(KXMLGUIFactory *factory, const QString &defaultToolbar);
    void loadToolBarCombo(const QString &defaultToolbar);
    void loadActions(const QDomElement &elem);

    QString xmlFile(const QString &xml_file) const
    {
        return xml_file.isEmpty() ? m_componentName + QStringLiteral("ui.xmlgui") : xml_file;
    }

    /**
     * Load in the specified XML file and dump the raw xml
     */
    QString loadXMLFile(const QString &_xml_file)
    {
        QString raw_xml;
        QString xml_file = xmlFile(_xml_file);
        //qDebug() << "loadXMLFile xml_file=" << xml_file;

        if (!QDir::isRelativePath(xml_file)) {
            raw_xml = KXMLGUIFactory::readConfigFile(xml_file);
        } else {
            raw_xml = KXMLGUIFactory::readConfigFile(xml_file, m_componentName);
        }

        return raw_xml;
    }

    /**
     * Look for a given item in the current toolbar
     */
    QDomElement findElementForToolBarItem(const ToolBarItem *item) const
    {
        //qDebug(240) << "looking for name=" << item->internalName() << "and tag=" << item->internalTag();
        for (QDomNode n = m_currentToolBarElem.firstChild(); !n.isNull(); n = n.nextSibling()) {
            QDomElement elem = n.toElement();
            if ((elem.attribute(QStringLiteral("name")) == item->internalName()) &&
                    (elem.tagName() == item->internalTag())) {
                return elem;
            }
        }
        //qDebug(240) << "no item found in the DOM with name=" << item->internalName() << "and tag=" << item->internalTag();
        return QDomElement();
    }

    void insertActive(ToolBarItem *item, ToolBarItem *before, bool prepend = false);
    void removeActive(ToolBarItem *item);
    void moveActive(ToolBarItem *item, ToolBarItem *before);
    void updateLocal(QDomElement &elem);

#ifndef NDEBUG
    void dump() const
    {
        QList<XmlData>::const_iterator xit = m_xmlFiles.begin();
        for (; xit != m_xmlFiles.end(); ++xit) {
            (*xit).dump();
        }
    }
#endif

    QComboBox *m_toolbarCombo;

    QToolButton *m_upAction;
    QToolButton *m_removeAction;
    QToolButton *m_insertAction;
    QToolButton *m_downAction;

    //QValueList<QAction*> m_actionList;
    KActionCollection *m_collection;
    KEditToolBarWidget *m_widget;
    KXMLGUIFactory *m_factory;
    QString m_componentName;

    QPixmap m_emptyIcon;

    XmlData     *m_currentXmlData;
    QDomElement m_currentToolBarElem;

    QString            m_xmlFile;
    QString            m_globalFile;
    QString            m_rcFile;
    QDomDocument       m_localDoc;

    ToolBarList        m_barList;
    ToolBarListWidget *m_inactiveList;
    ToolBarListWidget *m_activeList;

    QList<XmlData> m_xmlFiles;

    QLabel     *m_comboLabel;
    KSeparator *m_comboSeparator;
    QLabel *m_helpArea;
    bool m_isPart : 1;
    bool m_loadedOnce : 1;
};

}

using namespace KDEPrivate;

class KEditToolBarPrivate
{
public:
    KEditToolBarPrivate(KEditToolBar *q): q(q),
        m_accept(false), m_global(false),
        m_collection(0), m_factory(0), m_widget(0) {}

    void init();

    void _k_slotButtonClicked(QAbstractButton *button);
    void _k_acceptOK(bool);
    void _k_enableApply(bool);
    void okClicked();
    void applyClicked();
    void defaultClicked();

    KEditToolBar *q;
    bool m_accept;
    // Save parameters for recreating widget after resetting toolbar
    bool m_global;
    KActionCollection *m_collection;
    QString m_file;
    QString m_defaultToolBar;
    KXMLGUIFactory *m_factory;
    KEditToolBarWidget *m_widget;
    QVBoxLayout *m_layout;
    QDialogButtonBox *m_buttonBox;
};

Q_GLOBAL_STATIC(QString, s_defaultToolBarName)

KEditToolBar::KEditToolBar(KXMLGUIFactory *factory,
                           QWidget *parent)
    : QDialog(parent),
      d(new KEditToolBarPrivate(this))
{
    d->m_widget = new KEditToolBarWidget(this);
    d->init();
    d->m_factory = factory;
}

void KEditToolBarPrivate::init()
{
    m_accept = false;
    m_factory = 0;

    q->setDefaultToolBar(QString());

    q->setWindowTitle(i18n("Configure Toolbars"));
    q->setModal(false);

    m_layout = new QVBoxLayout;
    q->setLayout(m_layout);

    m_layout->addWidget(m_widget);

    m_buttonBox = new QDialogButtonBox(q);
    m_buttonBox->setStandardButtons(QDialogButtonBox::RestoreDefaults
                                    | QDialogButtonBox::Ok
                                    | QDialogButtonBox::Apply
                                    | QDialogButtonBox::Cancel);
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::apply());
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    q->connect(m_buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(_k_slotButtonClicked(QAbstractButton*)));
    q->connect(m_buttonBox, SIGNAL(rejected()), SLOT(reject()));
    m_layout->addWidget(m_buttonBox);

    q->connect(m_widget, SIGNAL(enableOk(bool)), SLOT(_k_acceptOK(bool)));
    q->connect(m_widget, SIGNAL(enableOk(bool)), SLOT(_k_enableApply(bool)));
    _k_enableApply(false);

    q->setMinimumSize(q->sizeHint());
}

void KEditToolBar::setResourceFile(const QString &file, bool global)
{
    d->m_file = file;
    d->m_global = global;
    d->m_widget->load(d->m_file, d->m_global, d->m_defaultToolBar);
}

KEditToolBar::~KEditToolBar()
{
    delete d;
    s_defaultToolBarName()->clear();
}

void KEditToolBar::setDefaultToolBar(const QString &toolBarName)
{
    if (toolBarName.isEmpty()) {
        d->m_defaultToolBar = *s_defaultToolBarName();
    } else {
        d->m_defaultToolBar = toolBarName;
    }
}

void KEditToolBarPrivate::_k_acceptOK(bool b)
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(b);
    m_accept = b;
}

void KEditToolBarPrivate::_k_enableApply(bool b)
{
    m_buttonBox->button(QDialogButtonBox::Apply)->setEnabled(b);
}

void KEditToolBarPrivate::defaultClicked()
{
    if (KMessageBox::warningContinueCancel(q, i18n("Do you really want to reset all toolbars of this application to their default? The changes will be applied immediately."), i18n("Reset Toolbars"), KGuiItem(i18n("Reset"))) != KMessageBox::Continue) {
        return;
    }

    KEditToolBarWidget *oldWidget = m_widget;
    m_widget = 0;
    m_accept = false;

    if (m_factory) {
        foreach (KXMLGUIClient *client, m_factory->clients()) {
            const QString file = client->localXMLFile();
            if (file.isEmpty()) {
                continue;
            }
            //qDebug(240) << "Deleting local xml file" << file;
            // << "for client" << client << typeid(*client).name();
            if (QFile::exists(file))
                if (!QFile::remove(file)) {
                    qWarning() << "Could not delete" << file;
                }
        }

        // Reload the xml files in all clients, now that the local files are gone
        oldWidget->rebuildKXMLGUIClients();

        m_widget = new KEditToolBarWidget(q);
        m_widget->load(m_factory, m_defaultToolBar);
    } else {
        int slash = m_file.lastIndexOf(QLatin1Char('/')) + 1;
        if (slash) {
            m_file = m_file.mid(slash);
        }
        const QString xml_file = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
            QStringLiteral("/kxmlgui5/") + QCoreApplication::instance()->applicationName() + QLatin1Char('/') + m_file;

        if (QFile::exists(xml_file))
            if (!QFile::remove(xml_file)) {
                qWarning() << "Could not delete " << xml_file;
            }

        m_widget = new KEditToolBarWidget(m_collection, q);
        q->setResourceFile(m_file, m_global);
    }

    // Copy the geometry to minimize UI flicker
    m_widget->setGeometry(oldWidget->geometry());
    delete oldWidget;
    m_layout->insertWidget(0, m_widget);

    q->connect(m_widget, SIGNAL(enableOk(bool)), SLOT(_k_acceptOK(bool)));
    q->connect(m_widget, SIGNAL(enableOk(bool)), SLOT(_k_enableApply(bool)));

    _k_enableApply(false);

    emit q->newToolBarConfig();
    emit q->newToolbarConfig(); // compat
}

void KEditToolBarPrivate::_k_slotButtonClicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton type = m_buttonBox->standardButton(button);

    switch (type) {
    case QDialogButtonBox::Ok:
        okClicked();
        break;
    case QDialogButtonBox::Apply:
        applyClicked();
        break;
    case QDialogButtonBox::RestoreDefaults:
        defaultClicked();
        break;
    default:
        break;
    }
}

void KEditToolBarPrivate::okClicked()
{
    if (!m_accept) {
        q->reject();
        return;
    }

    // Do not rebuild GUI and emit the "newToolBarConfig" signal again here if the "Apply"
    // button was already pressed and no further changes were made.
    if (m_buttonBox->button(QDialogButtonBox::Apply)->isEnabled()) {
        m_widget->save();
        emit q->newToolBarConfig();
        emit q->newToolbarConfig(); // compat
    }
    q->accept();
}

void KEditToolBarPrivate::applyClicked()
{
    (void)m_widget->save();
    _k_enableApply(false);
    emit q->newToolBarConfig();
    emit q->newToolbarConfig(); // compat
}

void KEditToolBar::setGlobalDefaultToolBar(const char *toolbarName)
{
    *s_defaultToolBarName() = QString::fromLatin1(toolbarName);
}

KEditToolBarWidget::KEditToolBarWidget(KActionCollection *collection,
                                       QWidget *parent)
    : QWidget(parent),
      d(new KEditToolBarWidgetPrivate(this, componentName(), collection))
{
    d->setupLayout();
}

KEditToolBarWidget::KEditToolBarWidget(QWidget *parent)
    : QWidget(parent),
      d(new KEditToolBarWidgetPrivate(this, componentName(), KXMLGUIClient::actionCollection() /*create new one*/))
{
    d->setupLayout();
}

KEditToolBarWidget::~KEditToolBarWidget()
{
    delete d;
}

void KEditToolBarWidget::load(const QString &file, bool global, const QString &defaultToolBar)
{
    d->initOldStyle(file, global, defaultToolBar);
}

void KEditToolBarWidget::load(KXMLGUIFactory *factory, const QString &defaultToolBar)
{
    d->initFromFactory(factory, defaultToolBar);
}

void KEditToolBarWidgetPrivate::initOldStyle(const QString &resourceFile,
        bool global,
        const QString &defaultToolBar)
{
    qDebug() << "initOldStyle";
    //TODO: make sure we can call this multiple times?
    if (m_loadedOnce) {
        return;
    }

    m_loadedOnce = true;
    //d->m_actionList = collection->actions();

    // handle the merging
    if (global) {
        m_widget->loadStandardsXmlFile();    // ui_standards.xmlgui
    }
    const QString localXML = loadXMLFile(resourceFile);
    m_widget->setXML(localXML, global ? true /*merge*/ : false);

    // first, get all of the necessary info for our local xml
    XmlData local(XmlData::Local, xmlFile(resourceFile), m_collection);
    QDomDocument domDoc;
    domDoc.setContent(localXML);
    local.setDomDocument(domDoc);
    m_xmlFiles.append(local);

    // then, the merged one (ui_standards + local xml)
    XmlData merge(XmlData::Merged, QString(), m_collection);
    merge.setDomDocument(m_widget->domDocument());
    m_xmlFiles.append(merge);

#ifndef NDEBUG
    dump();
#endif

    // now load in our toolbar combo box
    loadToolBarCombo(defaultToolBar);
    m_widget->adjustSize();
    m_widget->setMinimumSize(m_widget->sizeHint());
}

void KEditToolBarWidgetPrivate::initFromFactory(KXMLGUIFactory *factory,
        const QString &defaultToolBar)
{
    qDebug() << "initFromFactory";
    //TODO: make sure we can call this multiple times?
    if (m_loadedOnce) {
        return;
    }

    m_loadedOnce = true;

    m_factory = factory;

    // add all of the client data
    bool first = true;
    foreach (KXMLGUIClient *client, factory->clients()) {
        if (client->xmlFile().isEmpty()) {
            continue;
        }

        XmlData::XmlType type = XmlData::Part;
        if (first) {
            type = XmlData::Shell;
            first = false;
            Q_ASSERT(!client->localXMLFile().isEmpty()); // where would we save changes??
        }

        XmlData data(type, client->localXMLFile(), client->actionCollection());
        QDomDocument domDoc = client->domDocument();
        data.setDomDocument(domDoc);
        m_xmlFiles.append(data);

        //d->m_actionList += client->actionCollection()->actions();
    }

#ifndef NDEBUG
    //d->dump();
#endif

    // now load in our toolbar combo box
    loadToolBarCombo(defaultToolBar);
    m_widget->adjustSize();
    m_widget->setMinimumSize(m_widget->sizeHint());

    m_widget->actionCollection()->addAssociatedWidget(m_widget);
    foreach (QAction *action, m_widget->actionCollection()->actions()) {
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    }
}

void KEditToolBarWidget::save()
{
    //qDebug(240) << "KEditToolBarWidget::save";
    QList<XmlData>::Iterator it = d->m_xmlFiles.begin();
    for (; it != d->m_xmlFiles.end(); ++it) {
        // let's not save non-modified files
        if (!((*it).m_isModified)) {
            continue;
        }

        // let's also skip (non-existent) merged files
        if ((*it).type() == XmlData::Merged) {
            continue;
        }

        // Add noMerge="1" to all the menus since we are saving the merged data
        QDomNodeList menuNodes = (*it).domDocument().elementsByTagName(QStringLiteral("Menu"));
        for (int i = 0; i < menuNodes.length(); ++i) {
            QDomNode menuNode = menuNodes.item(i);
            QDomElement menuElement = menuNode.toElement();
            if (menuElement.isNull()) {
                continue;
            }
            menuElement.setAttribute(QStringLiteral("noMerge"), QLatin1String("1"));
        }

        //qDebug() << (*it).domDocument().toString();

        //qDebug(240) << "Saving " << (*it).xmlFile();
        // if we got this far, we might as well just save it
        KXMLGUIFactory::saveConfigFile((*it).domDocument(), (*it).xmlFile());
    }

    if (!d->m_factory) {
        return;
    }

    rebuildKXMLGUIClients();
}

void KEditToolBarWidget::rebuildKXMLGUIClients()
{
    if (!d->m_factory) {
        return;
    }

    const QList<KXMLGUIClient *> clients = d->m_factory->clients();
    //qDebug(240) << "factory: " << clients.count() << " clients";

    // remove the elements starting from the last going to the first
    if (!clients.count()) {
        return;
    }

    QListIterator<KXMLGUIClient *> clientIterator = clients;
    clientIterator.toBack();
    while (clientIterator.hasPrevious()) {
        KXMLGUIClient *client = clientIterator.previous();
        //qDebug(240) << "factory->removeClient " << client;
        d->m_factory->removeClient(client);
    }

    KXMLGUIClient *firstClient = clients.first();

    // now, rebuild the gui from the first to the last
    //qDebug(240) << "rebuilding the gui";
    foreach (KXMLGUIClient *client, clients) {
        //qDebug(240) << "updating client " << client << " " << client->componentName() << "  xmlFile=" << client->xmlFile();
        QString file(client->xmlFile());   // before setting ui_standards!
        if (!file.isEmpty()) {
            // passing an empty stream forces the clients to reread the XML
            client->setXMLGUIBuildDocument(QDomDocument());

            // for the shell, merge in ui_standards.xmlgui
            if (client == firstClient) { // same assumption as in the ctor: first==shell
                client->loadStandardsXmlFile();
            }

            // and this forces it to use the *new* XML file
            client->setXMLFile(file, client == firstClient /* merge if shell */);

            // [we can't use reloadXML, it doesn't load ui_standards.xmlgui]
        }
    }

    // Now we can add the clients to the factory
    // We don't do it in the loop above because adding a part automatically
    // adds its plugins, so we must make sure the plugins were updated first.
    foreach (KXMLGUIClient *client, clients) {
        d->m_factory->addClient(client);
    }
}

void KEditToolBarWidgetPrivate::setupLayout()
{
    // the toolbar name combo
    m_comboLabel = new QLabel(i18n("&Toolbar:"), m_widget);
    m_toolbarCombo = new QComboBox(m_widget);
    m_comboLabel->setBuddy(m_toolbarCombo);
    m_comboSeparator = new KSeparator(m_widget);
    QObject::connect(m_toolbarCombo, SIGNAL(activated(int)),
                     m_widget,       SLOT(slotToolBarSelected(int)));

//  QPushButton *new_toolbar = new QPushButton(i18n("&New"), this);
//  new_toolbar->setPixmap(BarIcon("document-new", KisIconUtils::SizeSmall));
//  new_toolbar->setEnabled(false); // disabled until implemented
//  QPushButton *del_toolbar = new QPushButton(i18n("&Delete"), this);
//  del_toolbar->setPixmap(BarIcon("edit-delete", KisIconUtils::SizeSmall));
//  del_toolbar->setEnabled(false); // disabled until implemented

    // our list of inactive actions
    QLabel *inactive_label = new QLabel(i18n("A&vailable actions:"), m_widget);
    m_inactiveList = new ToolBarListWidget(m_widget);
    m_inactiveList->setDragEnabled(true);
    m_inactiveList->setActiveList(false);
    m_inactiveList->setMinimumSize(180, 250);
    m_inactiveList->setDropIndicatorShown(false); // #165663
    inactive_label->setBuddy(m_inactiveList);
    QObject::connect(m_inactiveList, SIGNAL(itemSelectionChanged()),
                     m_widget,       SLOT(slotInactiveSelectionChanged()));
    QObject::connect(m_inactiveList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                     m_widget,       SLOT(slotInsertButton()));
    QObject::connect(m_inactiveList, SIGNAL(dropped(ToolBarListWidget*,int,ToolBarItem*,bool)),
                     m_widget,       SLOT(slotDropped(ToolBarListWidget*,int,ToolBarItem*,bool)));

    KListWidgetSearchLine *inactiveListSearchLine = new KListWidgetSearchLine(m_widget, m_inactiveList);
    inactiveListSearchLine->setPlaceholderText(i18n("Filter"));

    // our list of active actions
    QLabel *active_label = new QLabel(i18n("Curr&ent actions:"), m_widget);
    m_activeList = new ToolBarListWidget(m_widget);
    m_activeList->setDragEnabled(true);
    m_activeList->setActiveList(true);
    // With Qt-4.1 only setting MiniumWidth results in a 0-width icon column ...
    m_activeList->setMinimumSize(m_inactiveList->minimumWidth(), 100);
    active_label->setBuddy(m_activeList);

    QObject::connect(m_activeList, SIGNAL(itemSelectionChanged()),
                     m_widget,     SLOT(slotActiveSelectionChanged()));
    QObject::connect(m_activeList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                     m_widget,     SLOT(slotRemoveButton()));
    QObject::connect(m_activeList, SIGNAL(dropped(ToolBarListWidget*,int,ToolBarItem*,bool)),
                     m_widget,     SLOT(slotDropped(ToolBarListWidget*,int,ToolBarItem*,bool)));

    KListWidgetSearchLine *activeListSearchLine = new KListWidgetSearchLine(m_widget, m_activeList);
    activeListSearchLine->setPlaceholderText(i18n("Filter"));

    // The buttons in the middle

    m_upAction     = new QToolButton(m_widget);
    m_upAction->setIcon(KisIconUtils::loadIcon(QStringLiteral("arrow-up")));
    m_upAction->setEnabled(false);
    m_upAction->setAutoRepeat(true);
    QObject::connect(m_upAction, SIGNAL(clicked()), m_widget, SLOT(slotUpButton()));

    m_insertAction = new QToolButton(m_widget);
    m_insertAction->setIcon(KisIconUtils::loadIcon(QApplication::isRightToLeft() ? QStringLiteral("arrow-left") : QLatin1String("arrow-right")));
    m_insertAction->setEnabled(false);
    QObject::connect(m_insertAction, SIGNAL(clicked()), m_widget, SLOT(slotInsertButton()));

    m_removeAction = new QToolButton(m_widget);
    m_removeAction->setIcon(KisIconUtils::loadIcon(QApplication::isRightToLeft() ? QStringLiteral("arrow-right") : QLatin1String("arrow-left")));
    m_removeAction->setEnabled(false);
    QObject::connect(m_removeAction, SIGNAL(clicked()), m_widget, SLOT(slotRemoveButton()));

    m_downAction   = new QToolButton(m_widget);
    m_downAction->setIcon(KisIconUtils::loadIcon(QStringLiteral("arrow-down")));
    m_downAction->setEnabled(false);
    m_downAction->setAutoRepeat(true);
    QObject::connect(m_downAction, SIGNAL(clicked()), m_widget, SLOT(slotDownButton()));

    m_helpArea = new QLabel(m_widget);
    m_helpArea->setWordWrap(true);

    // now start with our layouts
    QVBoxLayout *top_layout = new QVBoxLayout(m_widget);
    top_layout->setMargin(0);

    QVBoxLayout *name_layout = new QVBoxLayout();
    QHBoxLayout *list_layout = new QHBoxLayout();

    QVBoxLayout *inactive_layout = new QVBoxLayout();
    QVBoxLayout *active_layout = new QVBoxLayout();

    QGridLayout *button_layout = new QGridLayout();

    name_layout->addWidget(m_comboLabel);
    name_layout->addWidget(m_toolbarCombo);
//  name_layout->addWidget(new_toolbar);
//  name_layout->addWidget(del_toolbar);

    button_layout->setSpacing(0);
    button_layout->setRowStretch(0, 10);
    button_layout->addWidget(m_upAction, 1, 1);
    button_layout->addWidget(m_removeAction, 2, 0);
    button_layout->addWidget(m_insertAction, 2, 2);
    button_layout->addWidget(m_downAction, 3, 1);
    button_layout->setRowStretch(4, 10);

    inactive_layout->addWidget(inactive_label);
    inactive_layout->addWidget(inactiveListSearchLine);
    inactive_layout->addWidget(m_inactiveList, 1);

    active_layout->addWidget(active_label);
    active_layout->addWidget(activeListSearchLine);
    active_layout->addWidget(m_activeList, 1);

    list_layout->addLayout(inactive_layout);
    list_layout->addLayout(button_layout);
    list_layout->addLayout(active_layout);

    top_layout->addLayout(name_layout);
    top_layout->addWidget(m_comboSeparator);
    top_layout->addLayout(list_layout, 10);
    top_layout->addWidget(m_helpArea);
    top_layout->addWidget(new KSeparator(m_widget));
}

void KEditToolBarWidgetPrivate::loadToolBarCombo(const QString &defaultToolBar)
{
    const QLatin1String attrName("name");
    // just in case, we clear our combo
    m_toolbarCombo->clear();

    int defaultToolBarId = -1;
    int count = 0;
    // load in all of the toolbar names into this combo box
    QList<XmlData>::const_iterator xit = m_xmlFiles.constBegin();
    for (; xit != m_xmlFiles.constEnd(); ++xit) {
        // skip the merged one in favor of the local one,
        // so that we can change icons
        // This also makes the app-defined named for "mainToolBar" appear rather than the ui_standards-defined name.
        if ((*xit).type() == XmlData::Merged) {
            continue;
        }

        // each xml file may have any number of toolbars
        ToolBarList::const_iterator it = (*xit).barList().begin();
        for (; it != (*xit).barList().constEnd(); ++it) {
            const QString text = (*xit).toolBarText(*it);
            m_toolbarCombo->addItem(text);
            const QString name = (*it).attribute(attrName);
            if (defaultToolBarId == -1 && name == defaultToolBar) {
                defaultToolBarId = count;
            }
            count++;
        }
    }
    const bool showCombo = (count > 1);
    m_comboLabel->setVisible(showCombo);
    m_comboSeparator->setVisible(showCombo);
    m_toolbarCombo->setVisible(showCombo);
    if (defaultToolBarId == -1) {
        defaultToolBarId = 0;
    }
    // we want to the specified item selected and its actions loaded
    m_toolbarCombo->setCurrentIndex(defaultToolBarId);
    slotToolBarSelected(m_toolbarCombo->currentIndex());
}

void KEditToolBarWidgetPrivate::loadActions(const QDomElement &elem)
{
    const QLatin1String tagSeparator("Separator");
    const QLatin1String tagMerge("Merge");
    const QLatin1String tagActionList("ActionList");
    const QLatin1String tagAction("Action");
    const QLatin1String attrName("name");

    int     sep_num = 0;
    QString sep_name(QStringLiteral("separator_%1"));

    // clear our lists
    m_inactiveList->clear();
    m_activeList->clear();
    m_insertAction->setEnabled(false);
    m_removeAction->setEnabled(false);
    m_upAction->setEnabled(false);
    m_downAction->setEnabled(false);

    // We'll use this action collection
    KActionCollection *actionCollection = m_currentXmlData->actionCollection();

    // store the names of our active actions
    QSet<QString> active_list;

    // Filtering message requested by translators (scripting).
    KLocalizedString nameFilter = ki18nc("@item:intable Action name in toolbar editor", "%1");

    // see if our current action is in this toolbar
    QDomNode n = elem.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        QDomElement it = n.toElement();
        if (it.isNull()) {
            continue;
        }
        if (it.tagName() == tagSeparator) {
            ToolBarItem *act = new ToolBarItem(m_activeList, tagSeparator, sep_name.arg(sep_num++), QString());
            act->setSeparator(true);
            act->setText(SEPARATORSTRING);
            it.setAttribute(attrName, act->internalName());
            continue;
        }

        if (it.tagName() == tagMerge) {
            // Merge can be named or not - use the name if there is one
            QString name = it.attribute(attrName);
            ToolBarItem *act = new ToolBarItem(m_activeList, tagMerge, name, i18n("This element will be replaced with all the elements of an embedded component."));
            if (name.isEmpty()) {
                act->setText(i18n("<Merge>"));
            } else {
                act->setText(i18n("<Merge %1>", name));
            }
            continue;
        }

        if (it.tagName() == tagActionList) {
            ToolBarItem *act = new ToolBarItem(m_activeList, tagActionList, it.attribute(attrName), i18n("This is a dynamic list of actions. You can move it, but if you remove it you will not be able to re-add it."));
            act->setText(i18n("ActionList: %1", it.attribute(attrName)));
            continue;
        }

        // iterate through this client's actions
        // This used to iterate through _all_ actions, but we don't support
        // putting any action into any client...
        foreach (QAction *action, actionCollection->actions()) {
            // do we have a match?
            if (it.attribute(attrName) == action->objectName()) {
                // we have a match!
                ToolBarItem *act = new ToolBarItem(m_activeList, it.tagName(), action->objectName(), action->toolTip());
                act->setText(nameFilter.subs(KLocalizedString::removeAcceleratorMarker(action->iconText())).toString());
                act->setIcon(!action->icon().isNull() ? action->icon() : m_emptyIcon);
                act->setTextAlongsideIconHidden(action->priority() < QAction::NormalPriority);

                active_list.insert(action->objectName());
                break;
            }
        }
    }

    // go through the rest of the collection
    foreach (QAction *action, actionCollection->actions()) {
        // skip our active ones
        if (active_list.contains(action->objectName())) {
            continue;
        }

        ToolBarItem *act = new ToolBarItem(m_inactiveList, tagAction, action->objectName(), action->toolTip());
        act->setText(nameFilter.subs(KLocalizedString::removeAcceleratorMarker(action->text())).toString());
        act->setIcon(!action->icon().isNull() ? action->icon() : m_emptyIcon);
    }

    m_inactiveList->sortItems(Qt::AscendingOrder);

    // finally, add default separators to the inactive list
    ToolBarItem *act = new ToolBarItem(0L, tagSeparator, sep_name.arg(sep_num++), QString());
    act->setSeparator(true);
    act->setText(SEPARATORSTRING);
    m_inactiveList->insertItem(0, act);
}

KActionCollection *KEditToolBarWidget::actionCollection() const
{
    return d->m_collection;
}

void KEditToolBarWidgetPrivate::slotToolBarSelected(int index)
{
    const QLatin1String attrName("name");
    // We need to find the XmlData and toolbar element for this index
    // To do that, we do the same iteration as the one which filled in the combobox.

    int toolbarNumber = 0;
    QList<XmlData>::iterator xit = m_xmlFiles.begin();
    for (; xit != m_xmlFiles.end(); ++xit) {

        // skip the merged one in favor of the local one,
        // so that we can change icons
        if ((*xit).type() == XmlData::Merged) {
            continue;
        }

        // each xml file may have any number of toolbars
        ToolBarList::Iterator it = (*xit).barList().begin();
        for (; it != (*xit).barList().end(); ++it) {

            // is this our toolbar?
            if (toolbarNumber == index) {

                // save our current settings
                m_currentXmlData = & (*xit);
                m_currentToolBarElem = *it;

                //qDebug() << "found toolbar" << m_currentXmlData->toolBarText(*it) << "m_currentXmlData set to";
                m_currentXmlData->dump();

                // If this is a Merged xmldata, clicking the "change icon" button would assert...
                Q_ASSERT(m_currentXmlData->type() != XmlData::Merged);

                // load in our values
                loadActions(m_currentToolBarElem);

                if ((*xit).type() == XmlData::Part || (*xit).type() == XmlData::Shell) {
                    m_widget->setDOMDocument((*xit).domDocument());
                }
                return;
            }
            ++toolbarNumber;

        }
    }
}

void KEditToolBarWidgetPrivate::slotInactiveSelectionChanged()
{
    if (m_inactiveList->selectedItems().count()) {
        m_insertAction->setEnabled(true);
        QString statusText = static_cast<ToolBarItem *>(m_inactiveList->selectedItems().first())->statusText();
        m_helpArea->setText(i18nc("@label Action tooltip in toolbar editor, below the action list", "%1", statusText));
    } else {
        m_insertAction->setEnabled(false);
        m_helpArea->setText(QString());
    }
}

void KEditToolBarWidgetPrivate::slotActiveSelectionChanged()
{
    ToolBarItem *toolitem = 0;
    if (!m_activeList->selectedItems().isEmpty()) {
        toolitem = static_cast<ToolBarItem *>(m_activeList->selectedItems().first());
    }

    m_removeAction->setEnabled(toolitem);

    if (toolitem) {
        m_upAction->setEnabled(toolitem->index() != 0);
        m_downAction->setEnabled(toolitem->index() != toolitem->listWidget()->count() - 1);

        QString statusText = toolitem->statusText();
        m_helpArea->setText(i18nc("@label Action tooltip in toolbar editor, below the action list", "%1", statusText));
    } else {
        m_upAction->setEnabled(false);
        m_downAction->setEnabled(false);
        m_helpArea->setText(QString());
    }
}

void KEditToolBarWidgetPrivate::slotInsertButton()
{
    QString internalName = static_cast<ToolBarItem *>(m_inactiveList->currentItem())->internalName();

    insertActive(m_inactiveList->currentItem(), m_activeList->currentItem(), false);
    // we're modified, so let this change
    emit m_widget->enableOk(true);

    slotToolBarSelected(m_toolbarCombo->currentIndex());

    selectActiveItem(internalName);
}

void KEditToolBarWidgetPrivate::selectActiveItem(const QString &internalName)
{
    int activeItemCount = m_activeList->count();
    for (int i = 0; i < activeItemCount; i++) {
        ToolBarItem *item = static_cast<ToolBarItem *>(m_activeList->item(i));
        if (item->internalName() == internalName) {
            m_activeList->setCurrentItem(item);
            break;
        }
    }
}

void KEditToolBarWidgetPrivate::slotRemoveButton()
{
    removeActive(m_activeList->currentItem());

    slotToolBarSelected(m_toolbarCombo->currentIndex());
}

void KEditToolBarWidgetPrivate::insertActive(ToolBarItem *item, ToolBarItem *before, bool prepend)
{
    if (!item) {
        return;
    }

    QDomElement new_item;
    // let's handle the separator specially
    if (item->isSeparator()) {
        new_item = m_widget->domDocument().createElement(QStringLiteral("Separator"));
    } else {
        new_item = m_widget->domDocument().createElement(QStringLiteral("Action"));
    }

    new_item.setAttribute(QStringLiteral("name"), item->internalName());

    Q_ASSERT(!m_currentToolBarElem.isNull());

    if (before) {
        // we have the item in the active list which is before the new
        // item.. so let's try our best to add our new item right after it
        QDomElement elem = findElementForToolBarItem(before);
        Q_ASSERT(!elem.isNull());
        m_currentToolBarElem.insertAfter(new_item, elem);
    } else {
        // simply put it at the beginning or the end of the list.
        if (prepend) {
            m_currentToolBarElem.insertBefore(new_item, m_currentToolBarElem.firstChild());
        } else {
            m_currentToolBarElem.appendChild(new_item);
        }
    }

    // and set this container as a noMerge
    m_currentToolBarElem.setAttribute(QStringLiteral("noMerge"), QLatin1String("1"));

    // update the local doc
    updateLocal(m_currentToolBarElem);
}

void KEditToolBarWidgetPrivate::removeActive(ToolBarItem *item)
{
    if (!item) {
        return;
    }

    // we're modified, so let this change
    emit m_widget->enableOk(true);

    // now iterate through to find the child to nuke
    QDomElement elem = findElementForToolBarItem(item);
    if (!elem.isNull()) {
        // nuke myself!
        m_currentToolBarElem.removeChild(elem);

        // and set this container as a noMerge
        m_currentToolBarElem.setAttribute(QStringLiteral("noMerge"), QLatin1String("1"));

        // update the local doc
        updateLocal(m_currentToolBarElem);
    }
}

void KEditToolBarWidgetPrivate::slotUpButton()
{
    ToolBarItem *item = m_activeList->currentItem();

    if (!item) {
        Q_ASSERT(false);
        return;
    }

    int row = item->listWidget()->row(item) - 1;
    // make sure we're not the top item already
    if (row < 0) {
        Q_ASSERT(false);
        return;
    }

    // we're modified, so let this change
    emit m_widget->enableOk(true);

    moveActive(item, static_cast<ToolBarItem *>(item->listWidget()->item(row - 1)));
}

void KEditToolBarWidgetPrivate::moveActive(ToolBarItem *item, ToolBarItem *before)
{
    QDomElement e = findElementForToolBarItem(item);

    if (e.isNull()) {
        return;
    }

    // remove item
    m_activeList->takeItem(m_activeList->row(item));

    // put it where it's supposed to go
    m_activeList->insertItem(m_activeList->row(before) + 1, item);

    // make it selected again
    m_activeList->setCurrentItem(item);

    // and do the real move in the DOM
    if (!before) {
        m_currentToolBarElem.insertBefore(e, m_currentToolBarElem.firstChild());
    } else {
        m_currentToolBarElem.insertAfter(e, findElementForToolBarItem((ToolBarItem *)before));
    }

    // and set this container as a noMerge
    m_currentToolBarElem.setAttribute(QStringLiteral("noMerge"), QLatin1String("1"));

    // update the local doc
    updateLocal(m_currentToolBarElem);
}

void KEditToolBarWidgetPrivate::slotDownButton()
{
    ToolBarItem *item = m_activeList->currentItem();

    if (!item) {
        Q_ASSERT(false);
        return;
    }

    // make sure we're not the bottom item already
    int newRow = item->listWidget()->row(item) + 1;
    if (newRow >= item->listWidget()->count()) {
        Q_ASSERT(false);
        return;
    }

    // we're modified, so let this change
    emit m_widget->enableOk(true);

    moveActive(item, static_cast<ToolBarItem *>(item->listWidget()->item(newRow)));
}

void KEditToolBarWidgetPrivate::updateLocal(QDomElement &elem)
{
    QList<XmlData>::Iterator xit = m_xmlFiles.begin();
    for (; xit != m_xmlFiles.end(); ++xit) {
        if ((*xit).type() == XmlData::Merged) {
            continue;
        }

        if ((*xit).type() == XmlData::Shell ||
                (*xit).type() == XmlData::Part) {
            if (m_currentXmlData->xmlFile() == (*xit).xmlFile()) {
                (*xit).m_isModified = true;
                return;
            }

            continue;
        }

        (*xit).m_isModified = true;
        const QLatin1String attrName("name");
        ToolBarList::Iterator it = (*xit).barList().begin();
        for (; it != (*xit).barList().end(); ++it) {
            QString name((*it).attribute(attrName));
            QString tag((*it).tagName());
            if ((tag != elem.tagName()) || (name != elem.attribute(attrName))) {
                continue;
            }

            QDomElement toolbar = (*xit).domDocument().documentElement().toElement();
            toolbar.replaceChild(elem, (*it));
            return;
        }

        // just append it
        QDomElement toolbar = (*xit).domDocument().documentElement().toElement();
        Q_ASSERT(!toolbar.isNull());
        toolbar.appendChild(elem);
    }
}


void KEditToolBarWidgetPrivate::slotDropped(ToolBarListWidget *list, int index, ToolBarItem *item, bool sourceIsActiveList)
{
    //qDebug() << "slotDropped list=" << (list==m_activeList?"activeList":"inactiveList")
    //         << "index=" << index << "sourceIsActiveList=" << sourceIsActiveList;
    if (list == m_activeList) {
        ToolBarItem *after = index > 0 ? static_cast<ToolBarItem *>(list->item(index - 1)) : 0;
        //qDebug() << "after" << after->text() << after->internalTag();
        if (sourceIsActiveList) {
            // has been dragged within the active list (moved).
            moveActive(item, after);
        } else {
            // dragged from the inactive list to the active list
            insertActive(item, after, true);
        }
    } else if (list == m_inactiveList) {
        // has been dragged to the inactive list -> remove from the active list.
        removeActive(item);
    }

    delete item; // not needed anymore. must be deleted before slotToolBarSelected clears the lists

    // we're modified, so let this change
    emit m_widget->enableOk(true);

    slotToolBarSelected(m_toolbarCombo->currentIndex());
}

void KEditToolBar::showEvent(QShowEvent *event)
{
    if (!event->spontaneous()) {
        // The dialog has been shown, enable toolbar editing
        if (d->m_factory) {
            // call the xmlgui-factory version
            d->m_widget->load(d->m_factory, d->m_defaultToolBar);
        } else {
            // call the action collection version
            d->m_widget->load(d->m_file, d->m_global, d->m_defaultToolBar);
        }

        KToolBar::setToolBarsEditable(true);
    }
    QDialog::showEvent(event);
}

void KEditToolBar::hideEvent(QHideEvent *event)
{
    // The dialog has been hidden, disable toolbar editing
    KToolBar::setToolBarsEditable(false);

    QDialog::hideEvent(event);
}

#include "moc_kedittoolbar.cpp"
#include "moc_kedittoolbar_p.cpp"

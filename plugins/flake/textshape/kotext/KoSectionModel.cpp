#include "KoSectionModel.h"

#include <climits>
#include <KoTextDocument.h>
#include <KLocalizedString>

KoSectionModel::KoSectionModel(QTextDocument *doc)
    : QAbstractItemModel()
    , m_doc(doc)
{
    KoTextDocument(m_doc).setSectionModel(this);
}

KoSectionModel::~KoSectionModel()
{
    Q_FOREACH (KoSection *sec, m_registeredSections) {
        delete sec; // This will delete associated KoSectionEnd in KoSection destructor
    }
}

KoSection *KoSectionModel::createSection(const QTextCursor &cursor, KoSection *parent, const QString &name)
{
    if (!isValidNewName(name)) {
        return 0;
    }

    KoSection *result = new KoSection(cursor, name, parent);

    // Lets find our number in parent's children by cursor position
    QVector<KoSection *> children = (parent ? parent->children() : m_rootSections);
    int childrenId = children.size();
    for (int i = 0; i < children.size(); i++) {
        if (cursor.position() < children[i]->bounds().first) {
            childrenId = i;
            break;
        }
    }
    // We need to place link from parent to children in childId place
    // Also need to find corresponding index and declare operations in terms of model
    insertToModel(result, childrenId);

    return result;
}

KoSection *KoSectionModel::createSection(const QTextCursor &cursor, KoSection *parent)
{
    return createSection(cursor, parent, possibleNewName());
}

KoSectionEnd *KoSectionModel::createSectionEnd(KoSection *section)
{
    return new KoSectionEnd(section);
}

KoSection *KoSectionModel::sectionAtPosition(int pos)
{
    // TODO: Rewrite it by traversing Model as tree
    KoSection *result = 0;
    int level = -1; // Seeking the section with maximum level
    QHash<QString, KoSection *>::iterator it = m_sectionNames.begin();
    for (; it != m_sectionNames.end(); it++) {
        QPair<int, int> bounds = it.value()->bounds();
        if (bounds.first > pos || bounds.second < pos) {
            continue;
        }

        if (it.value()->level() > level) {
            result = it.value();
            level = it.value()->level();
        }
    }

    return result;
}

bool KoSectionModel::isValidNewName(const QString &name) const
{
    return (m_sectionNames.constFind(name) == m_sectionNames.constEnd());
}

QString KoSectionModel::possibleNewName()
{
    QString newName;
    int i = m_registeredSections.count();
    do {
        i++;
        newName = i18nc("new numbered section name", "New section %1", i);
    } while (!isValidNewName(newName));

    return newName;
}

bool KoSectionModel::setName(KoSection *section, const QString &name)
{
    if (section->name() == name || isValidNewName(name)) {
        section->setName(name);
        //TODO: we don't have name in columns, but need something to notify views about change
        emit dataChanged(m_modelIndex[section], m_modelIndex[section]);
        return true;
    }
    return false;
}

void KoSectionModel::allowMovingEndBound()
{
    QSet<KoSection *>::iterator it = m_registeredSections.begin();
    for (; it != m_registeredSections.end(); it++) {
        (*it)->setKeepEndBound(false);
    }
}

int KoSectionModel::findRowOfChild(KoSection *section) const
{
    QVector<KoSection *> lookOn;
    if (!section->parent()) {
        lookOn = m_rootSections;
    } else {
        lookOn = section->parent()->children();
    }

    int result = lookOn.indexOf(section);
    Q_ASSERT(result != -1);
    return result;
}

QModelIndex KoSectionModel::index(int row, int column, const QModelIndex &parentIdx) const
{
    if (!hasIndex(row, column, parentIdx)) {
        return QModelIndex();
    }

    if (!parentIdx.isValid()) {
        return createIndex(row, column, m_rootSections[row]);
    }

    KoSection *parent = static_cast<KoSection *>(parentIdx.internalPointer());
    return createIndex(row, column, parent->children()[row]);
}

QModelIndex KoSectionModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    KoSection *section = static_cast<KoSection *>(child.internalPointer());
    KoSection *parent = section->parent();
    if (parent) {
        return createIndex(findRowOfChild(parent), 0, parent);
    }
    return QModelIndex();
}

int KoSectionModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_rootSections.size();
    }
    return static_cast<KoSection *>(parent.internalPointer())->children().size();
}

int KoSectionModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant KoSectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.column() == 0 && role == PointerRole) {
        QVariant v;
        v.setValue(static_cast<KoSection *>(index.internalPointer()));
        return v;
    }
    return QVariant();
}

void KoSectionModel::insertToModel(KoSection *section, int childIdx)
{
    Q_ASSERT(isValidNewName(section->name()));

    KoSection *parent = section->parent();
    if (parent) { // Inserting to some section
        beginInsertRows(m_modelIndex[parent], childIdx, childIdx);
        parent->insertChild(childIdx, section);
        endInsertRows();
        m_modelIndex[section] = QPersistentModelIndex(index(childIdx, 0, m_modelIndex[parent]));
    } else { // It will be root section
        beginInsertRows(QModelIndex(), childIdx, childIdx);
        m_rootSections.insert(childIdx, section);
        endInsertRows();
        m_modelIndex[section] = QPersistentModelIndex(index(childIdx, 0, QModelIndex()));
    }

    m_registeredSections.insert(section);
    m_sectionNames[section->name()] = section;
}

void KoSectionModel::deleteFromModel(KoSection *section)
{
    KoSection *parent = section->parent();
    int childIdx = findRowOfChild(section);
    if (parent) { // Deleting non root section
        beginRemoveRows(m_modelIndex[parent], childIdx, childIdx);
        parent->removeChild(childIdx);
        endRemoveRows();
    } else { // Deleting root section
        beginRemoveRows(QModelIndex(), childIdx, childIdx);
        m_rootSections.remove(childIdx);
        endRemoveRows();
    }
    m_modelIndex.remove(section);
    m_sectionNames.remove(section->name());
}

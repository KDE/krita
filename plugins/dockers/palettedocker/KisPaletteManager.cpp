#include <QHash>
#include <QString>
#include <QScopedPointer>
#include <QPointer>

#include <KoColorSet.h>
#include <KisSwatchGroup.h>
#include <kis_signal_compressor.h>
#include <KisViewManager.h>
#include <KisDocument.h>
#include <KisChangePaletteCommand.h>
#include <KoResourceServer.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceServerProvider.h>

#include "KisPaletteManager.h"

struct KisPaletteManager::PaletteInfo {
    QString name;
    QString filename;
    int columnCount;
    bool isGlobal;
    bool isReadOnly;
    QHash<QString, KisSwatchGroup> groups;
};

struct KisPaletteManager::Private
{
    bool isGlobalModified;
    bool isReadOnlyModified;
    bool isNameModified;
    bool isFilenameModified;
    bool isColumnCountModified;
    QSet<QString> modifiedGroupNames;
    QSet<QString> newGroupNames;
    QPointer<KoColorSet> palette;
    QScopedPointer<PaletteInfo> modified;
    QPointer<KisViewManager> view;
    KoResourceServer<KoColorSet> *rServer;
    KisSignalCompressor *updateSigCompressor;
};

KisPaletteManager::KisPaletteManager(QObject *parent)
    : QObject(parent)
    , m_d(new Private)
{
    m_d->palette = Q_NULLPTR;
    m_d->updateSigCompressor = new KisSignalCompressor(40, KisSignalCompressor::FIRST_ACTIVE, this);
    m_d->rServer = KoResourceServerProvider::instance()->paletteServer();
    connect(m_d->updateSigCompressor, SIGNAL(timeout()), SLOT(slotUpdatePalette()));
}

KisPaletteManager::~KisPaletteManager()
{
    delete m_d->updateSigCompressor;
}

void KisPaletteManager::setPalette(KoColorSet *palette)
{
    if (!palette) { return; }
    m_d->modified->groups.clear();
    m_d->palette = palette;
    m_d->modified.reset(new PaletteInfo);
    m_d->modified->name = palette->name();
    m_d->modified->filename = palette->filename();
    m_d->modified->columnCount = palette->columnCount();
    m_d->modified->isGlobal = palette->isGlobal();
    m_d->modified->isReadOnly = !palette->isEditable();

    Q_FOREACH (const QString &groupName, palette->getGroupNames()) {
        KisSwatchGroup *cs = palette->getGroup(groupName);
        m_d->modified->groups[groupName] = KisSwatchGroup(*cs);
    }
}

void KisPaletteManager::setView(KisViewManager *view)
{
    m_d->view = view;
}

void KisPaletteManager::addGroup(const QString &name, int rowNumber)
{
    m_d->newGroupNames.insert(name);
    m_d->modified->groups[name] = KisSwatchGroup();
    KisSwatchGroup &newGroup = m_d->modified->groups[name];
    newGroup.setName(name);
    newGroup.setRowCount(rowNumber);
}

void KisPaletteManager::removeGroup(const QString &name)
{
    m_d->modified->groups.remove(name);
}

void KisPaletteManager::renameGroup(const QString &oldName, const QString &newName)
{
    m_d->modifiedGroupNames.insert(oldName);
    m_d->modified->groups[oldName].setName(newName);
}

void KisPaletteManager::setEntry(const KisSwatch &swatch, int col, int row, const QString &groupName)
{
    m_d->modifiedGroupNames.insert(groupName);
    m_d->modified->groups[groupName].setEntry(swatch, col, row);
}

void KisPaletteManager::removeEntry(int col, int row, const QString &groupName)
{
    m_d->modifiedGroupNames.insert(groupName);
    m_d->modified->groups[groupName].removeEntry(col, row);
}

void KisPaletteManager::addEntry(const KisSwatch &swatch, const QString &groupName)
{
    m_d->modifiedGroupNames.insert(groupName);
    m_d->modified->groups[groupName].addEntry(swatch);
}

void KisPaletteManager::slotUpdatePalette()
{
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }
    KisDocument *doc = m_d->view->document();
    KoColorSet *palette = m_d->palette;
    PaletteInfo *modified = m_d->modified.data();

    doc->addCommand(new KisChangePaletteCommand());

    if (m_d->isGlobalModified) {
        palette->setIsGlobal(modified->isGlobal);
    }
    if (m_d->isReadOnlyModified) {
        palette->setIsEditable(palette->isEditable());
    }
    if (m_d->isColumnCountModified) {
        palette->setColumnCount(modified->columnCount);
    }
    if (m_d->isNameModified) {
        palette->setName(modified->name);
    }
    if (m_d->isFilenameModified) {
        palette->setFilename(modified->filename);
    }
    Q_FOREACH (const QString &groupName, palette->getGroupNames()) {
        if (!modified->groups.contains(groupName)) {
            palette->removeGroup(groupName);
        }
    }
    Q_FOREACH (const QString &groupName, palette->getGroupNames()) {
        KisSwatchGroup *g = palette->getGroup(groupName);
        if (m_d->modifiedGroupNames.contains(groupName)) {
            *g = modified->groups[groupName];
            palette->changeGroupName(groupName, modified->groups[groupName].name());
        }
    }
    Q_FOREACH (const QString &newGroupName, m_d->newGroupNames) {
        palette->addGroup(newGroupName);
        *palette->getGroup(newGroupName) = modified->groups[newGroupName];
    }
}

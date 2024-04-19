/*  This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2016 L. E. Segovia <amy@amyspark.me>
 *  SPDX-FileCopyrightText: 2005..2022 Halla Rempt <halla@valdyas.org>
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <sys/types.h>

#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QVector>
#include <QTextStream>
#include <QTextCodec>
#include <QHash>
#include <QList>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QString>
#include <QStringList>
#include <QImage>
#include <QPainter>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QtEndian> // qFromLittleEndian
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <DebugPigment.h>
#include <klocalizedstring.h>
#include <kundo2command.h>

#include <KoStore.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>
#include "KisSwatch.h"
#include "kis_dom_utils.h"

#include "KoColorSet.h"
#include "KoColorSet_p.h"

namespace {

/**
 * readAllLinesSafe() reads all the lines in the byte array
 * using the automated UTF8 and CR/LF transformations. That
 * might be necessary for opening GPL palettes created on Linux
 * in Windows environment.
 */
QStringList readAllLinesSafe(QByteArray *data)
{
    QStringList lines;

    QBuffer buffer(data);
    buffer.open(QBuffer::ReadOnly);
    QTextStream stream(&buffer);
    stream.setCodec("UTF-8");

    QString line;
    while (stream.readLineInto(&line)) {
        lines << line;
    }

    return lines;
}
}


const QString KoColorSet::GLOBAL_GROUP_NAME = QString();
const QString KoColorSet::KPL_VERSION_ATTR = "version";
const QString KoColorSet::KPL_GROUP_ROW_COUNT_ATTR = "rows";
const QString KoColorSet::KPL_PALETTE_COLUMN_COUNT_ATTR = "columns";
const QString KoColorSet::KPL_PALETTE_NAME_ATTR = "name";
const QString KoColorSet::KPL_PALETTE_COMMENT_ATTR = "comment";
const QString KoColorSet::KPL_PALETTE_FILENAME_ATTR = "filename";
const QString KoColorSet::KPL_PALETTE_READONLY_ATTR = "readonly";
const QString KoColorSet::KPL_COLOR_MODEL_ID_ATTR = "colorModelId";
const QString KoColorSet::KPL_COLOR_DEPTH_ID_ATTR = "colorDepthId";
const QString KoColorSet::KPL_GROUP_NAME_ATTR = "name";
const QString KoColorSet::KPL_SWATCH_ROW_ATTR = "row";
const QString KoColorSet::KPL_SWATCH_COL_ATTR = "column";
const QString KoColorSet::KPL_SWATCH_NAME_ATTR = "name";
const QString KoColorSet::KPL_SWATCH_ID_ATTR = "id";
const QString KoColorSet::KPL_SWATCH_SPOT_ATTR = "spot";
const QString KoColorSet::KPL_SWATCH_BITDEPTH_ATTR = "bitdepth";
const QString KoColorSet::KPL_PALETTE_PROFILE_TAG = "Profile";
const QString KoColorSet::KPL_SWATCH_POS_TAG = "Position";
const QString KoColorSet::KPL_SWATCH_TAG = "ColorSetEntry";
const QString KoColorSet::KPL_GROUP_TAG = "Group";
const QString KoColorSet::KPL_PALETTE_TAG = "ColorSet";

const int MAXIMUM_ALLOWED_COLUMNS = 4096;


struct AddSwatchCommand : public KUndo2Command
{

   AddSwatchCommand(KoColorSet *colorSet, const KisSwatch &swatch, const QString &groupName, int column, int row)
       : m_colorSet(colorSet)
       , m_swatch(swatch)
       , m_groupName(groupName)
       , m_x(column)
       , m_y(row)
   {
   }

   ~AddSwatchCommand() override {}

    /// redo the command
    void redo() override
    {
        KisSwatchGroupSP modifiedGroup = m_colorSet->getGroup(m_groupName);
        if (m_x < 0 || m_y < 0) {
            QPair<int, int> pos = modifiedGroup->addSwatch(m_swatch);
            m_x = pos.first;
            m_y = pos.second;
        }
        else {
            modifiedGroup->setSwatch(m_swatch, m_x, m_y);
        }
    }

    /// revert the actions done in redo
    void undo() override
    {
        KisSwatchGroupSP modifiedGroup = m_colorSet->getGroup(m_groupName);
        modifiedGroup->removeSwatch(m_x, m_y);
    }

private:
    KoColorSet *m_colorSet;
    KisSwatch m_swatch;
    QString m_groupName;
    int m_x;
    int m_y;
};


struct RemoveSwatchCommand : public KUndo2Command
{
   RemoveSwatchCommand(KoColorSet *colorSet, int column, int row, KisSwatchGroupSP group)
       : m_colorSet(colorSet)
       , m_swatch(group->getSwatch(column, row))
       , m_group(group)
       , m_x(column)
       , m_y(row)

   {
   }

   /// redo the command
    void redo() override
    {
        m_group->removeSwatch(m_x, m_y);
    }

    /// revert the actions done in redo
    void undo() override
    {
        m_group->setSwatch(m_swatch, m_x, m_y);
    }

private:
    KoColorSet *m_colorSet;
    KisSwatch m_swatch;
    KisSwatchGroupSP m_group;
    int m_x;
    int m_y;
};

struct ChangeGroupNameCommand : public KUndo2Command
{

   ChangeGroupNameCommand(KoColorSet *colorSet, QString oldGroupName, const QString &newGroupName)
       : m_colorSet(colorSet)
       , m_oldGroupName(oldGroupName)
       , m_newGroupName(newGroupName)
   {
   }

   ~ChangeGroupNameCommand() override {}

    /// redo the command
    void redo() override
    {
        KisSwatchGroupSP group = m_colorSet->getGroup(m_oldGroupName);
        group->setName(m_newGroupName);
    }

    /// revert the actions done in redo
    void undo() override
    {
        KisSwatchGroupSP group = m_colorSet->getGroup(m_newGroupName);
        group->setName(m_oldGroupName);
    }

private:
    KoColorSet *m_colorSet;
    QString m_oldGroupName;
    QString m_newGroupName;
};

struct MoveGroupCommand : public KUndo2Command
{

   MoveGroupCommand(KoColorSet *colorSet, QString groupName, const QString &groupNameInsertBefore)
       : m_colorSet(colorSet)
       , m_groupName(groupName)
       , m_groupNameInsertBefore(groupNameInsertBefore)
   {
       int idx = 0;
       for (const KisSwatchGroupSP &group : m_colorSet->d->swatchGroups) {

           if (group->name() == m_groupName) {
               m_oldIndex = idx;
           }

           if (group->name() == m_groupNameInsertBefore) {
               m_newIndex = idx;
           }

           idx++;
       }
   }

    /// redo the command
    void redo() override
    {
        if (m_groupNameInsertBefore != KoColorSet::GLOBAL_GROUP_NAME &&
                m_groupName != KoColorSet::GLOBAL_GROUP_NAME)
        {
            KisSwatchGroupSP group = m_colorSet->d->swatchGroups.takeAt(m_oldIndex);
            m_colorSet->d->swatchGroups.insert(m_newIndex, group);
        }
    }


    /// revert the actions done in redo
    void undo() override
    {
        KisSwatchGroupSP group = m_colorSet->d->swatchGroups.takeAt(m_newIndex);
        m_colorSet->d->swatchGroups.insert(m_oldIndex, group);
    }

private:
    KoColorSet *m_colorSet;
    QString m_groupName;
    QString m_groupNameInsertBefore;
    int m_oldIndex;
    int m_newIndex;
};

struct AddGroupCommand : public KUndo2Command
{

   AddGroupCommand(KoColorSet *colorSet, QString groupName, int columnCount, int rowCount)
       : m_colorSet(colorSet)
       , m_groupName(groupName)
       , m_columnCount(columnCount)
       , m_rowCount(rowCount)
   {
   }

    /// redo the command
    void redo() override
    {
        KisSwatchGroupSP group(new KisSwatchGroup);
        group->setName(m_groupName);
        group->setColumnCount(m_columnCount);
        group->setRowCount(m_rowCount);
        m_colorSet->d->swatchGroups.append(group);
    }


    /// revert the actions done in redo
    void undo() override
    {
        int idx = 0;
        bool found = false;
        for(const KisSwatchGroupSP &group : m_colorSet->d->swatchGroups) {
            if (group->name() == m_groupName) {
                found = true;
                break;
            }
            idx++;
        }
        if (found) {
            m_colorSet->d->swatchGroups.takeAt(idx);
        }
    }

private:
    KoColorSet *m_colorSet;
    QString m_groupName;
    int m_columnCount;
    int m_rowCount;
};

struct RemoveGroupCommand : public KUndo2Command
{
   RemoveGroupCommand(KoColorSet *colorSet, QString groupName, bool keepColors = true)
       : m_colorSet(colorSet)
       , m_groupName(groupName)
       , m_keepColors(keepColors)
       , m_oldGroup(m_colorSet->getGroup(groupName))
       , m_startingRow(m_colorSet->getGlobalGroup()->rowCount())
   {
       for (m_groupIndex = 0; m_groupIndex < colorSet->d->swatchGroups.size(); ++ m_groupIndex) {
           if (colorSet->d->swatchGroups[m_groupIndex]->name() == m_oldGroup->name()) {
               break;
           }
       }
   }

    /// redo the command
    void redo() override
    {
        if (m_keepColors) {
            // put all colors directly below global
            KisSwatchGroupSP globalGroup = m_colorSet->getGlobalGroup();
            for (const KisSwatchGroup::SwatchInfo &info : m_oldGroup->infoList()) {
                globalGroup->setSwatch(info.swatch,
                                      info.column,
                                      info.row + m_startingRow);
            }
        }

        m_colorSet->d->swatchGroups.removeOne(m_oldGroup);
    }

    /// revert the actions done in redo
    void undo() override
    {
        m_colorSet->d->swatchGroups.insert(m_groupIndex, m_oldGroup);

        // remove all colors that were inserted into global
        if (m_keepColors) {
            KisSwatchGroupSP globalGroup = m_colorSet->getGlobalGroup();
            for (const KisSwatchGroup::SwatchInfo &info : globalGroup->infoList()) {
                m_oldGroup->setSwatch(info.swatch, info.column, info.row - m_startingRow);
                globalGroup->removeSwatch(info.column,
                                         info.row + m_startingRow);
            }
        }
    }

private:
    KoColorSet *m_colorSet;
    QString m_groupName;
    bool m_keepColors;
    KisSwatchGroupSP m_oldGroup;
    int m_groupIndex;
    int m_startingRow;
};

struct ClearCommand : public KUndo2Command
{
   ClearCommand(KoColorSet *colorSet)
       : m_colorSet(colorSet)
       , m_OldColorSet(new KoColorSet(*colorSet))
   {
   }


   ~ClearCommand() override
   {
       delete m_OldColorSet;
   }

    /// redo the command
    void redo() override
    {
        m_colorSet->d->swatchGroups.clear();
        KisSwatchGroupSP global(new KisSwatchGroup);
        global->setName(KoColorSet::GLOBAL_GROUP_NAME);
        m_colorSet->d->swatchGroups.append(global);
    }


    /// revert the actions done in redo
    void undo() override
    {
        m_colorSet->d->swatchGroups = m_OldColorSet->d->swatchGroups;
        KUndo2Command::undo();
    }

private:
    KoColorSet *m_colorSet;
    KoColorSet *m_OldColorSet;
};

struct SetColumnCountCommand : public KUndo2Command
{
   SetColumnCountCommand(KoColorSet *colorSet, int columnCount)
       : m_colorSet(colorSet)
       , m_columnsCount(columnCount)
       , m_oldColumnsCount(colorSet->columnCount())
   {
   }

    /// redo the command
    void redo() override
    {
        for (KisSwatchGroupSP &group : m_colorSet->d->swatchGroups) {
            group->setColumnCount(m_columnsCount);
        }
        m_colorSet->d->columns = m_columnsCount;
    }


    /// revert the actions done in redo
    void undo() override
    {
        for (KisSwatchGroupSP &group : m_colorSet->d->swatchGroups) {
            group->setColumnCount(m_oldColumnsCount);
        }
        m_colorSet->d->columns = m_oldColumnsCount;
    }

private:
    KoColorSet *m_colorSet;
    int m_columnsCount;
    int m_oldColumnsCount;
};


struct SetCommentCommand : public KUndo2Command
{
   SetCommentCommand(KoColorSet *colorSet, const QString &comment)
       : m_colorSet(colorSet)
       , m_comment(comment)
       , m_oldComment(colorSet->comment())
   {
   }

    /// redo the command
    void redo() override
    {
        m_colorSet->d->comment = m_comment;
    }


    /// revert the actions done in redo
    void undo() override
    {
        m_colorSet->d->comment = m_oldComment;
    }

private:
    KoColorSet *m_colorSet;
    QString m_comment;
    QString m_oldComment;
};

struct SetPaletteTypeCommand : public KUndo2Command
{
   SetPaletteTypeCommand(KoColorSet *colorSet, const KoColorSet::PaletteType &paletteType)
       : m_colorSet(colorSet)
       , m_paletteType(paletteType)
       , m_oldPaletteType(colorSet->paletteType())
   {
   }

    /// redo the command
    void redo() override
    {
        m_colorSet->d->paletteType = m_paletteType;
        QStringList fileName = m_colorSet->filename().split(".");
        fileName.last() = suffix(m_paletteType).replace(".", "");
        m_colorSet->setFilename(fileName.join("."));
    }


    /// revert the actions done in redo
    void undo() override
    {
        m_colorSet->d->paletteType = m_oldPaletteType;
        QStringList fileName = m_colorSet->filename().split(".");
        fileName.last() = suffix(m_oldPaletteType).replace(".", "");
        m_colorSet->setFilename(fileName.join("."));
    }

private:

    QString suffix(KoColorSet::PaletteType paletteType) const
    {
        QString suffix;
        switch(paletteType) {
        case KoColorSet::GPL:
            suffix = ".gpl";
            break;
        case KoColorSet::ACT:
            suffix = ".act";
            break;
        case KoColorSet::RIFF_PAL:
        case KoColorSet::PSP_PAL:
            suffix = ".pal";
            break;
        case KoColorSet::ACO:
            suffix = ".aco";
            break;
        case KoColorSet::XML:
            suffix = ".xml";
            break;
        case KoColorSet::KPL:
            suffix = ".kpl";
            break;
        case KoColorSet::SBZ:
            suffix = ".sbz";
            break;
        case KoColorSet::CSS:
            suffix = ".css";
            break;
        default:
            suffix = m_colorSet->defaultFileExtension();
        }
        return suffix;
    }

    KoColorSet *m_colorSet;
    KoColorSet::PaletteType m_paletteType;
    KoColorSet::PaletteType m_oldPaletteType;
};

KoColorSet::KoColorSet(const QString& filename)
    : QObject()
    , KoResource(filename)
    , d(new Private(this))
{
    connect(&d->undoStack, SIGNAL(canUndoChanged(bool)), this, SLOT(canUndoChanged(bool)));
    connect(&d->undoStack, SIGNAL(canRedoChanged(bool)), this, SLOT(canRedoChanged(bool)));
}

/// Create an copied palette
KoColorSet::KoColorSet(const KoColorSet& rhs)
    : QObject()
    , KoResource(rhs)
    , d(new Private(this))
{
    d->paletteType = rhs.d->paletteType;
    d->data = rhs.d->data;
    d->comment = rhs.d->comment;
    d->swatchGroups = rhs.d->swatchGroups;
}

KoColorSet::~KoColorSet()
{
}

KoResourceSP KoColorSet::clone() const
{
    return KoResourceSP(new KoColorSet(*this));
}

bool KoColorSet::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    if (!dev->isOpen()) dev->open(QIODevice::ReadOnly);

    d->data = dev->readAll();

    Q_ASSERT(d->data.size() != 0);

    return d->init();
}

bool KoColorSet::saveToDevice(QIODevice *dev) const
{
    bool res = false;
    switch(d->paletteType) {
    case GPL:
        res = d->saveGpl(dev);
        break;
    default:
        res = d->saveKpl(dev);
    }

    if (res) const_cast<KoColorSet*>(this)->setDirty(false);
    d->undoStack.clear();

    return res;
}

bool KoColorSet::fromByteArray(QByteArray &data, KisResourcesInterfaceSP resourcesInterface)
{
    QBuffer buf(&data);
    buf.open(QIODevice::ReadOnly);
    return loadFromDevice(&buf, resourcesInterface);
}

KoColorSet::PaletteType KoColorSet::paletteType() const
{
    return d->paletteType;
}

void KoColorSet::setPaletteType(PaletteType paletteType)
{
    if (d->isLocked || paletteType == d->paletteType) return;

    SetPaletteTypeCommand *cmd = new SetPaletteTypeCommand(this, paletteType);

    d->undoStack.push(cmd);
}


void KoColorSet::addSwatch(const KisSwatch &swatch, const QString &groupName, int column, int row)
{
    if (d->isLocked) return;

    AddSwatchCommand *cmd = new AddSwatchCommand(this, swatch, groupName, column, row);

    d->undoStack.push(cmd);

}

void KoColorSet::removeSwatch(int column, int row, KisSwatchGroupSP group)
{
    if (d->isLocked) return;
    RemoveSwatchCommand *cmd = new RemoveSwatchCommand(this, column, row, group);

    d->undoStack.push(cmd);
}

void KoColorSet::clear()
{
    if (d->isLocked) return;

    ClearCommand *cmd = new ClearCommand(this);

    d->undoStack.push(cmd);
}

KisSwatch KoColorSet::getColorGlobal(quint32 column, quint32 row) const
{
    KisSwatchGroupSP group = getGroup(row);
    Q_ASSERT(group);

    int titleRow = startRowForGroup(group->name());
    int rowInGroup = -1;

    if (group->name().isEmpty()) {
        rowInGroup = (int)row - titleRow;
    }
    else {
        rowInGroup = (int)row - (titleRow + 1);
    }

    Q_ASSERT((isGroupTitleRow(titleRow) && titleRow > 0) || titleRow == 0);
    Q_ASSERT(rowInGroup < group->rowCount());

    return group->getSwatch(column, rowInGroup);

}

KisSwatch KoColorSet::getSwatchFromGroup(quint32 column, quint32 row, QString groupName) const
{
    KisSwatch swatch;
    for (const KisSwatchGroupSP &group: d->swatchGroups) {
        if (group->name() == groupName) {
            if (group->checkSwatchExists(column, row)) {
                swatch = group->getSwatch(column, row);
            }
            break;
        }
    }
    return swatch;
}

QStringList KoColorSet::swatchGroupNames() const
{
    QStringList groupNames;
    for (const KisSwatchGroupSP &group : d->swatchGroups) {
        groupNames << group->name();
    }
    return groupNames;
}

bool KoColorSet::isGroupTitleRow(int row) const
{
    int idx = 0;
    for (const KisSwatchGroupSP &group : d->swatchGroups) {
        idx += group->rowCount();
        if (group->name() != KoColorSet::GLOBAL_GROUP_NAME) {
            idx++;
        }
        if (idx == row) {
            return true;
        }
    }
    return false;
}

int KoColorSet::startRowForGroup(const QString &groupName) const
{
    if (groupName.isEmpty()) return 0;

    int row = 0;
    for (const KisSwatchGroupSP &group : d->swatchGroups) {
        if (group->name() == groupName) {
            return row;
        }
        row += group->rowCount();
        if (group->name() != KoColorSet::GLOBAL_GROUP_NAME) {
            row++;
        }
    }
    return row;
}

int KoColorSet::rowNumberInGroup(int rowNumber) const
{
    if (isGroupTitleRow(rowNumber)) {
        return -1;
    }

    int rowInGroup = -1;
    for (int i = rowNumber; i > -1; i--) {
        if (isGroupTitleRow(i)) {
            return rowInGroup;
        }
        else {
            rowInGroup++;
        }
    }

    return rowInGroup;
}

void KoColorSet::setModified(bool _modified)
{
    setDirty(_modified);
    if (_modified) {
        emit modified();
    }
}


void KoColorSet::canUndoChanged(bool canUndo)
{
    if (canUndo) {
        setModified(true);
    }
    else {
        setModified(false);
    }
}

void KoColorSet::canRedoChanged(bool /*canRedo*/)
{
    if (d->undoStack.canUndo()) {
        setModified(true);
    }
    else {
        setModified(false);
    }
}

void KoColorSet::changeGroupName(const QString &oldGroupName, const QString &newGroupName)
{
    if (!swatchGroupNames().contains(oldGroupName) || (oldGroupName == newGroupName) || d->isLocked) return;

    ChangeGroupNameCommand *cmd = new ChangeGroupNameCommand(this, oldGroupName, newGroupName);
    d->undoStack.push(cmd);
}

void KoColorSet::setColumnCount(int columns)
{
    if (d->isLocked || (columns == d->columns)) return;

    SetColumnCountCommand *cmd = new SetColumnCountCommand (this, columns);

    d->undoStack.push(cmd);
}

int KoColorSet::columnCount() const
{
    Q_ASSERT(d->swatchGroups.size() > 0);

    return d->swatchGroups.first()->columnCount();
}

QString KoColorSet::comment()
{
    return d->comment;
}

void KoColorSet::setComment(QString comment)
{
    if (d->isLocked || comment == d->comment) return;

    SetCommentCommand *cmd = new SetCommentCommand(this, comment);

    d->undoStack.push(cmd);
}

void KoColorSet::addGroup(const QString &groupName, int columnCount, int rowCount)
{
    if (swatchGroupNames().contains(groupName) || d->isLocked) return;

    AddGroupCommand *cmd = new AddGroupCommand(this, groupName, columnCount, rowCount);

    d->undoStack.push(cmd);
}

void KoColorSet::moveGroup(const QString &groupName, const QString &groupNameInsertBefore)
{
    QStringList groupNames = swatchGroupNames();
    if (!groupNames.contains(groupName)
            || !groupNames.contains(groupNameInsertBefore)
            || d->isLocked) return;

    MoveGroupCommand *cmd = new MoveGroupCommand(this, groupName, groupNameInsertBefore);

    d->undoStack.push(cmd);

}

void KoColorSet::removeGroup(const QString &groupName, bool keepColors)
{

    if (!swatchGroupNames().contains(groupName) || (groupName == GLOBAL_GROUP_NAME) || d->isLocked) return;

    RemoveGroupCommand *cmd = new RemoveGroupCommand(this, groupName, keepColors);

    d->undoStack.push(cmd);
}

QString KoColorSet::defaultFileExtension() const
{
    return (d->paletteType == GPL) ? ".gpl" : ".kpl";
}

KUndo2Stack *KoColorSet::undoStack() const
{
    return &d->undoStack;
}

void KoColorSet::setLocked(bool lock)
{
    d->isLocked = lock;
}

bool KoColorSet::isLocked() const
{
    return d->isLocked;
}

int KoColorSet::rowCount() const
{
    int res = 0;
    for (const KisSwatchGroupSP &group : d->swatchGroups) {
        res += group->rowCount();
    }
    return res;
}

int KoColorSet::rowCountWithTitles() const
{
    return rowCount() + d->swatchGroups.size() - 1;
}

quint32 KoColorSet::colorCount() const
{
    int colorCount = 0;
    for (const KisSwatchGroupSP &group : d->swatchGroups) {
        colorCount += group->colorCount();
    }
    return colorCount;
}

KisSwatchGroupSP KoColorSet::getGroup(const QString &name) const
{
    for (KisSwatchGroupSP &group : d->swatchGroups) {
        if (group->name() == name) {
            return group;
        }
    }
    return 0;
}

KisSwatchGroupSP KoColorSet::getGroup(int row) const
{
//    qDebug() << "------------";

    if (row >= rowCountWithTitles()) return nullptr;

    int currentRow = 0;

    for (KisSwatchGroupSP &group : d->swatchGroups) {

        int groupRowCount = group->rowCount();
        if (group->name() != KoColorSet::GLOBAL_GROUP_NAME) {
            groupRowCount++;
        }

//        qDebug() << group->name()
//                 << "row" << row << "currentRow" << currentRow << "group rowcount" << groupRowCount
//                 << "hit" << (currentRow <= row && row < currentRow + groupRowCount);

        bool hit = (currentRow <= row && row < currentRow + groupRowCount);

        if  (hit) {
            return group;
        }

        currentRow += group->rowCount();

        if (group->name() != KoColorSet::GLOBAL_GROUP_NAME) {
             currentRow += 1;
        }

        if (currentRow >= rowCountWithTitles()) return nullptr;
    }

    return nullptr;

}

KisSwatchGroupSP KoColorSet::getGlobalGroup() const
{
    Q_ASSERT(d->swatchGroups.size() > 0);
    Q_ASSERT(d->swatchGroups.first()->name() == GLOBAL_GROUP_NAME);
    return d->swatchGroups.first();
}

KisSwatchGroup::SwatchInfo KoColorSet::getClosestSwatchInfo(KoColor compare, bool useGivenColorSpace) const
{
    KisSwatchGroup::SwatchInfo closestSwatch;

    quint8 highestPercentage = 0;
    quint8 testPercentage = 0;

    for (const KisSwatchGroupSP &group : d->swatchGroups) {
        for (const KisSwatchGroup::SwatchInfo &currInfo : group->infoList()) {
            KoColor color = currInfo.swatch.color();
            if (useGivenColorSpace == true && compare.colorSpace() != color.colorSpace()) {
                color.convertTo(compare.colorSpace());

            } else if (compare.colorSpace() != color.colorSpace()) {
                compare.convertTo(color.colorSpace());
            }
            testPercentage = (255 - compare.colorSpace()->difference(compare.data(), color.data()));
            if (testPercentage > highestPercentage)
            {
                highestPercentage = testPercentage;
                closestSwatch = currInfo;
            }
        }
    }
    return closestSwatch;
}

void KoColorSet::updateThumbnail()
{
    int rows = 0;

    // Determine the last filled row in each group
    for (const KisSwatchGroupSP &group : d->swatchGroups) {
        int lastRowInGroup =  0;
        for (const KisSwatchGroup::SwatchInfo &info : group->infoList()) {
            lastRowInGroup = qMax(lastRowInGroup, info.row);
        }
        rows += (lastRowInGroup + 1);
    }

    QImage img(d->global()->columnCount() * 4, rows * 4, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(img.rect(), Qt::darkGray);

    int lastRow = 0;
    for (const KisSwatchGroupSP &group : d->swatchGroups) {
        int lastRowGroup = 0;
        for (const KisSwatchGroup::SwatchInfo &info : group->infoList()) {
            QColor c = info.swatch.color().toQColor();
            gc.fillRect(info.column * 4, (lastRow + info.row) * 4, 4, 4, c);
            lastRowGroup = qMax(lastRowGroup, info.row);
        }
        lastRow += (lastRowGroup + 1);
    }

    setImage(img);
}

/********************************KoColorSet::Private**************************/

KoColorSet::Private::Private(KoColorSet *a_colorSet)
    : colorSet(a_colorSet)
{
    undoStack.setUndoLimit(100);
    KisSwatchGroupSP group(new KisSwatchGroup);
    group->setName(KoColorSet::GLOBAL_GROUP_NAME);
    swatchGroups.clear();
    swatchGroups.append(group);
}

KoColorSet::PaletteType KoColorSet::Private::detectFormat(const QString &fileName, const QByteArray &ba)
{
    QFileInfo fi(fileName);

    // .pal
    if (ba.startsWith("RIFF") && ba.indexOf("PAL data", 8)) {
        return KoColorSet::RIFF_PAL;
    }
    // .gpl
    else if (ba.startsWith("GIMP Palette")) {
        return KoColorSet::GPL;
    }
    // .pal
    else if (ba.startsWith("JASC-PAL")) {
        return KoColorSet::PSP_PAL;
    }
    else if (ba.contains("krita/x-colorset") || ba.contains("application/x-krita-palette")) {
        return KoColorSet::KPL;
    }
    else if (fi.suffix().toLower() == "aco") {
        return KoColorSet::ACO;
    }
    else if (fi.suffix().toLower() == "act") {
        return KoColorSet::ACT;
    }
    else if (fi.suffix().toLower() == "xml") {
        return KoColorSet::XML;
    }
    else if (fi.suffix().toLower() == "sbz") {
        return KoColorSet::SBZ;
    }
    else if (fi.suffix().toLower() == "ase" || ba.startsWith("ASEF")) {
        return KoColorSet::ASE;
    }
    else if (fi.suffix().toLower() == "acb" || ba.startsWith("8BCB")) {
        return KoColorSet::ACB;
    }
    else if (fi.suffix().toLower() == "css") {
        return KoColorSet::CSS;
    }
    return KoColorSet::UNKNOWN;
}

void KoColorSet::Private::scribusParseColor(KoColorSet *set, QXmlStreamReader *xml)
{
    KisSwatch colorEntry;
    // It's a color, retrieve it
    QXmlStreamAttributes colorProperties = xml->attributes();

    QStringRef colorName = colorProperties.value("NAME");
    colorEntry.setName(colorName.isEmpty() || colorName.isNull() ? i18n("Untitled") : colorName.toString());

    // RGB or CMYK?
    if (colorProperties.hasAttribute("RGB")) {
        dbgPigment << "Color " << colorProperties.value("NAME") << ", RGB " << colorProperties.value("RGB");

        KoColor currentColor(KoColorSpaceRegistry::instance()->rgb8());
        QStringRef colorValue = colorProperties.value("RGB");

        if (colorValue.length() != 7 && colorValue.at(0) != '#') { // Color is a hexadecimal number
            xml->raiseError("Invalid rgb8 color (malformed): " + colorValue);
            return;
        } else {
            bool rgbOk;
            quint32 rgb = colorValue.mid(1).toUInt(&rgbOk, 16);
            if  (!rgbOk) {
                xml->raiseError("Invalid rgb8 color (unable to convert): " + colorValue);
                return;
            }

            quint8 r = rgb >> 16 & 0xff;
            quint8 g = rgb >> 8 & 0xff;
            quint8 b = rgb & 0xff;

            dbgPigment << "Color parsed: "<< r << g << b;

            currentColor.data()[0] = r;
            currentColor.data()[1] = g;
            currentColor.data()[2] = b;
            currentColor.setOpacity(OPACITY_OPAQUE_U8);
            colorEntry.setColor(currentColor);

            set->addSwatch(colorEntry);

            while(xml->readNextStartElement()) {
                //ignore - these are all unknown or the /> element tag
                xml->skipCurrentElement();
            }
            return;
        }
    }
    else if (colorProperties.hasAttribute("CMYK")) {
        dbgPigment << "Color " << colorProperties.value("NAME") << ", CMYK " << colorProperties.value("CMYK");

        KoColor currentColor(KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id(), QString()));

        QStringRef colorValue = colorProperties.value("CMYK");
        if (colorValue.length() != 9 && colorValue.at(0) != '#') { // Color is a hexadecimal number
            xml->raiseError("Invalid cmyk color (malformed): " % colorValue);
            return;
        }
        else {
            bool cmykOk;
            quint32 cmyk = colorValue.mid(1).toUInt(&cmykOk, 16); // cmyk uses the full 32 bits
            if  (!cmykOk) {
                xml->raiseError("Invalid cmyk color (unable to convert): " % colorValue);
                return;
            }

            quint8 c = cmyk >> 24 & 0xff;
            quint8 m = cmyk >> 16 & 0xff;
            quint8 y = cmyk >> 8 & 0xff;
            quint8 k = cmyk & 0xff;

            dbgPigment << "Color parsed: "<< c << m << y << k;

            currentColor.data()[0] = c;
            currentColor.data()[1] = m;
            currentColor.data()[2] = y;
            currentColor.data()[3] = k;
            currentColor.setOpacity(OPACITY_OPAQUE_U8);
            colorEntry.setColor(currentColor);

            set->addSwatch(colorEntry);

            while(xml->readNextStartElement()) {
                //ignore - these are all unknown or the /> element tag
                xml->skipCurrentElement();
            }
            return;
        }
    }
    else {
        xml->raiseError("Unknown color space for color " + colorEntry.name());
    }
}

bool KoColorSet::Private::loadScribusXmlPalette(KoColorSet *set, QXmlStreamReader *xml)
{

    //1. Get name
    QXmlStreamAttributes paletteProperties = xml->attributes();
    QStringRef paletteName = paletteProperties.value("Name");
    dbgPigment << "Processed name of palette:" << paletteName;
    set->setName(paletteName.toString());

    //2. Inside the SCRIBUSCOLORS, there are lots of colors. Retrieve them

    while(xml->readNextStartElement()) {
        QStringRef currentElement = xml->name();
        if(QStringRef::compare(currentElement, "COLOR", Qt::CaseInsensitive) == 0) {
            scribusParseColor(set, xml);
        }
        else {
            xml->skipCurrentElement();
        }
    }

    if(xml->hasError()) {
        return false;
    }

    return true;
}

quint8 KoColorSet::Private::readByte(QIODevice *io)
{
    quint8 val;
    quint64 read = io->read((char*)&val, 1);
    if (read != 1) return false;
    return val;
}

quint16 KoColorSet::Private::readShort(QIODevice *io) {
    quint16 val;
    quint64 read = io->read((char*)&val, 2);
    if (read != 2) return false;
    return qFromBigEndian(val);
}

qint32 KoColorSet::Private::readInt(QIODevice *io)
{
    qint32 val;
    quint64 read = io->read((char*)&val, 4);
    if (read != 4) return false;
    return qFromBigEndian(val);
}

float KoColorSet::Private::readFloat(QIODevice *io)
{
    float val;
    quint64 read = io->read((char*)&val, 4);
    if (read != 4) return false;
    return qFromBigEndian(val);
}

QString KoColorSet::Private::readUnicodeString(QIODevice *io, bool sizeIsInt)
{
    QString unicode;
    qint32 size = 0;
    if (sizeIsInt) {
        size = readInt(io);
    } else {
        size = readShort(io)-1;
    }
    if (size>0) {
        QByteArray ba = io->read(size*2);
        if (ba.size() == int(size)*2) {
            QTextCodec *Utf16Codec = QTextCodec::codecForName("UTF-16BE");
            unicode = Utf16Codec->toUnicode(ba);
        } else {
            warnPigment << "Unicode name block is the wrong size" << colorSet->filename();
        }
    }
    if (!sizeIsInt) {
        readShort(io); // when the size is quint16, the string is 00 terminated;
    }
    return unicode.trimmed();
}

bool KoColorSet::Private::init()
{
    // just in case this is a reload (eg by KoEditColorSetDialog),
    swatchGroups.clear();
    KisSwatchGroupSP globalGroup(new KisSwatchGroup);
    globalGroup->setName(KoColorSet::GLOBAL_GROUP_NAME);
    swatchGroups.append(globalGroup);
    undoStack.clear();

    if (colorSet->filename().isNull()) {
        warnPigment << "Cannot load palette" << colorSet->name() << "there is no filename set";
        return false;
    }
    if (data.isNull()) {
        QFile file(colorSet->filename());
        if (file.size() == 0) {
            warnPigment << "Cannot load palette" << colorSet->name() << "there is no data available";
            return false;
        }
        file.open(QIODevice::ReadOnly);
        data = file.readAll();
        file.close();
    }

    bool res = false;
    paletteType = detectFormat(colorSet->filename(), data);
    switch(paletteType) {
    case GPL:
        res = loadGpl();
        break;
    case ACT:
        res = loadAct();
        break;
    case RIFF_PAL:
        res = loadRiff();
        break;
    case PSP_PAL:
        res = loadPsp();
        break;
    case ACO:
        res = loadAco();
        break;
    case XML:
        res = loadXml();
        break;
    case KPL:
        res = loadKpl();
        break;
    case SBZ:
        res = loadSbz();
        break;
    case ASE:
        res = loadAse();
        break;
    case ACB:
        res = loadAcb();
        break;
    case CSS:
        res = loadCss();
        break;
    default:
        res = false;
    }
    if (paletteType != KPL) {
        int rowCount = global()->colorCount() / global()->columnCount();
        if (global()->colorCount() % global()->columnCount() > 0) {
            rowCount ++;
        }
        global()->setRowCount(rowCount);
    }
    colorSet->setValid(res);
    colorSet->updateThumbnail();

    data.clear();
    undoStack.clear();

    return res;
}

bool KoColorSet::Private::saveGpl(QIODevice *dev) const
{
    Q_ASSERT(dev->isOpen());
    Q_ASSERT(dev->isWritable());

    QTextStream stream(dev);
    stream.setCodec("UTF-8");
    stream << "GIMP Palette\nName: " << colorSet->name() << "\nColumns: " << colorSet->columnCount() << "\n#\n";

    KisSwatchGroupSP global = colorSet->getGlobalGroup();
    for (int y = 0; y < global->rowCount(); y++) {
        for (int x = 0; x < colorSet->columnCount(); x++) {
            if (!global->checkSwatchExists(x, y)) {
                continue;
            }
            const KisSwatch& entry = global->getSwatch(x, y);
            QColor c = entry.color().toQColor();
            stream << c.red() << " " << c.green() << " " << c.blue() << "\t";
            if (entry.name().isEmpty())
                stream << "Untitled\n";
            else
                stream << entry.name() << "\n";
        }
    }

    return true;
}

bool KoColorSet::Private::loadGpl()
{
    if (data.isEmpty() || data.isNull() || data.length() < 50) {
        warnPigment << "Illegal Gimp palette file: " << colorSet->filename();
        return false;
    }

    quint32 index = 0;

    QStringList lines = readAllLinesSafe(&data);

    if (lines.size() < 3) {
        warnPigment << "Not enough lines in palette file: " << colorSet->filename();
        return false;
    }

    QString columnsText;
    qint32 r, g, b;
    KisSwatch swatch;

    // Read name
    if (!lines[0].startsWith("GIMP") || !lines[1].toLower().contains("name")) {
        warnPigment << "Illegal Gimp palette file: " << colorSet->filename();
        return false;
    }

    // translated name will be in a tooltip, here don't translate
    colorSet->setName(lines[1].split(":")[1].trimmed());

    index = 2;

    // Read columns
    int columns = 0;
    if (lines[index].toLower().contains("columns")) {
        columnsText = lines[index].split(":")[1].trimmed();
        columns = columnsText.toInt();
        if (columns > MAXIMUM_ALLOWED_COLUMNS) {
            warnPigment << "Refusing to set unreasonable number of columns (" << columns << ") in GIMP Palette file " << colorSet->filename() << " - using maximum number of allowed columns instead";
            global()->setColumnCount(MAXIMUM_ALLOWED_COLUMNS);
        }
        else {
            global()->setColumnCount(columns);
        }
        index = 3;
    }


    for (qint32 i = index; i < lines.size(); i++) {
        if (lines[i].startsWith('#')) {
            comment += lines[i].mid(1).trimmed() + ' ';
        } else if (!lines[i].isEmpty()) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
            QStringList a = lines[i].replace('\t', ' ').split(' ', Qt::SkipEmptyParts);
#else
            QStringList a = lines[i].replace('\t', ' ').split(' ', QString::SkipEmptyParts);
#endif

            if (a.count() < 3) {
                continue;
            }

            r = qBound(0, a[0].toInt(), 255);
            g = qBound(0, a[1].toInt(), 255);
            b = qBound(0, a[2].toInt(), 255);

            swatch.setColor(KoColor(QColor(r, g, b), KoColorSpaceRegistry::instance()->rgb8()));

            for (int i = 0; i != 3; i++) {
                a.pop_front();
            }
            QString name = a.join(" ");
            swatch.setName(name.isEmpty() || name == "Untitled" ? i18n("Untitled") : name);

            global()->addSwatch(swatch);
        }
    }
    return true;
}

bool KoColorSet::Private::loadAct()
{
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.completeBaseName());
    KisSwatch swatch;
    for (int i = 0; i < data.size(); i += 3) {
        quint8 r = data[i];
        quint8 g = data[i+1];
        quint8 b = data[i+2];
        swatch.setColor(KoColor(QColor(r, g, b), KoColorSpaceRegistry::instance()->rgb8()));
        global()->addSwatch(swatch);
    }
    return true;
}

bool KoColorSet::Private::loadRiff()
{
    // https://worms2d.info/Palette_file
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.completeBaseName());
    KisSwatch swatch;

    RiffHeader header;
    memcpy(&header, data.constData(), sizeof(RiffHeader));
    header.colorcount = qFromBigEndian(header.colorcount);

    for (int i = sizeof(RiffHeader);
         (i < (int)(sizeof(RiffHeader) + (header.colorcount * 4)) && i < data.size());
         i += 4) {
        quint8 r = data[i];
        quint8 g = data[i+1];
        quint8 b = data[i+2];
        swatch.setColor(KoColor(QColor(r, g, b), KoColorSpaceRegistry::instance()->rgb8()));
        colorSet->getGlobalGroup()->addSwatch(swatch);
    }
    return true;
}


bool KoColorSet::Private::loadPsp()
{
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.completeBaseName());
    KisSwatch swatch;
    qint32 r, g, b;

    QStringList l = readAllLinesSafe(&data);
    if (l.size() < 4) return false;
    if (l[0] != "JASC-PAL") return false;
    if (l[1] != "0100") return false;

    int entries = l[2].toInt();

    KisSwatchGroupSP global = colorSet->getGlobalGroup();

    for (int i = 0; i < entries; ++i)  {

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QStringList a = l[i + 3].replace('\t', ' ').split(' ', Qt::SkipEmptyParts);
#else
        QStringList a = l[i + 3].replace('\t', ' ').split(' ', QString::SkipEmptyParts);
#endif

        if (a.count() != 3) {
            continue;
        }

        r = qBound(0, a[0].toInt(), 255);
        g = qBound(0, a[1].toInt(), 255);
        b = qBound(0, a[2].toInt(), 255);

        swatch.setColor(KoColor(QColor(r, g, b),
                                KoColorSpaceRegistry::instance()->rgb8()));

        QString name = a.join(" ");
        swatch.setName(name.isEmpty() ? i18n("Untitled") : name);

        global->addSwatch(swatch);
    }
    return true;
}

bool KoColorSet::Private::loadCss()
{
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.completeBaseName());
    
    QString text = readAllLinesSafe(&data).join("").replace("\t", "").replace(" ", ""); 
    
    QRegularExpression re("/\\*.*?\\*/");

    text.remove(re); // Remove comments

    KisSwatch swatch;
    
    // Regex to detect a color in the palette
    QRegularExpression palette("(.*?){(?:[^:;]+:[^;]+;)*?color:(.*?)(?:;.*?)*?}");

    QRegularExpressionMatchIterator colors = palette.globalMatch(text);

    if (!colors.hasNext()) {
        warnPigment << "No color found in CSS palette : " << colorSet->filename();
        return false;
    }
    
    while (colors.hasNext()) {
        QRegularExpressionMatch match = colors.next();
        QString colorInfo = match.captured();
        QString colorName = match.captured(1);
        QString colorValue = match.captured(2);
        
        if (!colorInfo.startsWith(".") || colorValue.isEmpty()) {
            warnPigment << "Illegal CSS palette syntax : " << colorInfo;
            return false;
        }

        QColor qColor;

        colorName.remove(".");
        swatch.setName(colorName);

        if (colorValue.startsWith("rgb")) {
            QStringList color;
            
            if (colorValue.startsWith("rgba")) {
                colorValue.remove("rgba(").remove(")");
                color = colorValue.split(",");

                if (color.size() != 4) {
                    warnPigment << "Invalid RGBA color definition : " << colorInfo;
                    return false;
                }
                
                int alpha = color[3].toFloat() * 255;
                
                if (alpha < 0 || alpha > 255) {
                    warnPigment << "Invalid alpha parameter : " << colorInfo;
                    return false;
                }
            }
            else {
                colorValue.remove("rgb(").remove(")");
                
                color = colorValue.split(",");

                if (color.size() != 3) {
                    warnPigment << "Invalid RGB color definition : " << colorInfo;
                    return false;
                }
            }

            int rgb[3];

            for (int i = 0; i < 3; i++) {
                if (color[i].endsWith("%")) {
                    color[i].replace("%", "");
                    rgb[i] = color[i].toFloat() / 100 * 255 ;
                }
                else {
                    rgb[i] = color[i].toInt();
                };
            }

            qColor = QColor(rgb[0], rgb[1], rgb[2]);
        }
        else if (colorValue.startsWith("hsl")) {
            QStringList color;

            if (colorValue.startsWith("hsla")) {
                colorValue.remove("hsla(").remove(")").replace("%", "");
                color = colorValue.split(",");
                if (color.size() != 4) {
                    warnPigment << "Invalid HSLA color definition : " << colorInfo;
                    return false;
                }

                float alpha = color[3].toFloat();

                if (alpha < 0.0 || alpha > 1.0) {
                    warnPigment << "Invalid alpha parameter : " << colorInfo;
                    return false;
                }

            }
            else {
                colorValue.remove("hsl(").remove(")").replace("%", "");
                color = colorValue.split(",");
                if (color.size() != 3) {
                    warnPigment << "Invalid HSL color definition : " << colorInfo;
                    return false;
                }
            }
            
            float hue = color[0].toFloat() / 359;
            float saturation = color[1].toFloat() / 100;
            float lightness = color[2].toFloat() / 100;

            if (hue < 0.0 || hue > 1.0) {
                warnPigment << "Invalid hue parameter : " << colorInfo;
                return false;
            }

            if (saturation < 0.0 || saturation > 1.0) {
                warnPigment << "Invalid saturation parameter : " << colorInfo;
                return false;
            }

            if (lightness < 0.0 || lightness > 1.0) {
                warnPigment << "Invalid lightness parameter : " << colorInfo;
                return false;
            }

            qColor = QColor::fromHslF(hue, saturation, lightness);

        }
        else if (colorValue.startsWith("#")) {
            if (colorValue.size() == 9) {
                // Convert the CSS format #RRGGBBAA to #RRGGBB 
                // Due to QColor's 8 digits format being #AARRGGBB and that we do not load the alpha channel
                colorValue.truncate(7);
            }
            
            qColor = QColor(colorValue);
        }
        else {
            warnPigment << "Unknown color declaration : " << colorInfo;
            return false;
        }
 
        if (!qColor.isValid()) {
            warnPigment << "Invalid color definition : " << colorInfo;
            return false;
        }

        swatch.setColor(KoColor(qColor, KoColorSpaceRegistry::instance()->rgb8()));

        global()->addSwatch(swatch);
    }

    return true;
}

const KoColorProfile *KoColorSet::Private::loadColorProfile(QScopedPointer<KoStore> &store,
                                                            const QString &path,
                                                            const QString &modelId,
                                                            const QString &colorDepthId)
{
    if (!store->open(path)) {
        return nullptr;
    }

    QByteArray bytes = store->read(store->size());
    store->close();

    const KoColorProfile *profile = KoColorSpaceRegistry::instance()
        ->createColorProfile(modelId, colorDepthId, bytes);
    if (!profile || !profile->valid()) {
        return nullptr;
    }

    KoColorSpaceRegistry::instance()->addProfile(profile);
    return profile;
}

bool KoColorSet::Private::loadKplProfiles(QScopedPointer<KoStore> &store)
{
    if (!store->open("profiles.xml")) {
        return false;
    }

    QByteArray bytes = store->read(store->size());
    store->close();

    QDomDocument doc;
    if(!doc.setContent(bytes)) {
        return false;
    }

    QDomElement root = doc.documentElement();
    for (QDomElement c = root.firstChildElement(KPL_PALETTE_PROFILE_TAG);
         !c.isNull();
         c = c.nextSiblingElement(KPL_PALETTE_PROFILE_TAG)) {
        QString name         = c.attribute(KPL_PALETTE_NAME_ATTR);
        QString filename     = c.attribute(KPL_PALETTE_FILENAME_ATTR);
        QString colorModelId = c.attribute(KPL_COLOR_MODEL_ID_ATTR);
        QString colorDepthId = c.attribute(KPL_COLOR_DEPTH_ID_ATTR);

        if (KoColorSpaceRegistry::instance()->profileByName(name)) {
            continue;
        }

        loadColorProfile(store, filename, colorModelId, colorDepthId);
        // TODO: What should happen if this fails?
    }

    return true;
}

bool KoColorSet::Private::loadKplColorset(QScopedPointer<KoStore> &store)
{
    if (!store->open("colorset.xml")) {
        return false;
    }

    QByteArray bytes = store->read(store->size());
    store->close();

    QDomDocument doc;
    if (!doc.setContent(bytes)) {
        return false;
    }

    QDomElement root = doc.documentElement();
    colorSet->setName(root.attribute(KPL_PALETTE_NAME_ATTR));
    QString version = root.attribute(KPL_VERSION_ATTR);
    comment         = root.attribute(KPL_PALETTE_COMMENT_ATTR);

    int desiredColumnCount = root.attribute(KPL_PALETTE_COLUMN_COUNT_ATTR).toInt();
    if (desiredColumnCount > MAXIMUM_ALLOWED_COLUMNS) {
        warnPigment << "Refusing to set unreasonable number of columns (" << desiredColumnCount
                    << ") in KPL palette file " << colorSet->filename()
                    << " - setting maximum allowed column count instead.";
        colorSet->setColumnCount(MAXIMUM_ALLOWED_COLUMNS);
    } else {
        colorSet->setColumnCount(desiredColumnCount);
    }

    loadKplGroup(doc, root, colorSet->getGlobalGroup(), version);

    for (QDomElement g = root.firstChildElement(KPL_GROUP_TAG);
         !g.isNull();
         g = g.nextSiblingElement(KPL_GROUP_TAG)) {
        QString groupName = g.attribute(KPL_GROUP_NAME_ATTR);
        colorSet->addGroup(groupName);
        loadKplGroup(doc, g, colorSet->getGroup(groupName), version);
    }

    return true;
}

bool KoColorSet::Private::loadKpl()
{
    QBuffer buf(&data);
    buf.open(QBuffer::ReadOnly);

    QScopedPointer<KoStore> store(
        KoStore::createStore(&buf, KoStore::Read,
                             "application/x-krita-palette",
                             KoStore::Zip));
    if (!store || store->bad()) {
        return false;
    }

    if (store->hasFile("profiles.xml") && !loadKplProfiles(store)) {
        return false;
    }

    if (!loadKplColorset(store)) {
        return false;
    }

    buf.close();
    return true;
}

bool KoColorSet::Private::loadAco()
{
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.completeBaseName());

    QBuffer buf(&data);
    buf.open(QBuffer::ReadOnly);

    quint16 version = readShort(&buf);
    quint16 numColors = readShort(&buf);
    KisSwatch swatch;

    if (version == 1 && buf.size() > 4+numColors*10) {
        buf.seek(4+numColors*10);
        version = readShort(&buf);
        numColors = readShort(&buf);
    }

    const quint16 quint16_MAX = 65535;

    KisSwatchGroupSP group = colorSet->getGlobalGroup();

    for (int i = 0; i < numColors && !buf.atEnd(); ++i) {

        quint16 colorSpace = readShort(&buf);
        quint16 ch1 = readShort(&buf);
        quint16 ch2 = readShort(&buf);
        quint16 ch3 = readShort(&buf);
        quint16 ch4 = readShort(&buf);

        bool skip = false;
        if (colorSpace == 0) { // RGB
            const KoColorProfile *srgb = KoColorSpaceRegistry::instance()->rgb8()->profile();
            KoColor c(KoColorSpaceRegistry::instance()->rgb16(srgb));
            reinterpret_cast<quint16*>(c.data())[0] = ch3;
            reinterpret_cast<quint16*>(c.data())[1] = ch2;
            reinterpret_cast<quint16*>(c.data())[2] = ch1;
            c.setOpacity(OPACITY_OPAQUE_U8);
            swatch.setColor(c);
        }
        else if (colorSpace == 1) { // HSB
            QColor qc;
            qc.setHsvF(ch1 / 65536.0, ch2 / 65536.0, ch3 / 65536.0);
            KoColor c(qc, KoColorSpaceRegistry::instance()->rgb16());
            c.setOpacity(OPACITY_OPAQUE_U8);
            swatch.setColor(c);
        }
        else if (colorSpace == 2) { // CMYK
            KoColor c(KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer16BitsColorDepthID.id(), QString()));
            reinterpret_cast<quint16*>(c.data())[0] = quint16_MAX - ch1;
            reinterpret_cast<quint16*>(c.data())[1] = quint16_MAX - ch2;
            reinterpret_cast<quint16*>(c.data())[2] = quint16_MAX - ch3;
            reinterpret_cast<quint16*>(c.data())[3] = quint16_MAX - ch4;
            c.setOpacity(OPACITY_OPAQUE_U8);
            swatch.setColor(c);
        }
        else if (colorSpace == 7) { // LAB
            KoColor c = KoColor(KoColorSpaceRegistry::instance()->lab16());
            reinterpret_cast<quint16*>(c.data())[0] = ch3;
            reinterpret_cast<quint16*>(c.data())[1] = ch2;
            reinterpret_cast<quint16*>(c.data())[2] = ch1;
            c.setOpacity(OPACITY_OPAQUE_U8);
            swatch.setColor(c);
        }
        else if (colorSpace == 8) { // GRAY
            KoColor c(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), QString()));
            reinterpret_cast<quint16*>(c.data())[0] = ch1 * (quint16_MAX / 10000);
            c.setOpacity(OPACITY_OPAQUE_U8);
            swatch.setColor(c);
        }
        else {
            warnPigment << "Unsupported colorspace in palette" << colorSet->filename() << "(" << colorSpace << ")";
            skip = true;
        }

        if (version == 2) {
            QString name = readUnicodeString(&buf, true);
            swatch.setName(name);
        }
        if (!skip) {
            group->addSwatch(swatch);
        }
    }
    return true;
}

bool KoColorSet::Private::loadSbzSwatchbook(QScopedPointer<KoStore> &store)
{
    if (!store->open("swatchbook.xml")) {
        return false;
    }

    QByteArray bytes = store->read(store->size());
    store->close();

    dbgPigment << "XML palette: " << colorSet->filename() << ", SwatchBooker format";

    QDomDocument doc;
    int errorLine, errorColumn;
    QString errorMessage;
    if (!doc.setContent(bytes, &errorMessage, &errorLine, &errorColumn)) {
        warnPigment << "Illegal XML palette:" << colorSet->filename();
        warnPigment << "Error (line" << errorLine
                    << ", column" << errorColumn
                    << "):" << errorMessage;
        return false;
    }

    QDomElement root = doc.documentElement(); // SwatchBook

    // Start reading properties...
    QDomElement metadata = root.firstChildElement("metadata");
    if (metadata.isNull()) {
        warnPigment << "Palette metadata not found";
        return false;
    }

    QDomElement title = metadata.firstChildElement("dc:title");
    QString colorName = title.text();
    colorName = colorName.isEmpty() ? i18n("Untitled") : colorName;
    colorSet->setName(colorName);
    dbgPigment << "Processed name of palette:" << colorSet->name();
    // End reading properties

    // Also read the swatch book...
    QDomElement book = root.firstChildElement("book");
    if (book.isNull()) {
        warnPigment << "Palette book (swatch composition) not found (line" << root.lineNumber()
                    << ", column" << root.columnNumber()
                    << ")";
        return false;
    }

    // Which has lots of "swatch"es (todo: support groups)
    QDomElement swatch = book.firstChildElement();
    if (swatch.isNull()) {
        warnPigment << "Swatches/groups definition not found (line" << book.lineNumber()
                    << ", column" << book.columnNumber()
                    << ")";
        return false;
    }

    // Now read colors...
    QDomElement materials = root.firstChildElement("materials");
    if (materials.isNull()) {
        warnPigment << "Materials (color definitions) not found";
        return false;
    }

    // This one has lots of "color" elements
    if (materials.firstChildElement("color").isNull()) {
        warnPigment << "Color definitions not found (line" << materials.lineNumber()
                    << ", column" << materials.columnNumber()
                    << ")";
        return false;
    }

    // We'll store colors here, and as we process swatches
    // we'll add them to the palette
    QHash<QString, KisSwatch> materialsBook;
    QHash<QString, const KoColorSpace*> fileColorSpaces;

    // Color processing
    store->enterDirectory("profiles"); // Color profiles (icc files) live here
    for (QDomElement colorElement = materials.firstChildElement("color");
         !colorElement.isNull();
         colorElement = colorElement.nextSiblingElement("color")) {
        KisSwatch currentEntry;
        // Set if color is spot
        currentEntry.setSpotColor(colorElement.attribute("usage") == "spot");

        // <metadata> inside contains id and name
        // one or more <values> define the color
        QDomElement currentColorMetadata = colorElement.firstChildElement("metadata");
        // Get color name
        QDomElement colorTitle = currentColorMetadata.firstChildElement("dc:title");
        QDomElement colorId = currentColorMetadata.firstChildElement("dc:identifier");
        // Is there an id? (we need that at the very least for identifying a color)
        if (colorId.text().isEmpty()) {
            warnPigment << "Unidentified color (line" << colorId.lineNumber()
                        << ", column" << colorId.columnNumber()
                        << ")";
            return false;
        }

        if (materialsBook.contains(colorId.text())) {
            warnPigment << "Duplicated color definition (line" << colorId.lineNumber()
                        << ", column" << colorId.columnNumber()
                        << ")";
            return false;
        }

        // Get a valid color name
        currentEntry.setId(colorId.text());
        currentEntry.setName(colorTitle.text().isEmpty() ? colorId.text() : colorTitle.text());

        // Get a valid color definition
        if (colorElement.firstChildElement("values").isNull()) {
            warnPigment << "Color definitions not found (line" << colorElement.lineNumber()
                        << ", column" << colorElement.columnNumber()
                        << ")";
            return false;
        }

        bool status;
        bool firstDefinition = false;
        KoColorSpaceRegistry *colorSpaceRegistry = KoColorSpaceRegistry::instance();
        const QString colorDepthId = Float32BitsColorDepthID.id();
        // Priority: Lab, otherwise the first definition found
        for (QDomElement colorValueE = colorElement.firstChildElement("values");
             !colorValueE.isNull();
             colorValueE = colorValueE.nextSiblingElement("values")) {
            QString model = colorValueE.attribute("model");

            QString modelId;
            const KoColorProfile *profile = nullptr;
            if (model == "Lab") {
                modelId = LABAColorModelID.id();
            } else if (model == "sRGB") {
                modelId = RGBAColorModelID.id();
                profile = colorSpaceRegistry->rgb8()->profile();
            } else if (model == "XYZ") {
                modelId = XYZAColorModelID.id();
            } else if (model == "CMYK") {
                modelId = CMYKAColorModelID.id();
            } else if (model == "GRAY") {
                modelId = GrayAColorModelID.id();
            } else if (model == "RGB") {
                modelId = RGBAColorModelID.id();
            } else {
                warnPigment << "Color space not implemented:" << model
                            << "(line" << colorValueE.lineNumber()
                            << ", column "<< colorValueE.columnNumber()
                            << ")";
                continue;
            }

            const KoColorSpace *colorSpace = colorSpaceRegistry->colorSpace(modelId, colorDepthId, profile);

            // The 'space' attribute is the name of the icc file
            // sitting in the 'profiles' directory in the zip.
            QString space = colorValueE.attribute("space");
            if (!space.isEmpty()) {
                if (fileColorSpaces.contains(space)) {
                    colorSpace = fileColorSpaces.value(space);
                } else {
                    // Try loading the profile and add it to the registry
                    profile = loadColorProfile(store, space, modelId, colorDepthId);
                    if (profile) {
                        colorSpace = colorSpaceRegistry->colorSpace(modelId, colorDepthId, profile);
                        fileColorSpaces.insert(space, colorSpace);
                    }
                }
            }

            KoColor c(colorSpace);

            // sRGB,RGB,HSV,HSL,CMY,CMYK,nCLR: 0 -> 1
            // YIQ: Y 0 -> 1 : IQ -0.5 -> 0.5
            // Lab: L 0 -> 100 : ab -128 -> 127
            // XYZ: 0 -> ~100
            QVector<float> channels;
            for (const QString &str : colorValueE.text().split(" ")) {
                float channelValue = str.toFloat(&status);
                if (!status) {
                    warnPigment << "Invalid float definition (line" << colorValueE.lineNumber()
                                << ", column" << colorValueE.columnNumber()
                                << ")";

                    channelValue = 0;
                }

                channels.append(channelValue);
            }
            channels.append(OPACITY_OPAQUE_F); // Alpha channel
            colorSpace->fromNormalisedChannelsValue(c.data(), channels);

            currentEntry.setColor(c);
            firstDefinition = true;

            if (model == "Lab") {
                break; // Immediately add this one
            }
        }

        if (firstDefinition) {
            materialsBook.insert(currentEntry.id(), currentEntry);
        } else {
            warnPigment << "No supported color  spaces for the current color (line" << colorElement.lineNumber()
                        << ", column "<< colorElement.columnNumber()
                        << ")";
            return false;
        }
    }

    store->leaveDirectory(); // Return to root
    // End colors
    // Now decide which ones will go into the palette

    KisSwatchGroupSP global = colorSet->getGlobalGroup();
    for(; !swatch.isNull(); swatch = swatch.nextSiblingElement()) {
        QString type = swatch.tagName();
        if (type.isEmpty() || type.isNull()) {
            warnPigment << "Invalid swatch/group definition (no id) (line" << swatch.lineNumber()
                        << ", column" << swatch.columnNumber()
                        << ")";
            return false;
        } else if (type == "swatch") {
            QString id = swatch.attribute("material");
            if (id.isEmpty() || id.isNull()) {
                warnPigment << "Invalid swatch definition (no material id) (line" << swatch.lineNumber()
                            << ", column" << swatch.columnNumber()
                            << ")";
                return false;
            }

            if (materialsBook.contains(id)) {
                global->addSwatch(materialsBook.value(id));
            } else {
                warnPigment << "Invalid swatch definition (material not found) (line" << swatch.lineNumber()
                            << ", column" << swatch.columnNumber()
                            << ")";
                return false;
            }
        } else if (type == "group") {
            QDomElement groupMetadata = swatch.firstChildElement("metadata");
            if (groupMetadata.isNull()) {
                warnPigment << "Invalid group definition (missing metadata) (line" << groupMetadata.lineNumber()
                            << ", column" << groupMetadata.columnNumber()
                            << ")";
                return false;
            }
            QDomElement groupTitle = metadata.firstChildElement("dc:title");
            if (groupTitle.isNull()) {
                warnPigment << "Invalid group definition (missing title) (line" << groupTitle.lineNumber()
                            << ", column" << groupTitle.columnNumber()
                            << ")";
                return false;
            }
            QString currentGroupName = groupTitle.text();
            colorSet->addGroup(currentGroupName);

            for (QDomElement groupSwatch = swatch.firstChildElement("swatch");
                 !groupSwatch.isNull();
                 groupSwatch = groupSwatch.nextSiblingElement("swatch")) {
                QString id = groupSwatch.attribute("material");
                if (id.isEmpty() || id.isNull()) {
                    warnPigment << "Invalid swatch definition (no material id) (line" << groupSwatch.lineNumber()
                                << ", column" << groupSwatch.columnNumber()
                                << ")";
                    return false;
                }

                if (materialsBook.contains(id)) {
                    colorSet->getGroup(currentGroupName)->addSwatch(materialsBook.value(id));
                } else {
                    warnPigment << "Invalid swatch definition (material not found) (line" << groupSwatch.lineNumber()
                                << ", column" << groupSwatch.columnNumber()
                                << ")";
                    return false;
                }
            }
        }
    }
    // End palette

    return true;
}

bool KoColorSet::Private::loadSbz() {
    QBuffer buf(&data);
    buf.open(QBuffer::ReadOnly);

    // &buf is a subclass of QIODevice
    QScopedPointer<KoStore> store(
        KoStore::createStore(&buf, KoStore::Read,
                             "application/x-swatchbook",
                             KoStore::Zip));
    if (!store || store->bad()) {
        return false;
    }

    if (store->hasFile("swatchbook.xml") && !loadSbzSwatchbook(store)) {
        return false;
    }

    buf.close();
    return true;
}

bool KoColorSet::Private::loadAse()
{
    QFileInfo info(colorSet->filename());
    colorSet->setName(info.completeBaseName());

    QBuffer buf(&data);
    buf.open(QBuffer::ReadOnly);

    QByteArray signature; // should be "ASEF";
    signature = buf.read(4);
    quint16 version = readShort(&buf);
    quint16 version2 = readShort(&buf);

    if (signature != "ASEF" && version!= 1 && version2 != 0) {
        qWarning() << "incorrect header:" << signature << version << version2;
        return false;
    }
    qint32 numBlocks = readInt(&buf);

    QByteArray groupStart("\xC0\x01");
    QByteArray groupEnd("\xC0\x02");
    QByteArray swatchSig("\x00\x01");

    bool inGroup = false;
    QString groupName;
    for (qint32 i = 0; i < numBlocks; i++) {
        QByteArray blockType;
        blockType = buf.read(2);
        qint32 blockSize = readInt(&buf);
        qint64 pos = buf.pos();

        if (blockType == groupStart) {
            groupName = readUnicodeString(&buf);
            colorSet->addGroup(groupName);
            inGroup = true;
        }
        else if (blockType == groupEnd) {
            int colorCount = colorSet->getGroup(groupName)->colorCount();
            int columns = colorSet->columnCount();
            int rows = colorCount/columns;
            if (colorCount % columns > 0) {
                rows += 1;
            }
            colorSet->getGroup(groupName)->setRowCount(rows);
            inGroup = false;
        }
        else /* if (blockType == swatchSig)*/ {
            KisSwatch swatch;
            swatch.setName(readUnicodeString(&buf).trimmed());
            QByteArray colorModel;
            QDomDocument doc;
            colorModel = buf.read(4);
            if (colorModel == "RGB ") {
                QDomElement elt = doc.createElement("sRGB");

                elt.setAttribute("r", readFloat(&buf));
                elt.setAttribute("g", readFloat(&buf));
                elt.setAttribute("b", readFloat(&buf));

                KoColor color = KoColor::fromXML(elt, "U8");
                swatch.setColor(color);
            } else if (colorModel == "CMYK") {
                QDomElement elt = doc.createElement("CMYK");

                elt.setAttribute("c", readFloat(&buf));
                elt.setAttribute("m", readFloat(&buf));
                elt.setAttribute("y", readFloat(&buf));
                elt.setAttribute("k", readFloat(&buf));
                //try to select the default PS icc profile if possible.
                elt.setAttribute("space", "U.S. Web Coated (SWOP) v2");

                KoColor color = KoColor::fromXML(elt, "U8");
                swatch.setColor(color);
            } else if (colorModel == "LAB ") {
                QDomElement elt = doc.createElement("Lab");

                elt.setAttribute("L", readFloat(&buf)*100.0);
                elt.setAttribute("a", readFloat(&buf));
                elt.setAttribute("b", readFloat(&buf));

                KoColor color = KoColor::fromXML(elt, "U16");
                swatch.setColor(color);
            } else if (colorModel == "GRAY") {
                QDomElement elt = doc.createElement("Gray");

                elt.setAttribute("g", readFloat(&buf));

                KoColor color = KoColor::fromXML(elt, "U8");
                swatch.setColor(color);
            }
            qint16 type = readShort(&buf);
            if (type == 1) { //0 is global, 2 is regular;
                swatch.setSpotColor(true);
            }
            if (inGroup) {
                colorSet->addSwatch(swatch, groupName);
            } else {
                colorSet->addSwatch(swatch);
            }
        }
        buf.seek(pos + qint64(blockSize));
    }
    return true;
}

bool KoColorSet::Private::loadAcb()
{

    QFileInfo info(colorSet->filename());
    colorSet->setName(info.completeBaseName());

    QBuffer buf(&data);
    buf.open(QBuffer::ReadOnly);

    QByteArray signature; // should be "8BCB";
    signature = buf.read(4);
    quint16 version = readShort(&buf);
    quint16 bookID = readShort(&buf);
    Q_UNUSED(bookID);

    if (signature != "8BCB" && version!= 1) {
        return false;
    }

    QStringList metadata;
    for (int i = 0; i< 4; i++) {

        QString metadataString = readUnicodeString(&buf, true);
        if (metadataString.startsWith("\"")) {
            metadataString = metadataString.remove(0, 1);
        }
        if (metadataString.endsWith("\"")) {
            metadataString.chop(1);
        }
        if (metadataString.startsWith("$$$/")) {
            if (metadataString.contains("=")) {
                metadataString = metadataString.split("=").last();
            } else {
                metadataString = QString();
            }
        }
        metadata.append(metadataString);
    }
    QString title = metadata.at(0);
    QString prefix = metadata.at(1);
    QString postfix = metadata.at(2);
    QString description = metadata.at(3);
    colorSet->setComment(description);

    quint16 numColors = readShort(&buf);
    quint16 numColumns = readShort(&buf);
    numColumns = numColumns > 0 ? numColumns : 8; // overwrite with sane default in case of 0
    colorSet->setColumnCount(numColumns);
    quint16 numKeyColorPage = readShort(&buf);
    Q_UNUSED(numKeyColorPage);
    quint16 colorType = readShort(&buf);

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    if (colorType == 2) {
        QString profileName = "U.S. Web Coated (SWOP) v2";
        cs = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id(), profileName);
    } else if (colorType == 7) {
        cs = KoColorSpaceRegistry::instance()->colorSpace(LABAColorModelID.id(), Integer8BitsColorDepthID.id(), QString());
    }

    for (quint16 i = 0; i < numColors; i++) {
        KisSwatch swatch;
        QStringList name;
        name << prefix;
        name << readUnicodeString(&buf, true);
        name << postfix;
        swatch.setName(name.join(" ").trimmed());
        QByteArray key; // should be "8BCB";
        key = buf.read(6);
        swatch.setId(QString::fromLatin1(key));
        swatch.setSpotColor(true);
        quint8 c1 = readByte(&buf);
        quint8 c2 = readByte(&buf);
        quint8 c3 = readByte(&buf);
        KoColor c(cs);
        if (colorType == 0) {
            c.data()[0] = c3;
            c.data()[1] = c2;
            c.data()[2] = c1;
        } else if (colorType == 2) {
            quint8 c4 = readByte(&buf);
            c.data()[0] = c1;
            c.data()[1] = c2;
            c.data()[2] = c3;
            c.data()[3] = c4;
        } else if (colorType == 7) {
            c.data()[0] = c1;
            c.data()[1] = c2;
            c.data()[2] = c3;
        }
        c.setOpacity(1.0);
        swatch.setColor(c);
        colorSet->addSwatch(swatch);
    }

    return true;
}

bool KoColorSet::Private::loadXml() {
    bool res = false;

    QXmlStreamReader *xml = new QXmlStreamReader(data);

    if (xml->readNextStartElement()) {
        QStringRef paletteId = xml->name();
        if (QStringRef::compare(paletteId, "SCRIBUSCOLORS", Qt::CaseInsensitive) == 0) { // Scribus
            dbgPigment << "XML palette: " << colorSet->filename() << ", Scribus format";
            res = loadScribusXmlPalette(colorSet, xml);
        }
        else {
            // Unknown XML format
            xml->raiseError("Unknown XML palette format. Expected SCRIBUSCOLORS, found " + paletteId);
        }
    }

    // If there is any error (it should be returned through the stream)
    if (xml->hasError() || !res) {
        warnPigment << "Illegal XML palette:" << colorSet->filename();
        warnPigment << "Error (line"<< xml->lineNumber() << ", column" << xml->columnNumber() << "):" << xml->errorString();
        return false;
    }
    else {
        dbgPigment << "XML palette parsed successfully:" << colorSet->filename();
        return true;
    }
}

bool KoColorSet::Private::saveKpl(QIODevice *dev) const
{
    QScopedPointer<KoStore> store(KoStore::createStore(dev, KoStore::Write, "application/x-krita-palette", KoStore::Zip));
    if (!store || store->bad()) {
        qWarning() << "saveKpl could not create store";
        return false;
    }

    QSet<const KoColorSpace *> colorSpaces;

    {
        QDomDocument doc;
        QDomElement root = doc.createElement(KPL_PALETTE_TAG);
        root.setAttribute(KPL_VERSION_ATTR, "2.0");
        root.setAttribute(KPL_PALETTE_NAME_ATTR, colorSet->name());
        root.setAttribute(KPL_PALETTE_COMMENT_ATTR, comment);
        root.setAttribute(KPL_PALETTE_COLUMN_COUNT_ATTR, colorSet->columnCount());
        root.setAttribute(KPL_GROUP_ROW_COUNT_ATTR, colorSet->getGlobalGroup()->rowCount());

        saveKplGroup(doc, root, colorSet->getGroup(KoColorSet::GLOBAL_GROUP_NAME), colorSpaces);

        for (const KisSwatchGroupSP &group : swatchGroups) {
            if (group->name() == KoColorSet::GLOBAL_GROUP_NAME) { continue; }
            QDomElement gl = doc.createElement(KPL_GROUP_TAG);
            gl.setAttribute(KPL_GROUP_NAME_ATTR, group->name());
            root.appendChild(gl);
            saveKplGroup(doc, gl, group, colorSpaces);
        }

        doc.appendChild(root);
        if (!store->open("colorset.xml")) { return false; }
        QByteArray ba = doc.toByteArray();
        if (store->write(ba) != ba.size()) { return false; }
        if (!store->close()) { return false; }
    }

    QDomDocument doc;
    QDomElement profileElement = doc.createElement("Profiles");

    for (const KoColorSpace *colorSpace : colorSpaces) {
        QString fn = QFileInfo(colorSpace->profile()->fileName()).fileName();
        if (!store->open(fn)) { qWarning() << "Could not open the store for profiles directory"; return false; }
        QByteArray profileRawData = colorSpace->profile()->rawData();
        if (!store->write(profileRawData)) { qWarning() << "Could not write the profiles data into the store"; return false; }
        if (!store->close()) { qWarning() << "Could not close the store for profiles directory"; return false; }
        QDomElement el = doc.createElement(KPL_PALETTE_PROFILE_TAG);
        el.setAttribute(KPL_PALETTE_FILENAME_ATTR, fn);
        el.setAttribute(KPL_PALETTE_NAME_ATTR, colorSpace->profile()->name());
        el.setAttribute(KPL_COLOR_MODEL_ID_ATTR, colorSpace->colorModelId().id());
        el.setAttribute(KPL_COLOR_DEPTH_ID_ATTR, colorSpace->colorDepthId().id());
        profileElement.appendChild(el);

    }
    doc.appendChild(profileElement);

    if (!store->open("profiles.xml")) { qWarning() << "Could not open profiles.xml"; return false; }
    QByteArray ba = doc.toByteArray();

    int bytesWritten = store->write(ba);
    if (bytesWritten != ba.size()) { qWarning() << "Bytes written is wrong" << ba.size(); return false; }

    if (!store->close()) { qWarning() << "Could not close the store"; return false; }

    bool r = store->finalize();
    if (!r) { qWarning() << "Could not finalize the store"; }
    return r;
}

void KoColorSet::Private::saveKplGroup(QDomDocument &doc,
                                       QDomElement &groupEle,
                                       const KisSwatchGroupSP group,
                                       QSet<const KoColorSpace *> &colorSetSet) const
{
    groupEle.setAttribute(KPL_GROUP_ROW_COUNT_ATTR, QString::number(group->rowCount()));

    for (const KisSwatchGroup::SwatchInfo &info : group->infoList()) {
        const KoColorProfile *profile = info.swatch.color().colorSpace()->profile();
        // Only save non-builtin profiles.=
        if (!profile->fileName().isEmpty()) {
            bool alreadyIncluded = false;
            Q_FOREACH(const KoColorSpace* colorSpace, colorSetSet) {
                if (colorSpace->profile()->fileName() == profile->fileName()) {
                    alreadyIncluded = true;
                    break;
                }
            }
            if(!alreadyIncluded) {
                colorSetSet.insert(info.swatch.color().colorSpace());
            }
        }
        QDomElement swatchEle = doc.createElement(KPL_SWATCH_TAG);
        swatchEle.setAttribute(KPL_SWATCH_NAME_ATTR, info.swatch.name());
        swatchEle.setAttribute(KPL_SWATCH_ID_ATTR, info.swatch.id());
        swatchEle.setAttribute(KPL_SWATCH_SPOT_ATTR, info.swatch.spotColor() ? "true" : "false");
        swatchEle.setAttribute(KPL_SWATCH_BITDEPTH_ATTR, info.swatch.color().colorSpace()->colorDepthId().id());
        info.swatch.color().toXML(doc, swatchEle);

        QDomElement positionEle = doc.createElement(KPL_SWATCH_POS_TAG);
        positionEle.setAttribute(KPL_SWATCH_ROW_ATTR, info.row);
        positionEle.setAttribute(KPL_SWATCH_COL_ATTR, info.column);
        swatchEle.appendChild(positionEle);

        groupEle.appendChild(swatchEle);
    }
}

void KoColorSet::Private::loadKplGroup(const QDomDocument &doc, const QDomElement &parentEle, KisSwatchGroupSP group, QString version)
{
    Q_UNUSED(doc);
    if (!parentEle.attribute(KPL_GROUP_ROW_COUNT_ATTR).isNull()) {
        group->setRowCount(parentEle.attribute(KPL_GROUP_ROW_COUNT_ATTR).toInt());
    }
    group->setColumnCount(colorSet->columnCount());

    for (QDomElement swatchEle = parentEle.firstChildElement(KPL_SWATCH_TAG);
         !swatchEle.isNull();
         swatchEle = swatchEle.nextSiblingElement(KPL_SWATCH_TAG)) {
        QString colorDepthId = swatchEle.attribute(KPL_SWATCH_BITDEPTH_ATTR, Integer8BitsColorDepthID.id());
        KisSwatch swatch;

        if (version == "1.0" && swatchEle.firstChildElement().tagName() == "Lab") {
            // previous version of krita had the values wrong, and scaled everything between 0 to 1,
            // but lab requires L = 0-100 and AB = -128-127.
            // TODO: write unittest for this.
            QDomElement el = swatchEle.firstChildElement();
            double L = KisDomUtils::toDouble(el.attribute("L"));
            el.setAttribute("L", L*100.0);
            double ab = KisDomUtils::toDouble(el.attribute("a"));
            if (ab <= .5) {
                ab = (0.5 - ab) * 2 * -128.0;
            } else {
                ab = (ab - 0.5) * 2 * 127.0;
            }
            el.setAttribute("a", ab);

            ab = KisDomUtils::toDouble(el.attribute("b"));
            if (ab <= .5) {
                ab = (0.5 - ab) * 2 * -128.0;
            } else {
                ab = (ab - 0.5) * 2 * 127.0;
            }
            el.setAttribute("b", ab);
            swatch.setColor(KoColor::fromXML(el, colorDepthId));
        } else {
            swatch.setColor(KoColor::fromXML(swatchEle.firstChildElement(), colorDepthId));
        }
        swatch.setName(swatchEle.attribute(KPL_SWATCH_NAME_ATTR));
        swatch.setId(swatchEle.attribute(KPL_SWATCH_ID_ATTR));
        swatch.setSpotColor(swatchEle.attribute(KPL_SWATCH_SPOT_ATTR, "false") == "true" ? true : false);
        QDomElement positionEle = swatchEle.firstChildElement(KPL_SWATCH_POS_TAG);
        if (!positionEle.isNull()) {
            int rowNumber = positionEle.attribute(KPL_SWATCH_ROW_ATTR).toInt();
            int columnNumber = positionEle.attribute(KPL_SWATCH_COL_ATTR).toInt();
            if (columnNumber < 0 ||
                    columnNumber >= colorSet->columnCount() ||
                    rowNumber < 0
                    ) {
                warnPigment << "Swatch" << swatch.name()
                            << "of palette" << colorSet->name()
                            << "has invalid position.";
                continue;
            }
            group->setSwatch(swatch, columnNumber, rowNumber);
        } else {
            group->addSwatch(swatch);
        }
    }

    if (parentEle.attribute(KPL_GROUP_ROW_COUNT_ATTR).isNull()
            && group->colorCount() > 0
            && group->columnCount() > 0
            && (group->colorCount() / (group->columnCount()) + 1) < 20) {
        group->setRowCount((group->colorCount() / group->columnCount()) + 1);
    }

}

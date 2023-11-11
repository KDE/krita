/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Halla Rempt <halla@valdyas.org>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PALETTEMODEL_H
#define KIS_PALETTEMODEL_H

#include <QPointer>
#include <QModelIndex>
#include <QMap>

#include <KoColorDisplayRendererInterface.h>

#include "kritawidgets_export.h"
#include <KoColorSet.h>
#include <KisSwatchGroup.h>
#include <QScopedPointer>

class KisPaletteView;

/**
 * @brief The KisPaletteModel class
 * This, together with KisPaletteView and KisPaletteDelegate forms a mvc way to access kocolorsets.
 * A display renderer is given to this model to convert KoColor to QColor when
 * colors are requested
 */
class KRITAWIDGETS_EXPORT KisPaletteModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit KisPaletteModel(QObject* parent = 0);
    ~KisPaletteModel() override;

    /**
     * Installs a display renderer object for a palette that will
     * convert the KoColor to the displayable QColor. Default is the
     * dumb renderer.
     */
    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer);


    enum AdditionalRoles {
        IsGroupNameRole = Qt::UserRole + 1,
        CheckSlotRole,
        GroupNameRole,
        RowInGroupRole
    };

public: // QAbstractTableModel

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    QStringList mimeTypes() const override;

    Qt::DropActions supportedDropActions() const override;

Q_SIGNALS:

    /**
     * @brief sigPaletteModified
     * emitted when palette associated with the model is modified
     */
    void sigPaletteModified();

    /**
     * @brief sigPaletteChanged
     * emitted when the palette associated with the model is changed for another palette
     */
    void sigPaletteChanged();

public:

    void setColorSet(KoColorSetSP colorSet);

    KoColorSetSP colorSet() const;

    void addSwatch(const KisSwatch &entry,
                   const QString &groupName = KoColorSet::GLOBAL_GROUP_NAME);

    void setSwatch(const KisSwatch &entry, const QModelIndex &index);

    void removeSwatch(const QModelIndex &index, bool keepColors=true);

    void changeGroupName(const QString &groupName, const QString &newName);

    void removeGroup(const QString &groupName, bool keepColors);

    KisSwatchGroupSP addGroup(const QString &groupName, int columnCount = KisSwatchGroup::DEFAULT_COLUMN_COUNT, int rowCount = KisSwatchGroup::DEFAULT_ROW_COUNT);

    void setRowCountForGroup(const QString &groupName, int rowCount);

    void setColumnCount(int colCount);

    void clear();

    void clear(int defaultColumnsCount);

    KisSwatch getSwatch(const QModelIndex &index) const;

    QModelIndex indexForClosest(const KoColor &compare);


    void slotExternalPaletteModified(QSharedPointer<KoColorSet> resource);

private Q_SLOTS:

    void slotDisplayConfigurationChanged();

    void slotPaletteModified();

private:

    friend class TestKisPaletteModel;

    int rowNumberInGroup(int rowInModel) const;
    int indexRowForInfo(const KisSwatchGroup::SwatchInfo &info);

    QVariant dataForGroupNameRow(const QModelIndex &idx, int role) const;
    QVariant dataForSwatch(const QModelIndex &idx, int role) const;

private /* member variables */:
    KoColorSetSP m_colorSet;
    QPointer<const KoColorDisplayRendererInterface> m_displayRenderer;

};

#endif

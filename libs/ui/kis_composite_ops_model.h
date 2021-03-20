/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_COMPOSITE_OPS_MODEL_H_
#define _KIS_COMPOSITE_OPS_MODEL_H_

#include <KoID.h>
#include "kis_categorized_list_model.h"

class KoColorSpace;

struct KoIDToQStringConverter {
    QString operator() (const KoID &id) {
        return id.name();
    }
};

typedef KisCategorizedListModel<KoID,KoIDToQStringConverter> BaseKoIDCategorizedListModel;

class KRITAUI_EXPORT KisCompositeOpListModel: public BaseKoIDCategorizedListModel
{
public:
    static KisCompositeOpListModel* sharedInstance();

    virtual QString  categoryToString(const KoID& val) const { return val.name(); }
    virtual QString  entryToString   (const KoID& val) const { return val.name(); }
    bool     setData         (const QModelIndex& idx, const QVariant& value, int role=Qt::EditRole) override;
    QVariant data            (const QModelIndex& idx, int role=Qt::DisplayRole) const override;

    void validate(const KoColorSpace *cs);
    void readFavoriteCompositeOpsFromConfig();
    void writeFavoriteCompositeOpsToConfig() const;

    static KoID favoriteCategory();

    void initialize();
    void initializeForLayerStyles();

private:
    void addFavoriteEntry(const KoID &entry);
    void removeFavoriteEntry(const KoID &entry);
};

/**
 * @brief The KisSortedCompositeOpListModel class provides a model for the composite op combobox.
 *
 * It intentionally does NOT use the shared instance of KisCompositeOpListModel because it is
 * perfect valid for two composite comboboboxes to show a different set of valid composite ops.
 */
class KRITAUI_EXPORT KisSortedCompositeOpListModel : public KisSortedCategorizedListModel<KisCompositeOpListModel>
{
public:
    KisSortedCompositeOpListModel(bool limitToLayerStyles, QObject *parent)
        : KisSortedCategorizedListModel<KisCompositeOpListModel>(parent)
    {
        if (limitToLayerStyles) {
            m_model.initializeForLayerStyles();
        } else {
            m_model.initialize();
        }

        initializeModel(&m_model);
    }

    void validate(const KoColorSpace *cs) {
        m_model.validate(cs);
    }

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
        return lessThanPriority(left, right, KisCompositeOpListModel::favoriteCategory().name());
    }
private:
    KisCompositeOpListModel m_model;
};

#endif

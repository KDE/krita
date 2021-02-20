/*
 *  kis_cmb_imagetype.h - part of KImageShop/Krayon/Krita
 *
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt (boud@valdyas.org)
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CMB_IDLIST_H_
#define KIS_CMB_IDLIST_H_

#include <QComboBox>
#include <QString>
#include <KoID.h>
#include <kritaui_export.h>

/**
 * A combobox that is associated with a list of KoID's. The
 * descriptive (i18n'ed) text is displayed, but the various
 * signals return a KoID.
 */
class KRITAUI_EXPORT KisCmbIDList: public QComboBox
{
    Q_OBJECT

public:
    KisCmbIDList(QWidget * parent = 0, const char * name = 0);
    ~KisCmbIDList() override;

    static const KoID AutoOptionID;

    /**
     * @brief setIDList clears the combobox and sorts
     *    the given list by user-visible name and then adds
     *    the items to the combobox
     * @param list the (unsorted) list of KoID's
     * @param sorted if true, the id's will be sorted by name
     */
    void setIDList(const QList<KoID> & list, bool sorted = true);

    /**
     * @brief allowAuto sets whether the combobox should keep
     *    an extra "AUTO" option, where the use allows the
     *    program to decide an appropriate option based on context.
     */
    void allowAuto(bool setAuto = true);

    void setAutoHint(const QString & hint);

    void setCurrent(const KoID id);
    void setCurrent(const QString & id);

    KoID currentItem() const;

Q_SIGNALS:
    void activated(const KoID &);
    void highlighted(const KoID &);

private Q_SLOTS:
    void slotIDActivated(int index);
    void slotIDHighlighted(int index);

private:
    void buildItems();

    QList<KoID> m_idList;
    bool m_sorted = true;
    bool m_autoOption = false;
    QString m_autoHint;
};
#endif

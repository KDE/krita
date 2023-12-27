/*
 *  SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WARNING_BLOCK_H
#define KIS_WARNING_BLOCK_H

#include <QGroupBox>

#include "kritawidgets_export.h"

class KRITAWIDGETS_EXPORT KisWarningBlock : public QGroupBox
{
    Q_OBJECT
public:
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)

    KisWarningBlock(QWidget *parent = nullptr);
    ~KisWarningBlock() override;

    KisWarningBlock(const KisWarningBlock &) = delete;
    KisWarningBlock &operator=(const KisWarningBlock &) = delete;

    QString text() const;
    QPixmap pixmap() const;

    void setText(const QString &);
    void setPixmap(const QPixmap &);

Q_SIGNALS:
    void linkActivated(const QString &link);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif

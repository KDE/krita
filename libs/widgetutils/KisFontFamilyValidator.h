/*
 *  SPDX-FileCopyrightText: 2024 Igor Danilets <danilec.igor@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef KIS_FONT_FAMILY_VALIDATOR_H
#define KIS_FONT_FAMILY_VALIDATOR_H

#include <QValidator>

class KisFontFamilyValidator : public QValidator
{
    Q_OBJECT
public:
    explicit KisFontFamilyValidator(QObject *parent = nullptr);
    KisFontFamilyValidator(const QStringList &families, QObject *parent);

    ~KisFontFamilyValidator();

    QValidator::State validate(QString &input, int &pos) const;
    void fixup(QString &input) const;

private:
    QStringList m_families;
};

#endif // KIS_FONT_FAMILY_VALIDATOR_H

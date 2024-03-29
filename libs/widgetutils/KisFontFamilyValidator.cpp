/*
 *  SPDX-FileCopyrightText: 2024 Igor Danilets <danilec.igor@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisFontFamilyValidator.h"

KisFontFamilyValidator::KisFontFamilyValidator(QObject *parent)
    : QValidator(parent)
{}

KisFontFamilyValidator::KisFontFamilyValidator(const QStringList &families, QObject *parent)
    : m_families(families)
    , QValidator(parent)
{}

QValidator::State KisFontFamilyValidator::validate(QString &input, int &pos) const
{
    for (const QString &family : m_families) {
        if (input == family) {
            return State::Acceptable;
        }

        if (family.startsWith(input, Qt::CaseInsensitive)) {
            return State::Intermediate;
        }
    }

    return State::Invalid;
}

void KisFontFamilyValidator::fixup(QString &input) const
{
    for (const QString &family : m_families) {
        if (!family.compare(input, Qt::CaseInsensitive)) {
            input = family;
        }
    }
}

KisFontFamilyValidator::~KisFontFamilyValidator(){}

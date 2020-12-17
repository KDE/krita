/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoResourceUpdateMediator.h"


struct KoResourceUpdateMediator::Private
{
    Private(int _key) : key(_key) {}
    int key;
};


KoResourceUpdateMediator::KoResourceUpdateMediator(int key)
    : m_d(new Private(key))
{
}

KoResourceUpdateMediator::~KoResourceUpdateMediator()
{
}

int KoResourceUpdateMediator::key() const
{
    return m_d->key;
}

/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "bristle.h"

Bristle::Bristle(float x, float y, float length)
    : m_x(x)
    , m_y(y)
    , m_prevX(x)
    , m_prevY(y)
    , m_length(length)
{}

Bristle::~Bristle()
{
}

void Bristle::setLength(float length)
{
    m_length = length;
}


void Bristle::addInk(float value)
{
    m_inkAmount = m_inkAmount + value;
}

void Bristle::removeInk(float value)
{
    m_inkAmount = m_inkAmount - value;
}

void Bristle::setInkAmount(float inkAmount)
{
    if (inkAmount > 1.0f) {
        inkAmount = 1.0f;
    }
    else if (inkAmount < -1.0f) {
        inkAmount = -1.0f;
    }

    m_inkAmount = inkAmount;
}

void Bristle::setColor(const KoColor &color)
{
    m_color = color;
}


void Bristle::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

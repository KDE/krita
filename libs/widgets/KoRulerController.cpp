/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoRulerController.h"
#include "KoRulerController_p.h"
#include "KoText.h"
#include "styles/KoParagraphStyle.h"

#include <KoCanvasResourceManager.h>
#include <KoTextDocument.h>

#include <WidgetsDebug.h>

#include <QVariant>
#include <QTextOption>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextLayout>
#include <QTextCursor>
#include <QLocale>

#include <KoRuler.h>

KoRulerController::KoRulerController(KoRuler *horizontalRuler, KoCanvasResourceManager *crp)
        : QObject(horizontalRuler),
        d(new Private(horizontalRuler, crp))
{
    connect(crp, SIGNAL(canvasResourceChanged(int, const QVariant &)), this, SLOT(canvasResourceChanged(int)));
    connect(horizontalRuler, SIGNAL(indentsChanged(bool)), this, SLOT(indentsChanged()));
    connect(horizontalRuler, SIGNAL(aboutToChange()), this, SLOT(tabChangeInitiated()));
    connect(horizontalRuler, SIGNAL(tabChanged(int, KoRuler::Tab*)), this, SLOT(tabChanged(int, KoRuler::Tab*)));
}

KoRulerController::~KoRulerController()
{
    delete d;
}

//have to include this because of Q_PRIVATE_SLOT
#include <moc_KoRulerController.cpp>

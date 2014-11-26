/*
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KoSectionUtils.h>
#include <KoSection.h>
#include <KoSectionEnd.h>

bool KoSectionUtils::getNextBlock(QTextCursor &cur)
{
    QTextCursor next = cur;
    bool ok = next.movePosition(QTextCursor::NextBlock);

    while (ok && next.currentFrame() != cur.currentFrame()) {
        ok = next.movePosition(QTextCursor::NextBlock);
    }

    if (!ok || next.currentFrame() != cur.currentFrame()) {
        // There is no previous block.
        return false;
    }
    cur = next;
    return true;
}

QString KoSectionUtils::sectionStartName(const QVariant &q)
{
    return static_cast<KoSection *>(q.value<void *>())->name();
}

QString KoSectionUtils::sectionEndName(const QVariant &q)
{
    return static_cast<KoSectionEnd *>(q.value<void *>())->name();
}

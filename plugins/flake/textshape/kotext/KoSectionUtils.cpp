/*
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#include <KoParagraphStyle.h>

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

void KoSectionUtils::setSectionStartings(QTextBlockFormat &fmt, const QList<KoSection *> &list)
{
    if (list.empty()) {
        fmt.clearProperty(KoParagraphStyle::SectionStartings);
    } else {
        fmt.setProperty(KoParagraphStyle::SectionStartings,
            QVariant::fromValue< QList<KoSection *> >(list));
    }
}

void KoSectionUtils::setSectionEndings(QTextBlockFormat &fmt, const QList<KoSectionEnd *> &list)
{
    if (list.empty()) {
        fmt.clearProperty(KoParagraphStyle::SectionEndings);
    } else {
        fmt.setProperty(KoParagraphStyle::SectionEndings,
            QVariant::fromValue< QList<KoSectionEnd *> >(list));
    }
}

QList<KoSection *> KoSectionUtils::sectionStartings(const QTextBlockFormat &fmt)
{
    if (!fmt.hasProperty(KoParagraphStyle::SectionStartings)) {
        return QList<KoSection *>();
    } else {
        return fmt.property(KoParagraphStyle::SectionStartings).value< QList<KoSection *> >();
    }
}

QList<KoSectionEnd *> KoSectionUtils::sectionEndings(const QTextBlockFormat &fmt)
{
    if (!fmt.hasProperty(KoParagraphStyle::SectionEndings)) {
        return QList<KoSectionEnd *>();
    } else {
        return fmt.property(KoParagraphStyle::SectionEndings).value< QList<KoSectionEnd *> >();
    }
}

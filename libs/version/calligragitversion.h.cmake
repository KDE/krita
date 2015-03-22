/* This file is part of the KDE project
 * Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 * Copyright (C) 2014 Jarosław Staniek <staniek@kde.org>
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

#ifndef __CALLIGRA_GIT_VERSION_H
#define __CALLIGRA_GIT_VERSION_H

/**
 * @def CALLIGRA_GIT_SHA1_STRING
 * @ingroup CalligraMacros
 * @brief Indicates the git sha1 commit which was used for compilation of Calligra
 */
#cmakedefine CALLIGRA_GIT_SHA1_STRING "@CALLIGRA_GIT_SHA1_STRING@"

/**
 * @def CALLIGRA_GIT_BRANCH_STRING
 * @ingroup CalligraMacros
 * @brief Indicates the git branch name which was used for compilation of Calligra
 */
#cmakedefine CALLIGRA_GIT_BRANCH_STRING "@CALLIGRA_GIT_BRANCH_STRING@"

#endif /* __CALLIGRA_GIT_VERSION_H */

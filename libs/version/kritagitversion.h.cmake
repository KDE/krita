/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 * SPDX-FileCopyrightText: 2014 Jarosław Staniek <staniek@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KRITA_GIT_VERSION_H
#define __KRITA_GIT_VERSION_H

/**
 * @def KRITA_GIT_SHA1_STRING
 * @ingroup KritaMacros
 * @brief Indicates the git sha1 commit which was used for compilation of Krita
 */
#cmakedefine KRITA_GIT_SHA1_STRING "@KRITA_GIT_SHA1_STRING@"

/**
 * @def KRITA_GIT_BRANCH_STRING
 * @ingroup KritaMacros
 * @brief Indicates the git branch name which was used for compilation of Krita
 */
#cmakedefine KRITA_GIT_BRANCH_STRING "@KRITA_GIT_BRANCH_STRING@"

/**
 * @def KRITA_GIT_DESCRIBE_STRING
 * @ingroup KritaMacros
 * @brief Indicates the git describe string
 */
#cmakedefine KRITA_GIT_DESCRIBE_STRING "@KRITA_GIT_DESCRIBE_STRING@"

#endif /* __KRITA_GIT_VERSION_H */

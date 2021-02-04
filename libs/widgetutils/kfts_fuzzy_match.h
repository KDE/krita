/*
    SPDX-FileCopyrightText: 2017 Forrest Smith
    SPDX-FileCopyrightText: 2020 Waqar Ahmed

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KFTS_FUZZY_MATCH_H
#define KFTS_FUZZY_MATCH_H

#include <QString>

/**
 * This is based on https://github.com/forrestthewoods/lib_fts/blob/master/code/fts_fuzzy_match.h
 * with modifications for Qt
 *
 * Dont include this file in a header file, please :)
 */

namespace kfts
{
/**
 * @brief simple fuzzy matching of chars in @a pattern with chars in @a str sequentially
 */
Q_DECL_UNUSED static bool fuzzy_match_simple(const QStringView pattern, const QStringView str);

/**
 * @brief This should be the main function you should use. @a outscore is the score
 * of this match and should be used to sort the results later. Without sorting of the
 * results this function won't be as effective.
 */
Q_DECL_UNUSED static bool fuzzy_match(const QStringView pattern, const QStringView str, int &outScore);
Q_DECL_UNUSED static bool fuzzy_match(const QStringView pattern, const QStringView str, int &outScore, uint8_t *matches, int maxMatches);

/**
 * @brief This is a special case function which doesn't score separator matches higher than sequential matches.
 * This is currently used in Kate's command bar where the string items are usually space separated names.
 */
Q_DECL_UNUSED static bool fuzzy_match_sequential(const QStringView pattern, const QStringView str, int &outScore);
/**
 * @brief get string for display in treeview / listview. This should be used from style delegate.
 * For example: with @a pattern = "kate", @a str = "kateapp" and @htmlTag = "<b>
 * the output will be <b>k</b><b>a</b><b>t</b><b>e</b>app which will be visible to user as
 * <b>kate</b>app.
 *
 * TODO: improve this so that we don't have to put html tags on every char probably using some kind
 * of interval container
 */
Q_DECL_UNUSED static QString to_fuzzy_matched_display_string(const QStringView pattern, QString &str, const QString &htmlTag, const QString &htmlTagClose);
}

namespace kfts
{
// Forward declarations for "private" implementation
namespace fuzzy_internal
{
static bool fuzzy_match_recursive(QStringView::const_iterator pattern,
                                  QStringView::const_iterator str,
                                  int &outScore,
                                  const QStringView::const_iterator strBegin,
                                  const QStringView::const_iterator strEnd,
                                  const QStringView::const_iterator patternEnd,
                                  uint8_t const *srcMatches,
                                  uint8_t *newMatches,
                                  int maxMatches,
                                  int nextMatch,
                                  int &recursionCount,
                                  int seqBonus = 15);
}

// Public interface
static bool fuzzy_match_simple(const QStringView pattern, const QStringView str)
{
    auto patternIt = pattern.cbegin();
    for (auto strIt = str.cbegin(); strIt != str.cend() && patternIt != pattern.cend(); ++strIt) {
        if (strIt->toLower() == patternIt->toLower()) {
            ++patternIt;
        }
    }
    return patternIt == pattern.cend();
}

static bool fuzzy_match(const QStringView pattern, const QStringView str, int &outScore)
{
    uint8_t matches[256];
    return fuzzy_match(pattern, str, outScore, matches, sizeof(matches));
}

static bool fuzzy_match(const QStringView pattern, const QStringView str, int &outScore, uint8_t *matches, int maxMatches)
{
    int recursionCount = 0;

    auto strIt = str.cbegin();
    auto patternIt = pattern.cbegin();
    const auto patternEnd = pattern.cend();
    const auto strEnd = str.cend();

    return fuzzy_internal::fuzzy_match_recursive(patternIt, strIt, outScore, strIt, strEnd, patternEnd, nullptr, matches, maxMatches, 0, recursionCount);
}

static bool fuzzy_match_sequential(const QStringView pattern, const QStringView str, int &outScore)
{
    int recursionCount = 0;
    uint8_t matches[256];
    auto maxMatches = sizeof(matches);
    auto strIt = str.cbegin();
    auto patternIt = pattern.cbegin();
    const auto patternEnd = pattern.cend();
    const auto strEnd = str.cend();

    return fuzzy_internal::fuzzy_match_recursive(patternIt, strIt, outScore, strIt, strEnd, patternEnd, nullptr, matches, maxMatches, 0, recursionCount, 40);
}

// Private implementation
static bool fuzzy_internal::fuzzy_match_recursive(QStringView::const_iterator pattern,
                                                  QStringView::const_iterator str,
                                                  int &outScore,
                                                  const QStringView::const_iterator strBegin,
                                                  const QStringView::const_iterator strEnd,
                                                  const QStringView::const_iterator patternEnd,
                                                  const uint8_t *srcMatches,
                                                  uint8_t *matches,
                                                  int maxMatches,
                                                  int nextMatch,
                                                  int &recursionCount,
                                                  int seqBonus)
{
    // Count recursions
    static constexpr int recursionLimit = 10;
    ++recursionCount;
    if (recursionCount >= recursionLimit) {
        return false;
    }

    // Detect end of strings
    if (pattern == patternEnd || str == strEnd) {
        return false;
    }

    // Recursion params
    bool recursiveMatch = false;
    uint8_t bestRecursiveMatches[256];
    int bestRecursiveScore = 0;

    // Loop through pattern and str looking for a match
    bool first_match = true;
    while (pattern != patternEnd && str != strEnd) {
        // Found match
        if (pattern->toLower() == str->toLower()) {
            // Supplied matches buffer was too short
            if (nextMatch >= maxMatches) {
                return false;
            }

            // "Copy-on-Write" srcMatches into matches
            if (first_match && srcMatches) {
                memcpy(matches, srcMatches, nextMatch);
                first_match = false;
            }

            // Recursive call that "skips" this match
            uint8_t recursiveMatches[256];
            int recursiveScore;
            auto strNextChar = std::next(str);
            if (fuzzy_match_recursive(pattern,
                                      strNextChar,
                                      recursiveScore,
                                      strBegin,
                                      strEnd,
                                      patternEnd,
                                      matches,
                                      recursiveMatches,
                                      sizeof(recursiveMatches),
                                      nextMatch,
                                      recursionCount)) {
                // Pick best recursive score
                if (!recursiveMatch || recursiveScore > bestRecursiveScore) {
                    memcpy(bestRecursiveMatches, recursiveMatches, 256);
                    bestRecursiveScore = recursiveScore;
                }
                recursiveMatch = true;
            }

            // Advance
            matches[nextMatch++] = (uint8_t)(std::distance(strBegin, str));
            ++pattern;
        }
        ++str;
    }

    // Determine if full pattern was matched
    bool matched = pattern == patternEnd ? true : false;

    // Calculate score
    if (matched) {
        int sequential_bonus = seqBonus; // bonus for adjacent matches
        static constexpr int separator_bonus = 30; // bonus if match occurs after a separator
        static constexpr int camel_bonus = 30; // bonus if match is uppercase and prev is lower
        static constexpr int first_letter_bonus = 15; // bonus if the first letter is matched

        static constexpr int leading_letter_penalty = -5; // penalty applied for every letter in str before the first match
        static constexpr int max_leading_letter_penalty = -15; // maximum penalty for leading letters
        static constexpr int unmatched_letter_penalty = -1; // penalty for every letter that doesn't matter

        // Iterate str to end
        while (str != strEnd) {
            ++str;
        }

        // Initialize score
        outScore = 100;

        // Apply leading letter penalty
        int penalty = leading_letter_penalty * matches[0];
        if (penalty < max_leading_letter_penalty) {
            penalty = max_leading_letter_penalty;
        }
        outScore += penalty;

        // Apply unmatched penalty
        const int unmatched = (int)(std::distance(strBegin, str)) - nextMatch;
        outScore += unmatched_letter_penalty * unmatched;

        // Apply ordering bonuses
        for (int i = 0; i < nextMatch; ++i) {
            uint8_t currIdx = matches[i];

            if (i > 0) {
                uint8_t prevIdx = matches[i - 1];

                // Sequential
                if (currIdx == (prevIdx + 1)) {
                    outScore += sequential_bonus;
                }
            }

            // Check for bonuses based on neighbor character value
            if (currIdx > 0) {
                // Camel case
                QChar neighbor = *(strBegin + currIdx - 1);
                QChar curr = *(strBegin + currIdx);
                if (neighbor.isLower() && curr.isUpper()) {
                    outScore += camel_bonus;
                }

                // Separator
                bool neighborSeparator = neighbor == QLatin1Char('_') || neighbor == QLatin1Char(' ');
                if (neighborSeparator) {
                    outScore += separator_bonus;
                }
            } else {
                // First letter
                outScore += first_letter_bonus;
            }
        }
    }

    // Return best result
    if (recursiveMatch && (!matched || bestRecursiveScore > outScore)) {
        // Recursive score is better than "this"
        memcpy(matches, bestRecursiveMatches, maxMatches);
        outScore = bestRecursiveScore;
        return true;
    } else if (matched) {
        // "this" score is better than recursive
        return true;
    } else {
        // no match
        return false;
    }
}

static QString to_fuzzy_matched_display_string(const QStringView pattern, QString &str, const QString &htmlTag, const QString &htmlTagClose)
{
    /**
     * FIXME Don't do so many appends. Instead consider using some interval based solution to wrap a range
     * of text with the html <tag></tag>
     */
    int j = 0;
    for (int i = 0; i < str.size() && j < pattern.size(); ++i) {
        if (str.at(i).toLower() == pattern.at(j).toLower()) {
            str.replace(i, 1, htmlTag + str.at(i) + htmlTagClose);
            i += htmlTag.size() + htmlTagClose.size();
            ++j;
        }
    }
    return str;
}
} // namespace kfts

#endif // KFTS_FUZZY_MATCH_H

/*
 *  SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPainterPath>

/**
 * @brief Tasty tofu for Kiki.
 *
 * Whenever we try to render a piece of text and it contains Unicode codepoints
 * that have no suitable glyphs in any of the matched fonts, a default glyph,
 * typically a small rectangle that resembles a block of "tofu" is rendered in
 * its place.
 *
 * Some applications (Firefox for example) take it further and, instead of
 * showing a hollow rectangle, actually print the hex representation of the
 * Unicode codepoint inside the rectangle, as a missing glyph indicator.
 * KisTofuGlyph does exactly this for Krita.
 */
namespace KisTofuGlyph
{

/**
 * @brief Creates a tofu missing glyph indicator representing the provided
 * Unicode codepoint.
 * 
 * @param codepoint Unicode codepoint to display
 * @return QPainterPath the glyph scaled to the specified height.
 */
QPainterPath create(char32_t codepoint, double height);

} // namespace KisTofuGlyph

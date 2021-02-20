/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

uniform vec4 fragColor;

out vec4 resultFragmentColor;

void main(void)
{
    resultFragmentColor = vec4(fragColor.x, fragColor.y, fragColor.z, fragColor.w);
}

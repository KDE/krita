/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

uniform vec4 fragColor;
uniform sampler2D texture0;

in vec4 v_textureCoordinate;
out vec4 resultFragmentColor;

void main(void)
{
    resultFragmentColor = fragColor * vec4(vec3(1.0 - texture(texture0, v_textureCoordinate.xy)), 1.0);
}

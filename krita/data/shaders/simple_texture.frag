/*
 *  shader for handling scaling
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

uniform sampler2D texture0;

in vec4 v_textureCoordinate;
out vec4 fragColor;

void main() {
    fragColor = texture(texture0, v_textureCoordinate.st);
}

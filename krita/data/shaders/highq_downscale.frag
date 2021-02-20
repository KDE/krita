/*
 *  shader for handling scaling
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

uniform sampler2D texture0;

#ifdef USE_OCIO
uniform sampler3D texture1;
#endif

in vec4 v_textureCoordinate;
out vec4 fragColor;

const float eps = 1e-6;

#if defined HIGHQ_SCALING || defined DIRECT_LOD_FETCH
uniform float viewportScale;
#endif /* defined HIGHQ_SCALING || defined DIRECT_LOD_FETCH */

#ifdef DIRECT_LOD_FETCH
uniform float fixedLodLevel;
#endif

#ifdef HIGHQ_SCALING

uniform float texelSize;

vec4 filterPureLinear8(sampler2D texture, vec2 texcoord)
{
    float newTexelSize = texelSize / viewportScale;
    float support = 0.5 * newTexelSize;
    float step = texelSize * 1.0;

    float offset = support - 0.5*texelSize;

    float level = 0.0;


    if (viewportScale < 0.03125) {
        level = 4.0;
    } else if (viewportScale < 0.0625) {
        level = 3.0;
    } else if (viewportScale < 0.125) {
        level = 2.0;
    } else if (viewportScale < 0.25) {
        level = 1.0;
    }

/*
    vec4 p1 = textureLod(texture, vec2(texcoord.x - offset, texcoord.y - offset), level);
    vec4 p2 = 2.0*textureLod(texture, vec2(texcoord.x         , texcoord.y - offset), level);
    vec4 p3 = textureLod(texture, vec2(texcoord.x + offset, texcoord.y - offset), level);

    vec4 p4 = 2.0*textureLod(texture, vec2(texcoord.x - offset, texcoord.y), level);
    vec4 p5 = 5.0*textureLod(texture, vec2(texcoord.x         , texcoord.y), level);
    vec4 p6 = 2.0*textureLod(texture, vec2(texcoord.x + offset, texcoord.y), level);

    vec4 p7 = textureLod(texture, vec2(texcoord.x - offset, texcoord.y + offset), level);
    vec4 p8 = 2.0*textureLod(texture, vec2(texcoord.x         , texcoord.y + offset), level);
    vec4 p9 = textureLod(texture, vec2(texcoord.x + offset, texcoord.y + offset), level);

    return (p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) / 17.0;
*/

    vec4 p1 = textureLod(texture, vec2(texcoord.x - offset, texcoord.y - offset), level);
    vec4 p2 = textureLod(texture, vec2(texcoord.x + offset, texcoord.y - offset), level);

    vec4 p3 = vec4(4.0) * textureLod(texture, vec2(texcoord.x, texcoord.y), level);

    vec4 p4 = textureLod(texture, vec2(texcoord.x - offset, texcoord.y + offset), level);
    vec4 p5 = textureLod(texture, vec2(texcoord.x + offset, texcoord.y + offset), level);

    vec4 p6 = vec4(3.0) * textureLod(texture, vec2(texcoord.x, texcoord.y), level + 1.0);

    return (p1 + p2 + p3 + p4 + p5 + p6) / vec4(11.0);

}

#endif /* HIGHQ_SCALING */

void main() {
    vec4 col;

#if defined HIGHQ_SCALING || defined DIRECT_LOD_FETCH

    if (viewportScale < 0.5 - eps) {

#ifdef DIRECT_LOD_FETCH

        if (fixedLodLevel > eps) {
            col = textureLod(texture0, v_textureCoordinate.st, fixedLodLevel);
        } else

#endif /* DIRECT_LOD_FETCH */

        {

#ifdef HIGHQ_SCALING
            col = filterPureLinear8(texture0, v_textureCoordinate.st);
#else /* HIGHQ_SCALING */
            col = texture(texture0, v_textureCoordinate.st);
#endif /* HIGHQ_SCALING */

        }
    } else

#endif /* defined HIGHQ_SCALING || defined DIRECT_LOD_FETCH */

    {
#ifdef DIRECT_LOD_FETCH

        if (fixedLodLevel > eps) {
            col = textureLod(texture0, v_textureCoordinate.st, fixedLodLevel);
        } else

#endif /* DIRECT_LOD_FETCH */
        {
            col = texture(texture0, v_textureCoordinate.st);
        }
    }

#ifdef USE_OCIO
    fragColor = OCIODisplay(col, texture1);
#else /* USE_OCIO */
    fragColor = col;
#endif /* USE_OCIO */

}

#version 130

/*
 * shader for handling scaling
 */
uniform sampler2D texture0;
uniform float viewportScale;
uniform float texelSize;
in vec4 v_textureCoordinate;
out vec4 fragColor;

const float eps = 1e-6;

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

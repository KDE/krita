#version 150 core
/*
 * shader for handling scaling
 */
uniform sampler2D texture0;

in vec4 pass_textureCoordinate;
out vec4 fragColor;

void main() {
    fragColor = texture(texture0, pass_textureCoordinate.st);
}


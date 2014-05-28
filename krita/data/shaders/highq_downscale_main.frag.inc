void main() {

    if (viewportScale < 0.5 - eps) {
        fragColor = filterPureLinear8(texture0, v_textureCoordinate.st);
    } else {
        fragColor = texture2D(texture0, v_textureCoordinate.st);
    }
}


uniform vec3 uColor;

out vec4 oColor;

void main()
{

    float average = (uColor.r + uColor.g + uColor.b) / 3;
    float biggest = max(uColor.r, max(uColor.g, uColor.b));
    float scale = biggest == 0 ? 0 : 1 / biggest;
    oColor = mix(vec4(0.7, 0.7, 0.7, 1),
                 vec4(scale * uColor, 1),
                 3 * average + 0.1);
}
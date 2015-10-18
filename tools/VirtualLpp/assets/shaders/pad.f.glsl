
uniform vec3 uColor;
uniform float uBrightness;

in vec2 texCoord;

out vec4 oColor;

void main()
{
    const float gradLength = 0.5;
    float gradStart = sqrt(uBrightness * 15);
    float dist = 2 * distance(texCoord, vec2(0.5, 0.5));
    float gradAmt = clamp(dist - gradStart, 0, gradLength) / gradLength;
    oColor = mix(vec4(uColor, 1), vec4(0.7, 0.7, 0.7, 1), gradAmt);
}
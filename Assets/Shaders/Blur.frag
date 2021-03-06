#include "ScreenPass.frag"

uniform int B_BlurRadius = 3;

void main()
{
    vec4 meanColor = vec4(0);
    for (int i = -B_BlurRadius; i <= B_BlurRadius; ++i)
    {
        for (int j = -B_BlurRadius; j <= B_BlurRadius; ++j)
        {
            meanColor += B_SampleColorOffset( vec2(i,j) );
        }
    }

    int kernelSize = (B_BlurRadius * 2 + 1);
    meanColor /= (kernelSize * kernelSize);
    B_GIn_Color = meanColor;
}

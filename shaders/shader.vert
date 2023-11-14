#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
//layout(location = 2) in vec2 inId;

layout(location = 0) out vec4 fragColor;

void main() {
    if(true)
    {
        gl_PointSize = 36.0 + 30 * inColor.a * inColor.a;
        gl_Position = vec4(inPosition.x/1920*1080,inPosition.y-0.25, 1.0, 1.0);
        fragColor = inColor;
        fragColor.g *= (0.3f + inColor.a);
    }
    else
    {
        gl_PointSize = 36.0;
        gl_Position = vec4(inPosition.xy, 1.0, 1.0);
        fragColor = inColor;
    }
}
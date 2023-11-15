#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inId;

layout(location = 0) out vec4 fragColor;

vec3 ApplyHue(vec3 col, float hueAdjust)
{
    const vec3 k = vec3(0.57735, 0.57735, 0.57735);
    float cosAngle = cos(hueAdjust);
    return col * cosAngle + cross(k, col) * sin(hueAdjust) + k * dot(k, col) * (1.0 - cosAngle);
}

/*
layout (binding = 0) uniform ParameterUBO {
    float deltaTime;
    float Battery;
} ubo;
*/

void main() {
    if(inId.y == 1)
    {
        gl_PointSize = 36.0 + 30 * inId.x * inId.x;
        gl_Position = vec4(inPosition.x/1920*1080,inPosition.y-0.25, 1.0, 1.0);
        fragColor = inColor;
        fragColor.a = inId.x;
        fragColor.g *= (1.3f - inId.x);
    }
    else
    {
        gl_PointSize = 36.0;
        gl_Position = vec4(inPosition.xy, 1.0, 1.0);

        vec3 col = mix(vec3(1,0,0),vec3(0.8f,0.8f,0),inColor.a);
        vec3 col1 = mix(vec3(0.8f,0.8f,0),inColor.rgb,inColor.a);
        fragColor.rgb = mix(col,col1,inColor.a);
        fragColor.a = inId.x;
        if(inId.x > 0.8f)
            fragColor.a *= 5.f-inId.x*5.f;
    }    
}
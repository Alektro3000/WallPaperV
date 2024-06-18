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

layout (std140,binding = 0) uniform ParameterUBO  {
    float deltaTime;
    float battery;
    float Volume;
    float Random;
    int MonitorCount;
    float Ratio;
    vec2 Dec;
    vec2 Position;
    vec2 PositionPrev;
    vec2 PositionPrev2;
    vec2 FullResolution;
    vec4[4] Monitors;
    vec4[25] Volumes;
} ubo;


float Remainder(float left, float right)
{
    return left/right - int(left/right);
}
float GetVecAsArr(vec4 vec, int id)
{
    if(id%4 > 1)
        return (id%2==2 ? vec.z : vec.w );
    else
        return (id%2==0 ? vec.x : vec.y );
}
void main() {
    vec4 MonInfo = ubo.Monitors[floatBitsToUint(inPosition.z)%ubo.MonitorCount];
    
    if(inId.y == 1)
    {
        vec2 Resolution = MonInfo.zw * ubo.FullResolution;
        gl_PointSize = 36.0 + 30 * inId.x * inId.x;
        gl_Position = vec4(inPosition.x*Resolution.y/Resolution.x,inPosition.y-0.25, 1.0, 1.0);
        //fragColor = mix(vec4(0.2,0.3,1,1),inColor,inId.x);
        fragColor = inColor;
        fragColor.a = inId.x;
        fragColor.g *= (1.2f - inId.x);
    }
    else    if(inId.y == 2)
    {
        gl_PointSize = 36.0;
        gl_Position = vec4(inPosition.xy, 1.0, 1.0);

        float alp = inColor.a;

        vec3 col = mix(vec3(1,0,0),vec3(0.8f,0.8f,0),alp);
        vec3 col1 = mix(vec3(0.8f,0.8f,0),inColor.rgb,alp);
        fragColor.rgb = mix(col,col1,alp);

        fragColor.a = inId.x;
        if(inId.x > 0.8f)
            fragColor.a *= 5.f-inId.x*5.f;
    }
    else if(inId.y == 3)
    {
        gl_PointSize = 14.0;
        gl_Position = vec4(inPosition.xy, 1.0, 1.0);
        
        
        float fid = inPosition.x*70+50;
            int id = int(fid);
            
            if(0 <= id && id < 100)
            {
                float u,k;
                
                u = GetVecAsArr(ubo.Volumes[id/4],id%4);
                k = GetVecAsArr(ubo.Volumes[(id+1)/4],(id+1)%4);
                //gl_Position.y -= mix(u,k,fid-id)/10;
                
            }
            


        fragColor = vec4(0.3f, 0.7f, 0.9f, 0.0f);
        fragColor.a = inId.x * inId.x/1.6;
    }
    else if(inId.y == 4)
    {
        gl_PointSize = 25.1;
        gl_Position = vec4(inPosition.xy, 1.0, 1.0);
        
        fragColor = vec4(0.2f, 0.9f, 1.f, 0.0f);
        fragColor.a = inId.x;
    }
    else
    {
        gl_PointSize = 15.1;
        const vec2 fact = ubo.FullResolution / 30.f;
        gl_Position = vec4(round(inPosition.xy*fact)/fact, 1.0, 1.0);
            
        fragColor = vec4(0.9f, 0.2f, 1.f, 0.0f);
        fragColor.a = inId.x;
        if(inId.x > 0.8f)
            fragColor.a *= 5.f-inId.x*5.f;
        //We return as cursor don't need to reposition between monitors
        return;
    }
    gl_Position.xy = MonInfo.xy+MonInfo.zw/2 + (gl_Position.xy*MonInfo.zw/2);
}
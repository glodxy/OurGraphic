#version 450

#extension GL_EXT_debug_printf : enable

#define PI 3.14159265359
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform uCommon {
    vec2 u_resolution;
};

layout(binding = 1) uniform uTime {
    float time;
};


void main() {
    vec2 p = (2.0*gl_FragCoord.xy-u_resolution.xy)/u_resolution.y;
    float tau = 3.1415926535*2.0;
    float a = atan(p.x,p.y);
    float r = length(p)*0.75;
    vec2 uv = vec2(a/tau,r);

    //get the color
    float xCol = (uv.x - (time / 3.0)) * 3.0;
    xCol = sign(xCol)*mod(xCol, 3.0);
    vec3 horColour = vec3(0.25, 0.25, 0.25);

    if (xCol < 1.0) {
        horColour.b += 1.0 - xCol;
        horColour.g += xCol;
        horColour.r += xCol;
    } else if (xCol < 2.0) {
        xCol -= 1.0;
        horColour.b += 1.0 - xCol;
        horColour.g += xCol;
        horColour.r += xCol;
    } else {
        xCol -= 2.0;
        horColour.b += 1.0 - xCol;
        horColour.g += xCol;
        horColour.r += xCol;
    }

    // draw color beam
    uv = (2.0 * uv) - 1.0;
    float beamWidth = (0.7+0.5*cos(uv.x*10.0*tau*0.15*clamp(floor(5.0 + 10.0*cos(time)), 0.0, 10.0))) * abs(1.0 / (30.0 * uv.y));
    vec3 horBeam = vec3(beamWidth,beamWidth,beamWidth);
    fragColor = vec4((( horBeam)* horColour ), 1.0);

}
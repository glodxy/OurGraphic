#version 450

#define PI 3.14159265359

layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform uCommon {
    vec2 u_resolution;
};

layout(binding = 1) uniform uTime {
    float time;
};

float circle(float r, float a) {
    return (1.-step(.705, r))*step(.695, r) * 1.;
}

float circle2(float r, float a) {
    float w = ( mod(a + PI / 4., 2.*PI) )/(2.*PI) *.1 + .01;
    return (1.-step(.7 + w, r))*step(.7 - w, r);
}



void main() {
    float u_time1 = .0;
    
    vec2 p = (2.0*gl_FragCoord.xy-u_resolution.xy)/u_resolution.y;
    float tau = 3.1415926535*2.0;
    float a = atan(p.x,p.y);
    float r = length(p)*1.;
    vec2 uv = vec2(a/tau,r);

    //get the color
    float xCol = (uv.x - (u_time1 / 3.0)) * 3.0;
    xCol = sign(xCol)*mod(xCol, 3.0);
    vec3 horColour = vec3(0., 0.25, 0.25);

    // xCol = -.;
    if (xCol < .0) {
        horColour.b += 1.0 - xCol;
        // horColour.g += xCol;
        horColour.r += xCol;
    } //else if (xCol < 2.0) {
    //     xCol -= 1.0;
    //     horColour.b += 1.0 - xCol;
    //     horColour.g += xCol;
    //     horColour.r += xCol;
    // } else {
    //     xCol -= 2.0;
    //     horColour.b += 1.0 - xCol;
    //     horColour.g += xCol;
    //     horColour.r += xCol;
    // }

    // draw color beam
    uv = (2.0 * uv) - 1.0;
    float beamWidth = (0.7+0.5*1.) * abs(1.0 / (30.0 * uv.y));
    vec3 horBeam = vec3(beamWidth,beamWidth,beamWidth);
    fragColor = vec4((( horBeam)* horColour ), 1.0);
}
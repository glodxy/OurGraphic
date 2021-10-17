#version 450

#extension GL_EXT_debug_printf : enable

#define PI 3.14159265359
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform uCommon {
    vec2 u_resolution;
};

vec3 colorA = vec3(0.149,0.141,0.912);
vec3 colorB = vec3(1.000,0.833,0.224);

// float plot (vec2 st, float pct){
//   return  smoothstep( pct-0.01, pct, st.y) -
//           smoothstep( pct, pct+0.01, st.y);
// }

float plot (vec2 st, int i) {
    float d = 0.5 - 0.02 * float(i);
    if(pow(st.x-0.5, 2.) + pow(st.y, 2.) > pow(d + 0.01, 2.)) return 0.;
    else if(pow(st.x-0.5, 2.) + pow(st.y, 2.) < pow(d - 0.01, 2.)) return 0.;
    //   else return 1.;

    float d1 = pow(d - 0.01, 2.);
    float d2 = pow(d + 0.01, 2.);
    float d3 = pow(d - 0.02, 2.);
    float r = pow(st.x-0.5, 2.) + pow(st.y, 2.);

    return smoothstep(d2, d1, r);
    //   return 1. - step(0.03, abs(st.y - sqrt(pow(0.5 - 0.02 * float(i), 2.) - pow(st.x-0.5, 2.))));
}


void main() {
    debugPrintfEXT("coord y: %f", gl_FragCoord.y);
    vec2 st = gl_FragCoord.xy/u_resolution.xy;
    st.y = 1.0 - st.y;
    // vec3 color = vec3(0.0);

    vec3 pct = vec3(st.x);

    // pct.r = smoothstep(0.0,1.0, st.x);
    // pct.g = sin(st.x*PI);
    // pct.b = pow(st.x,0.5);

    // color = mix(colorA, colorB, pct);

    // Plot transition lines for each channel
    // color = mix(color,vec3(1.0,0.0,0.0),plot(st,pct.r));
    // color = mix(color,vec3(0.0,1.0,0.0),plot(st,pct.g));
    // color = mix(color,vec3(0.0,0.0,1.0),plot(st,pct.b));

    vec3 colors[8];
    colors[0] = vec3(1, 0, 0);
    colors[1] = vec3(1, .5, 0);
    colors[2] = vec3(1, 1, 0);
    colors[3] = vec3(0, 1, 0);
    colors[4] = vec3(0, 1, 1);
    colors[5] = vec3(0, 0, 1);
    colors[6] = vec3(139./255., 0, 1);
    colors[7] = vec3(0, 0,0);

    vec3 color = vec3(0, 0,0);

    float distance = sqrt(pow(st.x-0.5, 2.) + st.y*st.y);

    int i = int(floor(25.5-50.*distance));

    for(int x = 0; x < 7; x++) {
        if(x == i) {
            color =  mix(colors[x], colors[x+1], plot(st, x));
            break;
        }
    }

    fragColor = vec4(color,1.0);
}
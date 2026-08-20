#version 450

layout(location = 0) in vec2 inFragmentTextureCoordinate;
layout(location = 1) in float iTime;

layout(location = 0) out vec4 outColor;

// Triangle intersection. Returns { t, u, v }.
vec3 triIntersect( in vec3 ro, in vec3 rd, in vec3 v0, in vec3 v1, in vec3 v2 )
{
    vec3 v1v0 = v1 - v0;
    vec3 v2v0 = v2 - v0;
    vec3 rov0 = ro - v0;

#if 0
    // Cramer's rule for solcing p(t) = ro+t·rd = p(u,v) = vo + u·(v1-v0) + v·(v2-v1).
    float d = 1.0/determinant(mat3(v1v0, v2v0, -rd ));
    float u =   d*determinant(mat3(rov0, v2v0, -rd ));
    float v =   d*determinant(mat3(v1v0, rov0, -rd ));
    float t =   d*determinant(mat3(v1v0, v2v0, rov0));
#else
    // The four determinants above have lots of terms in common. Knowing the changing
    // the order of the columns/rows doesn't change the volume/determinant, and that
    // the volume is dot(cross(a,b,c)), we can precompute some common terms and reduce
    // it all to:
    vec3  n = cross( v1v0, v2v0 );
    vec3  q = cross( rov0, rd );
    float d = 1.0/dot( rd, n );
    float u = d*dot( -q, v2v0 );
    float v = d*dot(  q, v1v0 );
    float t = d*dot( -n, rov0 );
#endif
    if( u<0.0 || v<0.0 || (u+v)>1.0 ) t = -1.0;
    return vec3( t, u, v );
}

// Triangle occlusion (if fully visible).
float triOcclusion( in vec3 pos, in vec3 nor, in vec3 v0, in vec3 v1, in vec3 v2 ) {
    vec3 a = normalize(v0-pos);
    vec3 b = normalize(v1-pos);
    vec3 c = normalize(v2-pos);
    float s = -sign(dot(v0-pos,cross(v0-v1,v2-v1))); // other side of the triangle
    // page 300 in http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.466.963&rep=rep1&type=pdf
    float r = dot(nor,normalize(cross(a,b))) * acos(dot(a,b)) +
              dot(nor,normalize(cross(b,c))) * acos(dot(b,c)) +
              dot(nor,normalize(cross(c,a))) * acos(dot(c,a));

    return 1.0-max(0.0,s*r)/6.2831;
}

float iPlane( in vec3 ro, in vec3 rd )
{
    return (-1.0 - ro.y)/rd.y;
}

vec3 pattern( in vec2 uv )
{
    vec3 col = vec3(0.6);
    col += 0.4*smoothstep(-0.01,0.01,cos(uv.x*0.5)*cos(uv.y*0.5));
    col *= smoothstep(-1.0,-0.98,cos(uv.x))*smoothstep(-1.0,-0.98,cos(uv.y));
    return col;
}

#define AA 3

void main()
{
    vec2 iResolution = vec2( 1920, 1080 );
    vec3 tot = vec3(0.0);
    #if AA>1
    for( int m=0; m<AA; m++ )
    for( int n=0; n<AA; n++ )
    {
        // Pixel coordinates.
        vec2 o = vec2(float(m),float(n)) / float(AA) - 0.5;
        vec2 p = (-iResolution.xy + 2.0*(gl_FragCoord.xy+o))/-iResolution.y;
        #else
        vec2 p = (-iResolution.xy + 2.0*gl_FragCoord.xy)/-iResolution.y;
        #endif
        vec3 ro = vec3(0.0, 0.0, 4.0 );
        vec3 rd = normalize( vec3(p,-2.0) );
        // Triangle animation.
        vec3 v1 = cos( iTime*1.0 + vec3(2.0,1.0,1.0) + 1.0 )*vec3(1.5,1.0,1.0);
	    vec3 v2 = cos( iTime*1.0 + vec3(5.0,2.0,3.0) + 2.0 )*vec3(1.5,1.0,1.0);
	    vec3 v3 = cos( iTime*1.2 + vec3(1.0,3.0,5.0) + 4.0 )*vec3(1.5,1.0,1.0);
        vec3 col = vec3(0.08) + 0.02*rd.y;
        float tmin = 1e10;
        float t1 = iPlane( ro, rd );
        if( t1>0.0 )
        {
            tmin = t1;
            vec3 pos = ro + tmin*rd;
            vec3 nor = vec3(0.0,1.0,0.0);
            float occ = triOcclusion( pos, nor, v1, v2, v3 );
            col = mix( col*3.0*occ*occ, col, 1.0-exp(-0.02*tmin) );
        }
        col *= 1.0-0.3*length(p);
        vec3 res = triIntersect( ro, rd, v1, v2, v3 );
        float t2 = res.x;
        if( t2>0.0 && t2<tmin )
        {
            tmin = t2;
            float t = t2;
            vec3 pos = ro + t*rd;
            vec3 nor = normalize( cross( v2-v1, v3-v1 ) );
            col = pattern(64.0*res.yz);
            col *= 0.55 + 0.45*faceforward(-nor, -rd, nor).y;
        }
        col = sqrt( col );
	    tot += col;
    #if AA>1
    }
    tot /= float(AA*AA);
    #endif
    // Dither to remove banding in the background.
    tot += fract(sin(gl_FragCoord.x*vec3(13,1,11)+gl_FragCoord.y*vec3(1,7,5))*158.391832)/255.0;
	outColor = vec4( tot, 1.0 );
}

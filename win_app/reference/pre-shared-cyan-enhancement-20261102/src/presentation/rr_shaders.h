#ifndef RR_SHADERS_H
#define RR_SHADERS_H
#if defined(PLATFORM_WEB)
#define RR_GLSL "#version 300 es\nprecision highp float;\n"
#else
#define RR_GLSL "#version 330\n"
#endif

static const char *rr_world_vertex=RR_GLSL
"in vec3 vertexPosition; in vec2 vertexTexCoord; in vec3 vertexNormal; in vec4 vertexColor;\n"
"uniform mat4 mvp; uniform mat4 matModel; uniform mat4 matNormal;\n"
"out vec2 uv; out vec3 worldPos; out vec3 normal; out vec4 tint;\n"
"void main(){ uv=vertexTexCoord; tint=vertexColor; worldPos=vec3(matModel*vec4(vertexPosition,1.)); normal=normalize(vec3(matNormal*vec4(vertexNormal,0.))); gl_Position=mvp*vec4(vertexPosition,1.); }\n";
static const char *rr_world_fragment=RR_GLSL
"in vec2 uv; in vec3 worldPos; in vec3 normal; in vec4 tint;\n"
"uniform sampler2D texture0; uniform vec4 colDiffuse; uniform vec3 viewPos,viewForward,fogColor; uniform float fogDensity; out vec4 finalColor;\n"
"void main(){ vec4 t=texture(texture0,uv)*colDiffuse*tint; vec3 n=normalize(normal); float light=max(dot(n,normalize(vec3(-.3,.8,.6))),0.);\n"
"vec3 color=t.rgb*(vec3(.42,.40,.65)+light*vec3(.65,.72,.9));\n"
"float hi=max(t.r,max(t.g,t.b)); float lo=min(t.r,min(t.g,t.b)); float neon=smoothstep(.14,.55,hi-lo)*smoothstep(.38,.8,hi); color+=t.rgb*neon*.9;\n"
"float d=dot(worldPos-viewPos,viewForward); float fog=1.-exp(-fogDensity*fogDensity*d*d); color=mix(color,fogColor,fog); finalColor=vec4(color,t.a); }\n";
static const char *rr_post_fragment=RR_GLSL
"in vec2 fragTexCoord; in vec4 fragColor; uniform sampler2D texture0; uniform vec2 resolution; out vec4 finalColor;\n"
"void main(){ vec2 uv=fragTexCoord; vec3 color=texture(texture0,uv).rgb; vec3 glow=vec3(0.);\n"
"for(int i=-2;i<=2;i++){ for(int j=-2;j<=2;j++){ vec3 s=texture(texture0,uv+vec2(float(i),float(j))*2.5/resolution).rgb; glow+=max(s-vec3(.65),vec3(0.)); }}\n"
"color+=glow*.055; vec2 p=uv-.5; color*=1.-.38*dot(p,p); finalColor=vec4(color,1.); }\n";
#endif

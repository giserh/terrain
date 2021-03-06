#version 330 core

// Light properties.
uniform vec3 Ia, Id, Is;

// Texture 0. Defined by glActiveTexture and passed by glUniform1i.
uniform sampler2D heightMapTex;

// Texture 1. Defined by glActiveTexture and passed by glUniform1i.
// Shadow sampler for percentage closer filtering (PCF).
//uniform sampler2D shadowMapTex;
uniform sampler2DShadow shadowMapTex;

// Environmental textures 2-7. Defined by glActiveTexture and passed by glUniform1i.
uniform sampler2D sandTex, iceMoutainTex, treeTex, stoneTex, underWaterTex, snowTex;

// Vertices 3D position (after heightmap displacement) in world space.
in vec3 vertexPosition3DWorld;

// Vertex position in light source clip space.
// Coordinates for shadowmap texture look-up.
in vec3 shadowCoord;

// Light and view directions.
in vec3 lightDirWorld, viewDirCamera;

// First output buffer is pixel color (mandatory output, gl_FragColor).
layout(location = 0) out vec3 color;


// Different levels of height for texture mapping.
const float lowest = -0.05;
const float ground = 0.000;  //water height level
const float sandMin = 0.000;
const float sandMax = 0.010;
const float forest = 0.16;
const float snowMin = 0.21;


vec3 compute_normal(vec3 position) {

    const ivec3 off = ivec3(-1, 0, 1);
    // Retrieve the size of the generated height map texture.
    int width = textureSize(heightMapTex, 0).x;
    vec2 size = vec2(2.0/width, 0.0);

    //current UV coordinate
    vec2 UV = vec2((position.xy +1.0)/2.0);

    //first calculate the normal vector using finite difference
    float s11 = texture(heightMapTex, UV).r;
    float s01 = textureOffset(heightMapTex, UV, off.xy).r;
    float s21 = textureOffset(heightMapTex, UV, off.zy).r;
    float s10 = textureOffset(heightMapTex, UV, off.yx).r;
    float s12 = textureOffset(heightMapTex, UV, off.yz).r;

    vec3 va = normalize(vec3(size.xy, s21 - s01));
    vec3 vb = normalize(vec3(0.0, size.x,  s12 - s10));
    vec3 tmp = cross(va,vb);
    vec3 normal = vec3(tmp.xy, 2*tmp.z);

    return normalize(normal);

}

float rand(vec2 co){
    float a = fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
	
	
	return a;
}

float offset_calc(vec2 co){
	float a = 0.2*sin(0.5f*co.x);
	float b = 0.4*sin(0.5f*co.y);

	return min(max(a+b, -0.04), 0.04);
}

float Cosine( vec2 co, float slope )
{
    float tRemapCosine = ( 1 - cos( slope * 3.14f ) ) * 0.5f;
    return mix( co.x, co.y, tRemapCosine );
}

vec3 texture_mapping(vec3 position, vec3 normal) {

    // Color dependent on the elevation (similar to texture mapping).
    vec3 mapped;

    float slope = smoothstep(0.35, 0.65 , normal.z);

	float offset = rand(position.xy)*0.05;

    if (position.z < ground) {
        mapped = texture2D(sandTex, 60.0*position.xy).rgb;
    } else if(position.z < sandMin) {
        mapped = texture2D(sandTex, 60.0*position.xy).rgb;
    } else if (position.z < sandMax) {
        float w = (position.z - sandMin)/(sandMax-sandMin);
        vec3 sand = texture2D(sandTex, 60.0*position.xy).rgb;
        vec3 stone = texture2D(stoneTex, 10.0*position.xy).rgb;
        vec3 forest = texture2D(treeTex, 10.0*position.xy).rgb;
        vec3 stone_forest = mix(stone, forest, slope);
		//float weight = blend_func(w);
        mapped = mix(sand, stone_forest, w);
    } else if (position.z  < forest) {  //mix between forest and rock
        vec3 stone = texture2D(stoneTex, 10.0*position.xy).rgb;
        vec3 forest = texture2D(treeTex, 10.0*position.xy).rgb;
        mapped = mix(stone, forest, slope);
    } else if (position.z < (snowMin + min(Cosine(position.xy, slope), 0.2f))) {
        float w = (position.z - forest)/(snowMin-forest) ;
		float opacity = rand(position.xy);
        vec4 snow = vec4(texture2D(snowTex, 30.0*position.xy).rgb, w);
        vec3 stone = texture2D(stoneTex, 10.0*position.xy).rgb;
        vec3 forest = texture2D(treeTex, 10.0*position.xy).rgb;
        vec4 stone_forest = vec4(mix(stone, forest, slope), 1.0);
        vec4 tmp = mix(stone_forest, snow, w);
		mapped = vec3(tmp);
		
    } else {
        float w = (position.z - forest)/(snowMin-forest) + rand(position.xy)/20.0f;
        vec4 snow = vec4(texture2D(snowTex, 60.0*position.xy).rgb, w);
        vec3 stone = texture2D(stoneTex, 10.0*position.xy).rgb;
        vec3 forest = texture2D(treeTex, 10.0*position.xy).rgb;
        vec4 stone_forest = vec4(mix(stone, forest, slope/2.0), 1.0 - w + 0.3);
        vec4 tmp = mix(stone_forest, snow, w);
		mapped = vec3(tmp);
    }

    return mapped;

}


float shadowmap(vec3 coord) {

    // Small epsilon to avoid Z-fighting : from 0.001 to 0.00001.
    const float bias = 0.001;

    // The texture only stores one component : r (red).
    // Z is the distance to the camera in camera space.

    // Simple binary shadow test.
//    float visibility = 1.0;
//    if(texture(shadowMapTex, coord.xy).r  <  (coord.z-bias)) {
//        visibility = 0.0;
//    }

    // Percentage closer filtering (PCF).
    // Need sampler2DShadow and compare function in texture parameters.
    vec3 UVC = vec3(coord.xy, coord.z-bias);
    float visibility = texture(shadowMapTex, UVC);

    return visibility;
}


void main() {

    // Normalize light and view directions.
    vec3 L = normalize(lightDirWorld);
    vec3 V = normalize(viewDirCamera);

    // Compute the normal.
    vec3 normal = compute_normal(vertexPosition3DWorld);

    // Retrieve material properties from textures.
    vec3 material = texture_mapping(vertexPosition3DWorld, normal);

    // Add some buewish color, according to depth.
    vec3 bluewish = vec3(0.32, 0.56, 0.64);
    if(vertexPosition3DWorld.z < ground) {
        material = mix(material, bluewish, vertexPosition3DWorld.z / lowest);
    } else if(vertexPosition3DWorld.z < lowest) {
        material = bluewish;
    }

    // Specular lightning only relevant for water surfaces.
    float ka, kd;
    ka = 0.4f;
    kd = 0.6f;

    // Compute diffuse : "color" of the object.
    vec3 diffuse = Id * kd * material * max(dot(normal,L),0.0);

    // Compute ambient : simulates indirect lighting.

    /* different setting for snow */
    vec3 ambient;
   /// if (vertexPosition3DWorld.z >= forest){
        //ambient = vec3(0.9f,0.9f,0.9f)*0.8f*material;
    //} else {
        ambient = Ia * ka * material;
    //}

    // Query the visibility.
    float visibility = shadowmap(shadowCoord);

    // Assemble the colors. No specular term

    color = ambient + visibility * diffuse;
    //color = vec4(0.0, 0.0, 1.0, 1.0);


}


#version 330 core
// point position
layout (location = 0) in vec2 texcoord;

out vec4 vPosition;
out vec4 vColor;
out vec4 vNormRad;

uniform sampler2D cSampler;
uniform sampler2D drSampler;
uniform usampler2D sSampler;
uniform isampler2D indexSampler;
uniform sampler2D vertConfSampler;
uniform sampler2D colorTimeSampler;
uniform sampler2D normRadSampler;

uniform vec4 cam; //cx, cy, 1/fx, 1/fy
uniform float cols;
uniform float rows;
uniform int scale;
uniform float texDim;
uniform mat4 pose;
uniform float minDepth;
uniform float maxDepth;
uniform float time;
uniform float fuseThresh;

#include "surfels.glsl"
#include "color.glsl"
#include "geometry.glsl"

bool checkNeighbours(vec2 texCoord, sampler2D depth)
{
    float z = float(textureLod(depth, vec2(texCoord.x - (1.0 / cols), texCoord.y), 0.0));
    if(z == 0)
    return false;

    z = float(textureLod(depth, vec2(texCoord.x, texCoord.y - (1.0 / rows)), 0.0));
    if(z == 0)
    return false;

    z = float(textureLod(depth, vec2(texCoord.x + (1.0 / cols), texCoord.y), 0.0));
    if(z == 0)
    return false;

    z = float(textureLod(depth, vec2(texCoord.x, texCoord.y + (1.0 / rows)), 0.0));
    if(z == 0)
    return false;

    return true;
}

float angleBetween(vec3 a, vec3 b)
{
    return acos(dot(a, b) / (length(a) * length(b)));
}

void main()
{
    // Raw data pixel, should be guaranteed to be in bounds and centred on pixels
    float x = texcoord.x * cols;
    float y = texcoord.y * rows;

    // unit plane coordinates, change into the cam coordinate
    float xl = (x - cam.x) * cam.z;
    float yl = (y - cam.y) * cam.w;

    // unit plane vector
    vec3 ray = vec3(xl, yl, 1);
    float lambda = sqrt(xl * xl + yl * yl + 1);  // length of ray

    // pixel and sub pixel size
    float pix_size_x = 1.0f / cols;
    float pix_size_y = 1.0f / rows;

    float subpix_size_x = 1.0f / (cols * scale);
    float subpix_size_y = 1.0f / (rows * scale);

    // intensity of this pixel, drSampler is depth image's tid
    float value = float(texture(drSampler, texcoord));


    //============ Associate with Model Surfels ============//

    // If this point is actually a valid vertex
    if(checkNeighbours(texcoord.xy, drSampler) && value > minDepth && value < maxDepth
    && (int(x) + int(y)) % 2 == 1           // 1/2 sparse
    //                                               && int(x) % 2 == 0 && int(y) % 2 == 0   // 1/4 sparse
    //                                               && int(x) % 3 == 0 && int(y) % 3 == 0   // 1/9 sparse
    //                                               && int(x) % 4 == 0 && int(y) % 4 == 0   // 1/16 sparse
    )
    {
        //============ Calculate new surfels locally ============//

        // position
        vec3 vPosLocal = getVertex(texcoord.xy, x, y, cam, drSampler);
        // normal
        vec3 vNormLocal = getNormal(vPosLocal, texcoord.xy, x, y, cam, drSampler);

        // confidence todo new method
        //float maxRadDist2 = (cols / 2) * (cols / 2) + (rows / 2) * (rows / 2);
        //float c_n = confidence(maxRadDist2, x, y, 1.0);
        float c_n = 0.9;

        // color, cSampler is rgb's tid
        vec4 texColor = textureLod(cSampler, texcoord.xy, 0.0);
        vec3 color_n = texColor.xyz;
        float radii_n = getRadius(vPosLocal.z, vNormLocal.z);

        // semantic, sSampler is semantic's tid
        uint sem_n = uint(texture(sSampler, texcoord.xy));

        //============ construct the new unstable surfel ============//
        // position
        vPosition = pose * vec4(vPosLocal, 1);
        vPosition.w = c_n;

        // normal & radii
        vNormRad = vec4(mat3(pose) * vNormLocal, radii_n);  // rotation * normal
        vNormRad.xyz = normalize(vNormRad.xyz);

        // color
        vColor.x = encodeColor(color_n, sem_n);
        vColor.y = -1.f;                           // marks as new unstable
        vColor.z = time;
        vColor.w = time;
    }
    else  //============ Other vertex ============//
    {
        vPosition = vec4(0.0);
        vNormRad = vec4(0.0);
        vColor.y = -10.f;
    }
}
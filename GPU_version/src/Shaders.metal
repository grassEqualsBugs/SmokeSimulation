#include <metal_stdlib>
#import "../include/SimParams.h"
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float2 uv;
};

vertex VertexOut vertex_main(uint vertexID [[vertex_id]]) {
    float2 positions[6] = {
        float2(-1.0, -1.0),
        float2( 1.0, -1.0),
        float2(-1.0,  1.0),
        float2(-1.0,  1.0),
        float2( 1.0, -1.0),
        float2( 1.0,  1.0),
    };
    VertexOut out;
    out.position = float4(positions[vertexID], 0.0, 1.0);
    out.uv = positions[vertexID] * 0.5 + float2(0.5);
    return out;
}

fragment float4 fragment_main(
	VertexOut in [[stage_in]],
	texture2d<float> texture [[texture(0)]])
{
	constexpr sampler s(filter::linear);
    return texture.sample(s, in.uv);
}

kernel void inject_velocity(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velXTemp [[texture(1)]],
    texture2d<float, access::read_write> velY     [[texture(2)]],
    texture2d<float, access::read_write> velYTemp [[texture(3)]],
    texture2d<float, access::read_write> pressure [[texture(4)]],
    texture2d<float, access::read_write> smoke  [[texture(5)]],
    texture2d<float, access::read_write> smokeTemp [[texture(6)]],
    texture2d<uint,  access::read>       solids   [[texture(7)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void inject_smoke(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velXTemp [[texture(1)]],
    texture2d<float, access::read_write> velY     [[texture(2)]],
    texture2d<float, access::read_write> velYTemp [[texture(3)]],
    texture2d<float, access::read_write> pressure [[texture(4)]],
    texture2d<float, access::read_write> smoke  [[texture(5)]],
    texture2d<float, access::read_write> smokeTemp [[texture(6)]],
    texture2d<uint,  access::read>       solids   [[texture(7)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void advect_velX(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velXTemp [[texture(1)]],
    texture2d<float, access::read_write> velY     [[texture(2)]],
    texture2d<float, access::read_write> velYTemp [[texture(3)]],
    texture2d<float, access::read_write> pressure [[texture(4)]],
    texture2d<float, access::read_write> smoke  [[texture(5)]],
    texture2d<float, access::read_write> smokeTemp [[texture(6)]],
    texture2d<uint,  access::read>       solids   [[texture(7)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void advect_velY(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velXTemp [[texture(1)]],
    texture2d<float, access::read_write> velY     [[texture(2)]],
    texture2d<float, access::read_write> velYTemp [[texture(3)]],
    texture2d<float, access::read_write> pressure [[texture(4)]],
    texture2d<float, access::read_write> smoke  [[texture(5)]],
    texture2d<float, access::read_write> smokeTemp [[texture(6)]],
    texture2d<uint,  access::read>       solids   [[texture(7)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void advect_smoke(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velXTemp [[texture(1)]],
    texture2d<float, access::read_write> velY     [[texture(2)]],
    texture2d<float, access::read_write> velYTemp [[texture(3)]],
    texture2d<float, access::read_write> pressure [[texture(4)]],
    texture2d<float, access::read_write> smoke  [[texture(5)]],
    texture2d<float, access::read_write> smokeTemp [[texture(6)]],
    texture2d<uint,  access::read>       solids   [[texture(7)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{
	uint width = (uint) constants.width;
	uint height = (uint) constants.height;

    if (gid.x >= width || gid.y >= height) return;
    float u = (float) gid.x / (float) width;
    float v = (float) gid.y / (float) height;
    float aspect = (float) width / (float) height;

    float2 uvCorrected = float2(u * aspect , v);
    float2 mouseCorrected = float2(frame.mouse.pos.x * aspect, frame.mouse.pos.y);

    float3 color = float3(u, v, 0.f);
    if (distance(uvCorrected, mouseCorrected) <= constants.mouseRadius) {
    	color = float3(v, u, 0.f);
    }
    smoke.write(float4(color, 0.0), gid);
}

kernel void gs_red(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velXTemp [[texture(1)]],
    texture2d<float, access::read_write> velY     [[texture(2)]],
    texture2d<float, access::read_write> velYTemp [[texture(3)]],
    texture2d<float, access::read_write> pressure [[texture(4)]],
    texture2d<float, access::read_write> smoke  [[texture(5)]],
    texture2d<float, access::read_write> smokeTemp [[texture(6)]],
    texture2d<uint,  access::read>       solids   [[texture(7)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void gs_black(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velXTemp [[texture(1)]],
    texture2d<float, access::read_write> velY     [[texture(2)]],
    texture2d<float, access::read_write> velYTemp [[texture(3)]],
    texture2d<float, access::read_write> pressure [[texture(4)]],
    texture2d<float, access::read_write> smoke  [[texture(5)]],
    texture2d<float, access::read_write> smokeTemp [[texture(6)]],
    texture2d<uint,  access::read>       solids   [[texture(7)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void update_velocities(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velXTemp [[texture(1)]],
    texture2d<float, access::read_write> velY     [[texture(2)]],
    texture2d<float, access::read_write> velYTemp [[texture(3)]],
    texture2d<float, access::read_write> pressure [[texture(4)]],
    texture2d<float, access::read_write> smoke  [[texture(5)]],
    texture2d<float, access::read_write> smokeTemp [[texture(6)]],
    texture2d<uint,  access::read>       solids   [[texture(7)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{}

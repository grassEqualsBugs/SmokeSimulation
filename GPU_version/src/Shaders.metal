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

float2 get_uv(uint2 gid, constant SimConstants& constants) {
	return float2(gid.x / (float)constants.width, gid.y / (float)constants.height);
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
{
    if (gid.x >= (uint)constants.width || gid.y >= (uint)constants.height) return;
    if (solids.read(gid).x > 0) {
        smoke.write(float4(1.0, 1.0, 1.0, 1.0), gid);
        return;
    }
}

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
{}

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

kernel void init_solids(
    texture2d<uint, access::write> solids [[texture(0)]],
    constant SimConstants& constants      [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= (uint)constants.width || gid.y >= (uint)constants.height) return;
    bool isBorder = gid.x == 0 || gid.x == (uint)constants.width - 1
                 || gid.y == 0 || gid.y == (uint)constants.height - 1;
    solids.write(uint4(isBorder ? 1 : 0), gid);
}

kernel void clear_textures(
    texture2d<float, access::write> velX     [[texture(0)]],
    texture2d<float, access::write> velXTemp [[texture(1)]],
    texture2d<float, access::write> velY     [[texture(2)]],
    texture2d<float, access::write> velYTemp [[texture(3)]],
    texture2d<float, access::write> pressure [[texture(4)]],
    texture2d<float, access::write> smoke    [[texture(5)]],
    texture2d<float, access::write> smokeTemp[[texture(6)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x < velX.get_width() && gid.y < velX.get_height()) velX.write(float4(0), gid);
    if (gid.x < velXTemp.get_width() && gid.y < velXTemp.get_height()) velXTemp.write(float4(0), gid);
    if (gid.x < velY.get_width() && gid.y < velY.get_height()) velY.write(float4(0), gid);
    if (gid.x < velYTemp.get_width() && gid.y < velYTemp.get_height()) velYTemp.write(float4(0), gid);
    if (gid.x < pressure.get_width() && gid.y < pressure.get_height()) pressure.write(float4(0), gid);
    if (gid.x < smoke.get_width() && gid.y < smoke.get_height()) smoke.write(float4(0), gid);
    if (gid.x < smokeTemp.get_width() && gid.y < smokeTemp.get_height()) smokeTemp.write(float4(0), gid);
}

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

// -----------------------
// ------ UV HELPERS -----
// -----------------------

// returns if a gid is inbounds
bool in_bounds(uint2 gid, constant SimConstants& constants) {
	return (gid.x < (uint)constants.width && gid.y < (uint)constants.height);
}

bool in_bounds_x1(uint2 gid, constant SimConstants& constants) {
    return (gid.x <= (uint)constants.width && gid.y < (uint)constants.height);
}

bool in_bounds_y1(uint2 gid, constant SimConstants& constants) {
    return (gid.x < (uint)constants.width && gid.y <= (uint)constants.height);
}

// returns the uv coords [0,1] for any given gid
float2 get_uv(uint2 gid, constant SimConstants& constants) {
	return float2(gid.x / (float)constants.width, gid.y / (float)constants.height);
}

// corrects UV coordinates for stretch/squash
float2 c_uv(float2 uv, constant SimConstants& constants) {
	return float2(uv.x * constants.width / constants.height, uv.y);
}

// x range is [0, aspect] y range is [0, 1]
float2 get_uv_c(uint2 gid, constant SimConstants& constants) {
	return c_uv(get_uv(gid, constants), constants);
}

kernel void inject_velocity(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velY     [[texture(1)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (!frame.mouse.leftDown) return;

    float2 mouse_uv_c = c_uv(frame.mouse.pos, constants);
    float2 delta = frame.mouse.delta;

    // Update velX (grid size: width+1 x height)
    // velX[i, j] is at (i, j+0.5)
    if (in_bounds_x1(gid, constants)) {
        float2 uv = float2(
        	gid.x / (float)constants.width,
        	(gid.y + 0.5f) / (float)constants.height
        );
        if (distance(c_uv(uv, constants), mouse_uv_c) <= constants.mouseRadius) {
            float v = velX.read(gid).r + delta.x * constants.velocityStrength;
            velX.write(float4(v, 0, 0, 0), gid);
        }
    }

    // Update velY (grid size: width x height+1)
    // velY[i, j] is at (i+0.5, j)
    if (in_bounds_y1(gid, constants)) {
        float2 uv = float2(
        	(gid.x + 0.5f) / (float)constants.width,
         	gid.y / (float)constants.height
        );
        if (distance(c_uv(uv, constants), mouse_uv_c) <= constants.mouseRadius) {
            float v = velY.read(gid).r + delta.y * constants.velocityStrength;
            velY.write(float4(v, 0, 0, 0), gid);
        }
    }
}

kernel void inject_smoke(
    texture2d<float, access::read_write> smoke  [[texture(0)]],
    constant SimConstants& constants [[buffer(0)]],
    constant FrameData&    frame     [[buffer(1)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (!frame.mouse.rightDown) return;
    if (!in_bounds(gid, constants)) return;

    float2 uv_c = get_uv_c(gid, constants);
    float2 mouse_uv_c = c_uv(frame.mouse.pos, constants);
    if (distance(uv_c, mouse_uv_c) <= constants.mouseRadius) {
    	smoke.write(float4(1), gid);
    }
}

kernel void advect_velX(
    texture2d<float, access::read>  velX      [[texture(0)]],
    texture2d<float, access::read>  velY      [[texture(1)]],
    texture2d<uint,  access::read>  solids    [[texture(2)]],
    texture2d<float, access::write> velXTemp  [[texture(3)]],
    constant SimConstants& constants          [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void advect_velY(
    texture2d<float, access::read>  velX      [[texture(0)]],
    texture2d<float, access::read>  velY      [[texture(1)]],
    texture2d<uint,  access::read>  solids    [[texture(2)]],
    texture2d<float, access::write> velYTemp  [[texture(3)]],
    constant SimConstants& constants          [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void advect_smoke(
    texture2d<float, access::read>  velX      [[texture(0)]],
    texture2d<float, access::read>  velY      [[texture(1)]],
    texture2d<float, access::read>  smoke     [[texture(2)]],
    texture2d<uint,  access::read>  solids    [[texture(3)]],
    texture2d<float, access::write> smokeTemp [[texture(4)]],
    constant SimConstants& constants          [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
	// placeholder: just copy smoke into smokeTemp so swap doesn't lose data
    smokeTemp.write(smoke.read(gid), gid);
}

kernel void gs_red(
    texture2d<float, access::read_write> pressure [[texture(0)]],
    texture2d<float, access::read>       velX     [[texture(1)]],
    texture2d<float, access::read>       velY     [[texture(2)]],
    texture2d<uint,  access::read>       solids   [[texture(3)]],
    constant SimConstants& constants [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void gs_black(
    texture2d<float, access::read_write> pressure [[texture(0)]],
    texture2d<float, access::read>       velX     [[texture(1)]],
    texture2d<float, access::read>       velY     [[texture(2)]],
    texture2d<uint,  access::read>       solids   [[texture(3)]],
    constant SimConstants& constants [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void update_velocities(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velY     [[texture(1)]],
    texture2d<float, access::read>       pressure [[texture(2)]],
    texture2d<uint,  access::read>       solids   [[texture(3)]],
    constant SimConstants& constants [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{}

kernel void init_solids(
    texture2d<uint, access::write> solids [[texture(0)]],
    constant SimConstants& constants      [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (!in_bounds(gid, constants)) return;
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
    constant SimConstants& constants      [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (in_bounds_x1(gid, constants)) {
    	velX.write(float4(0), gid);
     	velXTemp.write(float4(0), gid);
    }
    if (in_bounds_y1(gid, constants)) {
    	velY.write(float4(0), gid);
     	velYTemp.write(float4(0), gid);
    }
    if (in_bounds(gid, constants)) {
    	pressure.write(float4(0), gid);
    	smoke.write(float4(0), gid);
     	smokeTemp.write(float4(0), gid);
    }
}

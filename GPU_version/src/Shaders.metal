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

fragment float4 fragment_smoke(
	VertexOut in [[stage_in]],
	texture2d<float> texture [[texture(0)]])
{
	constexpr sampler s(filter::linear);
    return texture.sample(s, in.uv);
}

fragment float4 fragment_divergence(
    VertexOut in [[stage_in]],
    texture2d<float> texture       [[texture(0)]],
    texture2d<uint>  solids        [[texture(1)]],
    constant SimConstants& constants [[buffer(0)]])
{
    constexpr sampler s(filter::linear);
    float4 sampled = texture.sample(s, in.uv);

    uint2 pixelCoord = uint2(in.uv * float2(constants.width, constants.height));
    uint isSolid = solids.read(pixelCoord).r;

    if (isSolid == 1) return float4(float3(0.05), 1.0);

    float div = sampled.r;
    float divergenceColorRange = 0.4f;
    float t = min(abs(div) / divergenceColorRange, 1.0f);
    float4 bg = float4(float3(0.12), 1.0);
    float4 target = (div < 0) ? float4(0.96, 0.26, 0.26, 1.0)
                               : float4(0.26, 0.53, 0.96, 1.0);
    return mix(bg, target, t);
}

// -----------------------
// ------ UV HELPERS -----
// -----------------------

constexpr sampler linearSampler(filter::linear, address::clamp_to_edge);
uint2 gxp1(uint2 gid) {
	return uint2(gid.x + 1, gid.y);
}

uint2 gyp1(uint2 gid) {
	return uint2(gid.x, gid.y + 1);
}

// returns if a gid is inbounds
bool in_bounds(uint2 gid, constant SimConstants& constants) {
	return (gid.x < (uint)constants.width && gid.y < (uint)constants.height && gid.x >= 0 && gid.y >= 0);
}

bool in_bounds_x1(uint2 gid, constant SimConstants& constants) {
	return (gid.x <= (uint)constants.width && gid.y < (uint)constants.height && gid.x >= 0 && gid.y >= 0);

}

bool in_bounds_y1(uint2 gid, constant SimConstants& constants) {
	return (gid.x < (uint)constants.width && gid.y <= (uint)constants.height && gid.x >= 0 && gid.y >= 0);
}

// returns the uv coords [0,1] for any given gid
float2 get_uv(uint2 gid, constant SimConstants& constants) {
	return float2(gid.x / (float)constants.width, gid.y / (float)constants.height);
}

// corrects UV coordinates for stretch/squash
float2 c_uv(float2 uv, constant SimConstants& constants) {
	return float2(uv.x * (float)constants.width / (float)constants.height, uv.y);
}

// x range is [0, aspect] y range is [0, 1]
float2 get_uv_c(uint2 gid, constant SimConstants& constants) {
	return c_uv(get_uv(gid, constants), constants);
}

// helpers for Gauss-Seidel stuff
bool is_solid(
	uint x,
	uint y,
	texture2d<uint, access::read>  solids,
	constant SimConstants&         constants)
{
	if (!in_bounds(uint2(x, y), constants)) return true;
	return solids.read(uint2(x, y)).x == 1;
}

float get_pressure(
	uint x,
	uint y,
	texture2d<float, access::read_write>  pressure,
	constant SimConstants&               constants)
{
	if (!in_bounds(uint2(x, y), constants)) return 0.f;
	return pressure.read(uint2(x, y)).x;
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
    texture2d<float, access::sample> velX      [[texture(0)]],
    texture2d<float, access::sample> velY      [[texture(1)]],
    texture2d<uint,  access::read>   solids    [[texture(2)]],
    texture2d<float, access::write>  velXTemp  [[texture(3)]],
    constant SimConstants& constants          [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (!in_bounds_x1(gid, constants)) return;

    float2 uv = float2(float(gid.x), float(gid.y) + 0.5f) / float2(constants.width + 1.f, constants.height);

    float u = velX.sample(linearSampler, uv).r;
    float v = velY.sample(linearSampler, uv).r;
    float2 vel = float2(u, v);

    float2 dt_uv = constants.deltaTime * vel / (float2(constants.width, constants.height) * constants.cellSize);
    float2 prev_uv = uv - dt_uv;

    float val = velX.sample(linearSampler, prev_uv).r;
    velXTemp.write(float4(val, 0, 0, 0), gid);
}

kernel void advect_velY(
    texture2d<float, access::sample> velX      [[texture(0)]],
    texture2d<float, access::sample> velY      [[texture(1)]],
    texture2d<uint,  access::read>   solids    [[texture(2)]],
    texture2d<float, access::write>  velYTemp  [[texture(3)]],
    constant SimConstants& constants          [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (!in_bounds_y1(gid, constants)) return;

    // velY[i,j] is at (i+0.5, j)
    float2 uv = float2(float(gid.x) + 0.5f, float(gid.y)) / float2(constants.width, constants.height + 1.f);

    float u = velX.sample(linearSampler, uv).r;
    float v = velY.sample(linearSampler, uv).r;
    float2 vel = float2(u, v);

    float2 dt_uv = constants.deltaTime * vel / (float2(constants.width, constants.height) * constants.cellSize);
    float2 prev_uv = uv - dt_uv;

    float val = velY.sample(linearSampler, prev_uv).r;
    velYTemp.write(float4(val, 0, 0, 0), gid);
}

kernel void advect_smoke(
    texture2d<float, access::sample> velX      [[texture(0)]],
    texture2d<float, access::sample> velY      [[texture(1)]],
    texture2d<float, access::sample> smoke     [[texture(2)]],
    texture2d<uint,  access::read>   solids    [[texture(3)]],
    texture2d<float, access::write>  smokeTemp [[texture(4)]],
    constant SimConstants& constants          [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (!in_bounds(gid, constants)) return;
    if (is_solid(gid.x, gid.y, solids, constants)) {
        smokeTemp.write(float4(0, 0, 0, 1), gid);
        return;
    }
    float2 uv = (float2(gid) + 0.5f) / float2(constants.width, constants.height);

    float u = velX.sample(linearSampler, uv).r;
    float v = velY.sample(linearSampler, uv).r;
    float2 vel = float2(u, v);

    float2 dt_uv = constants.deltaTime * vel / (float2(constants.width, constants.height) * constants.cellSize);
    float2 prev_uv = uv - dt_uv;

    float4 s = smoke.sample(linearSampler, prev_uv);
    smokeTemp.write(s, gid);
}

void solve_pressure(
	texture2d<float, access::read_write> pressure,
    texture2d<float, access::read>       velX,
    texture2d<float, access::read>       velY,
    texture2d<uint,  access::read>       solids,
    constant SimConstants& constants,
    uint2 gid)
{
	float p_new = 0.f;
	if (!is_solid(gid.x, gid.y, solids, constants)) {
		int flow_t = !is_solid(gid.x, gid.y + 1, solids, constants);
		int flow_b = !is_solid(gid.x, gid.y - 1, solids, constants);
		int flow_l = !is_solid(gid.x - 1, gid.y, solids, constants);
		int flow_r = !is_solid(gid.x + 1, gid.y, solids, constants);
		int n = flow_t + flow_b + flow_l + flow_r;
		if (n != 0) {
			float del_velocity_sum = (velX.read(gxp1(gid)) - velX.read(gid) + velY.read(gyp1(gid)) - velY.read(gid)).x;
			float p_sum = (flow_r ? get_pressure(gid.x + 1, gid.y, pressure, constants) : 0) +
						  (flow_l ? get_pressure(gid.x - 1, gid.y, pressure, constants) : 0) +
						  (flow_t ? get_pressure(gid.x, gid.y + 1, pressure, constants) : 0) +
						  (flow_b ? get_pressure(gid.x, gid.y - 1, pressure, constants) : 0);
			p_new = (p_sum - constants.fluidDensity * constants.cellSize * del_velocity_sum / constants.deltaTime) / n;
		}
	}
	float p_old = pressure.read(gid).x;
	pressure.write(p_old + (p_new - p_old) * constants.weightSOR, gid);
}

kernel void gs_red(
    texture2d<float, access::read_write> pressure [[texture(0)]],
    texture2d<float, access::read>       velX     [[texture(1)]],
    texture2d<float, access::read>       velY     [[texture(2)]],
    texture2d<uint,  access::read>       solids   [[texture(3)]],
    constant SimConstants& constants [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
	if (!in_bounds(gid, constants)) return;
	if ((gid.x + gid.y) % 2 == 0) return;
	solve_pressure(pressure, velX, velY, solids, constants, gid);
}

kernel void gs_black(
    texture2d<float, access::read_write> pressure [[texture(0)]],
    texture2d<float, access::read>       velX     [[texture(1)]],
    texture2d<float, access::read>       velY     [[texture(2)]],
    texture2d<uint,  access::read>       solids   [[texture(3)]],
    constant SimConstants& constants [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
	if (!in_bounds(gid, constants)) return;
	if ((gid.x + gid.y) % 2 == 1) return;
	solve_pressure(pressure, velX, velY, solids, constants, gid);
}

kernel void update_velocities(
    texture2d<float, access::read_write> velX     [[texture(0)]],
    texture2d<float, access::read_write> velY     [[texture(1)]],
    texture2d<float, access::read>       pressure [[texture(2)]],
    texture2d<uint,  access::read>       solids   [[texture(3)]],
    constant SimConstants& constants [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    // Update velX (grid size: width+1 x height)
    if (in_bounds_x1(gid, constants)) {
        if (is_solid(gid.x, gid.y, solids, constants) || is_solid(gid.x - 1, gid.y, solids, constants)) {
            velX.write(float4(0), gid);
        } else {
            float p_r = pressure.read(gid).x;
            float p_l = pressure.read(uint2(gid.x - 1, gid.y)).x;
            float v = velX.read(gid).r - constants.deltaTime / (constants.fluidDensity * constants.cellSize) * (p_r - p_l);
            velX.write(float4(v), gid);
        }
    }

    // Update velY (grid size: width x height+1)
    if (in_bounds_y1(gid, constants)) {
        if (is_solid(gid.x, gid.y, solids, constants) || is_solid(gid.x, gid.y - 1, solids, constants)) {
            velY.write(float4(0), gid);
        } else {
            float p_t = pressure.read(gid).x;
            float p_b = pressure.read(uint2(gid.x, gid.y - 1)).x;
            float v = velY.read(gid).r - constants.deltaTime / (constants.fluidDensity * constants.cellSize) * (p_t - p_b);
            velY.write(float4(v), gid);
        }
    }
}

kernel void update_divergence(
	texture2d<float, access::read>  velX       [[texture(0)]],
    texture2d<float, access::read>  velY       [[texture(1)]],
    texture2d<float, access::write> divergence [[texture(2)]],
    texture2d<uint,  access::read>    solids   [[texture(3)]],
    constant SimConstants& constants [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
	if (!in_bounds(gid, constants)) return;
	if (solids.read(gid).r == 1) {
		divergence.write(float4(0), gid);
		return;
	}
	float du = velX.read(gxp1(gid)).r - velX.read(gid).r;
	float dv = velY.read(gyp1(gid)).r - velY.read(gid).r;
	float div = (du + dv) / constants.cellSize;
	divergence.write(float4(div), gid);
}

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
    texture2d<float, access::write> velX       [[texture(0)]],
    texture2d<float, access::write> velXTemp   [[texture(1)]],
    texture2d<float, access::write> velY       [[texture(2)]],
    texture2d<float, access::write> velYTemp   [[texture(3)]],
    texture2d<float, access::write> pressure   [[texture(4)]],
    texture2d<float, access::write> smoke      [[texture(5)]],
    texture2d<float, access::write> smokeTemp  [[texture(6)]],
    texture2d<float, access::write> divergence [[texture(7)]],
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
      	divergence.write(float4(0), gid);
    }
}

#include <metal_stdlib>
#import "../include/SimParams.h"
using namespace metal;

kernel void compute_test(
	texture2d<float, access::write> output [[texture(0)]],
	constant SimConstants &simConstants [[buffer(0)]],
	constant FrameData &frameData [[buffer(1)]],
	uint2 gid [[thread_position_in_grid]]) {
	// gid ranges from 0 to grid size, defined in Renderer.mm

	// width and height of texture
	uint width = output.get_width();
	uint height = output.get_height();
	if (gid.x >= width || gid.y >= height) return;

	// [0, 1] (not NDC)
	float u = (float) gid.x / (float) width;
	float v = (float) gid.y / (float) height;

	float aspect = (float)width / (float)height;
	float2 uv_corrected = float2(u * aspect, v);
	float2 mouse_corrected = float2(frameData.mouse.pos.x * aspect, frameData.mouse.pos.y);

	float3 color = float3(u, v, 1.f);
	if (distance(uv_corrected, mouse_corrected) < simConstants.mouseRadius) {
		if (frameData.mouse.leftDown || frameData.mouse.rightDown) {
			color = float3(1.0, 1.0, 0.0);
		} else {
			color = float3(u, v, 0.f);
		}
	}
	output.write(float4(color, 1.0), gid);
}

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

fragment float4 fragment_main(VertexOut in [[stage_in]], texture2d<float> texture [[texture(0)]]) {
	constexpr sampler s(filter::linear);
    return texture.sample(s, in.uv);
}

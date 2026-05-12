#include <metal_stdlib>
using namespace metal;

kernel void compute_test(
	texture2d<float, access::write> output [[texture(0)]],
	uint2 gid [[thread_position_in_grid]]) {
	uint width = output.get_width();
	uint height = output.get_height();
	if (gid.x >= width || gid.y >= height) return;
	float r = (float) gid.x / (float) width;
	float g = (float) gid.y / (float) height;
	output.write(float4(r, g, 0.0, 1.0), gid);
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

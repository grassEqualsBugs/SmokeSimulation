#import "../include/Renderer.hpp"
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

// Actual implementation of renderer
@implementation Renderer {
	id<MTLDevice> _device;
	id<MTLCommandQueue> _commandQueue;
	id<MTLComputePipelineState> _computePipeline;
	id<MTLRenderPipelineState> _renderPipeline;
	id<MTLTexture> _texture;
}

// Constructor (initialization of renderer)
- (instancetype)initWithMetalKitView:(MTKView *)view {
  	self = [super init];
    if (!self) return nil;

    _device = view.device; // get access to the GPU
    _commandQueue = [_device newCommandQueue]; // "conveyer belt" for command buffers per frame

    // load the compiled shaders into the library of shaders
    id<MTLLibrary> library = [_device newDefaultLibrary];
    if (library == nil) {
    	NSLog(@"Failed to find default library.");
        return nil;
    }

    // ---- set up compute pipeline ----
    id<MTLFunction> kernelFn = [library newFunctionWithName:@"compute_test"];
    NSError *error = nil;
    _computePipeline = [_device newComputePipelineStateWithFunction: kernelFn error: &error];
    if (error) NSLog(@"Compute pipeline error: %@", error);

    // ---- set up rendering pipeline ----
    id<MTLFunction> vertexFn = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFn = [library newFunctionWithName:@"fragment_main"];
    MTLRenderPipelineDescriptor *pipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipeDesc.vertexFunction = vertexFn;
    pipeDesc.fragmentFunction = fragmentFn;
    pipeDesc.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    _renderPipeline = [_device newRenderPipelineStateWithDescriptor:pipeDesc error:&error];

    // allocate texture at initialization to attribute of Renderer, to be used later in drawInMTKView
    // data is all on gpu side, this is just to get the initial empty arrays over to gpu
    MTLTextureDescriptor *td = [[MTLTextureDescriptor alloc] init];
    td.textureType = MTLTextureType2D;
    td.pixelFormat = MTLPixelFormatRGBA32Float;
    td.width = 1600; td.height = 900;
    td.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
    td.storageMode = MTLStorageModePrivate; // Store texture only on GPU. fastest
    _texture = [_device newTextureWithDescriptor:td];

    return self;
}

// drawInMTKView method -- render loop for the program, called every frame
- (void)drawInMTKView:(MTKView *)view {
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer]; // list of commands to the gpu

    // ---- compute pass ----
    id<MTLComputeCommandEncoder> comp = [commandBuffer computeCommandEncoder];
    [comp setComputePipelineState:_computePipeline];
    [comp setTexture:_texture atIndex:0]; // index 0 matches index 0 in Shaders.metal
    // launch one thread per pixel
    MTLSize gridSize = MTLSizeMake(1600, 900, 1);
    MTLSize threadgroupSize = MTLSizeMake(16, 16, 1); // idfk what these numbers are
    [comp dispatchThreads:gridSize threadsPerThreadgroup:threadgroupSize];
    [comp endEncoding];

    // ---- render pass ----
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor; // what pass i'm rendering to
    if (renderPassDescriptor != nil) {
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        [renderEncoder setRenderPipelineState:_renderPipeline];
        [renderEncoder setFragmentTexture:_texture atIndex:0]; // index 0 again matches index 0 in Shaders.metal
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
        [renderEncoder endEncoding];
        // tell metal to display this frame's texture on screen when GPU finishes
        [commandBuffer presentDrawable:view.currentDrawable];
    }

    [commandBuffer commit]; // send command buffer to metal
    // GPU is now working async, CPU has done its job
}

// called when the window size changes. empty for now
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size { // (TODO): fill this in
}

@end

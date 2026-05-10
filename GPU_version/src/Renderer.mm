#import "../include/Renderer.hpp"
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

// Actual implementation of renderer
@implementation Renderer {
  id<MTLDevice> _device;
  id<MTLCommandQueue> _commandQueue;
  id<MTLRenderPipelineState> _pipelineState;
}

// Constructor
- (instancetype)initWithMetalKitView:(MTKView *)view {
    self = [super init];
    if (self) {
        _device = view.device;
        _commandQueue = [_device newCommandQueue];
    }

    // ----- Shaders for full screen quad ----
    // load the compiled shaders
    id<MTLLibrary> library = [_device newDefaultLibrary];
    id<MTLFunction> vertexFn   = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFn = [library newFunctionWithName:@"fragment_main"];

    // describe the pipeline
    MTLRenderPipelineDescriptor *pipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipeDesc.vertexFunction   = vertexFn;
    pipeDesc.fragmentFunction = fragmentFn;
    pipeDesc.colorAttachments[0].pixelFormat = view.colorPixelFormat;

    // compile it into a pipeline state object
    NSError *error = nil;
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipeDesc error:&error];
    return self;
}

// drawInMTKView method -- render loop for the program, called every frame
- (void)drawInMTKView:(MTKView *)view {
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer]; // list of commands to the gpu
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor; // what pass i'm rendering to

    if (renderPassDescriptor != nil) {
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        [renderEncoder setRenderPipelineState:_pipelineState];
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
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

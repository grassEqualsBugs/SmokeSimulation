#import "../include/Renderer.hpp"
#import <AppKit/AppKit.h>
#import "../include/SimParams.h"
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <simd/simd.h>

@implementation SimView
- (BOOL)acceptsFirstResponder { return YES; }

- (void)mouseDown:(NSEvent *)e         { [(id<MouseHandler>)self.delegate mouseDown:e]; }
- (void)mouseUp:(NSEvent *)e           { [(id<MouseHandler>)self.delegate mouseUp:e]; }
- (void)mouseMoved:(NSEvent *)e        { [(id<MouseHandler>)self.delegate mouseMoved:e]; }
- (void)mouseDragged:(NSEvent *)e      { [(id<MouseHandler>)self.delegate mouseDragged:e]; }
- (void)rightMouseDown:(NSEvent *)e    { [(id<MouseHandler>)self.delegate rightMouseDown:e]; }
- (void)rightMouseUp:(NSEvent *)e      { [(id<MouseHandler>)self.delegate rightMouseUp:e]; }
- (void)rightMouseDragged:(NSEvent *)e { [(id<MouseHandler>)self.delegate rightMouseDragged:e]; }
@end

// Actual implementation of renderer
@implementation Renderer {
    // basic stuff
	id<MTLDevice> _device;
	int _width;
	int _height;

    // setting up pipelines and such for sending commands to GPU
	id<MTLCommandQueue> _commandQueue;
	id<MTLComputePipelineState> _computePipeline;
	id<MTLRenderPipelineState> _renderPipeline;

    // data to be sent to GPU
	id<MTLTexture> _texture;
	id<MTLBuffer> _simConstantsBuffer;
    id<MTLBuffer> _frameDataBuffer;

	// CPU records of these, to be updated every frame
	simd_float2 _mousePos;
    bool _leftDown;
    bool _rightDown;
}

// Constructor (initialization of renderer)
- (instancetype)initWithMetalKitView:(MTKView *)view {
  	self = [super init];
    if (!self) return nil;

    _width = 1600;
    _height = 900;

    _device = view.device; // get access to the GPU
    _commandQueue = [_device newCommandQueue]; // "conveyer belt" for command buffers per frame

    // setting up data for sim parameters/constants and per-frame data
    SimConstants simConstants;
    simConstants.cellSize = 1.f / fmin(_width, _height);
    simConstants.deltaTime = 1.f / 30.f;
    simConstants.fluidDensity = 1.f;
    simConstants.width = _width;
    simConstants.height = _height;
    simConstants.mouseRadius = 0.08;
    _simConstantsBuffer = [_device newBufferWithBytes:&simConstants
                                            length:sizeof(SimConstants)
                                            options:MTLResourceStorageModeShared];
    _frameDataBuffer = [_device newBufferWithLength:sizeof(FrameData)
                                options:MTLResourceStorageModeShared];

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
    td.width = _width; td.height = _height;
    td.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
    td.storageMode = MTLStorageModePrivate; // Store texture only on GPU. fastest
    _texture = [_device newTextureWithDescriptor:td];

    return self;
}

- (void)updateMousePos:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    _mousePos = simd_make_float2(p.x / _width, p.y / _height);
}

- (void)mouseMoved:(NSEvent *)event {
    [self updateMousePos:event];
}
- (void)mouseDragged:(NSEvent *)event {
    [self updateMousePos:event];
}
- (void)rightMouseDragged:(NSEvent *)event {
    [self updateMousePos:event];
}
- (void)mouseDown:(NSEvent *)event {
    _leftDown = true;
    [self updateMousePos:event];
}
- (void)mouseUp:(NSEvent *)event {
    _leftDown = false;
    [self updateMousePos:event];
}
- (void)rightMouseDown:(NSEvent *)event {
    _rightDown = true;
    [self updateMousePos:event];
}
- (void)rightMouseUp:(NSEvent *)event {
    _rightDown = false;
    [self updateMousePos:event];
}

// drawInMTKView method -- render loop for the program, called every frame
- (void)drawInMTKView:(MTKView *)view {
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer]; // list of commands to the gpu

    // all passing of live per-frame data to GPU (FrameData struct)
    FrameData *frameData = (FrameData *)_frameDataBuffer.contents;
    frameData->mouse.pos = _mousePos;
    frameData->mouse.leftDown = _leftDown;
    frameData->mouse.rightDown = _rightDown;

    // ---- compute pass ----
    id<MTLComputeCommandEncoder> comp = [commandBuffer computeCommandEncoder];
    [comp setComputePipelineState:_computePipeline];
    [comp setTexture:_texture atIndex:0];
    [comp setBuffer:_simConstantsBuffer offset:0 atIndex:0];
    [comp setBuffer:_frameDataBuffer offset:0 atIndex:1];
    // launch one thread per pixel
    MTLSize gridSize = MTLSizeMake(1600, 900, 1);
    MTLSize threadgroupSize = MTLSizeMake(16, 16, 1); // 16 x 16 is standard, but 8 x 8 and 32 x 32 also works
    [comp dispatchThreads:gridSize threadsPerThreadgroup:threadgroupSize];
    [comp endEncoding];

    // ---- render pass ----
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor; // what pass i'm rendering to
    if (renderPassDescriptor != nil) {
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        [renderEncoder setRenderPipelineState:_renderPipeline];
        [renderEncoder setFragmentTexture:_texture atIndex:0];
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

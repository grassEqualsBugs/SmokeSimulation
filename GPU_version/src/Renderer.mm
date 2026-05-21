#import "../include/Renderer.hpp"
#import <AppKit/AppKit.h>
#import "../include/SimParams.h"
#import "../include/FluidSim.hpp"
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
    id<MTLBuffer> _frameDataBuffer;
   	simd_float2   _mousePos;
    simd_float2   _lastMousePos;
    bool          _firstFrameMouse;
    bool          _leftDown;
    bool          _rightDown;

	id<MTLDevice> _device;
	int           _width;
	int           _height;

	id<MTLCommandQueue>         _commandQueue;
	id<MTLComputePipelineState> _computePipeline;
	id<MTLRenderPipelineState>  _renderPipeline;
	FluidSim*                   _sim;
}

// Constructor (initialization of renderer)
- (instancetype)initWithMetalKitView:(MTKView *)view {
  	self = [super init];
    if (!self) return nil;

    _width = 1600;
    _height = 900;
    float simScale = 4.0f;
    int simWidth = _width / simScale;
    int simHeight = _height / simScale;

    _firstFrameMouse = true;

    _device       = view.device; // get access to the GPU
    _commandQueue = [_device newCommandQueue]; // "conveyer belt" for command buffers per frame

    // load the compiled shaders into the library of shaders
    id<MTLLibrary> library = [_device newDefaultLibrary];
    if (library == nil) {
    	NSLog(@"Failed to find default library.");
        return nil;
    }

    _frameDataBuffer = [_device newBufferWithLength:sizeof(FrameData)
                                options:MTLResourceStorageModeShared];
    _sim = [[FluidSim alloc] initWithDevice:_device
                                     library:library
                                commandQueue:_commandQueue
                                       width:simWidth
                                      height:simHeight];

    // ---- set up rendering pipeline ----
    id<MTLFunction> vertexFn = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFn = [library newFunctionWithName:@"fragment_divergence"];
    MTLRenderPipelineDescriptor *pipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipeDesc.vertexFunction = vertexFn;
    pipeDesc.fragmentFunction = fragmentFn;
    pipeDesc.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    NSError *error = nil;
    _renderPipeline = [_device newRenderPipelineStateWithDescriptor:pipeDesc error:&error];

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
    if (_firstFrameMouse && _leftDown) {
        _lastMousePos = _mousePos;
        _firstFrameMouse = false;
    }

    // update per-frame data on CPU side
    FrameData *frameData = (FrameData *)_frameDataBuffer.contents;
    frameData->mouse.pos = _mousePos;
    frameData->mouse.delta = _mousePos - _lastMousePos;
    frameData->mouse.leftDown = _leftDown;
    frameData->mouse.rightDown = _rightDown;

    _lastMousePos = _mousePos;

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer]; // list of commands to the gpu

    // ---- compute pass, all sim steps ----
    id<MTLComputeCommandEncoder> comp = [commandBuffer computeCommandEncoder];
    [_sim encodeSimStep:comp frameData:_frameDataBuffer];
    [comp endEncoding];

    // ---- render pass ----
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor; // what pass i'm rendering to
    if (renderPassDescriptor != nil) {
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        [renderEncoder setRenderPipelineState:_renderPipeline];

        [renderEncoder setFragmentTexture:[_sim divergenceTexture] atIndex:0];
        [renderEncoder setFragmentTexture:[_sim solidsTexture] atIndex:1];
        [renderEncoder setFragmentBuffer:_sim.simConstantsBuffer
                                  offset:0
                                 atIndex:0];

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

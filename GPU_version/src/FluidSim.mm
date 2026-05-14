#import "../include/FluidSim.hpp"
#import "../include/SimParams.h"
#import <Metal/Metal.h>

@implementation FluidSim {
    id<MTLDevice> _device;

    // sim textures
    id<MTLTexture> _velX;
    id<MTLTexture> _velXTemp;
    id<MTLTexture> _velY;
    id<MTLTexture> _velYTemp;
    id<MTLTexture> _pressure;
    id<MTLTexture> _smoke; // smokeMap analog from CPU version
    id<MTLTexture> _smokeTemp;
    id<MTLTexture> _solids;

    // compute pipelines (one needed for each kernel)
    id<MTLComputePipelineState> _injectSmokePipeline; // pipeline for drawing smoke
    id<MTLComputePipelineState> _injectVelocityPipeline; // pipeline for changing velocity
    id<MTLComputePipelineState> _advectVelXPipeline;
    id<MTLComputePipelineState> _advectVelYPipeline;
    id<MTLComputePipelineState> _advectSmokePipeline;
    id<MTLComputePipelineState> _clearTexturesPipeline;
    id<MTLComputePipelineState> _initSolidsPipeline;
    // red black Gauss-Seidel
    id<MTLComputePipelineState> _gsRedPipeline;
    id<MTLComputePipelineState> _gsBlackPipeline;
    id<MTLComputePipelineState> _updateVelocitiesPipeline;

    id<MTLBuffer> _simConstantsBuffer;
    int _width;
    int _height;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device
                       library:(id<MTLLibrary>)library
                  commandQueue:(id<MTLCommandQueue>)commandQueue {
    self = [super init];
    if (!self) return nil;

    _device = device;
    _width  = 1600;
    _height = 900;

    [self allocateTextures];
    [self buildPipelines:library];
    [self buildConstantsBuffer];
    [self clearTextures:commandQueue];
    [self initializeSolids:commandQueue];

    return self;
}

- (id<MTLTexture>)makeTextureWidth:(int)w height:(int)h {
    MTLTextureDescriptor *td = [[MTLTextureDescriptor alloc] init];
    td.textureType = MTLTextureType2D;
    td.pixelFormat = MTLPixelFormatR32Float;
    td.width       = w;
    td.height      = h;
    td.usage       = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
    td.storageMode = MTLStorageModePrivate;
    return [_device newTextureWithDescriptor:td];
}

- (void)allocateTextures {
    _velX        = [self makeTextureWidth:_width + 1 height:_height];
    _velXTemp    = [self makeTextureWidth:_width + 1 height:_height];
    _velY        = [self makeTextureWidth:_width     height:_height + 1];
    _velYTemp    = [self makeTextureWidth:_width     height:_height + 1];
    _pressure    = [self makeTextureWidth:_width     height:_height];

    // smoke textures use RGBA32Float
    MTLTextureDescriptor *smokeDesc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                                           width:_width
                                                          height:_height
                                                       mipmapped:NO];
    smokeDesc.usage       = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
    smokeDesc.storageMode = MTLStorageModePrivate;
    _smoke     = [_device newTextureWithDescriptor:smokeDesc];
    _smokeTemp = [_device newTextureWithDescriptor:smokeDesc];

    // solids uses R8Uint
    MTLTextureDescriptor *td = [[MTLTextureDescriptor alloc] init];
    td.textureType = MTLTextureType2D;
    td.pixelFormat = MTLPixelFormatR8Uint;
    td.width       = _width;
    td.height      = _height;
    td.usage       = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
    td.storageMode = MTLStorageModePrivate;
    _solids = [_device newTextureWithDescriptor:td];
}

- (id<MTLComputePipelineState>)makePipeline:(id<MTLLibrary>)library name:(NSString *)name {
    NSError *error = nil;
    id<MTLFunction> fn = [library newFunctionWithName:name];
    if (!fn) { NSLog(@"Kernel not found: %@", name); return nil; }
    id<MTLComputePipelineState> pipeline = [_device newComputePipelineStateWithFunction:fn error:&error];
    if (error) NSLog(@"Pipeline error for %@: %@", name, error);
    return pipeline;
}

- (void)buildPipelines:(id<MTLLibrary>)library {
    _injectVelocityPipeline   = [self makePipeline:library name:@"inject_velocity"];
    _injectSmokePipeline      = [self makePipeline:library name:@"inject_smoke"];
    _advectVelXPipeline       = [self makePipeline:library name:@"advect_velX"];
    _advectVelYPipeline       = [self makePipeline:library name:@"advect_velY"];
    _advectSmokePipeline      = [self makePipeline:library name:@"advect_smoke"];
    _gsRedPipeline            = [self makePipeline:library name:@"gs_red"];
    _gsBlackPipeline          = [self makePipeline:library name:@"gs_black"];
    _updateVelocitiesPipeline = [self makePipeline:library name:@"update_velocities"];
    _clearTexturesPipeline    = [self makePipeline:library name:@"clear_textures"];
    _initSolidsPipeline       = [self makePipeline:library name:@"init_solids"];
}

- (void)buildConstantsBuffer {
    SimConstants c;
    c.cellSize     = 1.f / fmin(_width, _height);
    c.deltaTime    = 1.f / 30.f;
    c.fluidDensity = 1.f;
    c.width        = _width;
    c.height       = _height;
    c.mouseRadius  = 0.08f;
    _simConstantsBuffer = [_device newBufferWithBytes:&c
                                               length:sizeof(SimConstants)
                                              options:MTLResourceStorageModeShared];
}

// helper to avoid repeating texture/buffer binds every dispatch
- (void)dispatch:(id<MTLComputeCommandEncoder>)encoder
        pipeline:(id<MTLComputePipelineState>)pipeline
            grid:(MTLSize)grid
     threadgroup:(MTLSize)threadgroup
       frameData:(id<MTLBuffer>)frameData {
    [encoder setComputePipelineState:pipeline];
    [encoder setTexture:_velX        atIndex:0];
    [encoder setTexture:_velXTemp    atIndex:1];
    [encoder setTexture:_velY        atIndex:2];
    [encoder setTexture:_velYTemp    atIndex:3];
    [encoder setTexture:_pressure    atIndex:4];
    [encoder setTexture:_smoke       atIndex:5];
    [encoder setTexture:_smokeTemp   atIndex:6];
    [encoder setTexture:_solids      atIndex:7];
    [encoder setBuffer:_simConstantsBuffer offset:0 atIndex:0];
    [encoder setBuffer:frameData           offset:0 atIndex:1];
    [encoder dispatchThreads:grid threadsPerThreadgroup:threadgroup];
}

- (void)encodeSimStep:(id<MTLComputeCommandEncoder>)encoder
            frameData:(id<MTLBuffer>)frameData {
    MTLSize grid        = MTLSizeMake(_width, _height, 1);
    MTLSize threadgroup = MTLSizeMake(16, 16, 1);

    // inject (mouse input)
    [self dispatch:encoder pipeline:_injectVelocityPipeline grid:grid threadgroup:threadgroup frameData:frameData];
    [self dispatch:encoder pipeline:_injectSmokePipeline     grid:grid threadgroup:threadgroup frameData:frameData];

    // advect
    [self dispatch:encoder pipeline:_advectVelXPipeline    grid:MTLSizeMake(_width+1, _height,   1) threadgroup:threadgroup frameData:frameData];
    [self dispatch:encoder pipeline:_advectVelYPipeline    grid:MTLSizeMake(_width,   _height+1, 1) threadgroup:threadgroup frameData:frameData];
    [self dispatch:encoder pipeline:_advectSmokePipeline   grid:grid threadgroup:threadgroup frameData:frameData];

    // pressure solve
    for (int i = 0; i < 40; i++) { // (TODO): easy changing of iterations, hard coded at 40 right now...
        [self dispatch:encoder pipeline:_gsRedPipeline   grid:grid threadgroup:threadgroup frameData:frameData];
        [self dispatch:encoder pipeline:_gsBlackPipeline grid:grid threadgroup:threadgroup frameData:frameData];
    }

    // update velocities
    [self dispatch:encoder pipeline:_updateVelocitiesPipeline grid:grid threadgroup:threadgroup frameData:frameData];
}


- (void)initializeSolids:(id<MTLCommandQueue>)commandQueue {
    id<MTLCommandBuffer> cmd = [commandQueue commandBuffer];
    id<MTLComputeCommandEncoder> enc = [cmd computeCommandEncoder];
    [enc setComputePipelineState:_initSolidsPipeline];
    [enc setTexture:_solids atIndex:0];
    [enc setBuffer:_simConstantsBuffer offset:0 atIndex:0];

    MTLSize grid        = MTLSizeMake(_width, _height, 1);
    MTLSize threadgroup = MTLSizeMake(16, 16, 1);
    [enc dispatchThreads:grid threadsPerThreadgroup:threadgroup];

    [enc endEncoding];
    [cmd commit];
    [cmd waitUntilCompleted];
}

- (void)clearTextures:(id<MTLCommandQueue>)commandQueue {
    id<MTLCommandBuffer> cmd = [commandQueue commandBuffer];
    id<MTLComputeCommandEncoder> enc = [cmd computeCommandEncoder];
    [enc setComputePipelineState:_clearTexturesPipeline];
    [enc setTexture:_velX      atIndex:0];
    [enc setTexture:_velXTemp  atIndex:1];
    [enc setTexture:_velY      atIndex:2];
    [enc setTexture:_velYTemp  atIndex:3];
    [enc setTexture:_pressure  atIndex:4];
    [enc setTexture:_smoke     atIndex:5];
    [enc setTexture:_smokeTemp atIndex:6];
    MTLSize grid        = MTLSizeMake(_width + 1, _height + 1, 1); // +1 covers velX and velY dims
    MTLSize threadgroup = MTLSizeMake(16, 16, 1);
    [enc dispatchThreads:grid threadsPerThreadgroup:threadgroup];
    [enc endEncoding];
    [cmd commit];
    [cmd waitUntilCompleted]; // block until clear finishes before first frame
}

- (id<MTLTexture>)smokeTexture {
    return _smoke;
}

@end

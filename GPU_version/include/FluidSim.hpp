#pragma once

#ifdef __OBJC__
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@interface FluidSim : NSObject
- (instancetype)initWithDevice:(id<MTLDevice>)device library:(id<MTLLibrary>)library commandQueue:(id<MTLCommandQueue>)commandQueue;
- (void)encodeSimStep:(id<MTLComputeCommandEncoder>)encoder frameData:(id<MTLBuffer>)frameData;
- (id<MTLTexture>)smokeTexture;
- (id<MTLTexture>)divergenceTexture;
@end

#endif

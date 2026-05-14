#pragma once

#ifdef __OBJC__
#import <MetalKit/MetalKit.h>
@interface SimView : MTKView
@end
@interface Renderer : NSObject <MTKViewDelegate>
- (instancetype)initWithMetalKitView:(MTKView *)view;
@end
#endif

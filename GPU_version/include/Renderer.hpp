#pragma once

#ifdef __OBJC__
#import <MetalKit/MetalKit.h>

@protocol InputHandler <NSObject>
- (void)mouseDown:(NSEvent *)e;
- (void)mouseUp:(NSEvent *)e;
- (void)mouseMoved:(NSEvent *)e;
- (void)mouseDragged:(NSEvent *)e;
- (void)rightMouseDown:(NSEvent *)e;
- (void)rightMouseUp:(NSEvent *)e;
- (void)rightMouseDragged:(NSEvent *)e;
- (void)keyDown:(NSEvent *)e;
- (void)keyUp:(NSEvent *)e;
@end

@interface SimView : MTKView
@end

@interface Renderer : NSObject <MTKViewDelegate, InputHandler>
- (instancetype)initWithMetalKitView:(MTKView *)view;
@end
#endif

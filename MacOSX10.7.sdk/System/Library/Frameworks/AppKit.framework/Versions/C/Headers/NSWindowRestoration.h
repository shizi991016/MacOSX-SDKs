/*
	NSWindowRestoration.h
	Application Kit
	Copyright (c) 1994-2011, Apple Inc.
	All rights reserved.
 */

#import <AppKit/NSWindow.h>
#import <AppKit/NSDocument.h>
#import <AppKit/NSDocumentController.h>
#import <AppKit/NSApplication.h>


/* NSWindowRestoration is the mechanism by which Cocoa apps can persist their user interface state, such as window frames, and integrate with the machinery of the rest of the system. */


@protocol NSWindowRestoration <NSObject>
#if NS_BLOCKS_AVAILABLE
/* The following message is sent to request that a window be restored.  If the receiver knows how to restore the identified window, it should invoke the completion handler with the window, possibly creating it.  It is acceptable to invoke the completion handler with a pre-existing window, though you should not pass the same window to more than one completion handler.  If the receiver cannot restore the identified window (for example, the window referenced a document that has been deleted), it should invoke the completion handler with a nil window.  In Mac OS X 10.7, the error parameter is ignored.
 
 The receiver is passed the identifier of the window, which allows the receiver to quickly check for known windows.  For example, you might give your preferences window an identifier of "preferences" in the nib, and then check for that identifier in your implementation.  The receiver is also passed the NSCoder containing the combined restorable state of the window, its delegate, the window controller, and any document.  The receiver may decode information previously stored in the coder to determine what window to restore.
 
 Note that the receiver may invoke the completion handler before or after the method returns, and on any queue.  If you plan to invoke the completion handler after the method returns, you must copy the completion handler via the -copy method, and -release it after you invoke it.  It is not necessary or recommended for implementations of this method to order restored windows onscreen (for example, the window may have been minimized, in which case it will not be ordered onscreen).
 
 */
+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler NS_AVAILABLE_MAC(10_7);
#endif
@end

/* NSDocumentController implements the NSWindowRestoration protocol.  It is set as the restoration class for document windows.  You may subclass it and override the restoreWindowWithIdentifier:state:completionHandler: method to control how documents are restored. */
@interface NSDocumentController (NSWindowRestoration) <NSWindowRestoration>
@end

@interface NSApplication (NSWindowRestoration)

#if NS_BLOCKS_AVAILABLE
/* The following message is the funnel point for all window restoration.  The default behavior is to "demux:" decode the restoration class and send it the class method restoreWindowWithIdentifier:state:completionHandler: above.
 
 You should not call this method, but you may override it.  Generally it is not necessary to override this method: it is better to set a restoration class on each window and implement the class method above.  However, if some version of an app removes a class that is set as the restoration class of a window, you can override this method to detect that case and restore the window differently.  Thus this method can be used to "patch up" old persistent state across changes in the class hierarchy.
 
 Unlike the class-level counterpart, this method returns BOOL.  A YES return means that the window is recognized and that the completion handler will be called (or has already been called).  A NO return means that the window was not recognized and the completion handler will not be called.  If you override this and return YES, it is important that the completion handler be called, even if the window you pass to it is nil; otherwise the app will never finish restoring its state.
*/
- (BOOL)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler NS_AVAILABLE_MAC(10_7);
#endif

@end

/* NSApplicationDidFinishRestoringWindowsNotification is posted when the application is finished restoring windows, that is, when all the completion handlers from restoreWindowWithIdentifier:state:completionHandler: have been called.  This is always posted after NSApplicationWillFinishLaunching, but may be posted before or after NSApplicationDidFinishLaunching, depending on whether clients copy the completion handlers and invoke them later.  If there were no windows to restore, then this notification is still posted at the corresponding point in app launch (between NSApplicationWillFinishLaunchingNotification and NSApplicationDidFinishLaunchingNotification).  The object is NSApplication, and there is no user info.
 */
APPKIT_EXTERN NSString * const NSApplicationDidFinishRestoringWindowsNotification NS_AVAILABLE_MAC(10_7);


@interface NSWindow (NSUserInterfaceRestoration)

/* Determines whether the window should be restored on relaunch.  By default, windows with NSTitledWindowMask set in the styleMask are restorable, and windows without it set are not.
 */
- (void)setRestorable:(BOOL)flag NS_AVAILABLE_MAC(10_7);
- (BOOL)isRestorable NS_AVAILABLE_MAC(10_7);

/* Set and get the class that is responsible for restoring the window.  The default implementation of -[NSWindowController setDocument:] will set the restoration class of the window to the shared NSDocumentController's class.
 */
- (void)setRestorationClass:(Class <NSWindowRestoration>)restorationClass NS_AVAILABLE_MAC(10_7);
- (Class <NSWindowRestoration>)restorationClass NS_AVAILABLE_MAC(10_7);

/* Disable or enable snapshot restoration. While snapshot restoration is disabled, the window will not be snapshotted for restorable state. */
- (void)disableSnapshotRestoration;
- (void)enableSnapshotRestoration;

 
@end


@interface NSResponder (NSRestorableState)

/* Method called to save the restorable state.  The receiver is passed an NSCoder that supports keyed encoding (but not decoding), and should encode its restorable state. If you override this method, you should call through to super.  You should not otherwise invoke this method.  If you encode an object that implements the NSUserInterfaceItemIdentification protocol, the object itself is not archived; only its identifier is stored.  Thus, for example, a window may efficiently store its firstResponder as restorable state.
 */
- (void)encodeRestorableStateWithCoder:(NSCoder *)coder NS_AVAILABLE_MAC(10_7);

/* Method called to restore state.  The receiver is passed an NSCoder that supports keyed decoding (but not encoding).  The receiver should decode any previously stored state.  If you override this method, you should call through to super.  You should not otherwise invoke this method.
*/
- (void)restoreStateWithCoder:(NSCoder *)coder NS_AVAILABLE_MAC(10_7);


/* Method that you may call to indicate that the restorable state is invalid.  At some point in the future, encodeRestorableStateWithCoder: will be called to encode the restorable state.  You should not override this method.
*/
- (void)invalidateRestorableState NS_AVAILABLE_MAC(10_7);


/* Returns a set of key paths, representing paths of properties that should be persistent.  The frameworks will observe these key paths via KVO and automatically persist their values as part of the persistent state, and restore them on relaunch.  The values of the key paths should implement keyed archiving.  The base implementation returns an empty array.
*/
+ (NSArray *)restorableStateKeyPaths NS_AVAILABLE_MAC(10_7);

@end


@interface NSApplication (NSRestorableStateExtension)

/* These methods allow the app to extend the period of state restoration.  If the app crashes before state restoration is complete, then it may offer to discard restorable state on the next launch.  If a window has some state that may take a long time to restore, such as a web page, you may use the following methods to extend the period of this crash protection beyond the default.
 
 Typically you would call -extendStateRestoration within your implementation of +restoreWindowWithIdentifier:state:completionHandler:.  You would then call -completeStateRestoration some time after the window is fully restored (e.g. the web page has loaded).  If the app crashes in the interim, then it may offer to discard restorable state on the next launch.  
 
 These methods act as a counter. Each call to -extend increments the counter, and must be matched with a corresponding call to -complete which decrements it.  When the counter reaches zero, the app is considered to have been fully restored, and any further calls are silently ignored.
 
 These methods are thread safe.
 */
- (void)extendStateRestoration NS_AVAILABLE_MAC(10_7);
- (void)completeStateRestoration NS_AVAILABLE_MAC(10_7);

@end

@interface NSDocument (NSRestorableState)

/* The following method is called by the default implementation of +[NSDocumentController restoreWindowWithIdentifier:state:completionHandler:] to restore a window that was associated with a document, after that document has been reopened.
 
 The default implementation first checks if the document has window controllers, and if not, it calls -makeWindowControllers.  If there is then exactly one window controller, it invokes the completion handler with its window.  If there is more than one, it searches the receiver's window controllers for a window that matches the given identifier, and then calls the completionHandler with it.  If no window could be found, it invokes the completion handler with a nil window.

If your document has variable or optional windows, you may override this to create the requested window, and then call the completion handler with it.  This allows you to use the default document reopening behavior, but intervene at the point of creating the windows.  The parameters are the same as in the class method +restoreWindowWithIdentifier:state:completionHandler:. 
*/
- (void)restoreDocumentWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler NS_AVAILABLE_MAC(10_7);

/* NSDocument implements the NSRestorableState methods, even though it itself is not an NSResponder. */
- (void)encodeRestorableStateWithCoder:(NSCoder *)coder NS_AVAILABLE_MAC(10_7);
- (void)restoreStateWithCoder:(NSCoder *)coder NS_AVAILABLE_MAC(10_7);
- (void)invalidateRestorableState NS_AVAILABLE_MAC(10_7);
+ (NSArray *)restorableStateKeyPaths NS_AVAILABLE_MAC(10_7);

@end

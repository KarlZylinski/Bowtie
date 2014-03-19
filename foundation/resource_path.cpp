#include "resource_path.h"

#ifdef __APPLE__

#import <Foundation/Foundation.h>

const char* resource_path()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	static char rpath[512];
    NSBundle* bundle = [NSBundle mainBundle];

    if (bundle == nil) {
#ifdef DEBUG
        NSLog(@"bundle is nil... thus no resources path can be found.");
#endif
    } else {
        NSString* path = [bundle resourcePath];
		strcpy(rpath, [path UTF8String]);
		strcat(rpath, "/");
    }

    [pool drain];

    return rpath;
}

#else

const char* resource_path()
{
    return "";
}

#endif
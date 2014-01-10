#import <Cocoa/Cocoa.h>

void disablePowerNap()
{
  NSProcessInfo *info = [NSProcessInfo processInfo];
  if(![info respondsToSelector:@selector(beginActivityWithOptions:reason:)]) return;
  [info beginActivityWithOptions:NSActivityBackground reason:@"DOWNLOAD"];
}
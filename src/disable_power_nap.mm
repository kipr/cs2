#import <Cocoa/Cocoa.h>

void disablePowerNap()
{
  id info = [NSProcessInfo processInfo];
  if(![info respondsToSelector:@selector(beginActivityWithOptions:reason:)]) return;
  [info beginActivityWithOptions:0xFFULL reason:@"DOWNLOAD"];
}
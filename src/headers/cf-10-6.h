#ifndef _SRC_HEADERS_CF_10_6_
#define _SRC_HEADERS_CF_10_6_

typedef struct __CFRuntimeBase {
    uintptr_t _cfisa;
    uint8_t _cfinfo[4];
#if __LP64__
    uint32_t _rc;
#endif
} CFRuntimeBase;

typedef struct __CFRunLoopMode *CFRunLoopModeRef;

struct __CFRunLoopMode {
  CFRuntimeBase _base;
  int32_t _lock;
  CFStringRef _name;
  char _stopped;
  char _padding[3];
  void* _sources;
  void* _observers;
  void* _timers;
  void* _portToV1SourceMap;
  void* _submodes;
  mach_port_t _portSet;
  signed long _observerMask;
  int _kq;
};

struct __CFRunLoop {
  CFRuntimeBase _base;
  int32_t _lock;
  mach_port_t _wakeUpPort;
  volatile uint32_t *_stopped;
  pthread_t _pthread;
  CFMutableSetRef _commonModes;
  CFMutableSetRef _commonModeItems;
  CFRunLoopModeRef _currentMode;
  CFMutableSetRef _modes;
  void* _blocks_head;
  void* _blocks_tail;
  void* _counterpart;
};

#endif // _SRC_HEADERS_CF_10_6_

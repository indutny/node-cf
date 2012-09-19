#ifndef _SRC_CF_H_
#define _SRC_CF_H_

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <mach/task.h>
#include <mach/mach_init.h>
#include <mach/mach_port.h>

namespace CF {
#include <CoreFoundation/CoreFoundation.h>
}

namespace cf {

using namespace node;
using namespace v8;

class Loop : ObjectWrap {
 public:
  Loop(CF::CFRunLoopRef loop, CF::CFStringRef mode);
  ~Loop();
  void Close();

  static void Init(Handle<Object> target);

 protected:

  static Handle<Value> New(const Arguments& args);
  static Handle<Value> AddRef(const Arguments& args);
  static Handle<Value> RemRef(const Arguments& args);

  static void Worker(void* arg);
  static void Callback(uv_async_t* async, int status);
  static void OnClose(uv_handle_t* handle);

  int signal_[2];
  CF::CFRunLoopRef cf_lp_;
  CF::CFStringRef cf_mode_;
  mach_port_t main_;
  mach_port_t wakeup_;
  uv_async_t* cb_;
  uv_thread_t thread_;
  bool closed_;
};

} // namespace cf

#endif // _SRC_CF_H_

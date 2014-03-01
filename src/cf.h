#ifndef _SRC_CF_H_
#define _SRC_CF_H_

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <CoreFoundation/CoreFoundation.h>

namespace cf {

using namespace node;
using namespace v8;
using v8::Handle;

class Loop : ObjectWrap {
 public:
  Loop(CFRunLoopRef loop, CFStringRef mode);
  ~Loop();
  void Close();

  static void Init(Handle<Object> target);

  inline uv_loop_t* uv() const { return uv_; }

 protected:

  static Handle<Value> New(const Arguments& args);
  static Handle<Value> AddRef(const Arguments& args);
  static Handle<Value> RemRef(const Arguments& args);

  static void Worker(void* arg);
  static void Perform(void* arg);

  CFRunLoopRef cf_lp_;
  CFStringRef cf_mode_;
  CFRunLoopSourceRef cb_;

  uv_loop_t* uv_;
  uv_sem_t sem_;
  uv_thread_t thread_;

  bool closed_;
};

} // namespace cf

#endif // _SRC_CF_H_

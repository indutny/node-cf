#include "cf.h"

#include <stdlib.h> // abort
#include <assert.h> // assert

namespace cf {

using namespace node;
using namespace v8;

Loop::Loop(CFRunLoopRef loop, CFStringRef mode) : cf_lp_(loop),
                                                  cf_mode_(mode),
                                                  closed_(false) {
  int r;

  // Allocate source context
  CFRunLoopSourceContext ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.info = this;
  ctx.perform = Perform;

  // Create source and add it to the loop
  cb_ = CFRunLoopSourceCreate(NULL, 0, &ctx);
  CFRunLoopAddSource(cf_lp_, cb_, cf_mode_);

  uv_sem_init(&sem_, 0);

  // And run our own watcher
  r = uv_thread_create(&thread_, Worker, this);
  assert(r == 0);
}


Loop::~Loop() {
  Close();
}


void Loop::Close() {
  if (closed_) return;
  closed_ = true;

  // Wait for thread to close
  uv_thread_join(&thread_);

  CFRunLoopRemoveSource(cf_lp_, cb_, cf_mode_);
  uv_sem_destroy(&sem_);

  cb_ = NULL;
}


void Loop::Worker(void* arg) {
  Loop* loop = reinterpret_cast<Loop*>(arg);

  // Wait for events on port or signal
  while (true) {
    if (loop->closed_) break;

    uv_wait(uv_default_loop(), 0);

    CFRunLoopSourceSignal(loop->cb_);
    CFRunLoopWakeUp(loop->cf_lp_);

    uv_sem_wait(&loop->sem_);
  }
}


void Loop::Perform(void* arg) {
  Loop* loop = reinterpret_cast<Loop*>(arg);

  uv_run_once(uv_default_loop());
  uv_sem_post(&loop->sem_);
}


Handle<Value> Loop::New(const Arguments& args) {
  HandleScope scope;

  Loop* loop = new Loop(CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
  loop->Wrap(args.Holder());

  return scope.Close(args.Holder());
}


Handle<Value> Loop::AddRef(const Arguments& args) {
  HandleScope scope;

  Loop* loop = ObjectWrap::Unwrap<Loop>(args.This());
  loop->Ref();

  return scope.Close(Null());
}


Handle<Value> Loop::RemRef(const Arguments& args) {
  HandleScope scope;

  Loop* loop = ObjectWrap::Unwrap<Loop>(args.This());
  loop->Unref();
  if (loop->refs_ == 0) {
    loop->Close();
  }

  return scope.Close(Null());
}


void Loop::Init(Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(Loop::New);

  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->SetClassName(String::NewSymbol("Loop"));

  NODE_SET_PROTOTYPE_METHOD(t, "ref", AddRef);
  NODE_SET_PROTOTYPE_METHOD(t, "unref", RemRef);

  target->Set(String::NewSymbol("Loop"), t->GetFunction());
}

} // namespace cf

NODE_MODULE(cf, cf::Loop::Init)

#include "cf.h"
#include "cf-headers.h"

#include <stdlib.h> // abort
#include <assert.h> // assert
#include <unistd.h> // pipe, close
#include <errno.h> // errno
#include <sys/types.h>
#include <sys/time.h>
#include <sys/event.h> // kqueue

namespace cf {

using namespace node;
using namespace v8;

Loop::Loop(CF::CFRunLoopRef loop, CF::CFStringRef mode) : cf_lp_(loop),
                                                          cf_mode_(mode),
                                                          closed_(false) {
  int r;

  // Get main port set and wake up port out of private loop fields
  // (YES, I KNOW IT'S HACKY)
  CF::CFRunLoopModeRef modes;

  // First - get real modes out of `_modes` set
  int mode_cnt = CF::CFSetGetCount(cf_lp_->_modes);
  modes = reinterpret_cast<CF::CFRunLoopModeRef>(
      new char[sizeof(*modes) * mode_cnt]);
  CF::CFSetGetValues(cf_lp_->_modes, (const void**) &modes);

  // Second - find specific mode in list
  for (int i = 0; i < mode_cnt; i++) {
    if (CFStringCompare(modes[i]._name, cf_mode_, 0) != 0) continue;

    // Third - take port set out of it
    main_ = modes[i]._portSet;
    break;
  }

  // Fourth - take wake up port out of loop
  wakeup_ = cf_lp_->_wakeUpPort;

  assert(main_ != 0);
  assert(wakeup_ != 0);

  // Init callback in original thread
  cb_ = new uv_async_t();
  uv_async_init(uv_default_loop(), cb_, Callback);
  cb_->data = this;

  // Init signal pipe
  r = pipe(signal_);
  assert(r == 0);

  r = uv_thread_create(&thread_, Worker, this);
  assert(r == 0);
}


Loop::~Loop() {
  Close();
}


void Loop::Close() {
  if (closed_) return;
  closed_ = true;

  int r;

  // Send signal
  do {
    r = write(signal_[1], "\0", 1);
  } while (r == EINTR);
  assert(r == 1);

  // Wait for thread to close
  uv_thread_join(&thread_);

  // Close everything
  close(signal_[0]);
  close(signal_[1]);
  signal_[0] = -1;
  signal_[1] = -1;

  uv_close(reinterpret_cast<uv_handle_t*>(cb_), OnClose);
  cb_ = NULL;

  main_ = -1;
  wakeup_  = -1;
}


void Loop::Worker(void* arg) {
  Loop* loop = reinterpret_cast<Loop*>(arg);
  int kq;
  int r;
  struct kevent ev[3];
  struct timespec ts;

  kq = kqueue();

  // Enqueue signal
  EV_SET(&ev[0], loop->signal_[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

  // Enqueue ports
  EV_SET(&ev[1], loop->main_, EVFILT_MACHPORT, EV_ADD | EV_ENABLE, 0, 0, 0);
  EV_SET(&ev[2], loop->wakeup_, EVFILT_MACHPORT, EV_ADD | EV_ENABLE, 0, 0, 0);

  ts.tv_sec = 0;
  ts.tv_nsec = 0;

  // XXX: Investigate how to add wakeup port
  r = kevent(kq, ev, 2, NULL, 0, &ts);
  assert(r == 0);

  // Wait for events on port or signal
  while (true) {
    r = kevent(kq, NULL, 0, ev, 1, NULL);

    // Stop looping on signal
    if (ev[0].filter == EVFILT_READ) break;

    // Invoke main thread
    uv_async_send(loop->cb_);
  }

  close(kq);
}


void Loop::Callback(uv_async_t* async, int status) {
  Loop* loop = reinterpret_cast<Loop*>(async->data);

  CFRunLoopRunInMode(loop->cf_mode_, 0.0, false);
}


void Loop::OnClose(uv_handle_t* handle) {
  delete handle;
}


Handle<Value> Loop::New(const Arguments& args) {
  HandleScope scope;

  Loop* loop = new Loop(CF::CFRunLoopGetCurrent(), CF::kCFRunLoopDefaultMode);
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

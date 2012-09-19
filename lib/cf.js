var cf = exports,
    binding = require('bindings')('cf'),
    $ = require('NodObjC');

cf.run = function run(loop, mode) {
  $.framework('Foundation');

  if (!loop) loop = $.CFRunLoopGetCurrent();
  if (!mode) mode = $.kCFRunLoopDefaultMode;

  return new binding.Loop(loop, mode);
};

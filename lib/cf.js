var cf = exports,
    bindings = require('bindings')('cf');

var loop,
    refcnt = 0;

cf.ref = function ref() {
  if (!loop) loop = new bindings.Loop();
  loop.ref();
  refcnt++;
};

cf.unref = function unref() {
  loop.unref();
  refcnt--;
  if (refcnt === 0) loop = null;
};

var cf = exports,
    binding = require('bindings')('cf');

cf.run = function run() {
  return new binding.Loop();
};

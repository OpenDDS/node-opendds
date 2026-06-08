"use strict";
var addon = null;
try {
  addon = require('../build/Release/node_opendds');
} catch (_e) {
  // Ignore error if Release build is not found, will try Debug below.
}
if (!addon) {
  try {
    addon = require('../build/Debug/node_opendds');
  } catch (e) {
    throw new Error("Could not load node-opendds addon module", { cause: e });
  }
}
module.exports = addon;

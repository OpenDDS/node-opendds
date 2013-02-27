"use strict";
var addon = null;
try {
  addon = require('../build/Release/node_opendds');
} catch (e) {
}
if (!addon) {
  try {
    addon = require('../build/Debug/node_opendds');
  } catch (e) {
    throw new Error("Could not load node-opendds addon module");
  }
}
module.exports = addon;

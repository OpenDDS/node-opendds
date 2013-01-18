var addon = null;
try {
    addon = require('../build/Release/node_opendds');
} catch (e) {
}
if (!addon) {
    try {
        addon = require('../build/Debug/node_opendds');
    } catch (e) {
    }
}
module.exports = addon;

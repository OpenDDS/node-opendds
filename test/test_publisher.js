"use strict";

var DOMAIN_ID = 32;
var ddsCerts = process.env.DDS_ROOT + "/tests/security/certs/identity";

var qos = {user_data: 'foo'};
var secure = process.argv.includes('--secure');
if (secure) {
  qos.property = { value: [

    {name: "dds.sec.auth.identity_ca", value: "file:" +
      ddsCerts + "/identity_ca_cert.pem"},

    {name: "dds.sec.access.permissions_ca", value: "file:" +
      ddsCerts + "/identity_ca_cert.pem"},

    {name: "dds.sec.access.governance", value: "file:" +
      "security/governance_signed.p7s"},

    {name: "dds.sec.auth.identity_certificate", value: "file:" +
      ddsCerts + "/test_participant_01_cert.pem"},

    {name: "dds.sec.auth.private_key", value: "file:" +
      ddsCerts + "/test_participant_01_private_key.pem"},

    {name: "dds.sec.access.permissions", value: "file:" +
      "security/pub_permissions_signed.p7s"},

  ]};
}

function init_opendds(opendds_addon) {
  if (secure) {
    return opendds_addon.initialize('-DCPSConfigFile', 'rtps_disc.ini');
  } else {
    return opendds_addon.initialize();
  }
}

var opendds_addon = require('../lib/node-opendds'),
  util = require('util'),
  sleep = util.promisify(setTimeout),
  factory = init_opendds(opendds_addon),
  library = opendds_addon.load('idl/NodeJSTest'),
  participant = factory.create_participant(DOMAIN_ID, qos),
  writer,
  last_sample_id = 24,
  dds_inf = 0x7fffffff,
  infinite = {sec: dds_inf, nanosec: dds_inf};

function log(label, object) {
  console.log(label + ': ' + JSON.stringify(object, null, 2));
}

function doStuff(writer) {
  var sample1 = {
    id: 23,
    data: "Hello, world\n",
    enu: "two",
    enu2: 42,
    bt: {
      o: "254",
      us: 65500,
      s: "32700",
      ul: 3000000000,
      l: "100000",
      ull: "12379813738877118345",
      ll: 5000000000,
      f: 2.17,
      d: 3.14,
      ld: 1.4142136,
      b: true,
      c: "x",
      str: "z012",
      wstr: "abcde"
    },
    seq1: [0, 45, 90, 135, 180],
    seq2: [32, 33, 34],
    ns: [ ["string1", "string2", "string3", "string4"], ["string5"] ],
    ca: ["n", "i", "n", "j", "a", "s"],
    sa: ["north", "east", "south", "west"]
  }

  var sample2 = JSON.parse(JSON.stringify(sample1));
  sample1.mu = { _d: "one", a: 6 };

  sample2.id = 24;
  sample2.mu = { _d: "four", s: [ ["string1", "string2", "string3", "string4"], ["string5"] ] };

  var handle = 0, retcode = 0;

  handle = writer.register_instance(sample1);

  retcode = writer.write(sample1, handle);

  retcode = writer.write(sample2);

  retcode = writer.dispose(sample1, handle);

  retcode = writer.unregister_instance(sample1, handle);

  retcode = writer.unregister_instance(sample2);
}

try {
  if (!library) {
    throw 'Failed to load shared library';
  }
  writer = participant.create_datawriter('topic', 'Mod::Sample', {
    DataWriterQos: {
      latency_budget: {sec: 1, nanosec: 0},
      liveliness: { lease_duration: {sec: 5, nanosec: 0} }
    }
  });

  setTimeout(function () { doStuff(writer); }, 3000);

} catch (e) {
  console.log(e);
}

process.on('exit', function () {
  factory.delete_participant(participant); // optional
  opendds_addon.finalize(factory);
});

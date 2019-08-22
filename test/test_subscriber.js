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
      ddsCerts + "/test_participant_02_cert.pem"}, 

    {name: "dds.sec.auth.private_key", value: "file:" +
      ddsCerts + "/test_participant_02_private_key.pem"}, 

    {name: "dds.sec.access.permissions", value: "file:" +
      "security/sub_permissions_signed.p7s"},

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
  factory = init_opendds(opendds_addon),
  library = opendds_addon.load('idl/NodeJSTest'),
  participant = factory.create_participant(DOMAIN_ID, qos),
  reader,
  last_sample_id = 24,
  dds_inf = 0x7fffffff,
  infinite = {sec: dds_inf, nanosec: dds_inf};

function log(label, object) {
  console.log(label + ': ' + JSON.stringify(object, null, 2));
}

try {
  if (!library) {
    throw 'Failed to load shared library';
  }
  reader = participant.subscribe('topic', 'Mod::Sample', {
    ContentFilteredTopic: {
      filter_expression: 'id < %0',
      expression_parameters: ['30']
    },
    SubscriberQos: {
      presentation: {
        access_scope: 'INSTANCE_PRESENTATION_QOS',
        coherent_access: false,
        ordered_access: false
      },
      partition: ['*'],
      group_data: 'test'
    },
    DataReaderQos: {
      durability: 'VOLATILE_DURABILITY_QOS',
      latency_budget: {sec: 1, nanosec: 0},
      liveliness: {
        kind: 'AUTOMATIC_LIVELINESS_QOS',
        lease_duration: infinite
      },
      reliability: {
        kind: 'RELIABLE_RELIABILITY_QOS',
        max_blocking_time: {sec: 1, nanosec: 0}
      },
      destination_order: 'BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS',
      history: {
        kind: 'KEEP_LAST_HISTORY_QOS',
        depth: 10
      },
      resource_limits: {
        max_samples: 1000,
        max_instances: 100,
        max_samples_per_instance: 10
      },
      user_data: 'arbitrary string',
      ownership: 'SHARED_OWNERSHIP_QOS',
      time_based_filter: {sec: 0, nanosec: 0},
      reader_data_lifecycle: {
        autopurge_nowriter_samples_delay: infinite,
        autopurge_disposed_samples_delay: infinite
      }
    }
  }, function (dr, sinfo, sample) {
    try {
      log('Received callback', sample);
      log('Sample Info', sinfo);
      if (sinfo.valid_data && sample.id == last_sample_id) {
        participant.unsubscribe(reader);
        if (!(sample.id == 23 || sample.id == 24) ||
            sample.data != "Hello, world\n" ||
            sample.enu != "two" ||
            sample.enu2 != "<<invalid>>" ||
            sample.bt.o != 254 ||
            sample.bt.us != 65500 ||
            sample.bt.us != 65500 ||
            sample.bt.s != 32700 ||
            sample.bt.ul != 3000000000 ||
            sample.bt.l != 100000 ||
            //sample.bt.ull != "12379813738877118345" || // This currently fails due to a truncation error from JS "number". Solution will write 64-bit integers (signed & unsigned) as decimal strings. Uncomment this line once truncation issue is resolved in latest release branch.
            sample.bt.ll != 5000000000 ||
            sample.bt.f > (2.17 + 1e-3) || sample.bt.f < (2.17 - 1e-3) || // avoid direct floating point comparisons by testing epsilon ranges
            sample.bt.d > (3.14 + 1e-6) || sample.bt.d < (3.14 - 1e-6) ||
            sample.bt.ld > (1.4142136 + 1e-9) || sample.bt.ld < (1.4142136 - 1e-9) ||
            sample.bt.b != true ||
            sample.bt.c != "x" ||
            sample.bt.str != "z012" ||
            sample.bt.wstr != "abcde" ||
            sample.seq1.length != 5 ||
            sample.seq1[0] != 0 ||
            sample.seq1[1] != 45 ||
            sample.seq1[2] != 90 ||
            sample.seq1[3] != 135 ||
            sample.seq1[4] != 180 ||
            sample.seq2.length != 3 ||
            sample.seq2[0] != 32 ||
            sample.seq2[1] != 33 ||
            sample.seq2[2] != 34 ||
            sample.ns.length != 2 ||
            sample.ns[0].length != 4 ||
            sample.ns[0][0] != "string1" ||
            sample.ns[0][1] != "string2" ||
            sample.ns[0][2] != "string3" ||
            sample.ns[0][3] != "string4" ||
            sample.ns[1].length != 1 ||
            sample.ns[1][0] != "string5" ||
            sample.ca[0] != "n" ||
            sample.ca[1] != "i" ||
            sample.ca[2] != "n" ||
            sample.ca[3] != "j" ||
            sample.ca[4] != "a" ||
            sample.ca[5] != "s" ||
            sample.sa[0] != "north" ||
            sample.sa[1] != "east" ||
            sample.sa[2] != "south" ||
            sample.sa[3] != "west" ||
            (sample.id == 23 && (sample.mu._d != "one" || sample.mu.a != 6)) ||
            (sample.id == 24 && (sample.mu._d != "four" || sample.mu.s.length != 2 || sample.mu.s[0].length != 4 || sample.mu.s[1].length != 1))) {
          console.log("Error in data!");
          process.exitCode = 1;
        }
      }
    } catch (e) {
      console.log("Error in callback: " + e);
    }
  });
} catch (e) {
  console.log(e);
}

process.on('exit', function () {
  factory.delete_participant(participant); // optional
  opendds_addon.finalize(factory);
});

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
      if (sinfo.valid_data && sample.id === last_sample_id) {
        participant.unsubscribe(reader);
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

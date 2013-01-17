#ifndef OPENDDS_NODEQOSCONVERSION_H
#define OPENDDS_NODEQOSCONVERSION_H

#include <v8.h>

#include <dds/DdsDcpsInfrastructureC.h>

namespace NodeOpenDDS {

  void convertQos(DDS::DomainParticipantQos& qos,
                  const v8::Local<v8::Object>& qos_js);

  void convertQos(DDS::SubscriberQos& qos,
                  const v8::Local<v8::Object>& qos_js);

  void convertQos(DDS::DataReaderQos& qos,
                  const v8::Local<v8::Object>& qos_js);
}

#endif

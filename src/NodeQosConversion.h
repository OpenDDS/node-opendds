#ifndef OPENDDS_NODEQOSCONVERSION_H
#define OPENDDS_NODEQOSCONVERSION_H

#include <nan.h>

#include <dds/DdsDcpsInfrastructureC.h>

namespace NodeOpenDDS {

  void convertQos(DDS::DomainParticipantQos& qos,
                  const v8::Local<v8::Object>& qos_js);

  void convertQos(DDS::PublisherQos& qos,
                  const v8::Local<v8::Object>& qos_js);

  void convertQos(DDS::SubscriberQos& qos,
                  const v8::Local<v8::Object>& qos_js);

  void convertQos(DDS::DataWriterQos& qos,
                  const v8::Local<v8::Object>& qos_js);

  void convertQos(DDS::DataReaderQos& qos,
                  const v8::Local<v8::Object>& qos_js);
}

#endif

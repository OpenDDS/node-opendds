#include "dds/DdsDcpsInfrastructureTypeSupportImpl.h"
#include "dds/DCPS/Marked_Default_Qos.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  try {
    DDS::DomainParticipantFactory_var dpf =
      TheParticipantFactoryWithArgs(argc, argv);

    DDS::DomainParticipant_var participant =
      dpf->create_participant(32, PARTICIPANT_QOS_DEFAULT, 0, 0);

    if (!participant) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("%N:%l: main()")
                        ACE_TEXT(" ERROR: create_participant failed!\n")),
                       -1);
    }

    DDS::Topic_var topic =
      participant->create_topic("topic",
                                "TOPIC_BUILT_IN_TOPIC_TYPE",
                                TOPIC_QOS_DEFAULT,
                                0, 0);

    if (!topic) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("%N:%l: main()")
                        ACE_TEXT(" ERROR: create_topic failed!\n")),
                       -1);
    }

    // Create Publisher
    DDS::Publisher_var pub =
      participant->create_publisher(PUBLISHER_QOS_DEFAULT, 0, 0);

    if (!pub) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("%N:%l: main()")
                        ACE_TEXT(" ERROR: create_publisher failed!\n")),
                       -1);
    }

    // Create DataWriter
    DDS::DataWriter_var dw =
      pub->create_datawriter(topic,
                             DATAWRITER_QOS_DEFAULT,
                             0, 0);

    DDS::TopicBuiltinTopicDataDataWriter_var writer =
      DDS::TopicBuiltinTopicDataDataWriter::_narrow(dw);

    DDS::TopicBuiltinTopicData data;
    data.name = "Hello, world\n";

    writer->write(data, DDS::HANDLE_NIL);
    ACE_OS::sleep(1);

    participant->delete_contained_entities();
    dpf->delete_participant(participant);
    TheServiceParticipant->shutdown();

  } catch (...) {
    std::cerr << "ERROR!\n";
    return 1;
  }
  return 0;
}

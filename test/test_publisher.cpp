#include "dds/DCPS/Marked_Default_Qos.h"
#include "dds/DCPS/WaitSet.h"

#include "idl/NodeJSTestTypeSupportImpl.h"

void wait_for_match(const DDS::DataWriter_var& writer)
{
  DDS::StatusCondition_var condition = writer->get_statuscondition();
  condition->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);

  DDS::WaitSet_var ws = new DDS::WaitSet;
  ws->attach_condition(condition);

  DDS::Duration_t timeout =
    { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC };

  DDS::ConditionSeq conditions;
  DDS::PublicationMatchedStatus matches = {0, 0, 0, 0, 0};

  while (true) {
    if (writer->get_publication_matched_status(matches) != ::DDS::RETCODE_OK) {
      ACE_ERROR((LM_ERROR,
                 ACE_TEXT("%N:%l: wait_for_match()")
                 ACE_TEXT(" ERROR: get_publication_matched_status failed!\n")));
      ACE_OS::exit(-1);
    }

    if (matches.current_count >= 1) {
      break;
    }
    if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
      ACE_ERROR((LM_ERROR,
                 ACE_TEXT("%N:%l: wait_for_match()")
                 ACE_TEXT(" ERROR: wait failed!\n")));
      ACE_OS::exit(-1);
    }
  }
  ws->detach_condition(condition);
}

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

    Mod::SampleTypeSupport_var ts = new Mod::SampleTypeSupportImpl;
    ts->register_type(participant, "");
    CORBA::String_var type = ts->get_type_name();

    DDS::Topic_var topic =
      participant->create_topic("topic",
                                type,
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
    wait_for_match(dw);

    Mod::SampleDataWriter_var writer = Mod::SampleDataWriter::_narrow(dw);

    Mod::Sample sample;
    sample.id = 23;
    sample.data = "Hello, world\n";

    writer->write(sample, DDS::HANDLE_NIL);
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

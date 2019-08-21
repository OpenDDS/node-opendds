#include "dds/DCPS/Service_Participant.h"
#include "dds/DCPS/Marked_Default_Qos.h"
#include "dds/DCPS/WaitSet.h"

#include "idl/NodeJSTestTypeSupportImpl.h"
#include <ace/Get_Opt.h>

#include <iostream>

const int DOMAIN_ID = 32;

const char DDSSEC_PROP_IDENTITY_CA[] = "dds.sec.auth.identity_ca";
const char DDSSEC_PROP_IDENTITY_CERT[] = "dds.sec.auth.identity_certificate";
const char DDSSEC_PROP_IDENTITY_PRIVKEY[] = "dds.sec.auth.private_key";
const char DDSSEC_PROP_PERM_CA[] = "dds.sec.access.permissions_ca";
const char DDSSEC_PROP_PERM_GOV_DOC[] = "dds.sec.access.governance";
const char DDSSEC_PROP_PERM_DOC[] = "dds.sec.access.permissions";

void append(DDS::PropertySeq& props, const char* name, const std::string& value)
{
  const DDS::Property_t prop = {
    name, (std::string("file:") + value).c_str(), false /*propagate*/};
  const unsigned int len = props.length();
  props.length(len + 1);
  props[len] = prop;
}

void wait_for_match(const DDS::DataReader_var& reader)
{
  DDS::StatusCondition_var condition = reader->get_statuscondition();
  condition->set_enabled_statuses(DDS::SUBSCRIPTION_MATCHED_STATUS);

  DDS::WaitSet_var ws = new DDS::WaitSet;
  ws->attach_condition(condition);

  DDS::Duration_t timeout =
    { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC };

  DDS::ConditionSeq conditions;
  DDS::SubscriptionMatchedStatus matches = {0, 0, 0, 0, 0};

  while (true) {
    if (reader->get_subscription_matched_status(matches) != ::DDS::RETCODE_OK) {
      ACE_ERROR((LM_ERROR,
                 ACE_TEXT("%N:%l: wait_for_match()")
                 ACE_TEXT(" ERROR: get_subscription_matched_status failed!\n")));
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

void wait_for_data(const Mod::SampleDataReader_var& reader, Mod::Sample& data)
{
  DDS::StatusCondition_var condition = reader->get_statuscondition();
  condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS);

  DDS::WaitSet_var ws = new DDS::WaitSet;
  ws->attach_condition(condition);

  DDS::Duration_t timeout =
    { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC };

  DDS::ConditionSeq conditions;

  DDS::SampleInfo si;
  Mod::Sample temp;
  while (true) {
    DDS::ReturnCode_t status = reader->take_next_sample(temp, si);
    if (status == ::DDS::RETCODE_OK) {
      if (si.valid_data) {
        data = temp;
        break;
      }
    } else if (status != ::DDS::RETCODE_NO_DATA) {
      ACE_ERROR((LM_ERROR,
                 ACE_TEXT("%N:%l: wait_for_data()")
                 ACE_TEXT(" ERROR: take_next_sample failed with error %d\n"), status));
      ACE_OS::exit(-1);
    }

    if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
      ACE_ERROR((LM_ERROR,
                 ACE_TEXT("%N:%l: wait_for_data()")
                 ACE_TEXT(" ERROR: wait failed!\n")));
      ACE_OS::exit(-1);
    }
  }
  ws->detach_condition(condition);
}

void sample_to_cerr(const Mod::Sample& sample) {
  std::cerr << "sample.id = " << sample.id << std::endl;
  std::cerr << "sample.data = \"" << std::string(sample.data.in()) << "\"" << std::endl;
  std::cerr << "sample.enu = " << sample.enu << std::endl;
  std::cerr << "sample.enu2 = " << sample.enu2 << std::endl;
  std::cerr << "sample.bt.o = " << static_cast<uint16_t>(sample.bt.o) << std::endl;
  std::cerr << "sample.bt.us = " << sample.bt.us << std::endl;
  std::cerr << "sample.bt.s = " << sample.bt.s << std::endl;
  std::cerr << "sample.bt.ul = " << sample.bt.ul << std::endl;
  std::cerr << "sample.bt.l = " << sample.bt.l << std::endl;
  std::cerr << "sample.bt.ull = " << sample.bt.ull << std::endl;
  std::cerr << "sample.bt.ll = " << sample.bt.ll << std::endl;
  std::cerr << "sample.bt.f = " << sample.bt.f << std::endl;
  std::cerr << "sample.bt.d = " << sample.bt.d << std::endl;
  std::cerr << "sample.bt.ld = " << sample.bt.ld << std::endl;
  std::cerr << "sample.bt.b = " << (sample.bt.b ? "TRUE" : "FALSE") << std::endl;
  std::cerr << "sample.bt.c = '" << sample.bt.c << "'" << std::endl;
  std::cerr << "sample.bt.str = \"" << std::string(sample.bt.str.in()) << "\"" << std::endl;
  std::wcerr << "sample.bt.wstr = \"" << std::wstring(sample.bt.wstr.in()) << "\"" << std::endl;
  std::cerr << "sample.seq1.length() = " << sample.seq1.length() << std::endl;
  if (sample.seq1.length() > 0) {
    std::cerr << "sample.seq1[0] = " << sample.seq1[0] << std::endl;
    if (sample.seq1.length() > 1) {
      std::cerr << "sample.seq1[1] = " << sample.seq1[1] << std::endl;
      if (sample.seq1.length() > 2) {
        std::cerr << "sample.seq1[2] = " << sample.seq1[2] << std::endl;
        if (sample.seq1.length() > 3) {
          std::cerr << "sample.seq1[3] = " << sample.seq1[3] << std::endl;
          if (sample.seq1.length() > 4) {
            std::cerr << "sample.seq1[4] = " << sample.seq1[4] << std::endl;
          }
        }
      }
    }
  }
  std::cerr << "sample.seq2.length() = " << sample.seq2.length() << std::endl;
  if (sample.seq2.length() > 0) {
    std::cerr << "sample.seq2[0] = " << sample.seq2[0] << std::endl;
    if (sample.seq2.length() > 1) {
      std::cerr << "sample.seq2[1] = " << sample.seq2[1] << std::endl;
      if (sample.seq2.length() > 2) {
        std::cerr << "sample.seq2[2] = " << sample.seq2[2] << std::endl;
      }
    }
  }
  std::cerr << "sample.ns.length() = " << sample.ns.length() << std::endl;
  if (sample.ns.length() > 0) {
    std::cerr << "sample.ns[0].length() = " << sample.ns[0].length() << std::endl;
    if (sample.ns[0].length() > 0) {
      std::cerr << "sample.ns[0][0] = \"" << std::string(sample.ns[0][0].in()) << "\"" << std::endl;
      if (sample.ns[0].length() > 0) {
        std::cerr << "sample.ns[0][1] = \"" << std::string(sample.ns[0][1].in()) << "\"" << std::endl;
        if (sample.ns[0].length() > 0) {
          std::cerr << "sample.ns[0][2] = \"" << std::string(sample.ns[0][2].in()) << "\"" << std::endl;
          if (sample.ns[0].length() > 0) {
            std::cerr << "sample.ns[0][3] = \"" << std::string(sample.ns[0][3].in()) << "\"" << std::endl;
          }
        }
      }
    }
    if (sample.ns.length() > 1) {
      std::cerr << "sample.ns[1].length() = " << sample.ns[1].length() << std::endl;
      if (sample.ns[1].length() > 0) {
        std::cerr << "sample.ns[1][0] = \"" << std::string(sample.ns[1][0].in()) << "\"" << std::endl;
      }
    }
  }
}

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  int result = 0;
  try {
    DDS::DomainParticipantFactory_var dpf =
      TheParticipantFactoryWithArgs(argc, argv);

    bool secure = false;

    const std::string help_message =
      "test_subscriber\n"
      "  -s | --secure\n"
      "    Try to run with security enabled.\n"
      "  -h | --help\n"
      "    Print this message.\n"
    ;
    static const ACE_TCHAR opts_string[] = ACE_TEXT("sh");
    ACE_Get_Opt opts(argc, argv, opts_string);
    opts.long_option(ACE_TEXT("secure"), 's');
    opts.long_option(ACE_TEXT("help"), 'h');
    int opt;
    while ((opt = opts()) != EOF) {
      switch (opt) {
      case 's':
        secure = true;
        break;
      case 'h':
        std::cout << help_message;
        return 0;
      default:
        std::cerr << "Invalid Arguments:\n" << help_message;
        return 1;
      }
    }

    DDS::DomainParticipantQos qos;
    dpf->get_default_participant_qos(qos);

    if (secure) {
#ifdef SECURE
      // Try to Setup Security
      const std::string dds_root(getenv("DDS_ROOT"));
      const std::string dds_certs(dds_root + "/tests/security/certs/identity");
      if (TheServiceParticipant->get_security()) {
        DDS::PropertySeq& props = qos.property.value;
        append(props, DDSSEC_PROP_IDENTITY_CA,
          dds_certs + "/identity_ca_cert.pem");
        append(props, DDSSEC_PROP_PERM_CA,
          dds_certs + "/identity_ca_cert.pem");
        append(props, DDSSEC_PROP_PERM_GOV_DOC,
          "security/governance_signed.p7s");
        append(props, DDSSEC_PROP_IDENTITY_CERT,
          dds_certs + "/test_participant_02_cert.pem");
        append(props, DDSSEC_PROP_IDENTITY_PRIVKEY,
          dds_certs + "/test_participant_02_private_key.pem");
        append(props, DDSSEC_PROP_PERM_DOC,
          "security/sub_permissions_signed.p7s");
      } else {
        ACE_ERROR_RETURN((LM_ERROR,
          ACE_TEXT("%N:%l: main() ERROR: Subscriber is trying to use security (--secure flag passed)\n")
          ACE_TEXT(", but security is not enabled in OpenDDS.\n")
          ), 1);
      }
#else
      ACE_ERROR_RETURN((LM_ERROR,
        ACE_TEXT("%N:%l: main() ERROR: Subscriber is trying to use security (--secure flag passed)\n")
        ACE_TEXT(", but OpenDDS wasn't built with security.\n")
        ), 1);
#endif
    }

    DDS::DomainParticipant_var participant =
      dpf->create_participant(DOMAIN_ID, qos, 0, 0);

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

    // Create Subscriber
    DDS::Subscriber_var sub =
      participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, 0, 0);

    if (!sub) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("%N:%l: main()")
                        ACE_TEXT(" ERROR: create_subscriber failed!\n")),
                       -1);
    }

    // Create DataReader
    DDS::DataReaderQos dr_qos;
    sub->get_default_datareader_qos(dr_qos);
    const DDS::Duration_t one = {1, 0}, five = {5, 0};
    dr_qos.latency_budget.duration = one;
    dr_qos.liveliness.lease_duration = five;
    DDS::DataReader_var dr = sub->create_datareader(topic, dr_qos, 0, 0);

    wait_for_match(dr);

    //std::cout << "matched!" << std::endl;

    Mod::SampleDataReader_var reader = Mod::SampleDataReader::_narrow(dr);

    Mod::Sample sample;

    wait_for_data(reader, sample);

    //std::cout << "got data 1!" << std::endl;

    if (sample.id != 23 ||
      std::string(sample.data.in()) != "Hello, world\n" ||
      sample.enu != Mod::two ||
      sample.enu2 != static_cast<Mod::MyEnum>(42) || // invalid enumerator
      sample.bt.o != 254 ||
      sample.bt.us != 65500 ||
      sample.bt.s != 32700 ||
      sample.bt.ul != 3000000000 ||
      sample.bt.l != 100000 ||
      sample.bt.ull != 0xABCDEF0123456789ULL ||
      sample.bt.ll != 5000000000LL ||
      sample.bt.f < 2.17f - 1e-3 || sample.bt.f > 2.17f + 1e-3 ||
      sample.bt.d < 3.14 - 1e-6 || sample.bt.d > 3.14 + 1e-6 ||
      sample.bt.ld < 1.4142136 - 1e-9 || sample.bt.ld > 1.4142136 + 1e-9 ||
      sample.bt.b != true ||
      sample.bt.c != 'x' ||
      std::string(sample.bt.str.in()) != "z012" ||
      std::wstring(sample.bt.wstr.in()) != L"abcde" ||
      sample.seq1.length() != 5 ||
      sample.seq1[0] != 0 ||
      sample.seq1[1] != 45 ||
      sample.seq1[2] != 90 ||
      sample.seq1[3] != 135 ||
      sample.seq1[4] != 180 ||
      sample.seq2.length() != 3 ||
      sample.seq2[0] != 32 ||
      sample.seq2[1] != 33 ||
      sample.seq2[2] != 34 ||
      sample.ns.length() != 2 ||
      sample.ns[0].length() != 4 ||
      std::string(sample.ns[0][0].in()) != "string1" ||
      std::string(sample.ns[0][1].in()) != "string2" ||
      std::string(sample.ns[0][2].in()) != "string3" ||
      std::string(sample.ns[0][3].in()) != "string4" ||
      sample.ns[1].length() != 1 ||
      std::string(sample.ns[1][0].in()) != "string5" ||
      sample.mu._d() != Mod::one ||
      sample.mu.a() != 6 ||
      sample.ca[0] != 'n' ||
      sample.ca[1] != 'i' ||
      sample.ca[2] != 'n' ||
      sample.ca[3] != 'j' ||
      sample.ca[4] != 'a' ||
      sample.ca[5] != 's' ||
      std::string(sample.sa[0].in()) != "north" ||
      std::string(sample.sa[1].in()) != "east" ||
      std::string(sample.sa[2].in()) != "south" ||
      std::string(sample.sa[3].in()) != "west")
    {
      std::cerr << "ERROR: Data does not match expected results:" << std::endl << std::endl;
      sample_to_cerr(sample);
      result = 1;
    }

    wait_for_data(reader, sample);

    //std::cout << "got data 2!" << std::endl;

    if (sample.id != 24 ||
      std::string(sample.data.in()) != "Hello, world\n" ||
      sample.enu != Mod::two ||
      sample.enu2 != static_cast<Mod::MyEnum>(42) || // invalid enumerator
      sample.bt.o != 254 ||
      sample.bt.us != 65500 ||
      sample.bt.s != 32700 ||
      sample.bt.ul != 3000000000 ||
      sample.bt.l != 100000 ||
      sample.bt.ull != 0xABCDEF0123456789ULL ||
      sample.bt.ll != 5000000000LL ||
      sample.bt.f < 2.17f - 1e-3 || sample.bt.f > 2.17f + 1e-3 ||
      sample.bt.d < 3.14 - 1e-6 || sample.bt.d > 3.14 + 1e-6 ||
      sample.bt.ld < 1.4142136 - 1e-9 || sample.bt.ld > 1.4142136 + 1e-9 ||
      sample.bt.b != true ||
      sample.bt.c != 'x' ||
      std::string(sample.bt.str.in()) != "z012" ||
      std::wstring(sample.bt.wstr.in()) != L"abcde" ||
      sample.seq1.length() != 5 ||
      sample.seq1[0] != 0 ||
      sample.seq1[1] != 45 ||
      sample.seq1[2] != 90 ||
      sample.seq1[3] != 135 ||
      sample.seq1[4] != 180 ||
      sample.seq2.length() != 3 ||
      sample.seq2[0] != 32 ||
      sample.seq2[1] != 33 ||
      sample.seq2[2] != 34 ||
      sample.ns.length() != 2 ||
      sample.ns[0].length() != 4 ||
      std::string(sample.ns[0][0].in()) != "string1" ||
      std::string(sample.ns[0][1].in()) != "string2" ||
      std::string(sample.ns[0][2].in()) != "string3" ||
      std::string(sample.ns[0][3].in()) != "string4" ||
      sample.ns[1].length() != 1 ||
      std::string(sample.ns[1][0].in()) != "string5" ||
      sample.mu._d() != Mod::four ||
      sample.mu.s().length() != 2 ||
      sample.mu.s()[0].length() != 4 ||
      std::string(sample.mu.s()[0][0].in()) != "string1" ||
      std::string(sample.mu.s()[0][1].in()) != "string2" ||
      std::string(sample.mu.s()[0][2].in()) != "string3" ||
      std::string(sample.mu.s()[0][3].in()) != "string4" ||
      sample.mu.s()[1].length() != 1 ||
      std::string(sample.mu.s()[1][0].in()) != "string5" ||
      sample.ca[0] != 'n' ||
      sample.ca[1] != 'i' ||
      sample.ca[2] != 'n' ||
      sample.ca[3] != 'j' ||
      sample.ca[4] != 'a' ||
      sample.ca[5] != 's' ||
      std::string(sample.sa[0].in()) != "north" ||
      std::string(sample.sa[1].in()) != "east" ||
      std::string(sample.sa[2].in()) != "south" ||
      std::string(sample.sa[3].in()) != "west")
    {
      std::cerr << "ERROR: Data does not match expected results:" << std::endl << std::endl;
      sample_to_cerr(sample);
      result = 1;
    }

    participant->delete_contained_entities();
    dpf->delete_participant(participant);
    TheServiceParticipant->shutdown();

    std::cout << "Success!" << std::endl;

  } catch (...) {
    std::cerr << "ERROR!\n";
    return 1;
  }
  return result;
}

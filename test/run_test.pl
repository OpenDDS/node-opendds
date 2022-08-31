eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

use Env (DDS_ROOT);
use lib "$DDS_ROOT/bin";
use Env (ACE_ROOT);
use lib "$ACE_ROOT/bin";
use PerlDDS::Run_Test;
use strict;

my $test = new PerlDDS::TestFramework();

print "\nTest: @ARGV\n";

my %args = map { $_ => 1 } @ARGV;
my $secure = exists($args{"--secure"});
my $rtps = $secure || exists($args{"--rtps"});

my $scenario = "cpp2node";
my $pub_node = 0;
my $sub_node = 1;
if (exists($args{"node2cpp"})) {
  $scenario = "node2cpp";
  $pub_node = 1;
  $sub_node = 0;
} elsif (exists($args{"node2node"})) {
  $scenario = "node2node";
  $pub_node = 1;
  $sub_node = 1;
} elsif (exists($args{"cpp2cpp"})) {
  $scenario = "cpp2cpp";
  $pub_node = 0;
  $sub_node = 0;
}

$test->setup_discovery("-ORBDebugLevel 1 -ORBLogFile DCPSInfoRepo.log") unless $rtps;

sub which {
  my $file = shift;
  my $exeext = ($^O eq 'MSWin32') ? '.exe' : '';
  for my $p (File::Spec->path()) {
    if (-x "$p/$file") {
      return "$p/$file";
    }
    elsif ($exeext ne '' && -x "$p/$file$exeext") {
      return "$p/$file";
    }
  }
  return undef;
}

if ($^O eq 'darwin') {
  $ENV{DYLD_LIBRARY_PATH} = "$DDS_ROOT/lib:$ACE_ROOT/lib";
}

PerlDDS::add_lib_path("idl");
PerlDDS::add_lib_path("../tools/v8stubs");

my $sub_exec_name = "";
my $sub_args = "";
if ($sub_node) {
  $sub_exec_name .= which("node");
  $sub_args .= "test_subscriber.js";
} else {
  $sub_exec_name .= "test_subscriber";
}

my $pub_exec_name = "";
my $pub_args = "";
if ($pub_node) {
  $pub_exec_name .= which("node");
  $pub_args .= "test_publisher.js";
} else {
  $pub_exec_name .= "test_publisher";
}

$pub_args .= " -ORBVerboseLogging 1 -DCPSDebugLevel 5 -DCPSTransportDebugLevel 5";
$sub_args .= " -ORBVerboseLogging 1 -DCPSDebugLevel 5 -DCPSTransportDebugLevel 5";

if ($secure) {
  $pub_args .= " --secure";
  $sub_args .= " --secure";
}

if ($rtps) {
  $pub_args .= " -DCPSConfigFile rtps_disc.ini";
  $sub_args .= " -DCPSConfigFile rtps_disc.ini";
}

$test->process("sub", $sub_exec_name, $sub_args);
$test->process("pub", $pub_exec_name, $pub_args);

$test->start_process("sub");
$test->start_process("pub");

exit $test->finish(60);

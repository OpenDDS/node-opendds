eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

use Env (DDS_ROOT);
use lib "$DDS_ROOT/bin";
use Env (ACE_ROOT);
use lib "$ACE_ROOT/bin";
use PerlDDS::Run_Test;
use strict;

my $status = 0;
my $dcpsrepo_ior = "repo.ior";
unlink $dcpsrepo_ior;

my %args = map { $_ => 1 } @ARGV;
my $secure = exists($args{"--secure"});

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

my $DCPSREPO;
if (not $secure) {
  $DCPSREPO = PerlDDS::create_process("$DDS_ROOT/bin/DCPSInfoRepo");

  $DCPSREPO->Spawn();
  if (PerlACE::waitforfile_timed($dcpsrepo_ior, 30) == -1) {
      print STDERR "ERROR: waiting for Info Repo IOR file\n";
      $DCPSREPO->Kill();
      exit 1;
  }
}

sub which {
  my $file = shift;
  my $exeext = ($^O eq 'MSWin32') ? '.exe' : '';
  for my $p (File::Spec->path()) {
    if (-x "$p/$file") {
      return "$p/$file";
    }
    elsif ($exeext ne '' && -x "$p/$file$exeext") {
      return "$p/$file$exeext";
    }
  }
  return undef;
}

if ($^O eq 'darwin') {
  $ENV{DYLD_LIBRARY_PATH} = "$DDS_ROOT/lib:$ACE_ROOT/lib";
}

PerlDDS::add_lib_path("idl");

my $sub_exec_name = "";
my $sub_args = "";
if ($sub_node) {
  $sub_exec_name .= which("node");
  $sub_args .= "test_subscriber.js";
  if ($secure) {
    $sub_args .= " --secure";
  }
} else {
  $sub_exec_name .= "test_subscriber";
  if ($secure) {
    $sub_args .= " --secure -DCPSConfigFile rtps_disc.ini";
  }
}

my $SUB = PerlDDS::create_process($sub_exec_name, $sub_args);
if ($sub_node) {
  $SUB->IgnoreExeSubDir(1);
}
$SUB->Spawn();

my $pub_exec_name = "";
my $pub_args = "";
if ($pub_node) {
  $pub_exec_name .= which("node");
  $pub_args .= "test_publisher.js";
  if ($secure) {
    $pub_args .= " --secure";
  }
} else {
  $pub_exec_name .= "test_publisher";
  if ($secure) {
    $pub_args .= " --secure -DCPSConfigFile rtps_disc.ini";
  }
}

my $PUB = PerlDDS::create_process($pub_exec_name, $pub_args);
if ($pub_node) {
  $PUB->IgnoreExeSubDir(1);
}
my $PubResult = $PUB->SpawnWaitKill(60);
if ($PubResult != 0) {
    print STDERR "ERROR: " . $pub_exec_name . " returned $PubResult\n";
    $status = 1;
}

my $SubResult = $SUB->WaitKill(10);
if ($SubResult != 0) {
    print STDERR "ERROR: " . $sub_exec_name . " returned $SubResult\n";
    $status = 1;
}

if (not $secure) {
  my $ir = $DCPSREPO->TerminateWaitKill(5);
  if ($ir != 0) {
      print STDERR "ERROR: DCPSInfoRepo returned $ir\n";
      $status = 1;
  }

  unlink $dcpsrepo_ior;
}

exit $status;

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

my $DCPSREPO = PerlDDS::create_process("$DDS_ROOT/bin/DCPSInfoRepo");

$DCPSREPO->Spawn();
if (PerlACE::waitforfile_timed($dcpsrepo_ior, 30) == -1) {
    print STDERR "ERROR: waiting for Info Repo IOR file\n";
    $DCPSREPO->Kill();
    exit 1;
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

my $NODE = PerlDDS::create_process(which("node"), "test.js");
$NODE->IgnoreExeSubDir(1);
$NODE->Spawn();

PerlDDS::add_lib_path("idl");

my $PUB = PerlDDS::create_process("test_publisher");
my $PubResult = $PUB->SpawnWaitKill(60);
if ($PubResult != 0) {
    print STDERR "ERROR: test_publisher returned $PubResult\n";
    $status = 1;
}

my $NodeResult = $NODE->WaitKill(10);
if ($NodeResult != 0) {
    print STDERR "ERROR: node.js returned $NodeResult\n";
    $status = 1;
}

my $ir = $DCPSREPO->TerminateWaitKill(5);
if ($ir != 0) {
    print STDERR "ERROR: DCPSInfoRepo returned $ir\n";
    $status = 1;
}

unlink $dcpsrepo_ior;

exit $status;

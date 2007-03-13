#!/usr/bin/perl
use Socket;
use CGI qw(:standard escape);
use strict;

my $IS_TRIAL = 0;

#
# Change to name of FogBugz server
#
my $BUGZ_SERVER = "nyc.cs.berkeley.edu";

#
# Change to top level URL for bugz server - must contain trailing slash
#
my $BUGZ_URL = "/fogbugz/";

#
# If you're running with PHP on Unix or Macintosh, change to "cvsSubmit.php"
# Otherwise, this should be "cvsSubmit.asp"
#
my $CVSSUBMIT = "cvsSubmit.php";

#
# Change only if you aren't running on normal port 80
#
my $BUGZ_PORT = "8080";


######### That's all you have to do! ###########
#                                              #
#                                              #
#  You shouldn't need to edit anything below   #
#                  here!                       #
################################################

my $TRIAL_ID = "";

my $BUGZ_URL_FINAL = "http:\/\/".$BUGZ_SERVER.":".$BUGZ_PORT.$BUGZ_URL;
if ($IS_TRIAL)
{
	$BUGZ_URL_FINAL = ""; # This line replaced automatically when script is downloaded from trial
}

my $sTrialClause = "";
if ($IS_TRIAL && length($TRIAL_ID) > 0)
{
	$sTrialClause = "&id=" . $TRIAL_ID;
}

my $SAVEERR;
my $logmsg = "";
my $bugIDstring = "";
my $wget = 0;

#
# turn off STDERR because 'which' does not exist not on Windows
#
open(SAVEERR, ">$STDERR");
close(STDERR);
 
my $wget_path = `which wget`; 

#
# turn on STDERR
#
open(STDERR, ">$SAVEERR");
close(SAVEERR);

chop($wget_path);
if ( -e $wget_path )
{ $wget = 1; }

while (<STDIN>)
{
    $logmsg = $_;

    #
    # Get BUG numbers if they're there. Allow multiple BugsID lines,
    # multiple bugs per line.
    #
    # Accepts any capitalization of BugID, BugsID or BugzID,
    # also optional spaces between Bug(sz) and ID, or plural ID.
    #

    if ($logmsg =~ /\s*Bug[zs]*\s*IDs*\s*[#:; ]+((\d+[ ,:;#]*)+)/i)
    {
		$bugIDstring .= " " . $1;
    }
}

#
# Strip out quotes from command line arg from loginfo
#

my $sArg = "";
foreach( @ARGV )
{
	print "ARG is $_\n";
	$sArg .= $_ . " ";
}
chop( $sArg );
$sArg =~ s/\\ /\:/g;

if ( $sArg =~ /^\'(.*)\'/ )
{
	$sArg = $1;
}

print "sARG is $sArg \n";

my @rgFiles = split(" ", $sArg);
foreach( @rgFiles )
{
  $_ =~ s/\:/ /g;
}

my $sDir = "/$rgFiles[0]/";
my $SUBJECT = "CVS: $rgFiles[0] ";

#
# Now, do the submission. We loop through the bug IDs, and submit
# all checked in files against each in turn.
#

my @bugIDlist = split("[ ,:;#]+", $bugIDstring);
my $ixBug = 0;
my $sFile = "";
my $sPrev = "";
my $sNew = "";

foreach (@bugIDlist)
{
    if (/\d+/)
	{
		$ixBug = int($_);
	
		print "Attempting to add bug info for Bug ID #$ixBug...\n";
	
		#
		# For each file open a socket to the right port on the bugz server and submit
		# data via GET or WGET 
		#
	
		for (my $i = 1; $i < scalar(@rgFiles); $i++)
		{
		    print "Looking at $rgFiles[$i] \n";
		    ($sFile,$sPrev,$sNew) = split(",", $rgFiles[$i]);
		    $SUBJECT .= $sFile . " ";
		    
		    if ( $ixBug > 0 )
		    {
				$sFile = escape($sDir . $sFile);		
				print "Notifying FogBugz about $sFile version $sNew...\n";
				if ($wget)
				{
					print "Using wget \n";
					my $url = $BUGZ_URL_FINAL . $CVSSUBMIT . "?ixBug=$ixBug&sFile=$sFile&sPrev=$sPrev&sNew=$sNew".$sTrialClause;
					system("wget '$url' -q -O /dev/null");
				}
				else
				{
					print "Using GET \n";
					my $get = "GET ".$BUGZ_URL_FINAL.$CVSSUBMIT."?ixBug=$ixBug&sFile=$sFile&sPrev=$sPrev&sNew=$sNew".$sTrialClause."\n\n";
					my $in_addr = (gethostbyname($BUGZ_SERVER))[4] || die("Error: $!\n");
					my $paddr = sockaddr_in($BUGZ_PORT, $in_addr) || die("Error: $!\n");
					socket(S, PF_INET, SOCK_STREAM, getprotobyname('tcp')) || die("Error: $!\n");
					connect(S, $paddr) || die("Error: $!\n");
					select(S);
					$|=1;
					select(STDOUT);
					print S $get;
					close(S);
				}
		    }
		}
    }
}

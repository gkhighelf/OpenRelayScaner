#!/usr/local/bin/perl

#/**
# * Created by GKHighElf - gkhighelf@gmail.com
# * in near 2000 year, for self education and use.
#**/

# ftp://ftp.arin.net/pub/stats/lacnic/delegated-lacnic-latest
# ftp://ftp.arin.net/pub/stats/apnic/delegated-apnic-latest
# ftp://ftp.arin.net/pub/stats/afrinic/delegated-afrinic-latest
# ftp://ftp.arin.net/pub/stats/ripencc/delegated-ripencc-latest

use strict;
use DBI;
use DBD::mysql;
use File::Fetch;

my $link;
my $val;
my @network_links = ("ftp://ftp.arin.net/pub/stats/lacnic/delegated-lacnic-latest",
    "ftp://ftp.arin.net/pub/stats/apnic/delegated-apnic-latest",
    "ftp://ftp.arin.net/pub/stats/afrinic/delegated-afrinic-latest",
    "ftp://ftp.arin.net/pub/stats/ripencc/delegated-ripencc-latest");
my $ff;
my $fn;
my $qvals;
my $cnt;
my $ass;

my $db = DBI->connect('dbi:mysql:dbname=statistics;host=127.0.0.1','statistics','statistics');
$db->do("truncate table world_subnets;");

foreach $link (@network_links) {
#    print $link =~ /.*\/.+$/g,"\n";
    $ff = File::Fetch->new( uri => $link );
    $fn = $ff->fetch() or die $ff->error;

    open( FN, $fn ) or die "error openning file.\n";

    $qvals = "";
    $cnt = 0;
    while( <FN> )
    {
        s/\n//;
        next if /(asn|^$|ipv6)/;
        next if not /((\d{1,3})\.){3}(\d{1,3})/;
        my @vals = split /\|/;
        my $mask = @vals[4]-1;
        $ass = 0;
        $ass = 1 if /assigned/;
        $qvals .= "( inet_aton('@vals[3]'), 0xFFFFFFFF ^ $mask, '@vals[1]', '', $ass, inet_aton('@vals[3]') >> 24, 0 ),";
        if( $cnt++ >= 100 )
        {
            $qvals =~ s/,$//;
            print $qvals."\n";
            $db->do("insert into world_subnets values $qvals;") or die "error executing query\n";
            $qvals = "";
            $cnt = 0;
        }
    }
}

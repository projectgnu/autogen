#!/usr/bin/perl

use strict;

my ($optlist,$oldoptlist);
my ($literal);
my ($line);
my ($count,$tableitemcount);

$optlist = 0;       ### 1 = bullet, 2 = enum, 3 = tag, 4 = item
$oldoptlist = 0;

while ($line = <STDIN>)
{
    $line = ParseMacro($line);
    print($line)
        if (defined $line);
}

sub ParseMacro #line
{
    my ($line) = @_;

    my (@words,$retval);
    $retval = '';
    @words =  split(/\s+/,$line);

    while ($_ = shift (@words))
    {
        if(/\@/){
            if(/\@code/){
                print "\n";
                $_ =~ s/\@code//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $_ = $retval."". $_;
                print ".Li ". $_;
                print "\n";
            }
            if(/\@kbd/){
                #print $_."\n";
                print "\n";
                $_ =~ s/\@kbd//;
                $_ =~ s/{//;
                $retval = '';
                while () {
                    if ($_ =~ m/\@key/ ){
                        $_ =~ s/\@key//;
                        $_ =~ s/\{//;
                        $_ =~ s/\}//;
                        if($_ =~ m/\}/){
                            $_ =~ s/\}//;
                            $retval = $retval." ".$_;
                            last;
                        }
                    } else {
                        if($_ =~ m/\}/){
                            $_ =~ s/\}//;
                            $retval = $retval." ".$_;
                            last;
                        }
                    }
                    $retval = $retval." ".$_;
                    $_ = shift (@words)
                }

                print ".Li ". $retval;
                print "\n";
            }
            if(/\@key/){
                $_ =~ s/\@key//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $_ = $retval." ". $_;
                print ".Li ".$_;
                print "\n";
            }
            if(/\@samp/){#while loop with var need to handle
                print "\n";
                $_ =~ s/\@samp//;
                $_ =~ s/\{//;
                while () {
                    if ($_ =~ m/\@var/ ){
                        $_ =~ s/\@var//;
                        $_ =~ s/\{//;
                        $_ =~ s/\}//;
                        if($_ =~ m/\}/){
                            $_ =~ s/\}//;
                            $retval = $retval." ".$_;
                            last;
                        }
                    } else {
                        if($_ =~ m/\}/){
                            $_ =~ s/\}//;
                            $retval = $retval." ".$_;
                            last;
                        }
                    }
                    $retval = $retval." ".$_;
                    $_ = shift (@words)
                }

                print ".Sq ".$retval;
                print "\n";
            }
            if(/\@emph/){
                print "\n";
                $_ =~ s/\@emph//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $_ = $retval." ". $_;
                print ".Em ". $_;
                print "\n";
            }
            if(/\@strong/){
                print "\n";
                $_ =~ s/\@strong//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $_ = $retval." ". $_;
                print ".Li ". $_;
                print "\n";
            }
            if (/\@verb{/){ #while loop need to handle.
                print " ";
                $_ =~ s/\@verb//;
                $_ =~ s/\{//;
                $_ =~ s/\|//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ =~ s/\|//;
                    $_ = shift(@words)
                }
                $_ =~ s/\|//;
                $_ =~ s/\}//;
                $_ = $retval." ". $_;
                print $_;
                print " ";
            }
            if (/\@var{/) {
                print "\n";
                $_ =~ s/\@var//;
                $_ =~ s/\{//;
                $_ =~ s/\}//;
                print ".Li ".$_;
                print "\n";
            }
            if (/\@env{/) {
                print "\n";
                $_ =~ s/\@env//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                print ".Li ".$_;
                print "\n";
            }
            if (/\@file{/) {
                print "\n";
                $_ =~ s/\@file//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                print ".Li ".$_;
                print "\n";
            }
            if (/\@command{/) {
                print "\n";
                $_ =~ s/\@command//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                print ".Li ".$_;
                print "\n";
            }
            if (/\@option{/) {
                print "\n";
                $_ =~ s/\@option//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $retval = $retval."".$_;
                print ".Li ".$retval;
                print "\n";
            }
            if (/\@dfn{/) {
                print "\n";
                $_ =~ s/\@dfn//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $retval = $retval."".$_;
                print ".Sq ".$retval;
                print "\n";
            }
            if (/\@abbr{/) {
                print "\n";
                $_ =~ s/\@abbr//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $retval = $retval."".$_;
                print ".Li ".$retval;
                print "\n";
            }
            if (/\@acronym{/) {
                print "\n";
                $_ =~ s/\@acronym//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $retval = $retval."".$_;
                print ".Li ".$retval;
                print "\n";
            }
            if (/\@indicateurl{/) {
                print "\n";
                $_ =~ s/\@indicateurl//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $retval = $retval."".$_;
                print ".Aq Ao ".$retval." Ac";
                print "\n";
            }
            if (/\@email{/) {
                print "\n";
                $_ =~ s/\@email//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $retval = $retval."".$_;
                $retval =~ s/\, [a-zA-Z0-9]+//;
                print ".Aq ".$retval;
                print "\n";
            }
            if (/\@strong{/) {
                print "\n";
                $_ =~ s/\@strong//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $retval = $retval."".$_;
                print ".Sy ".$retval;
                print "\n";
            }
            if (/\@emph{/) {
                print "\n";
                $_ =~ s/\@emph//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $retval = $retval."".$_;
                print ".Em ".$retval;
                print "\n";
            }
            if (/\@sc{/) {
                $_ =~ s/\@sc//;
                $_ =~ s/\{//;
                $retval = '';
                while ($_  !~ m/}/) {
                    $retval = $retval." ".$_;
                    $_ = shift(@words)
                }
                $_ =~ s/\}//;
                $retval = $retval."".$_;
                print ".Tn ".$retval;
            }
            if (/\@quotation/) {
                print "\n";
                $line = <STDIN>;
                $_ = shift(@words);
                print ".Sy ".$_."\n";
                $retval = '';
                while ($line !~ m/\@end/) {
                    $retval = $retval." ".$line;
                    $line = <STDIN>;
                }
                print $retval;
            }
            if (/\@example/) {
                #print "\n";
                $line = <STDIN>;
                #$_ = shift(@words);
                #print ".Li ".$line;
                $retval = '';
                while ($line !~ m/\@end/) {
                    #$retval = $retval." ".$line;
                    $line =~ s/\@var{//;
                    $line =~ s/\}//;
                    print ".Li ".$line;
                    $line = <STDIN>;
                }
            }
            if (/\@verbatim/) {
                #print "\n";
                $line = <STDIN>;
                #$_ = shift(@words);
                #print ".Li ".$line;
                $retval = '';
                while ($line !~ m/\@end/) {
                    #$retval = $retval." ".$line;
                    print ".Li ".$line;
                            
                    $line = <STDIN>;
                }   
            }
            if (/\@lisp/) {
                print "\n";
                $line = <STDIN>;
                #$_ = shift(@words);
                #print ".Li ".$line;
                $retval = '';
                while ($line !~ m/\@end/) {
                    #$retval = $retval." ".$line;
                    print ".Li ".$line;
                            
                    $line = <STDIN>;
                }
            }
            if (/\@display/) {
                print "\n";
                $line = <STDIN>;
                #$_ = shift(@words);
                #print ".Li ".$line;
                $retval = '';
                while ($line !~ m/\@end/) {
                    #$retval = $retval." ".$line;
                    print ".Li ".$line;
                            
                    $line = <STDIN>;
                }
            }
        } else {
            print " ";
            print $_;
        }
    }
}

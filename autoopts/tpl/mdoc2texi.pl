#! /usr/bin/perl

use strict;

my ($optlist,$oldoptlist);
my ($literal);
my ($line);
my ($count,$tableitemcount);
my ($progName);
my (@words, $retval,$columnline);

$optlist = 0;       ### 1 = bullet, 2 = enum, 3 = tag, 4 = item
$oldoptlist = 0;

while ($line = <STDIN>)
{
    if ($line !~ /^\./)
    {
        print $line;
        print ".br\n"
            if ($literal);
        next;
    }

    next
        if ($line =~ /^\.\\"/);

    $line = ParseMacro($line);
    print($line)
        if (defined $line);
}

sub Handle_Bl
{
    if ($words[0] eq '-bullet')
    {
        if (!$optlist)
        {
            $optlist = 1; #bullet
            $retval .= "\@itemize \@bullet\n" ;
            print "$retval";
            return 1;
        }
        else
        {
            $retval .= "\@itemize \@minus\n";
            print $retval;
            $oldoptlist = 1;
            return 1;
        }
    }
    if ($words[0] eq '-enum')
    {
        if (!$optlist)
        {
            $optlist = 2; #enum
            $retval .= "\@enumerate\n" ;
            print "$retval";
            return 1;
        }
        else
        {
            $retval .= "\@enumerate\n";
            print $retval;
            $oldoptlist = 2;
            return 1;
        }
    }
    if ($words[0] eq '-tag')
    {
        $optlist = 3; #tag
        $retval .= "\@table \@samp\n";
        print "$retval";
        return 1;
    }
    if ($words[0] eq '-column')
    {
        $optlist = 4; #column
        $retval = "\@multitable \@columnfractions ";#\.20 \.20 \.20\n";
        #print $retval;
        $columnline = "\@headitem ";
        #print $retval;
        foreach(@words)
        {
            if (!/^"./ && !/-column/ && !/indent/ && !/-offset/)
            {
                $_ =~ s/\"//g;

                $retval .= "\.20 ";
                if (!$count)
                {
                    $columnline .= $_;
                }
                else
                {
                    $columnline .= " \@tab ".$_;
                }
                $count++;
            }
        }
        print $retval."\n";
        print $columnline;
        return 1;
    }

    return 0;
}

sub Handle_It
{
    if ($optlist == 3)
    {
        $retval .= "\@item ".$words[0]."\n";
        print $retval;
        return 1;
    }
    elsif ($optlist == 4 )
    {
        if (!$tableitemcount)
        {
            $tableitemcount = 1;
            return 1;
        }
        else
        {
            foreach(@words)
            {
                if (/^Li$/)
                {
                    print "\n\@item ";
                    return 0;
                }
                elsif (/^Ta$/)
                {
                    print "\n\@tab ";
                    return 0;
                }
                else
                {
                    print $_;
                    return 0;
                }
            }
            return 1;
        }
    }
    else
    {
        print "\@item\n";
    }
    return 0;
}

sub Handle_El
{
    if ($oldoptlist)
    {
        if ($oldoptlist == 1)
        {
            $oldoptlist = 0;
            $retval .= "\@end itemize\n";
            print $retval;
        }
        elsif ($oldoptlist == 2)
        {
            $oldoptlist = 0;
            $retval .= "\@end enumerate\n";
            print $retval;
        }
    }
    else
    {
        if ($optlist == 1)
        {
            $oldoptlist = 0;
            $retval .= "\@end itemize\n";
            print $retval;
        }
        elsif ($optlist == 2)
        {
            $oldoptlist = 0;
            $retval .= "\@end enumerate\n";
            print $retval;
        }
        elsif ($optlist = 4)
        {
            $count = 0;
            $columnline = '';
            $oldoptlist = 0;
            $optlist = 0;
            $tableitemcount = 0;
            $retval .= "\n\@end multitable\n";
            print $retval;
        }
        $optlist = 0;
    }
}

sub Handle_Fl
{
    # .Cm is .Fl but no '-'.
    # Usage: .Fl <argument> ...
    #
    #	.Fl          -
    #	.Fl cfv      -cfv
    #	.Fl cfv .    -cfv.
    #	.Cm cfv .    cfv.
    #	.Fl s v t    -s -v -t
    #	.Fl - ,      --,
    #	.Fl xyz ) ,  -xyz),
    #	.Fl |        - |
    #
    my ($dash, $didOne);
    $dash = "-";	# or empty if .Cm
    $didOne = 0;

    do {
        if ($words[0] eq '' || $words[0] =~ /^[-\w]+$/)
        {
            print " " if $didOne;
            print '@code{', $dash, $words[0], '}';
        }
        elsif ($words[0] eq '|')
        {
            print " " if $didOne;
            print '@code{', $dash, '}', " $words[0]";
        }
        else
        {
            print "$words[0]";
        }
        shift @words;
        $didOne = 1;
    } while scalar(@words);
    print " ";
}

sub Handle_Nm
{
    # Usage: .Nm [<argument>] ...
    #
    #	.Nm groff_mdoc  groff_mdoc
    #	.Nm \-mdoc      -mdoc
    #	.Nm foo ) ) ,   foo)),
    #	.Nm :           groff_mdoc:
    #
    if (!defined $progName)
    {
        if (defined $ENV{AG_DEF_PROG_NAME})
        {
            $progName = $ENV{AG_DEF_PROG_NAME};
        }
        else
        {
            $progName = "XXX Program Name";
        }
    }

    if ($words[0] =~ /^[\\\w]/)
    {
        $progName = shift @words;
    }
    print '@code{', $progName, '}';

    # Anything after this should be punctuation

    while ($_ = shift @words)
    {
        print;
    }
    print "\n";
}

sub Handle_Xr
{
    # Usage: .Xr <man page name> [<section>] ...
    #	.Xr mdoc        mdoc
    #	.Xr mdoc ,      mdoc,
    #	.Xr mdoc 7      mdoc(7)
    #	.Xr xinit 1x ;  xinit(1x);
    #
    # Emitting things like @uref{/man.cgi/1/ls,,ls} would be OK,
    # but we'd have to allow for changing /man.cgi/ (at least).
    # I'm OK with:
    #	@code{mdoc}
    #	@code{mdoc},
    #	@code{mdoc(7)}
    #	@code{xinit(1x);
    #
    my ($xr_cmd, $xr_sec, $xr_punc);
    if (@words == 1)
    {
        $xr_cmd = $words[0];
    }
    elsif (@words == 2)
    {
        $xr_cmd = shift @words;
        if ($words[0] =~ /[[:punct:]]/)
        {
            $xr_punc = shift @words;
        }
        else
        {
            $xr_sec = shift @words;
        }
    }
    elsif (@words == 3)
    {
        $xr_cmd = shift @words;
        $xr_sec = shift @words;
        $xr_punc = shift @words;
    }
    else
    {
    }

    # HMS: do we really want 'defined' in the following tests?
    print '@code{',"$xr_cmd"	if (defined $xr_cmd);
    print "($xr_sec)"		if (defined $xr_sec);
    print "}"			if (defined $xr_cmd);
    print "$xr_punc"		if (defined $xr_punc);
    print "\n";
}

sub ParseMacro #line
{
    my ($line) = @_;

    @words = split(/\s+/, $line);
    $retval = '';

    # print('@words = ', scalar(@words), ': ', join(' ', @words), "\n");

    while ($_ = shift @words)
    {
        if    (/^\.Bl$/)                { last if (Handle_Bl()); }
        elsif ($optlist && /^\.It$/)    { last if (Handle_It()); }
        elsif (/^\.El$/)                { Handle_El(); }
        elsif (/^\.Fl$/)                { Handle_Fl(); }
        elsif (/^\.Nm/)                 { Handle_Nm(); }
        elsif (/^\.Pp$/)                { print "\n";  }
        elsif (/^\.Xr/)                 { Handle_Xr(); }
	else                            { print $_,"\n"; }
    }
}

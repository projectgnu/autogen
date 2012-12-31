#! /usr/bin/perl

use strict;

my ($literal);
my ($line);
my ($blCf,$blEl,@blEl,$blIt,@blIt,$count,$tableitemcount);
my ($progName);
my (@words, $retval,$columnline);
my ($extArg);
my (%anchor, $aCount);

$aCount = 0;
$optlist = 0;       ### 1 = bullet, 2 = enum, 3 = tag, 4 = item/column
$oldoptlist = 0;
$extArg = 0;

###
#
# We don't know what order we'll see .Sx
#
# Whenever we find one, we look it up.
# If it doesn't exist we assign it an anchor name.
# Regardless, we "return" the anchor name, as we're either going
# to define the anchor point using '@anchor{anchor-1}' or reference
# the anchor using something like '@xref{anchor-1,,whatever}'
#
###

while ($line = <STDIN>)
{
    if ($line !~ /^\./)
    {
        print $line;
        print "\@*\n"
            if ($literal);              # $literal is never 'true'...
        next;
    }

    next
        if ($line =~ /^\.\\"/);         # comment

    $line = ParseMacro($line);
    print($line)
        if (defined $line);
}

sub Anchor ($)
{
    my $string = shift;

    # Look up the provided string.
    # If it's not there, bump $aCount and "add" the anchor.
    # Return the anchor.

    if (!exists $anchor{$string})
    {
        ++$aCount;
        $anchor{$string} = "anchor-$aCount";
    }

    return $anchor{$string};
}

sub Handle_An
{
    # We should eventually support -nosplit and -split.
    # Usage: .An <author name> ...
    #   .An "Joe Author"                Joe Author
    #   .An "Joe Author" ,              Joe Author,
    #   .An "Joe Author" Aq user@site   Joe Author <user@site>
    #   .An "Joe Author" ) ) ,          Joe Author)),

    do {
        parseQuote(\@words) if ($words[0] =~ /^"/);
        print shift @words;
    } while scalar(@words);

    while ($_ = shift @words)
    {
        print;
    }

    # Line break after each author
    print "\@*\n";
}

sub Handle_Bl
{
    # Must end with a .El.  May be nested, including inside displays.
    #
    # .Bl {-hang | -ohang | -tag | -diag | -inset} [-width <string>] \
    #    [-offset <string>]
    # .Bl -column [-offset <string>] <string1> <string2> ...
    # .Bl {-item | -enum [-nested] | -bullet | -hyphen | -dash} \
    #    [-offset <string>] [-compact]
    # "-offset indent" uses a standard indent.
    # -compact suppresses insertion of vertical space before the list and
    # between the list items.
    my ($inMulti);

    # For nesting, save needed context:
    # $blEl
    # $blIt
    unshift @blEl, $blEl;
    unshift @blIt, $blIt;

    $blEl = "blEl - XXX!";
    $blIt = "";                         # Would undef be easier?

    $inMulti = 0;
    if ($words[0] eq '-hang')           # hanging tags
    {
        print "\@table \@asis\n";
        $blEl = "\@end table\n";
    }
    elsif ($words[0] eq '-ohang')       # tag on its own line, no indent
        {
        print "\@table \@asis\n";
        $blEl = "\@end table\n";
        }
    elsif ($words[0] eq '-tag')         # like hang.
        {
        print "\@table \@asis\n";
        $blEl = "\@end table\n";
        }
    elsif ($words[0] eq '-diag')        # man (4) diagnostic list - inset
    {
        print "\@table \@asis\n";
        $blEl = "\@end table\n";
    }
    elsif ($words[0] eq '-inset')       # inset list - See the mdoc page.
    {
        print "\@table \@asis\n";
        $blEl = "\@end table\n";
    }
    elsif ($words[0] eq '-column')      # Multiple columns
        {
        print "\@multitable \n";
        $blEl = "\@end multitable\n";
        $blCf = "\@columnfractions";
        $inMulti = 1;
        }
    elsif ($words[0] eq '-item')        # Item with no list markers
        {
        print "\@table \@asis\n";
        $blEl = "\@end table\n";
        }
    elsif ($words[0] eq '-enum')        # Enumerated (numbered) list
    {
        print "\@itemize \@enumerate\n";
        $blEl = "\@end enumerate\n";
    }
    elsif ($words[0] eq '-bullet')      # Bullet list
    {
        print "\@itemize \@bullet\n";
        $blEl = "\@end itemize\n";
    }
    elsif ($words[0] eq '-hyphen')      # dash/hyphen list
    {
        # What would be better?  Maybe a 2 column table...
        print "\@itemize \@bullet\n";
        $blEl = "\@end itemize\n";
        $blIt = "-";                    # @minus ?
    }
    elsif ($words[0] eq '-dash')        # dash/hyphen list
        {
        # What would be better?  Maybe a 2 column table...
        print "\@itemize \@bullet\n";
        $blEl = "\@end itemize\n";
        $blIt = "-";                    # @minus ?
    }
    else
            {
        die "Handle_Bl: Unknown list type <$words[0]>\n";
    }

    shift @words;

    while ($_ = shift @words)
                {
        if (/^-width$/)
        {
            # Maybe some day we will do something with the value...
            parseQuote(\@words) if ($words[0] =~ /^"/);
            shift @words;
                }
        elsif (/^-offset$/)
                {
            # Maybe some day we will do something with the value...
            shift @words;
        }
        elsif (/^-compact$/)
        {
            # No argument expected
                }
        elsif ($inMulti)                        # -column width
        {
            # Maybe some day we will do something with the value...
            $blCf .= " .2";
            }
        else
        {
            die "Handle_Bl: unexpected argument: <$_>\n";
        }
    }

    print $blCf,"\n" if ($blCf ne "");
    $blCf = "";
}

sub Handle_It
{
    # .It Li "sntp ntpserver.somewhere"

    print "\@item ";
    if ($blIt ne "")
    {
        print $blIt;
        # Assert @words is empty?
    }
    else
    {
        while ($_ = shift @words)
        {
            # print STDERR "Handle_It: looking at <$_>\n";     # XXX
            if (/^Op$/)
            {
                Handle_Op();
            }
            elsif (/^(Ar|Cm|Fl|Ic|Li)$/)
            {
                Handle_ArCmFlIcLi();
            }
            elsif (/^Ta$/)  # separate columns
            {
                print "\n\@tab\n";
            }
            else
            {
                print $_;
            }
        }
    }
    print "\n";
}

sub Handle_D
{
    # .D1 Fl abcdefg    <tt>-abcdefg<>          # 1 line of indented text
    # .Dl % ls \-l /etc <tt>% ls -l /etc<tt>    # 1 indented literal text line

    s/^\.//;
    if (/^D1$/)
    {
        die "Handle_D(): D1 isn't implemented yet.\n";
    }
    elsif (/^Dl$/)
    {
        print "\@example\n";
        while ($_ = shift @words)
        {
            s/\\//;
            print "$_ ";
        }
        print "\n\@end example\n";
    }
    else
    {
        die "Handle_D(): Unexpected mode: <$_>\n";
    }
}

sub Handle_El
{
    print $blEl;

    $blIt = shift @blIt;
    $blEl = shift @blEl;
}

sub Handle_Em
{
    # Usage: .Em stuff
    #   .Em word                <italic>word</italic>
    #   .Em or Ap ing           <italic>or</italic>'ing
    #

    parseQuote(\@words) if ($words[0] =~ /^"/);

    print '@emph{';
    do {
        print shift @words;
    } while (@words > 0 && $words[0] !~ /^[[:punct:]]$/);
    print "}";

    while ($_ = shift @words)
    {
        print;
    }

    print "\n";
}

sub Handle_ArCmFlIcLi
{
    # .Ar wants an italic code font, texi uses @kbd for that.
    # .Cm is .Fl but no '-'.
    # Usage: .Fl <argument> ...
    #
    #   .Fl          -
    #   .Fl cfv      -cfv
    #   .Fl cfv .    -cfv.
    #   .Cm cfv .    cfv.
    #   .Fl s v t    -s -v -t
    #   .Fl - ,      --,
    #   .Fl xyz ) ,  -xyz),
    #   .Fl |        - |
    #   .Ic "do while {...}"    do while {...}
    #   .Li M1 M2 ;  <tt>M1 M2<tt<tt>>;
    #
    my ($dash, $didOne, $font, $fontE, $spacing);

    s/^\.//;

    $dash = (/^Fl$/) ? "-" : "";
    $font = (/^Ar$/) ? "\@kbd{" : "\@code{";            # }
    $fontE = '}';
    $didOne = 0;
    $spacing = 1;

    do {
        if ($words[0] eq '|')
        {
            print " " if $didOne && $spacing;
            print $font, $dash, $fontE, ' ' if ($dash ne "");
            print "$words[0]";
        }
        elsif ($words[0] eq '-')
        {
            print " " if $didOne && $spacing;
            print $font, $dash, $words[0], $fontE;
        }
        elsif ($words[0] =~ /^"/)
        {
            print " " if $didOne && $spacing;
            print $font;
            print $dash if ($dash ne "");       # Do we need this?
            parseQuote(\@words);
            print $words[0];
            print $fontE;
        }
        elsif ($words[0] eq 'Ar')               # Argument
        {
            $font = '@kbd{';                    # } slanted tty
        }
        elsif ($words[0] eq 'Ic')               # Interactive/internal command
        {
            $font = '@code{';                   # }
        }
        elsif ($words[0] eq 'Xc')
        {
            $spacing = 1;
        }
        elsif ($words[0] eq 'Xo')
        {
            $spacing = 0;
        }
        elsif ($words[0] =~ /^[[:punct:]]$/)
        {
            # print " " if $didOne && $spacing;
            print $words[0];
        }
        else            # Should be empty or a word
        {
            print " " if $didOne && $spacing;
            print $font;
            print $dash if ($dash ne "");       # Do we need this?
            $words[0] =~ s/\\&//;
            print $words[0];
            print $fontE;
        }
        shift @words;
        $didOne = 1;
    } while (scalar(@words) && $words[0] ne "Op");
    print " ";
}

sub Handle_Fn
{
    # Usage: .Fn <function> [<parameter>] ...
    #   .Fn getchar             <code>getchar</code>()
    #   .Fn strlen ) ,          <code>strlen</code>()),
    #   .Fn align "char *pt" ,  <code>align</code(<slant>char *pt<slant>),
    #
    my ($didArg, $isOpen);

    print '@code{', $words[0], "}(";
    $isOpen = 1;
    shift;

    $didArg = 0;
    while ($_ = shift @words)
    {
        if ($words[0] =~ /^"/) {
            # assert $isOpen == 1
            print '@code{, }' if ($didArg);
            parseQuote(\@words);
            print '@emph{', $words[0], "}";
            $didArg = 1;
        } else {
            print ")" if ($isOpen);
            $isOpen = 0;
            print $words[0];
        }
    }

    print "\n";
}

sub Handle_Nm
{
    # Usage: .Nm [<argument>] ...
    #
    #   .Nm groff_mdoc  groff_mdoc
    #   .Nm \-mdoc      -mdoc
    #   .Nm foo ) ) ,   foo)),
    #   .Nm :           groff_mdoc:
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

sub Handle_Op
{
    # Usage: .Op [<option>] ...
    #   .Op                                     []
    #   .Op Fl k                                [-k]
    #   .Op Fl k ) .                            [-k]).
    #   .Op Fl c Ar objfil Op Ar corfil ,       [ -c objfil [corfil]],
    #   .Op word1 word2                         [word1 word2]
    #
    # If we decide to support Oo and Oc this almost becomes recursive,
    # but we can handle that with separate Handle_Oo and Handle_Oc
    # routines.

    my ($op);

    print '[';
    $op = 1;
    do {
        if ($op && $words[0] =~ /^(Ar|Cm|Fl|Ic)$/)
        {
                $_ = shift @words;
                Handle_ArCmFlIcLi();
        }
        elsif ($words[0] =~ /^[[:punct:]]$/)
        {
                print ']' if ($op);
                $op = 0;
                print shift @words;
        }
        else
        {
                print shift @words;
        }
    } while (@words > 0);
    print ']' if ($op);
    print "\n";                 # HMS: We may not want these in many places...
}

sub Handle_Pa
{
    # Usage: .Pa [<pathname>] ...
    #   .Pa                     ~
    #   .Pa /usr/share          /usr/share
    #   .Pa /tmp/fooXXXXX ) .   /tmp/fooXXXXX).
    #
    my ($pa_path);
    if (@words == 0)
    {
        $pa_path = "~";
    }
    else
    {
        $pa_path = shift @words;
    }

    print '@file{',"$pa_path","}";
    while ($_ = shift @words) {
        print;
    }
    print "\n";
}

sub Handle_Q
{
    # Usage: .Ql ...
    #   .Aq ...                 Angle bracket: <...>
    #   .Bq ...                 bracket: [...]
    #   .Brq ...                braces: {...}
    #   .Dq ...                 double quote: <lq><lq>...<rq><rq>
    #   .Eq XX YY ...           Enclose String: XX...YY
    #   .Pq XX ...              parenthesis: (...)
    #   .Ql ...                 Quoted literal: <lq>...<rq> or <tt>...<tt>
    #   .Qq ...                 Straight 2ble quote: "..."
    #   .Sq ...                 Single quote: <lq>...<rq>
    #

    my ($lq, $rq, $wc);
    $wc = 0;

    s/^\.//;

    if    (/^Aq$/)      { $lq = "<"; $rq = ">"; }
    elsif (/^Bq$/)      { $lq = "["; $rq = "]"; }
    elsif (/^Brq$/)     { $lq = "{"; $rq = "}"; }
    elsif (/^Dq$/)      { $lq = '@quotedblleft{}'; $rq = '@quotedblright{}'; }
    elsif (/^Eq$/)      { $lq = shift @words; $rq = shift @words; }
    elsif (/^Pq$/)      { $lq = "("; $rq = ")"; }
    elsif (/^Ql$/)      { $lq = '@quoteleft{}'; $rq = '@quoteright{}'; }
    elsif (/^Qq$/)      { $lq = '"'; $rq = '"'; }
    elsif (/^Sq$/)      { $lq = '@quoteleft{}'; $rq = '@quoteright{}'; }

    print "$lq";
    while ($_ = shift @words) {
        last if (/^[[:punct:]]$/);
        print " " if ($wc);
        if (/^(Ar|Cm|Fl|Ic|Li)$/)
        {
            Handle_ArCmFlIcLi();
        }
        else
        {
            s/^\\&//;
            print;
        }
        ++$wc;
    }
    print "$rq";
    while ($_ = shift @words)
    {
        print;
    }
    print "\n";
}

sub Handle_Sec
{
    # Usage: .Sh
    # Usage: .Ss
    #   .Sh word(s)
    #
    # Might be a quoted string.
    #
    # Drops an anchor.
    my ($a, $sh);

    $sh =~ /Sh/;

    parseQuote(\@words) if ($words[0] =~ /^"/);

    $a = $words[0];

    print '@node ', "$a\n";
    print '@', $sh ? "sub" : "", "section $a\n";
    print "@anchor{$a}\n";
}

sub Handle_Sx
{
    # Usage: .Sx <section reference> ...
    #   .Sh word(s)
    #
    # Might be a quoted string.
    #
    # References an anchor

    my ($a);

    parseQuote(\@words) if ($words[0] =~ /^"/);

    $a = $words[0];

    print '@ref{',"$a","}";
}

sub Handle_Ux
{
    # Usage: .Ux ...
    #   .Ux                     UNIX
    #   .Ux FOO                 FOO
    #
    my ($ux_name);
    if (@words == 0)
    {
        $ux_name = "UNIX";
    }
    else
    {
        $ux_name = shift @words;
    }

    print '@sc{',"$ux_name","}";
    while ($_ = shift @words)
    {
        print;
    }
    print "\n";
}

sub Handle_Xr
{
    # Usage: .Xr <man page name> [<section>] ...
    #   .Xr mdoc        mdoc
    #   .Xr mdoc ,      mdoc,
    #   .Xr mdoc 7      mdoc(7)
    #   .Xr xinit 1x ;  xinit(1x);
    #
    # Emitting things like @uref{/man.cgi/1/ls,,ls} would be OK,
    # but we'd have to allow for changing /man.cgi/ (at least).
    # I'm OK with:
    #   @code{mdoc}
    #   @code{mdoc},
    #   @code{mdoc(7)}
    #   @code{xinit(1x)};
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
    print '@code{',"$xr_cmd"    if (defined $xr_cmd);
    print "($xr_sec)"           if (defined $xr_sec);
    print "}"                   if (defined $xr_cmd);
    print "$xr_punc"            if (defined $xr_punc);
    print "\n";
}

sub parseQuote # ref to array of words
{
    my ($waref) = @_;   # word array reference
    my ($string);

    # Passing in "foo" will lose...

    $string = shift @{$waref};

    until ($string =~ /\"$/) {
        $string .= " ".shift @{$waref};
    }

    $string =~ s/^\"(.*)\"$/$1/;

    unshift @{$waref}, $string;
}

sub ParseMacro #line
{
    my ($line) = @_;

    @words = split(/\s+/, $line);
    $retval = '';

    # print('@words = ', scalar(@words), ': ', join(' ', @words), "\n");

    while ($_ = shift @words)
    {
        if    (/^\.An$/)                { Handle_An(); }
        elsif (/^\.Aq/)                 { Handle_Q(); }
        elsif (/^\.Ar$/)                { Handle_ArCmFlIcLi(); }
        elsif (/^\.Bl$/)                { Handle_Bl(); }
        elsif (/^\.Bq/)                 { Handle_Q(); }
        elsif (/^\.Brq/)                { Handle_Q(); }
        elsif (/^\.Cm$/)                { Handle_ArCmFlIcLi(); }
        elsif (/^\.D1/)                 { Handle_D(); }
        elsif (/^\.Dl/)                 { Handle_D(); }
        elsif (/^\.Dq/)                 { Handle_Q(); }
        elsif (/^\.El$/)                { Handle_El(); }
        elsif (/^\.Em$/)                { Handle_Em(); }
        elsif (/^\.Eq/)                 { Handle_Q(); }
        elsif (/^\.Fl$/)                { Handle_ArCmFlIcLi(); }
        elsif (/^\.Fn$/)                { Handle_Fn(); }
        elsif (/^\.Ic$/)                { Handle_ArCmFlIcLi(); }
        elsif (/^\.It$/)                { Handle_It(); }
        elsif (/^\.Nm$/)                { Handle_Nm(); }
        elsif (/^\.Op$/)                { Handle_Op(); }
        elsif (/^\.Pa$/)                { Handle_Pa(); }
        elsif (/^\.Pp$/)                { print "\n";  }
        elsif (/^\.Pq/)                 { Handle_Q(); }
        elsif (/^\.Ql/)                 { Handle_Q(); }
        elsif (/^\.Qq/)                 { Handle_Q(); }
        elsif (/^\.Sh/)                 { Handle_Sec(); } # Section Header
        elsif (/^\.Sq/)                 { Handle_Q(); }
        elsif (/^\.Ss/)                 { Handle_Sec(); } # Sub Section
        elsif (/^\.Sx/)                 { Handle_Sx(); } # Section xref
        elsif (/^\.Ux/)                 { Handle_Ux(); }
        elsif (/^\.Xc/)                 { $extArg = 0; }
        elsif (/^\.Xo/)                 { $extArg = 1; }
        elsif (/^\.Xr/)                 { Handle_Xr(); }
        else                            { print $_,"\n"; }
    }
}

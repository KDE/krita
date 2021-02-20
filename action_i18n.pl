#! /usr/bin/env perl
#
# SPDX-FileCopyrightText: 2004 Richard Evans <rich@ridas.com>
#
# SPDX-License-Identifier: GPL-3.0-or-later
#


sub usage
{
  warn <<"EOF";

extractrc [flags] filenames

This script extracts messages from designer (.ui) and XMLGUI (.rc) files and
writes on standard output (usually redirected to rc.cpp) the equivalent
i18n() calls so that xgettext can parse them.

--tag=name        : Also extract the tag name(s). Repeat the flag to specify
                    multiple names: --tag=tag_one --tag=tag_two

--tag-group=group : Use a group of tags - uses 'default' if omitted.
                    Valid groups are: @{[TAG_GROUPS()]}

--context=name    : Give i18n calls a context name: i18nc("name", ...)
--lines           : Include source line numbers in comments (deprecated, it is switched on by default now)
--cstart=chars    : Start of to-EOL style comments in output, defaults to //
--language=lang   : Create i18n calls appropriate for KDE bindings
                    in the given language. Currently known languages:
                    C++ (default), Python
--ignore-no-input : Do not warn if there were no filenames specified
--help|?          : Display this summary
--no-unescape-xml : Don't do xml unescaping

EOF

  exit;
}

###########################################################################################

use strict;
use warnings;
use Getopt::Long;
use Data::Dumper; # Provides debugging command: print Dumper(\%hash);

use constant TAG_GROUP =>
{
  default => "[tT][eE][xX][tT]|title|string|whatsthis|toolTip|label",
  koffice => "Example|GroupName|Text|Comment|Syntax|TypeName",
  krita   => "[tT][eE][xX][tT]|title|string|whatsThis|toolTip|iconText",
  none    => "",
};

use constant TAG_GROUPS => join ", ", map "'$_'", sort keys %{&TAG_GROUP};

# Specification to extract nice element-context for strings.
use constant CONTEXT_SPEC =>
{
  # Data structure: extension => {tag => [ctxlevel, [attribute, ...]], ...}
  # Order of attributes determines their order in the extracted comment.
  "ui" => {
    "widget" => [10, ["class", "name"]],
    "item" => [15, []],
    "property" => [20, ["name"]],
    "attribute" => [20, ["name"]],
  },
  "rc" => {
    "Menu" => [10, ["name"]],
    "ToolBar" => [10, ["name"]],
  },
  "kcfg" => {
    "group" => [10, ["name"]],
    "entry" => [20, ["name"]],
    "whatsthis" => [30, []],
    "tooltip" => [30, []],
    "label" => [30, []],
  },
  "action" => {
      "ActionCollection" => [10, ["name"]],
      "Actions" => [20, ["category"]],
      "Action" => [30, ["name"]],
  }
};

# Specification to exclude strings by trailing section of element-context.
use constant CONTEXT_EXCLUDE =>
[
    # Data structure: [[tag, attribute, attrvalue], [...]]
    # Empty ("") attribute means all elements with given tag,
    # empty attrvalue means element with given tag and attribute of any value.
    [["widget", "class", "KFontComboBox"], ["item", "", ""], ["property", "", ""]],
    [["widget", "class", "KPushButton"], ["attribute", "name", "buttonGroup"]],
    [["widget", "class", "QRadioButton"], ["attribute", "name", "buttonGroup"]],
    [["widget", "class", "QToolButton"], ["attribute", "name", "buttonGroup"]],
    [["widget", "class", "QCheckBox"], ["attribute", "name", "buttonGroup"]],
    [["widget", "class", "QPushButton"], ["attribute", "name", "buttonGroup"]],
    [["widget", "class", "KTimeZoneWidget"], ["property", "name", "text"]],
];

# The parts between the tags of the extensions will be copied verbatim
# Same data structure as in CONTEXT_EXCLUDE, but per extension.
my %EXTENSION_VERBATIM_TAGS = (
       "kcfg" => [["code", "", ""], ["default", "code", "true"],
                  ["min", "code", "true"], ["max", "code", "true"]],
     );

# Add attribute lists as hashes, for membership checks.
for my $ext ( keys %{&CONTEXT_SPEC} ) {
  for my $tag ( keys %{CONTEXT_SPEC->{$ext}} ) {
    my $arr = CONTEXT_SPEC->{$ext}{$tag}[1];
    CONTEXT_SPEC->{$ext}{$tag}[2] = {map {$_ => 1} @{$arr}};
  }
}

###########################################################################################
# Add options here as necessary - perldoc Getopt::Long for details on GetOptions

GetOptions ( "tag=s"       => \my @opt_extra_tags,
             "tag-group=s" => \my $opt_tag_group,
             "context=s"   => \my $opt_context,       # I18N context
             "lines"       => \my $opt_lines,
             "cstart=s"    => \my $opt_cstart,
             "language=s"  => \my $opt_language,
             "ignore-no-input" => \my $opt_ignore_no_input,
             "no-unescape-xml" => \my $opt_no_unescape_xml,
             "help|?"      => \&usage );

unless( @ARGV )
{
  warn "No filename specified" unless $opt_ignore_no_input;
  exit;
}

$opt_tag_group ||= "default";

die "Unknown tag group: '$opt_tag_group', should be one of " . TAG_GROUPS
    unless exists TAG_GROUP->{$opt_tag_group};

my $tags = TAG_GROUP->{$opt_tag_group};
my $extra_tags  = join "", map "|" . quotemeta, @opt_extra_tags;
my $text_string = qr/($tags$extra_tags)( [^>]*)?>/;    # Precompile regexp
my $cstart = $opt_cstart; # no default, selected by language if not given
my $language = $opt_language || "C++";
my $context_known_exts = join "|", keys %{&CONTEXT_SPEC};

###########################################################################################

# Unescape basic XML entities.
sub unescape_xml ($) {
    my $text = shift;

    if (not $opt_no_unescape_xml) {
        $text =~ s/&lt;/</g;
        $text =~ s/&gt;/>/g;
        $text =~ s/&amp;/&/g;
        $text =~ s/&quot;/"/g;
    }

    return $text;
}

# Convert uic to C escaping.
sub escape_uic_to_c ($) {
    my $text = shift;

    $text = unescape_xml($text);

    $text =~ s/\\/\\\\/g; # escape \
    $text =~ s/\"/\\\"/g; # escape "
    $text =~ s/\r//g; # remove CR (Carriage Return)
    $text =~ s/\n/\\n\"\n\"/g; # escape LF (Line Feed). uic also change the code line at a LF, we do not do that.

    return $text;
}

###########################################################################################

sub dummy_call_infix {
    my ($cstart, $stend, $ctxt, $text, @cmnts) = @_;
    for my $cmnt (@cmnts) {
        print qq|$cstart $cmnt\n|;
    }
    if (defined $text) {
        $text = escape_uic_to_c($text);
        if (defined $ctxt) {
            $ctxt = escape_uic_to_c($ctxt);
            print qq|i18nc("$ctxt", "$text")$stend\n|;
        } else {
            print qq|i18n("$text")$stend\n|;
        }
    }
}

my %dummy_calls = (
    "C++" => sub {
        dummy_call_infix($cstart || "//", ";", @_);
    },
    "Python" => sub {
        dummy_call_infix($cstart || "#", "", @_);
    },
);

die "unknown language '$language'" if not defined $dummy_calls{$language};
my $dummy_call = $dummy_calls{$language};

# Program start proper - outer loop runs once for each file in the argument list.
for my $file_name ( @ARGV )
{
  my $fh;

  unless ( open $fh, "<", $file_name )
  {
    # warn "Failed to open: '$file_name': $!";
    next;
  }

  # Ready element-context extraction.
  my $context_ext;
  my $context_string;  # Regexp used to validate context
  if ( $file_name =~ /\.($context_known_exts)(\.(in|cmake))?$/ ) {
    $context_ext = $1;
    my $context_tag_gr = join "|", keys %{CONTEXT_SPEC->{$context_ext}};
    $context_string = qr/($context_tag_gr)( [^>]*)?>/; # precompile regexp
  }

  my $string          = "";
  my $origstring      = "";
  my $in_text         = 0;   # Are we currently inside a block of raw text?
  my $start_line_no   = 0;
  my $in_skipped_prop = 0;   # Are we currently inside XML property that shouldn't be translated?
  my $tag = "";
  my $attr = "";
  my $context = "";
  my $notr = "";

  # Element-context data: [[level, tag, [[attribute, value], ...]], ...]
  # such that subarrays are ordered increasing by level.
  my @context = ();

  # All comments to pending dummy call.
  my @comments = ();

  # Begin looping through the file
  while ( <$fh> )
  {
     # If your Perl is a bit rusty: $. is the current line number
     # Also, =~ and !~ are pattern-matching operators. :)
     if ( $. == 1 and $_ !~ /^(?:<!DOCTYPE|<\?xml|<!--|<ui version=)/ )
     {
       print STDERR "Warning: $file_name does not have a recognised first line and texts won't be extracted\n";
       last;
     }

     chomp;

     $string .= "\n" . $_;
     $origstring = $string;

     # 'database', 'associations', 'populationText' and 'styleSheet' properties contain strings that shouldn't be translated
     if ( $in_skipped_prop == 0 and $string =~ /<property name=\"(?:database|associations|populationText|styleSheet)\"/ )
     {
       $in_skipped_prop = 1;
     }
     elsif ( $in_skipped_prop and $string =~ /<\/property/ )
     {
       $string          = "";
       $in_skipped_prop = 0;
     }

     $context = $opt_context unless $in_text;
     $notr = "" unless $in_text;

     # print "context = " . $opt_context . "\n";

     unless ( $in_skipped_prop or $in_text )
     {
       # Check if this line contains context-worthy element.
       if (    $context_ext
           and ( ($tag, $attr) = $string =~ /<$context_string/ ) # no /o here
           and exists CONTEXT_SPEC->{$context_ext}{$tag} )
       {
         my @atts;
         for my $context_att ( @{CONTEXT_SPEC->{$context_ext}{$tag}[1]} )
         {
           if ( $attr and $attr =~ /\b$context_att\s*=\s*(["'])([^"']*?)\1/ )
           {
             my $aval = $2;
             push @atts, [$context_att, $aval];
           }
         }
         # Kill all tags in element-context with level higher or equal to this,
         # and add it to the end.
         my $clevel = CONTEXT_SPEC->{$context_ext}{$tag}[0];
         for ( my $i = 0; $i < @context; ++$i )
         {
           if ( $clevel <= $context[$i][0] )
           {
             @context = @context[0 .. ($i - 1)];
             last;
           }
         }
         push @context, [$clevel, $tag, [@atts]];
       }

       if ( ($tag, $attr) = $string =~ /<$text_string/o )
       {
         my ($attr_comment) = $attr =~ /\bcomment=\"([^\"]*)\"/ if $attr;
         $context = $attr_comment if $attr_comment;
         my ($attr_context) = $attr =~ /\bcontext=\"([^\"]*)\"/ if $attr;
         $context = $attr_context if $attr_context;
         # It is unlikely that both attributes 'context' and 'comment'
         # will be present, but if so happens, 'context' has priority.
         my ($attr_extracomment) = $attr =~ /\bextracomment=\"([^\"]*)\"/ if $attr;
         push @comments, "i18n: $attr_extracomment" if $attr_extracomment;

         my ($attr_notr) = $attr =~ /\bnotr=\"([^\"]*)\"/ if $attr;
         $notr = $attr_notr if $attr_notr;

         my $nongreedystring = $string;
         $string        =~ s/^.*<$text_string//so;
         $nongreedystring  =~ s/^.*?<$text_string//so;
         if ($string cmp $nongreedystring)
         {
            print STDERR "Warning: Line $origstring in file $file_name has more than one tag to extract on the same line, that is not supported by extractrc\n";
         }
         if ( not $attr or $attr !~ /\/ *$/ )
         {
           $in_text       =  1;
           $start_line_no =  $.;
         }
       }
       else
       {
         @comments = ();
         $string = "";
       }
     }

     next unless $in_text;
     next unless $string =~ /<\/$text_string/o;

     my $text = $string;
     $text =~ s/<\/$text_string.*$//o;

     if ( $text cmp "" )
     {
       # See if the string should be excluded by trailing element-context.
       my $exclude_by_context = 0;
       my @rev_context = reverse @context;
       for my $context_tail (@{&CONTEXT_EXCLUDE})
       {
         my @rev_context_tail = reverse @{$context_tail};
         my $i = 0;
         $exclude_by_context = (@rev_context > 0 and @rev_context_tail > 0);
         while ($i < @rev_context and $i < @rev_context_tail)
         {
           my ($tag, $attr, $aval) = @{$rev_context_tail[$i]};
           $exclude_by_context = (not $tag or ($tag eq $rev_context[$i][1]));
           if ($exclude_by_context and $attr)
           {
             $exclude_by_context = 0;
             for my $context_attr_aval (@{$rev_context[$i][2]})
             {
               if ($attr eq $context_attr_aval->[0])
               {
                 $exclude_by_context = $aval ? $aval eq $context_attr_aval->[1] : 1;
                 last;
               }
             }
           }
           last if not $exclude_by_context;
           ++$i;
         }
         last if $exclude_by_context;
       }

       if (($context and $context eq "KDE::DoNotExtract") or ($notr eq "true"))
       {
         push @comments, "Manually excluded message at $file_name line $.";
       }
       elsif ( $exclude_by_context )
       {
         push @comments, "Automatically excluded message at $file_name line $.";
       }
       else
       {
         # Write everything to file
         (my $clean_file_name = $file_name) =~ s/^\.\///;
         push @comments, "i18n: file: $clean_file_name:$.";
         if ( @context ) {
           # Format element-context.
           my @tag_gr;
           for my $tgr (reverse @context)
           {
             my @attr_gr;
             for my $agr ( @{$tgr->[2]} )
             {
               #push @attr_gr, "$agr->[0]=$agr->[1]";
               push @attr_gr, "$agr->[1]"; # no real need for attribute name
             }
             my $attr = join(", ", @attr_gr);
             push @tag_gr, "$tgr->[1] ($attr)" if $attr;
             push @tag_gr, "$tgr->[1]" if not $attr;
           }
           my $context_str = join ", ", @tag_gr;
           push @comments, "i18n: context: $context_str";
         }
         push @comments, "xgettext: no-c-format" if $text =~ /%/o;
         $dummy_call->($context, $text, @comments);
         @comments = ();
       }
     }
     else
     {
       push @comments, "Skipped empty message at $file_name line $.";
     }

     $string  =~ s/^.*<\/$text_string//o;
     $in_text =  0;

     # Text can be multiline in .ui files (possibly), but we warn about it in XMLGUI .rc files.

     warn "there is <text> floating in: '$file_name'" if $. != $start_line_no and $file_name =~ /\.rc$/i;
  }

  close $fh or warn "Failed to close: '$file_name': $!";

  die "parsing error in $file_name" if $in_text;

  if ($context_ext && exists $EXTENSION_VERBATIM_TAGS{$context_ext})
  {
    unless ( open $fh, "<", $file_name )
    {
      # warn "Failed to open: '$file_name': $!";
      next;
    }

    while ( <$fh> )
    {
      chomp;
      $string .= "\n" . $_;

      for my $elspec (@{ $EXTENSION_VERBATIM_TAGS{$context_ext} })
      {
        my ($tag, $attr, $aval) = @{$elspec};
        my $rx;
        if ($attr and $aval) {
            $rx = qr/<$tag[^<]*$attr=["']$aval["'][^<]*>(.*)<\/$tag>/s
        } elsif ($attr) {
            $rx = qr/<$tag[^<]*$attr=[^<]*>(.*)<\/$tag>/s
        } else {
            $rx = qr/<$tag>(.*)<\/$tag>/s
        }
        if ($string =~ $rx)
        {
          # Add comment before any line that has an i18n substring in it.
          my @matched = split /\n/, $1;
          my $mlno = $.;
          (my $norm_fname = $file_name) =~ s/^\.\///;
          for my $mline (@matched) {
            # Assume verbatim code is in language given by --language.
            # Therefore format only comment, and write code line as-is.
            if ($mline =~ /i18n/) {
              $dummy_call->(undef, undef, ("i18n: file: $norm_fname:$mlno"));
            }
            $mline = unescape_xml($mline);
            print "$mline\n";
            ++$mlno;
          }
          $string = "";
        }
      }
    }

    close $fh or warn "Failed to close: '$file_name': $!";
  }
}

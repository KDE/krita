#!/usr/bin/perl -w
# $Id$
# This file is part of the KDE project
# Copyright (C) 2001 Daniel Naber <daniel.naber@t-online.de>

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

# Extract information from WordNet data files - only useful for development.
# cat together all of WordNet's data.* files and call this script. As a second
# argument you can use a word frequency list as "Alphabetical frequency list of 
# the whole corpus (lemmatized)" on http://www.comp.lancs.ac.uk/ucrel/bncfreq/flists.html
# This will remove all words whcih are not in the list, i.e. words that are rare.
# 
# Output of this script is:
# ;syn1;syn2;...;#;hyper1;hyper2;...;

# TODO:
# -"close" and "closely" are synonym which can irritate people!?
# -document this + clean up a bit

use strict;

sub prg()
{

    my $prefix = "";
    # TODO?: prefix (n,v,a or r) per line won't work as each word
    # can have a different prefix (it's rare but it happens)
    
	my $filename = $ARGV[0];
	my $filename_stat = $ARGV[1];
	if( ! $filename ) {
		print "Usage: $0 <wordnet_file> [occurence_statistics_file]\n";
		exit;
	}

	# read word frequency statistics to later filter out uncommon words:
	# format:
	#        abuse   Verb    %       12      97      0.92
	#        @       @       abuse   2       82      0.91
	# lines with "@" are non-lemmatized word forms
	
	my %stats;		# (lemmatized_term,occurences_in_1_million_words)
	if( $filename_stat ) {
		open(IN, "<$filename_stat") || die "Cannot open '$filename_stat': $!";
		while(<IN>) {
			my $line = $_;
			my @ar = split(/\t/, $line);
			if( $ar[1] eq '@' ) {
    			#print STDERR "$ar[3] -- $ar[4]\n";
    			$stats{lc($ar[1])} = $ar[4];
			} else {
    			#print "$ar[1] -- $ar[4]\n";
    			$stats{lc($ar[1])} = $ar[4];
            }
		}
		close(IN);
	}
	
	# build hashtable of synsets:
	open(IN, "<$filename") || die "Cannot open '$filename': $!";
	my %data;
	while(<IN>) {
		my $line = $_;
		next if( $line =~ m/^  / );	# copyright notice
		my @parts = split(/\s+/, $line);
		my $sysnset = "";
        my $occurences = hex($parts[3]);
		if( $occurences > 10 ) {
			#print STDERR "** $occurences synonyms ($line)\n";
		}
		for(my $i = 0; $i < 2*$occurences; $i += 2) {
			my $syn = $parts[4+$i];
			if( ! $syn ) {
				next;
			}
			$sysnset .= $syn.";";
		}
		$data{$parts[0]} = $sysnset;
	}
	close(IN);

print <<__EOF;
  1 This software and database is being provided to you, the LICENSEE, by  
  2 Princeton University under the following license.  By obtaining, using  
  3 and/or copying this software and database, you agree that you have  
  4 read, understood, and will comply with these terms and conditions.:  
  5   
  6 Permission to use, copy, modify and distribute this software and  
  7 database and its documentation for any purpose and without fee or  
  8 royalty is hereby granted, provided that you agree to comply with  
  9 the following copyright notice and statements, including the disclaimer,  
  10 and that the same appear on ALL copies of the software, database and  
  11 documentation, including modifications that you make for internal  
  12 use or for distribution.  
  13   
  14 WordNet 1.7 Copyright 2001 by Princeton University.  All rights reserved.  
  15   
  16 THIS SOFTWARE AND DATABASE IS PROVIDED "AS IS" AND PRINCETON  
  17 UNIVERSITY MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR  
  18 IMPLIED.  BY WAY OF EXAMPLE, BUT NOT LIMITATION, PRINCETON  
  19 UNIVERSITY MAKES NO REPRESENTATIONS OR WARRANTIES OF MERCHANT-  
  20 ABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE USE  
  21 OF THE LICENSED SOFTWARE, DATABASE OR DOCUMENTATION WILL NOT  
  22 INFRINGE ANY THIRD PARTY PATENTS, COPYRIGHTS, TRADEMARKS OR  
  23 OTHER RIGHTS.  
  24   
  25 The name of Princeton University or Princeton may not be used in  
  26 advertising or publicity pertaining to distribution of the software  
  28 and/or database.  Title to copyright in this software, database and  
  29 any associated documentation shall at all times remain with  
  30 Princeton University and LICENSEE agrees to preserve same.  
__EOF
	# for each synset, find its hypernyms:
	open(IN, "<$filename") || die "Cannot open '$filename': $!";
	while(<IN>) {
		my $line = $_;
		next if( $line =~ m/^  / );	# copyright notice
		my @parts = split(/\s+/, $line);
		my $pos = 0;
        my $occurences = hex($parts[3]);
        #print STDERR "# $parts[4] $occurences\n";
		my $line_result = ""; 
		my $occurs = 0;     # how often does it occur in texts according to the statistics?
		for(my $i = 0; $i < 2*$occurences; $i += 2) {
			my $syn = $parts[4+$i];
			if( ! $syn ) {
				next;
			}
			$line_result .= $syn.";";
			#print "##".$syn."\n";
			#print "$syn occurs: ".$stats{$syn}."\n";
			if( $filename_stat && $stats{lc($syn)} ) {
				$occurs += $stats{lc($syn)};
			}
		}
        $line_result =~ s/_/ /g;
		if( $filename_stat ) {
			if( $occurs > 0 ) {		# occurences of all synonyms together
				$line_result = "$prefix;$line_result";
			} else {
				next;
			}
		} else {
			$line_result = "$prefix;$line_result";
        }
		my $ct = 0;
		my $hyper = "";
		#print "**$line:\n";
        # walk through the hypernyms (resp "similar" for adjectives):
		while( $line =~ m!(\@|\&) (.*?) (.*?) (.*?)!ig ) {
			#print "@ $1 $2\n";
			if( $data{$2} ) {
                #print "OK\n";
				$hyper .= $data{$2};
				$ct++;
			}
		}
        #print "syns: $occurences, hypers: $ct\n";
		if($ct > 0) {
            $hyper =~ s/_/ /g;
   			$line_result .= "#;$hyper\n";
		} else {
   			$line_result .= "#\n";
		}
        if( $occurences > 1 || $ct > 0 ) {
            $line_result =~ s/\((a|p|ip)\)//igs;  # attributive or prenomial use isn't so interesting
			print "$line_result";
        }
	}
	close(IN);
}

prg();

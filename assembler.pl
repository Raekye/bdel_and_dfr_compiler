#!/usr/bin/perl

use strict;
use warnings;
use Devel::StackTrace;

exit main();

sub main {
	my (@files) = @ARGV;
	my @all_lines = <>;
=begin
	foreach my $f (@files) {
		load_file($f, \@all_lines);
	}
=end
=cut
	my @filtered_lines = ();
	my $labels = {};
	my $count = 0;
	foreach my $l (@all_lines) {
		$l =~ s/#.*$//;
		my @parts = split(/\s+/, $l);
		if (@parts) {
			if ($parts[0] =~ /([a-zA-Z_]\w*):/) {
				$labels->{$1} = $count;
			} else {
				push @filtered_lines, \@parts;
				$count++;
			}
		}
	}
	process_code(\@filtered_lines, $labels);
	return 0;
}

sub load_file {
	open (my $f, '<', $_[0]) or die "Failed to open file \"$_[0]\"";
	my @lines = <$f>;
	close $f;
	chomp(@lines);
	push @{$_[1]}, @lines;
}

sub process_code {
	my $lines = $_[0];
	my $i = 0;
	foreach my $l (@$lines) {
		process_instruction($i, $l->[0], [@$l[1..@$l - 1]], $_[1]);
		$i++;
	}
}

sub process_instruction {
	my $pos = $_[0];
	my $opcode = $_[1];
	my $args = $_[2];
	my $labels = $_[3];
	# Yes I know I just joined the line I split
	# I wanted to try out more perl stuff
	# e.g. slicing and passing an array ref, capturing matches
	my $instr = $opcode . ' ' . join(' ', @$args);
	my @match;
	if ($instr =~ /eof/) {
		print_binary_instruction(0);
	} elsif (@match = $instr =~ /(load) (\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($match[0]), 4, $match[1], 2, $match[2]);
	} elsif ($instr =~ /(store) r(\d+) (\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 4, $3);
	} elsif ($instr =~ /(literal) (-?\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 4, $2, 2, $3);
	} elsif ($instr =~ /(literal) :([a-zA-Z_]\w*) r(\d+)/) {
		if (exists($labels->{$2})) {
			print_binary_instruction(binary_opcode_from_string($1), 4, $labels->{$2}, 2, $3);
		} else {
			die "Unknown label \"$2\"";
		}
	} elsif ($instr =~ /(literal) "([a-z0-9 ])" r(\d+)/) {
		if ($2 eq " ") {
			print_binary_instruction(binary_opcode_from_string($1), 4, 36, 2, $3);
		} elsif (ord($2) - ord("a") < 0) {
			print_binary_instruction(binary_opcode_from_string($1), 4, $2, 2, $3);
		} else {
			print_binary_instruction(binary_opcode_from_string($1), 4, ord($2) - ord('a') + 10, 2, $3);
		}
	} elsif ($instr =~ /(input) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2);
	} elsif ($instr =~ /(output) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2);
	} elsif ($instr =~ /(add) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(sub) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(mul) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(div) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(cmp) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(branch) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2);
	} elsif ($instr =~ /(jump) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, 1, 2, $2);
	} elsif ($instr =~ /(jump) :([a-zA-Z_]\w*)/) {
		if (exists($labels->{$2})) {
			print_binary_instruction(binary_opcode_from_string($1), 2, 0, 4, $labels->{$2});
		} else {
			die "Unknown label \"$2\"";
		}
	} elsif ($instr =~ /(stack) (-?\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 4, $2);
	} elsif ($instr =~ /(supermandive)/) {
		print_binary_instruction(binary_opcode_from_string($1));
	} elsif ($instr =~ /(getup)/) {
		print_binary_instruction(binary_opcode_from_string($1));
	} elsif ($instr =~ /(draw) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(keyboard) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(heap) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3);
	} elsif ($instr =~ /(unheap) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3);
	} elsif ($instr =~ /(return) r(\d+) (\d+)/) {
		print_binary_instruction(binary_opcode_from_string("store"), 2, $2, 4, $3 + 15);
	} elsif ($instr =~ /(printhex) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(inc) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2);
	} elsif ($instr =~ /^(dec) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2);
	} elsif ($instr =~ /(keyhex) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(mov) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3);
	} elsif ($instr =~ /(printdec) r(\d+) r(\d+) r(\d+)/) {
		print_binary_instruction(binary_opcode_from_string($1), 2, $2, 2, $3, 2, $4);
	} elsif ($instr =~ /(die)/) {
		print_binary_instruction(binary_opcode_from_string($1));
	} else {
		die "Unknown instruction \"$instr\"";
	}
}

sub binary_opcode_from_string {
	my %map = (
		'eof' => 0,
		'load' => 1,
		'store' => 2,
		'literal' => 3,
		'input' => 4,
		'output' => 5,
		'add' => 6,
		'sub' => 7,
		'mul' => 8,
		'div' => 9,
		'cmp' => 10,
		'branch' => 11,
		'jump' => 12,
		'stack' => 13,
		'supermandive' => 14,
		'getup' => 15,
		'printhex' => 22,
		'draw' => 17,
		'keyboard' => 18,
		'heap' => 19,
		'unheap' => 20,
		'keyhex' => 21,
		'inc' => 23,
		'dec' => 24,
		'mov' => 25,
		'printdec' => 16,
		'die' => -1,
	);
	return $map{$_[0]};
}

sub print_binary_instruction {
	my $i = 2;
	print substr(sprintf('%02x', $_[0]), -2), ' ';
	shift(@_);
	my $width = 0;
	foreach (@_) {
		if ($width == 0) {
			$width = $_;
		} else {
			print substr(sprintf('%0' . $width . 'x', $_), -$width), ' ';
			$i += $width;
			$width = 0;
		}
	}
	foreach ($i..8 - 1) {
		print '0'
	}
	print("\n");
}

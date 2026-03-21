# Copyright 1999-2000 Microsoft Corporation

package MONITOR;          # The name of this module. For our purposes,
                          #  must be all in caps.

	
    # Hash of defined subroutines. Change value to 1 for every routine that
    #  you define a custom action for.

    use strict;           # Keep this code clean
	my(%SecRef);
    my($Version) = "5.1.2199.0"; # Version of this file
	my(@lines);
    sub TRUE  { return(1); }
    sub FALSE { return(0); }

1;

# Subroutine required to be defined
sub PrintHeader {        return(-1);}

# Subroutine required to be defined
sub PrintDetails {       return(-1);}

#-------------------------------------------------------------------------------------------------#
# Required subroutine! Used to verify a Directive as being device specific.                       #
#-- Exists ---------------------------------------------------------------------------------------#
sub DCDirective {
    my($Directive) = $_[1];
    return(FALSE); # Only return 1 if validating the line
}

#-------------------------------------------------------------------------------------------------#
# Required subroutine! Allows us to verify that the module loaded correctly.                      #
#-------------------------------------------------------------------------------------------------#
sub Exists {
    print(STDERR "Module \"MONITOR\" Loaded\n");
	return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# REQUIRED- Entry point for Class specific checking                                               #
#-------------------------------------------------------------------------------------------------#
sub InfCheck {
	ClassInstall32("ClassInstall32", $SecRef{"CLASSINSTALL32"})
		if (defined($SecRef{"CLASSINSTALL32"}));
	ClassInstall32("ClassInstall32.NT", $SecRef{"CLASSINSTALL32.NT"})
		if (defined($SecRef{"CLASSINSTALL32.NT"}));
	ClassInstall32("ClassInstall32.NTX86", $SecRef{"CLASSINSTALL32.NTX86"})
		if (defined($SecRef{"CLASSINSTALL32.NTX86"}));
	return(TRUE);
}


#-------------------------------------------------------------------------------------------------#
# Required subroutine                                                                             #
#-------------------------------------------------------------------------------------------------#
sub InitGlobals {
    # Is called when module is first included.
    # Use it to set your per-inf variables.
	%SecRef = ChkInf::INFSections;
    @lines  = ChkInf::GetInfLines;
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# Verifies required regkeys in ClassInstall32 sections                                            #
#-------------------------------------------------------------------------------------------------#
sub ClassInstall32 {
    my($Section)= $_[0];
    shift;
    my($line)   = $_[0];
    shift;
    my($DONE)   = ChkInf::FALSE;
    my($Directive, $Value, @Sections);

    while (! $DONE) {
		$line++, next if (!defined($lines[$line]));
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }
        $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));
        $Value     = uc(ChkInf::GetDirectiveValue($line,$lines[$line]));

        if ($Directive eq "ADDREG") {
			@Sections = split(/,/,$Value);
			foreach (@Sections)	{
				s/^[\s\t]*//;
				s/[\s\t]*$//;
				CheckAddReg($_, $SecRef{uc($_)}, @lines);
			}
        }

      $line++;
    }
	ChkInf::SecChecked($Section);
    return(TRUE);

}

#-------------------------------------------------------------------------------------------------#
# Helper routine for ClassInstall32()                                                             #
#-------------------------------------------------------------------------------------------------#
sub CheckAddReg {
	my($Section)  = $_[0];
    my($Heading) = quotemeta("[" . $_[0] . "]");
    shift;
	my($line)    = $_[0];
	shift;
    my(@lines)   = @_;
    my($DONE)    = FALSE;
    my(@RegKey);
    my(@NeededKeys)= (0,0,0);
    my($REG);
    my($SectionStart);
    $SectionStart = $line-1;

    my($Pattern1) = quotemeta("HKR,,NoInstallClass,,1");
    my($Pattern2) = quotemeta("HKR,,Icon,,-1");
    my($Pattern3) = quotemeta("HKR,,,,");
    my($Quote);
    while (! $DONE) {

        $line++, next if (! defined($lines[$line]));
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }
        $REG  = $lines[$line];
        if ($lines[$line] =~ /__QUOTED_STR_\((\d*)\)__/) {
            $Quote  = "\"" . ChkInf::QuotedString($1) . "\"";
            $REG    =~ s/__QUOTED_STR_\((\d*)\)__/$Quote/;
        }
        $REG =~ s/\"//g;
        if ($REG =~ /^$Pattern1$/i) {
            $NeededKeys[0] = 1;
        } elsif ($REG =~ /^$Pattern2$/i) {
            $NeededKeys[1] = 1;
        } elsif ($REG =~ /^$Pattern3.*/i) {
            $NeededKeys[2] = 1;
        }
        $line++;
    }

    my($i);
    if (! $NeededKeys[0]) {
            ChkInf::AddDeviceSpecificError($SectionStart,3001,"Missing a required regkey! (\"HKR,,NoInstallClass,,1\")");
    }
    if (! $NeededKeys[1]) {
            ChkInf::AddDeviceSpecificError($SectionStart,3002,"Missing a required regkey! (\"HKR,,Icon,,-1\"");
    }
    if (! $NeededKeys[2]) {
            ChkInf::AddDeviceSpecificError($SectionStart,3003,"Missing a required regkey! (\"HKR,,,,%MonitorClassName%\")");
    }


	ChkInf::SecChecked($_[0]);
    return(TRUE);
}

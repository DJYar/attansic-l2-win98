# Copyright 1999-2000 Microsoft Corporation

# The name of this module. For our purposes it must be all in caps.
package IMAGE;

    use strict;                # Keep the code clean
    my($DEBUG) = 0;            # Set to 1 for debugging output

    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my($Version) = "5.1.2199.0"; # Version of this file
    my($Current_INF_File);     # Stores name of the current INF file.
    my($Current_HTM_File);     # Name of the output file to use.

    my(@Device_Class_Options); # Array to store device specific options.
    my(@InfLines);             # Array to hold the INF lines

    my(%AllModels);            # Hash of all models present in INF
    my(%SecRef);               # Hash poitning to section headers
    my(%StringVals);

    my(%ErrorTable) = (1999 => [" ERROR NUMS < 2000 ARE RESERVED "],
                       2001 => ["The value of %s should be %s."],
                       2002 => ["Section [%s] could not be found."],
                       2003 => ["%s is expected to %s."],
                       2004 => ["Too many values for directive %s"],
                       2005 => ["Unrecognized directive: %s"],
                       9999 => [" DO NOT USE THIS ERROR NUM "]);

    my(%WarningTable)=(1999 => [" WARNING NUMS < 2000 ARE RESERVED "],
                       3001 => ["\"VendorSetup\" may be included it for backward compatibility with Windows 98, but it has no effect on Windows 2000."],
                       9999 => [" DO NOT USE THIS WARNING NUM "]);

    #--- Helper Subroutines ---#
    sub TRUE  { return(1); }
    sub FALSE { return(0); }


#-------------------------------------------------------------------------------------------------#
#-- CheckModel($Model) ---------------------------------------------------------------------------#
sub CheckModel {
    my($Model) = uc($_[0]);
    my($line)  = $SecRef{$Model};
    my($DONE)  = FALSE;
    my(@lines) = @InfLines;
    my($Directive, @Values, $Value, $Temp);

    # Only need to check device specific directives. ChkInf already checked universal ones!
    my(%ImageDirectives) = ('SUBCLASS'         => "StillImage",
                            'DEVICETYPE'       => " ",
                            'CAPABILITIES'     => " ",
                            'EVENTS'           => "EventSectionName",
                            'DEVICEDATA'       => "DeviceDataSectionName",
                            'PROPERTYPAGES'    => " ",
                            'DEVICESUBTYPE'    => " ",
                            'CONNECTION'       => " ",
                            'VENDORSETUP'      => " ",
                            'ICMPROFILES'      => " ",
                            'UNINSTALLSECTION' => "UninstallSectionName");

    # print("\tModel $Model not defined!\n"),
    return(FALSE) if (!defined($SecRef{$Model}));
    print(STDERR "\tEntering CheckModel($Model)...\n") if ($DEBUG);

    while (! $DONE) {
        $line++, next if (!defined($lines[$line]));
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }
        ChkInf::GetLine($line) if ($DEBUG);

        $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));

        if (! defined($ImageDirectives{$Directive})) {
            $line++;
            next;
        }

        # Once we get here, we know $Directive is valid =-)
        if ($Directive eq "SUBCLASS") {
            $Value=uc(ChkInf::GetDirectiveValue($line, $lines[$line]));
            $Value =~ s/^\s*//i; # Kill leading spaces
            $Value =~ s/\s*$//i; # Kill trailing spaces
            AddError($line, 2001, "Subclass", "StillImage") unless ($Value eq "STILLIMAGE");

        } elsif ($Directive eq "DEVICETYPE") {
            $Value = uc(ChkInf::GetDirectiveValue($line, $lines[$line]));
            $Value =~ s/^\s*//i; # Kill leading spaces
            $Value =~ s/\s*$//i; # Kill trailing spaces
            AddError($line, 2001, "DeviceType", "1 or 2") unless (($Value == 1) || ($Value == 2));

        } elsif ($Directive eq "CAPABILITIES") {
            $Value=uc(ChkInf::GetDirectiveValue($line, $lines[$line]));
            $Value =~ s/^\s*//i; # Kill leading spaces
            $Value =~ s/\s*$//i; # Kill trailing spaces
            if ($Value =~ /^0x/i) {
                $Value =~ s/^(0X)//;
                $Value = hex($Value);
            }
            AddError($line, 2001, "Capabilities", "numeric") unless ($Value =~ /^\d*$/);

        } elsif ($Directive eq "EVENTS") {
            @Values = split(/,/,ChkInf::GetDirectiveValue($line, $lines[$line]));
            foreach $Value (@Values) {
                $Value = uc($Value);
                $Value =~ s/^\s*//;
                $Value =~ s/\s*$//;
                $Value = uc($Value);
                if (defined($SecRef{uc($Value)})) {
                    VerifyEvents($SecRef{$Value}, $Value);
                } else {
                    AddError($line, 2002, $Value);
                }
            }

        } elsif ($Directive eq "DEVICEDATA") {
            @Values = split(/,/,ChkInf::GetDirectiveValue($line, $lines[$line]));
            foreach $Value (@Values) {
                $Value = uc($Value);
                $Value =~ s/^\s*//;
                $Value =~ s/\s*$//;
                if (defined($SecRef{$Value})) {
                    VerifyDeviceData($SecRef{$Value}, $Value);
                } else {
                    AddError($line, 2002, $Value);
                }
            }

        } elsif ($Directive eq "PROPERTYPAGES") {
            $Value=uc(ChkInf::GetDirectiveValue($line, $lines[$line]));
            if ($Value =~ /__QUOTED_STR_\((\d*)\)__/i) {
                $Temp = ChkInf::QuotedString($1);
                $Value =~ s/__QUOTED_STR_\(\d*\)__/$Temp/i;
                $Temp = "";
            }
            @Values = split(/,/,$Value);
            $Values[0] =~ s/^\s*//;
            $Values[0] =~ s/\s*$//;
            if ($Values[0] !~ /[\.dll|\.cpl]$/i) {
                AddError($line, 2003, "Control_Panel_DLL", "end in .dll or .cpl");
            }
            if (defined($Values[3])) {
                AddError($line, 2004, "PropertyPages");
            }

        } elsif ($Directive eq "DEVICESUBTYPE") {
            $Value=uc(ChkInf::GetDirectiveValue($line, $lines[$line]));
            $Value =~ s/^\s*//i; # Kill leading spaces
            $Value =~ s/\s*$//i; # Kill trailing spaces
            if ($Value =~ /^0x/i) {
                $Value =~ s/^(0X)//;
                $Value = hex($Value);
            }
            AddError($line, 2001, "DeviceSubType", "numeric") unless ($Value =~ /^\d*$/);

        } elsif ($Directive eq "CONNECTION") {
            $Value = uc(ChkInf::GetDirectiveValue($line, $lines[$line]));
            $Value =~ s/^\s*//i; # Kill leading spaces
            $Value =~ s/\s*$//i; # Kill trailing spaces
            AddError($line, 2001, "Connection", "serial or parallel")
                unless (($Value eq "SERIAL") or ($Value eq "PARALLEL"));

        } elsif ($Directive eq "VENDORSETUP") {
            AddWarning($line, 3001);

        } elsif ($Directive eq "ICMPROFILES") {
            $Value=uc(ChkInf::GetDirectiveValue($line, $lines[$line]));
            $Value = ChkInf::QuotedString($1) if ($Value =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
            $Value =~ s/^\s*//i; # Kill leading spaces
            $Value =~ s/\s*$//i; # Kill trailing spaces
            AddError($line, 2001, "ICMProfiles", "&lt;ProfileName.ICM&gt;") unless ($Value =~ /\.ICM$/i);

        } elsif ($Directive eq "UNINSTALLSECTION") {
            @Values = split(/,/,ChkInf::GetDirectiveValue($line, $lines[$line]));
            foreach $Value (@Values) {
                $Value = uc($Value);
                $Value =~ s/^\s*//;
                $Value =~ s/\s*$//;
                if (defined($SecRef{$Value})) {
                    VerifyUninstallSection($SecRef{$Value}, $Value);
                } else {
                    AddError($line, 2002, $Value);
                }
            }

        } else {
            die("Unexpected error: IMAGE::CheckModule():Directive=$Directive\n");
        }

        @Values    = split(/,/,ChkInf::GetDirectiveValue($line, $lines[$line]));

        $line++;
    }
    print(STDERR "\tExiting CheckModel($Model)...\n") if ($DEBUG);
}

#---------------------------------------------------#
# Required subroutine! Used to verify a Directive as
# being device specific.
#---------------------------------------------------#
sub DCDirective {
    my($Directive) = uc($_[1]);

    my(%ValidDirectives) = ( "SUBCLASS"         => TRUE,
                             "DEVICETYPE"       => TRUE,
                             "CAPABILITIES"     => TRUE,
                             "EVENTS"           => TRUE,
                             "DEVICEDATA"       => TRUE,
                             "PROPERTYPAGES"    => TRUE,
                             "DEVICESUBTYPE"    => TRUE,
                             "CONNECTION"       => TRUE,
                             "VENDORSETUP"      => TRUE,
                             "ICMPROFILES"      => TRUE,
                             "UNINSTALLSECTION" => TRUE);

    return(FALSE) unless (defined($ValidDirectives{$Directive}));
    return(TRUE);
    # Only return True if the directive is valid
}


#---------------------------------------------------#
# Verifies that the module loaded correctly.  Be sure
# to change the string to the name of your module.
#---------------------------------------------------#
sub Exists {
    print(STDERR "Module \"IMAGE\" Loaded\n");
    return(1); # return 1 on success
}


#-------------------------------------------------------------------------------------------------#
#-- ExpandModelSection($line, $Section) ----------------------------------------------------------#
sub ExpandModelSection {
    my($line)    =  $SecRef{$_[1]};
    my($Section) =  $_[1];
    my(@lines)   =  @InfLines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($SWAP)    =  TRUE;
    my(%Models)  =  ();
    my($TempVar);

    my($Directive, @Values);

    while (! $DONE) {

        $line++, next if (!defined($lines[$line]));
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }
        ChkInf::GetLine($line) if ($DEBUG);

        $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));
        @Values    = split(/,/,ChkInf::GetDirectiveValue($line, $lines[$line]));

        $Values[0] =~ s/^\s*//i; # Kill leading spaces
        $Values[0] =~ s/\s*$//i; # Kill trailing spaces

        $SWAP=TRUE;

        # Substitue all string references in the lines
        while ($SWAP) {
            if ($Values[0] =~ /%(.*?)%/) {
                $TempVar = quotemeta($1);
                if (defined($StringVals{uc($TempVar)})) {
                    $Values[0] =~ s/%$TempVar%/$StringVals{uc($TempVar)}/;
                } else {
                    # Error if the %string% is not defined.
                    AddError(1025,$line,"%$TempVar%");
                    $Values[0] =~ s/%$TempVar%/$TempVar/;
                }
            } else {
                $SWAP = FALSE;
            }

        # If there's a QuotedString, swap it out for it's literal
        $Values[0] = ChkInf::QuotedString($1) if ($Values[0] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
        }

        if (defined($Values[0])) {
            if (defined($Values[1])) {
                $Models{$Values[0]} = $Values[1];
            } else {
                $Models{$Values[0]} = "UNKNOWN";
            }
        }
        $line++;
    }

    return(%Models);
}


#---------------------------------------------------#
# The module's main checking routine.
# This is your entry point for verifying an INF
#---------------------------------------------------#
sub InfCheck {
    @InfLines   = ChkInf::GetInfLines;
    %SecRef     = ChkInf::INFSections;
    %StringVals = ChkInf::StringVals;

    %AllModels  = LearnModels();

    # For every model, check the DDInstall section
    foreach (sort(keys(%AllModels))) {
        warn("\tUndefined model found in array!\n"), next if (!defined($AllModels{$_}));
        CheckModel($_) unless ($AllModels{$_} eq "DONE");
        $AllModels{$_} = "DONE";
    }
    return(1); # return 1 on success
}


#---------------------------------------------------#
# Is called when module is first included.
# Use it to set your per-inf variables.
#---------------------------------------------------#
sub InitGlobals {
    # First parameter is the name of the current INF.
    $Current_INF_File = $_[1];

    # Output file = htm subdir + ((INF Name - .inf) + .htm)
    $Current_HTM_File = $Current_INF_File;
    $Current_HTM_File =~ s/\.inf$/\.htm/i;
    $Current_HTM_File = "htm\\" . $Current_HTM_File;

    # Second parameter is a list of Device Specific options.
    my($DC_Specific_Options) = $_[2];

    # $DC_Specific_Options is a string that contains all device
    #   class options delimited by &'s
    if (defined($DC_Specific_Options)) {
        @Device_Class_Options = split(/&/,$DC_Specific_Options);
    } else {
        @Device_Class_Options = ("NULL","NULL");
    }

    # Array must be shifted since first element also had a & prepended
    shift(@Device_Class_Options);

    return(1); # return 1 on success
}


#-------------------------------------------------------------------------------------------------#
# LearnModels - Returns an array of all models listed in the INF                                  #
#-- LearnModels() --------------------------------------------------------------------------------#
sub LearnModels {
    warn("\tNo [Manufacturer] section found!\n"), return() if (! defined($SecRef{"MANUFACTURER"}));

    my($line)   = $SecRef{"MANUFACTURER"};
    my(@lines)  = @InfLines;
    my($DONE)   = FALSE;
    my($count)  = 0;

    my($Directive, $Value);
    my(%Models);

    print(STDERR "\tInvoking LearnModels...\n") if ($DEBUG);

    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/); # Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {           # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        ChkInf::GetLine($line) if ($DEBUG);

        # [Manufacturers] may not have both a directive & a value
        if ($lines[$line] =~ /=/) {
            $Value     = uc(ChkInf::GetDirectiveValue($line,$lines[$line]));
        } else {
            $Value    =   uc($lines[$line]); # Set Value to the name
            $Value    =~  s/^\"(.*)\"/$1/;   # And Strip the Quotes!
        }

        $Value =~ s/^\s*//; # Kill leading spaces
        $Value =~ s/\s*$//; # Kill trailing spaces

        if (defined($SecRef{uc($Value)})) {
            $count++; # Count models successfully being checked.

            # Add the models from $INFSections{$Value} to our running list
            %Models = (%Models, ExpandModelSection($SecRef{uc($Value)}, uc($Value), "TEST" ));
        }
        $line++;
    }
    print(STDERR "\tExiting LearnModels...\n") if ($DEBUG);
    return(%Models);
}

#---------------------------------------------------#
# Allows to add Device specific information to the
# top of the INF summary page.  The information here
# should be brief and link to detailed summaries
# below. (Which will be written using PrintDetials).
#---------------------------------------------------#
sub PrintHeader {
    return(1); # return 1 on success
}


#---------------------------------------------------#
# Allows addition of device specific results to the
# detail section on the INF summary.
#---------------------------------------------------#
sub PrintDetails {
    return(1); # return 1 on success
}

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                   HELPER SUBROUTINES                                          |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################

#-------------------------------------------------------------------------------------------------#
# This sub adds a new error to the list of errors for the file.                                   #
#-- AddError($ErrorNum, $LineNum, @ErrorArgs) ----------------------------------------------------#
sub AddError {
    my($LineNum) = $_[0], shift;
    my($ErrorNum)  = $_[0], shift;
    my(@ErrorArgs)= @_;

    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    $info_err = $ErrorTable{"$ErrorNum"};

    if ( !defined($info_err) ) {
        $this_str = "Unknown error $ErrorNum.";
    } else {
        $format_err = $$info_err[0];
        $this_str = sprintf($format_err, @ErrorArgs);
    }
    ChkInf::AddDeviceSpecificError($LineNum, $ErrorNum, $this_str);
}

#-------------------------------------------------------------------------------------------------#
# This sub adds a new warning to the list of warnings for the file.                               #
#-- AddWarning($WarnNum, $LineNum, @ErrorArgs) ---------------------------------------------------#
sub AddWarning {
    my($LineNum)  = $_[0], shift;
    my($WarnNum)  = $_[0], shift;
    my(@WarnArgs) = @_;

    $WarnArgs[0] = " " if (! defined($WarnArgs[0]));
    my($str, $this_str, $info_wrn, $format_wrn);

    $info_wrn = $WarningTable{"$WarnNum"};

    if ( !defined($info_wrn) ) {
        $this_str = "Unknown warning $WarnNum.";
    } else {
        $format_wrn = $$info_wrn[0];
        $this_str = sprintf($format_wrn, @WarnArgs);
    }
    ChkInf::AddDeviceSpecificWarning($LineNum, $WarnNum, $this_str);
}

#-------------------------------------------------------------------------------------------------#
# Verifies DEVICEDATA Sections                                                                    #
#-- VerifyDeviceData($linenum, $SecName) ---------------------------------------------------------#
sub VerifyDeviceData {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @InfLines;
    my($DONE)    =  FALSE;
    my($Directive, $Value);

    print(STDERR "\tBeginning VerifyDeviceData (@_)....\n") if ($DEBUG);
    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }
        ChkInf::GetLine($line) if ($DEBUG);
        $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));
        $Value     = uc(ChkInf::GetDirectiveValue($line,$lines[$line]));
        $line++;
    }

    ChkInf::SecChecked($_[1]) if (defined($SecRef{$_[1]}));

    print(STDERR "\tExiting VerifyDeviceData ...\n") if ($DEBUG);
    return(1);
}

#-------------------------------------------------------------------------------------------------#
# Verifies EVENTS Sections                                                                        #
#-- VerifyEvents($linenum, $SecName) -------------------------------------------------------------#
sub VerifyEvents {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @InfLines;
    my($DONE)    =  FALSE;
    my($Directive, @Values);

    print(STDERR "\tBeginning VerifyEvents (@_)....\n") if ($DEBUG);
    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        ChkInf::GetLine($line) if ($DEBUG);
        $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));
        @Values    = split(/,/,uc(ChkInf::GetDirectiveValue($line,$lines[$line])));

        AddError($line, 2003, "Directive", "have 3 parameters") unless (defined($Values[2]) );
        $line++;
    }

    ChkInf::SecChecked($Section) if (defined($SecRef{$Section}));

    print(STDERR "\tExiting VerifyEvents ...\n") if ($DEBUG);
    return(1);
}

#-------------------------------------------------------------------------------------------------#
# Verifies UNINSTALLSECTION Sections                                                              #
#-- VerifyUninstallSectionEvents($linenum, $SecName) ---------------------------------------------#
sub VerifyUninstallSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @InfLines;
    my($DONE)    =  FALSE;
    my($Directive, $Value);

    my(%Directives) =  ("DELFILES" => TRUE,
                        "DELREG"   => TRUE);

    print(STDERR "\tBeginning VerifyUninstallSection (@_)....\n") if ($DEBUG);
    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }
        ChkInf::GetLine($line) if ($DEBUG);
        $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));
        $Value     = uc(ChkInf::GetDirectiveValue($line,$lines[$line]));

        if (defined $Directives{$Directive}) {
            no strict;
            ChkInf->$Directive($line, $Value);
            use strict;
        } else {
            AddError($line, 2005, $Directive);
        }

        $line++;
    }

    ChkInf::SecChecked($Section) if (defined($SecRef{$Section}));

    print(STDERR "\tExiting VerifyUninstallSection ...\n") if ($DEBUG);
    return(1);
}

return(1); # Module must return 1 to let chkinf know it loaded correctly.

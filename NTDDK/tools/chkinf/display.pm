# (C) Copyright 1999-2000 Microsoft Corporation

package DISPLAY;

    use strict;                # Keep the code clean

    sub FALSE { return(0); }     # Utility subs
    sub TRUE  { return(1); }

    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my($Version) = "5.1.2199.0"; # Version of this file
    my($DEBUG)        = 0;       # Set to 1 for debugging output

    my(@InfLines);               # Array to hold the INF lines

    my($Current_INF_File);       # Stores name of the current INF file.
    my($Current_HTM_File);       # Name of the output file to use.
    my($inffile);                # Holds name of INF currently being processed.
    my($TotalLines);             # Total Lines in Parsed INF

    my(%AllModels);              # Array of all models present in INF (Model) = (Type)
    my(%SecRef);                 # Hash poitning to section headers
    my(%SecChecked);             # Hash to mark sections as checked.
    my(%StringVals);

    my(@Device_Class_Options);   # Array to store device specific options.


    my(%DisplayErrorTable) = (1035 => ["Identifier %s not defined."],
                              6001 => ["Directive %s invalid in this section."],
                              6002 => ["%s takes a single numeric parameter."],
							  6003 => ["The only values %s allows are 0 and 1."],
                            );

    my(%DisplayWarningTable)=(2011 => ["White spaces in identifier \"%s\"."],
                        );

#---------------------------------------------------#
# Routine for checking device class specific
# directives that occur in common sections verified
# by ChkINF.  Return 0 if directive is unknown or
# 1 if the directive is known (you should also verify
# it and write any appropriate warnings/errors.
#---------------------------------------------------#
sub DCDirective {
    return(0); # return 1 on success
}

#-------------------------------------------------------------------------------------------------#
# Main Entry Point to Device Specific Module. REQUIRED!                                           #
#-- InfCheck() -----------------------------------------------------------------------------------#
sub InfCheck {
    my($Item);

    print(STDERR "\tInvoking Display::InfCheck...\n") if ($DEBUG);

    # Propagate our private variables.
    @InfLines   = ChkInf::GetInfLines;
    %SecRef     = ChkInf::INFSections;
    %SecChecked = ChkInf::INFSections;
    %StringVals = ChkInf::StringVals;
    $TotalLines = @InfLines;

    # Mark All Sections as Unchecked by DISPLAY.PM
    foreach (keys(%SecChecked)) {
        $SecChecked{$_} = FALSE;
    }

    # Propagate a list of all models found in the INF
    %AllModels  = LearnModels();


    if (defined($SecRef{"GENERALCONFIGDATA"})) {
        # We should work with this && .NTALPHA if exists
        VerifyGCData($SecRef{"GENERALCONFIGDATA"},"","GENERALCONFIGDATA");
    }

    # Create an array for each model.
    # Array name is uc(Model) and first element is Model
    foreach (keys(%AllModels)) {
        $Item = uc($_);
        no strict; # Indirect ref's not valid when using strict
        $$Item[0] = $_;
        use strict;
        # Check the model
        CheckModel($Item);
    }
    print(STDERR "\tLeaving Display::InfCheck...\n") if ($DEBUG);
}

#---------------------------------------------------#
# Verifies that the module loaded correctly.  Be sure
# to change the string to the name of your module.
#---------------------------------------------------#
sub Exists {
    print(STDERR "Module \"DISPLAY\" Loaded\n");
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
        @Device_Class_Options = ("NULL", "NULL");
    }

    # Array must be shifted since first element also had a & prepended
    shift(@Device_Class_Options);

    return(1); # return 1 on success
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
#|                                    WORK SUBROUTINES                                           |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################

#-------------------------------------------------------------------------------------------------#
# CheckModel - Verifies an individual model                                                       #
#-- CheckModel($Model)  --------------------------------------------------------------------------#
sub CheckModel($Model) {
    my($Model) = $_[0];

    print(STDERR "\tInvoking Display::CheckModel...\n") if ($DEBUG);

    if (defined($SecRef{$Model.".SOFTWARESETTINGS"})) {
        # We should work with this && .NTALPHA if exists
        VerifySoftwareSettings($SecRef{$Model.".SOFTWARESETTINGS"},$Model,".SOFTWARESETTINGS");
    }

    if (defined($SecRef{$Model.".OPENGLSOFTWARESETTINGS"})) {
        # We should work with this && .NTALPHA if exists
        VerifySoftwareSettings($SecRef{$Model.".OPENGLSOFTWARESETTINGS"},$Model,".OPENGLSOFTWARESETTINGS");
    }

    if (defined($SecRef{$Model.".GENERALCONFIGDATA"})) {
        # We should work with this && .NTALPHA if exists
        VerifyGCData($SecRef{$Model.".GENERALCONFIGDATA"},$Model,".GENERALCONFIGDATA");
    }

    print(STDERR "\tLeaving Display::CheckModel...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# LearnModels - Returns an array of all models listed in the INF                                  #
#-- LearnModels() --------------------------------------------------------------------------------#
sub LearnModels {
    die("Can't proceed\n") if (! defined($SecRef{"MANUFACTURER"}));

    my($line)   = $SecRef{"MANUFACTURER"};
    my(@lines)  = @InfLines;
    my($DONE)   = FALSE;
    my($count)  = 0;

    my($Directive, $Value);
    my(%Models);

    print(STDERR "\tInvoking Display::ProcManu...\n") if ($DEBUG);

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
            %Models = (%Models, ExpandModelSection($SecRef{uc($Value)}, uc($Value) ));
        }
        $line++;
    }

    print(STDERR "\tLeaving Display::ProcManu...\n") if ($DEBUG);
    return(%Models);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyGCData($line, $Model, $Extension) ------------------------------------------------------#
sub VerifyGCData {
    my($line)       = $_[0];
    my($Model)      = $_[1];
    my($Extension)  = $_[2];
    my($Section)    = $_[1].$_[2];
    $Section =~ s/\s*$//; # Chop trailing spaces!

    my($DONE)       = FALSE;
    my(@lines)      = ChkInf::GetInfLines;
    my($Directive, @Values, $Value);

    my(%SecToCheck) = ("MAXIMUMDEVICEMEMORYCONFIGURATION", TRUE,
                       "MAXIMUMNUMBEROFDEVICES"          , TRUE,
                       "KEEPEXISTINGDRIVERENABLED"       , TRUE );


    my($CurrentLine, $Temp, @LineItems, $SizeOfArray);
    ChkInf::MarkChecked($Section);

    if ($SecChecked{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyGCData (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if (!defined($lines[$line]));
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            ChkInf::GetLine($line) if ($DEBUG);
            $Directive = ChkInf::GetDirective($line,$lines[$line]);
            @Values     = split(/,/, uc(ChkInf::GetDirectiveValue($line,$lines[$line])));


            if (uc($Directive) eq "MAXIMUMDEVICEMEMORYCONFIGURATION") {
                if (defined($Values[0])) {
                    if (defined($Values[1])) {
                        AddDisplayError(6002,$line,$Directive);
                    }
                } else {
                    AddDisplayError(6002,$line,$Directive);
                }
            } elsif (uc($Directive) eq "MAXIMUMNUMBEROFDEVICES") {
                if (defined($Values[0])) {
                    if (defined($Values[1])) {
                        AddDisplayError(6002,$line,$Directive);
                    }
                } else {
                    AddDisplayError(6002,$line,$Directive);
                }
            } elsif (uc($Directive) eq "KEEPEXISTINGDRIVERENABLED") {
                if (defined($Values[0])) {
                    if (defined($Values[1])) {
                        AddDisplayError(6002,$line,$Directive);
                    }
					if (($Values[0] != 1) and ($Values[0] != 0)) {
	                    AddDisplayError(6003,$line,$Directive);
					}
                } else {
                    AddDisplayError(6002,$line,$Directive);
                }
            } else {
                AddDisplayError(6001,$line,$Directive);
            }
            $line++;

        }
    }
    print(STDERR "\tExiting VerifyGCData ...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifySoftwareSettings($line, $Model, $Extension) --------------------------------------------#
sub VerifySoftwareSettings {
    my($line)       = $_[0];
    my($Model)      = $_[1];
    my($Extension)  = $_[2];
    my($Section)    = $_[1].$_[2];
    $Section =~ s/\s*$//; # Chop trailing spaces!

    my($DONE)       = FALSE;
    my(@lines)      = ChkInf::GetInfLines;
    my($Directive, @Values, $Value);

    my(%SecToCheck) = ("ADDREG",    TRUE,
                       "COPYFILES", TRUE,
                       "NEEDS",     TRUE);
    my($CurrentLine, $Temp, @LineItems, $SizeOfArray);
    ChkInf::MarkChecked($Section);

    if ($SecChecked{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyModel (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if (!defined($lines[$line]));
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            ChkInf::GetLine($line) if ($DEBUG);
            $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));
            @Values     = split(/,/, uc(ChkInf::GetDirectiveValue($line,$lines[$line])));


            if (uc($Directive) eq "ADDREG") {
                # ADDREG is valid, so call the ChkInf ADDREG checking routine
                ChkInf::ADDREG($line, uc(ChkInf::GetDirectiveValue($line,$lines[$line])));
            } elsif (uc($Directive) eq "DELREG") {
                # ADDREG is valid, so call the ChkInf ADDREG checking routine
                ChkInf::DELREG($line, uc(ChkInf::GetDirectiveValue($line,$lines[$line])));
            } else {
                AddDisplayError(6001,$line,$Directive);
            }
            $line++;

        }
    }
    print(STDERR "\tExiting VerifyModel ...\n") if ($DEBUG);
    return(TRUE);
}

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                   HELPER SUBROUTINES                                          |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################

#-------------------------------------------------------------------------------------------------#
# This sub adds a new error to the list of errors for the file.                                   #
#-- AddDisplayError($ErrorNum, $LineNum, @ErrorArgs) ---------------------------------------------#
sub AddDisplayError {
    my($ErrorNum) = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@ErrorArgs)= @_;

    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    $info_err = $DisplayErrorTable{"$ErrorNum"};

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
#-- AddDisplayWarning($WarnNum, $LineNum, @ErrorArgs) --------------------------------------------#
sub AddDisplayWarning {
    my($WarnNum)  = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@WarnArgs) = @_;

    $WarnArgs[0] = " " if (! defined($WarnArgs[0]));
    my($str, $this_str, $info_wrn, $format_wrn);

    $info_wrn = $DisplayWarningTable{"$WarnNum"};

    if ( !defined($info_wrn) ) {
        $this_str = "Unknown warning $WarnNum.";
    } else {
        $format_wrn = $$info_wrn[0];
        $this_str = sprintf($format_wrn, @WarnArgs);
    }
    ChkInf::AddDeviceSpecificWarning($LineNum, $WarnNum, $this_str);
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

    print(STDERR "\tInvoking Display::ExpandModelSection...\n") if ($DEBUG);

    while (! $DONE) {

        $line++, next if (!defined($lines[$line]));
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }
        ChkInf::GetLine($line) if ($DEBUG);

        $Directive = uc(ChkInf::GetDirectiveModel($line,$lines[$line]));
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
                    AddDisplayError(1035,$line,"%$TempVar%");
                    $Values[0] =~ s/%$TempVar%/$TempVar/;
                }
            } else {
                $SWAP = FALSE;
            }

        # If there's a QuotedString, swap it out for it's literal
        $Values[0] = ChkInf::QuotedString($1) if ($Values[0] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
        }

        if (defined($Values[0])) {
            $Models{$Values[0]} = $Values[1];
        }
        $line++;
    }

    print(STDERR "\tLeaving Display::ExpandModelSection...\n") if ($DEBUG);
    return(%Models);
}

return(1); # Module must return 1 to let chkinf know it loaded correctly.

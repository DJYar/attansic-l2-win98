# Copyright 1999-2000 Microsoft Corporation

package NET;                     # The name of this module. For our purposes,
                                 #   must be all in caps.

    use strict;                  # Keep this code clean
    my($Version) = "5.1.2199.0"; # Version of this file
    my($DEBUG)        = 0;       # Set to 1 for debugging output

    sub FALSE { return(0); } # Utility subs
    sub TRUE  { return(1); }

    #---------------------------------------------------#
    # Module level variables
    #---------------------------------------------------#
    my(@InfLines);        # Array to hold the INF lines
    my(@AddRegs);         # Array holds AddRegs for current model

    my($inffile);         # Holds name of INF currently being processed.
    my($TotalLines);      # Total Lines in Parsed INF

    my(%AllAddRegs);      # Array holds all AddReg lines, index by INF Line Number
    my(%AllModels);       # Array of all models present in INF (Model) = (Type)
    my(%SecRef);          # Hash poitning to section headers
    my(%SecChecked);      # Hash to mark sections as checked.
    my(%StringVals);
    my(%AddRegList);


    my(%NetCardErrorTable) = (
    3043 => ["Keyword Characteristics not defined in install section"],
    3044 => ["Keyword BusType not defined in install section"],
    3045 => ["Keyword Compatible not defined in Version section"],
    3046 => ["Keyword Compatible should be equal to 1 if Signature is \$CHICAGO\$"],
    4001 => ["BusType does not match the NetCard model."],
    4002 => ["Characteristics can have only one of NCF_VIRTUAL,NCF_PHYSICAL & NCF_WRAPPER."],
    4003 => ["NCF_NO_SERVICE cannot be used with NCF_VIRTUAL or NCF_PHYSICAL."],
    4005 => ["Invalid interface used in Addreg section"],
    4006 => ["Lower interface cannot have an install section"],
    4007 => ["Reserved Word used as Parameter Key"],
    4008 => ["Invalid Value-Name for Parameter Key"],
    4009 => ["NCF_HAS_UI should be one of the characteristics for advanced parameters"],
    4011 => ["The AddService ServiceName parameter must match the NDI\\Service entry in the Registry."],
    4030 => ["Section [%s] is empty. It should contain \"*PNP8132\"."],
    4031 => ["String \"*PNP8132\" is missing for section [%s]."],
    4032 => ["Wrong strings in section [%s]. Only \"*PNP8132\" is allowed."],
    4033 => ["Incorrect number of parameters, expected %s."],

    4100 => ["Invalid Priority Value for ConfigPriority"],
    4101 => ["Invalid Config Type for ConfigPriority"],
    4102 => ["Invalid IRQ value for IRQconfig"],
    4103 => ["The hexadecimal integer value exceeds range"],
    4104 => ["IRQConfig not assigned according to DDK specification"],
    4105 => ["IOConfig not assigned according to DDK specification"],
    4106 => ["Invalid DMA value for DMAconfig"],
    4107 => ["DMAConfig not assigned according to DDK specification"],
    4108 => ["MEMConfig not assigned according to DDK specification"],

    4200 => ["Invalid value for ServiceType in Services Install Section"],
    4201 => ["Invalid value for StartType in Services Install Section"],
    4202 => ["Invalid value for ErrorControl in Services Install Section"],
    4203 => ["ServiceBinary value not defined in Services Install Section"],
    4204 => ["Invalid value for LoadOrderGroup in Services Install Section"],
    4205 => ["Invalid value for StartName in Services Install Section"],
    4206 => ["Services section not defined for \"%s\" model"],
    4207 => ["\"%s\" file not allowed in CopyFiles section"],
    );

    my(%NetCardWarningTable) = (
    5001 => ["PnP id \"%s\" requires ExcludeFromSelect. Vendors or INF floppy install users may ignore this message."],
    4014 => ["First field should be one of the HKR, HKU, HKLM, HKCR or HKCU keywords."],
    );

    my(%ValidInterfaces) = ('ETHERNET' => 1, 'ATM' => 1, 'TOKENRING' => 1, 'SERIAL' => 1, 'FDDI' => 1,
                    'BASEBAND' => 1, 'BROADBAND' => 1,'ARCNET' => 1, 'ISDN' => 1, 'NOLOWER' => 1,
                    'NDIS5' => 1,'NDISATM' => 1,'NETBIOS' => 1, 'IPX' => 1, 'TDI' => 1,
                    'STREAMS' => 1, 'WINSOCK' => 1, 'WINNET5' => 1, 'NOUPPER' =>1, 'NDISWAN' => 1);

    my(%LoadOrderGroup) = (
    'SYSTEM BUS EXTENDER' => 1, 'SCSI MINIPORT' => 1,'PORT' => 1, 'PRIMARY DISK' => 1,
    'SCSI CLASS' => 1, 'SCSI CDROM CLASS' => 1, 'FILTER' => 1, 'BOOT FILE SYSTEM' => 1,
    'BASE' => 1, 'POINTER PORT' => 1, 'KEYBOARD PORT' => 1, 'POINTER CLASS' => 1,
    'KEYBOARD CLASS' => 1, 'VIDEO INIT' => 1, 'VIDEO' => 1, 'VIDEO SAVE' => 1,
    'FILE SYSTEM' => 1, 'EVENT LOG' => 1, 'STREAMS DRIVERS' => 1, 'PNP_TDI' => 1,
    'NDIS' => 1, 'TDI' => 1, 'NETBIOSGROUP' => 1, 'SPOOLERGROUP' => 1, 'NETDDEGROUP' => 1,
    'PARALLEL ARBITRATOR' => 1, 'EXTENDED BASE' => 1, 'REMOTEVALIDATION' => 1,
    'PCI CONFIGURATION' => 1, );

    my(%ReservedWords) = (
    'CHARACTERISTICS' => 1, 'COMPONENTID' => 1, 'DESCRIPTION' => 1, 'DRIVERDESC' => 1,
    'INFPATH' => 1, 'INFSECTION' => 1, 'INFSECTIONEXT' => 1, 'MANUFACTURER' => 1,
    'NETCFGINSTANCEID' => 1, 'PROVIDER' => 1, 'PROVIDERNAME' => 1,);

    my(%ParamKeys) = ( 'PARAMDESC' => 1, 'DEFAULT' =>1, 'TYPE' => 1, 'MIN' => 1, 'MAX' => 1,
                'STEP' => 1, 'BASE' => 1,'OPTIONAL' => 1, 'LIMITTEXT' => 1, 'UPPERCASE' => 1, 'FLAG' => 1,);

#Assuming unsigned 64-bit value and unsigned 32-bit value

1; # To tell require this file is okay.


###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                 INTERFACE SUBROUTINES                                         |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################
#-------------------------------------------------------------------------------------------------#
# Required subroutine! Used to verify a Directive as being device specific.                       #
#-- DCDirective ----------------------------------------------------------------------------------#
sub DCDirective {
    my($Directive) = $_[1];

    my(%ValidDirectives) = ( "CHARACTERISTICS"  => TRUE,
                             "ADAPTERMASK"      => TRUE,
                             "EISACOMPRESSEDID" => TRUE,
                             "BUSTYPE"          => TRUE,
                             "COMPATIBLE"       => TRUE);
    return(FALSE) unless (defined($ValidDirectives{$Directive}));
    return(TRUE);
}


#-------------------------------------------------------------------------------------------------#
# Required subroutine! Allows us to verify that the module loaded correctly.                      #
#-- Exists ---------------------------------------------------------------------------------------#
sub Exists {
    print(STDERR "Module \"NET\" Loaded\n");
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# Main Entry Point to Device Specific Module. REQUIRED!                                           #
#-- InfCheck() -----------------------------------------------------------------------------------#
sub InfCheck {
    my($Item);

    # Propagate our private variables.
    @InfLines   = ChkInf::GetInfLines;
    %SecRef     = ChkInf::INFSections;
    %SecChecked = ChkInf::INFSections;
    %StringVals = ChkInf::StringVals;
    $TotalLines = @InfLines;

    # Mark All Sections as Unchecked by NETCARD.PM
    foreach (keys(%SecChecked)) {
        $SecChecked{$_} = FALSE;
    }


    CheckCompatibleKey();

    # Propagate a list of all models found in the INF
    %AllModels  = LearnModels();


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
}

#-------------------------------------------------------------------------------------------------#
# Early initiation of module. REQUIRED!                                                           #
#-- InitGlobals($inffile) ------------------------------------------------------------------------#
sub InitGlobals {
    # Is called when module is first included.
    # Use it to set your per-inf variables.
    %AddRegList = ();
    $inffile = $_[1];
    return(1);
}

#-------------------------------------------------------------------------------------------------#
# REQUIRED!                                                                                       #
#-- PrintDetails() -------------------------------------------------------------------------------#
sub PrintDetails {
    # Allows addition of device specific results to the detail section on the
    # INF summary.
    return(1);
}

#-------------------------------------------------------------------------------------------------#
# REQUIRED!                                                                                       #
#-- PrintHeader() --------------------------------------------------------------------------------#
sub PrintHeader {
    # Allows to add Device specific information to the top of the INF summary
    # Page.  The information here should be brief and link to detailed summaries
    # below. (Which will be writting using PrintDetials.
    return(1);
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

    if (defined($SecRef{$Model.".NTX86"})||defined($SecRef{$Model.".NTALPHA"})) {
        # We should work with this && .NTALPHA if exists
        VerifyModel($SecRef{$Model.".NTX86"},   $Model, ".NTX86")
            if (defined($SecRef{$Model.".NTX86"}));
        VerifyModel($SecRef{$Model.".NTALPHA"}, $Model, ".NTALPHA")
            if (defined($SecRef{$Model.".NTALPHA"}));
    } elsif (defined($SecRef{$Model.".NT"})) {
        # We should work with this model!
        VerifyModel($SecRef{$Model.".NT"}, $Model, ".NT");
    } elsif (defined($SecRef{$Model})) {
        # Our last choice section since dual OS INF use this for 9x
        VerifyModel($SecRef{$Model}, $Model, " ");
    } else {
        # Model Must not exist
        # We trust that ChkInf.pm has already flagged all missing sections, so move on.
        return(FALSE);
    }

    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# LearnModels - Returns an array of all models listed in the INF                                  #
#-- LearnModels() --------------------------------------------------------------------------------#
sub LearnModels {
    return() if (! defined($SecRef{"MANUFACTURER"}));

    my($line)   = $SecRef{"MANUFACTURER"};
    my(@lines)  = @InfLines;
    my($DONE)   = FALSE;
    my($count)  = 0;

    my($Directive, $Value);
    my(%Models);

    print(STDERR "\tInvoking ProcManu...\n") if ($DEBUG);

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
    return(%Models);
}

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#|                                   HELPER SUBROUTINES                                          |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################

#-------------------------------------------------------------------------------------------------#
# This sub adds a new error to the list of errors for the file.                                   #
#-- AddNetCardError($ErrorNum, $LineNum, @ErrorArgs) -------------------------------------------------#
sub AddNetCardError {
    my($ErrorNum) = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@ErrorArgs)= @_;

    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    $info_err = $NetCardErrorTable{"$ErrorNum"};

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
#-- AddNetCardWarning($WarnNum, $LineNum, @ErrorArgs) ----------------------------------------------#
sub AddNetCardWarning {
    my($WarnNum)  = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@WarnArgs) = @_;

    $WarnArgs[0] = " " if (! defined($WarnArgs[0]));
    my($str, $this_str, $info_wrn, $format_wrn);

    $info_wrn = $NetCardWarningTable{"$WarnNum"};

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
                    AddNetCardError(1025,$line,"%$TempVar%");
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

    return(%Models);
}

#-------------------------------------------------------------------------------------------------#
#-- GetRegLines($Section) ------------------------------------------------------------------------#
sub GetRegLines {
    my($line)       = $SecRef{$_[0]};
    my($Section)    = $_[0];

    $Section =~ s/\s*$//; # Chop trailing spaces
    $Section =~ s/^\s*//; # Chop starting spaces

    my($DONE)       = FALSE;
    my(@lines)      = ChkInf::GetInfLines;
    my(@AddReg);

    print(STDERR "\tBeginning GetRegLines (@_)...\n") if ($DEBUG);

    while (! $DONE) {
        $line++, next if (!defined($lines[$line]));
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        ChkInf::GetLine($line) if ($DEBUG);
        @AddReg = (@AddReg, $lines[$line]);
        $AllAddRegs{$line} = 1;
        $line++;
    }
    print(STDERR "\tExiting GetRegLines ...\n") if ($DEBUG);
    return(@AddReg);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyModel($line, $Model, $Extension) -------------------------------------------------------#
sub VerifyModel {
    my($line)       = $_[0];
    my($Model)      = $_[1];
    my($Extension)  = $_[2];
    my($Section)    = $_[1].$_[2];
    $Section =~ s/\s*$//; # Chop trailing spaces!

    my($DONE)       = FALSE;
    my(@lines)      = ChkInf::GetInfLines;
    my($Directive, @Values, $Value);
    my($characteristics,$bustype,$cpfiles,$lnum);
    my($Charac,$Btype) = (0,0);
    my(@AddRegSections,$addregsec);

    my(%NetDirectives) = (  'CHARACTERISTICS'   => " ",
                            'BUSTYPE'           => " ");

    my(%SecToCheck) = ("ADDREG",    TRUE,
                       "COPYFILES", TRUE,
                       "NEEDS",     TRUE);
    my($CurrentLine, $Temp, @LineItems, $SizeOfArray);
    @AddRegs     =  ();

    if (!defined($SecRef{uc($Section . ".SERVICES")}))
    {
        AddNetCardError(4206,$SecRef{uc($Section)},$Section);
    }
    if ($SecChecked{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyModel (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if (!defined($lines[$line]));
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            ChkInf::GetLine($line) if ($DEBUG);
            $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));
            @Values     = split(/,/, uc(ChkInf::GetDirectiveValue($line,$lines[$line])));

            if ($Directive eq "ADDREG") {

                foreach $Value (@Values) {
                    $Value =~ s/^\s*//;
                    $Value =~ s/\s*$//;
                    if (defined($SecRef{$Value})) {
                        @AddRegs = (@AddRegs, GetRegLines($Value));
                        push(@AddRegSections,$Value);
                    } else {
                        # This error is flagged by ChkInf.pm
                        #print(STDERR "\t\tCould not find section \"$Value\"\n");
                    }
                }

            }
            #check nic's specific directives
            if ($Directive eq "CHARACTERISTICS") {
                if (defined($Values[0]))
                {
                    $characteristics = $Values[0];
                    $characteristics =~ s/^\s*//;
                    $characteristics =~ s/^0x//;
                    $characteristics =~ s/^0X//;
                    $characteristics =~ s/\s*$//;
                    CheckCharacteristics($characteristics,$Section,$line);
                    $Charac++;
                }
            }
            if ($Directive eq "BUSTYPE") {
                if (defined($Values[0]))
                {
                    $bustype = $Values[0];
                    $bustype =~ s/^\s*//;
                    $bustype =~ s/\s*$//;
                    CheckBusType($bustype,$Section,$line);
                    $Btype++;
                }
            }
            if ($Directive eq "COPYFILES") {
                if (defined($Values[0]))
                {
                    $cpfiles = $Values[0];
                    $cpfiles =~ s/^\s*//;
                    $cpfiles =~ s/\s*$//;
                    if ($cpfiles !~ /^@/ )
                    {
                        CheckCopyFiles($SecRef{uc($cpfiles)},$cpfiles);
                    }
                }
                else
                {
                    #Chkinf should flag an error if Value is missing
                }
            }

            $line++;
        }
        AddNetCardError(3043,$SecRef{uc($Section)}) unless ($Charac);
    if ($Btype == 0) 
    {
        if (defined($characteristics) && ($characteristics =~ /4$/)) # Bustype needed for physical adapter
        {
            AddNetCardError(3044,$SecRef{uc($Section)});
        }
    }


        $characteristics = 0 unless (defined($characteristics));
        foreach $addregsec (@AddRegSections) {
            AddRegProc($addregsec,$characteristics);
        }

        CheckServiceName($Section.".SERVICES") if (defined($SecRef{uc($Section.".SERVICES")}));

        # We now have one massive array called @AddRegs that contins all the AddReg lines for
        # the modem we're currently looking at.

        #### Accumulate data about this modem
        #
        # Net Team will need to work on this routine!
        #
    }

    @AddRegs = ();
    @AddRegSections = ();

    print(STDERR "\tExiting VerifyModel ...\n") if ($DEBUG);
    return(TRUE);
}
#-------------------------------------------------------------
#
#   Checks if the given characteristics are correct
#
# Usage:
#   CheckCharacteristics(expected_section_name,line_num)
#
#-------------------------------------------------------------

sub
CheckCharacteristics
{
my($char,$sec, $Line_num) = @_;
my($RHS);
my($x,$y,$z);

    $RHS= hex($char);

    $x=$RHS&0x1;
    $y=($RHS&0x2)>>1;
    $z=($RHS&0x4)>>2;

    if ($x+$y+$z!=1)
    {
        AddNetCardError(4002,$Line_num);
    }
    if (($RHS&0x10)&&(($RHS&0x1)||($RHS&0x4)||($RHS&0x2)))
    {
        AddNetCardError(4003,$Line_num);
    }

}

#-------------------------------------------------------------
#
#   Checks if the install section has valid bustype
#
# Usage:
#   CheckBusType(bus_type,expected_section_name,line_num)
#
#-------------------------------------------------------------

sub
CheckBusType
{
my($RHS,$sec, $Line_num) = @_;
my($Item);
my($Section,$Extn,$HwId);
my(@Values,$Value);

    $sec = uc $sec;
    $Value = $sec;
    $Value =~ s/\.NT$//;
    $Value =~ s/\.NTX86$//;
    $Value =~ s/\.NTALPHA$//;
    foreach (keys(%AllModels)) {
        $Item = uc($_);
        no strict; # Indirect ref's not valid when using strict
        $$Item[0] = $_;
        use strict;
        if ($Value eq $Item) {
            $HwId =  $AllModels{$_};
            if (!defined($HwId)) {
                # This error is flagged by Chkinf.pm
                next;
            }

            if ($HwId =~ /ISAPNP/)
            {
                if ($RHS !~ /^14$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
            elsif ($HwId =~ /PCMCIA/)
            {
                if ($RHS !~ /^8$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
            elsif ($HwId =~ /PCI/)
            {
                if ($RHS !~ /^5$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
            elsif (($HwId =~ /EISA/)||($HwId =~ /^\*/))
            {
                if ($RHS !~ /^2$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
            else
            {
                if ($RHS !~ /^1$/)
                {
                    AddNetCardError(4001,$Line_num);
                }
            }
        }
        use strict;
        # Check the model
    }

}
#-- CheckCopyFiles($line, $Section) ------------------------------------------------------#
sub CheckCopyFiles {
    my($line,$Section)    =  @_ ;
    my(@lines)   =  ChkInf::GetInfLines;
    my($DONE)    =  FALSE;
    my($Destination, $Source, $Temp, $Flags);

    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/); # Skip if blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        ($Destination, $Source, $Temp, $Flags) = split(/,/,$lines[$line]);

        $Destination =~ s/^\s*//;
        $Destination =~ s/\s*$//;

        # Defer to ChkInf.pm's CopyFiles logic for this error.
        # if (!(($Destination =~ /sys$/i) || ($Destination =~ /dll$/i) || ($Destination =~ /hlp$/i )))
        # {
        #    AddNetCardError(4207,$line,$Destination);
        # }

            $line++;
    }
    return(TRUE);
}
#-- AddRegProc($Section) ------------------------------------------------------#
sub AddRegProc {

    my($Section,$ModelChar)    =  @_ ;
    my($line);
    my(@lines)   =  ChkInf::GetInfLines;
    my($DONE)    =  FALSE;
    my(@LineItems,$CurrentLine);
    my($Temp,$SizeOfArray,$TempVar);
    my($ParamsDefined) = 0;
    my($ParamVerify) = hex($ModelChar)&0x80;

    $line = $SecRef{$Section};
    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        $CurrentLine = $lines[$line];

        @LineItems = (); # Clear the array
        #if ($CurrentLine =~ /__QUOTED_STR_\((\d*)\)__/) {
        #   $Temp = ChkInf::QuotedString($1);
        #   $CurrentLine =~ s/__QUOTED_STR_\(\d*\)__/$Temp/;
        #   $Temp = "";
        #}
        @LineItems = split(/,/,$CurrentLine);

        $SizeOfArray = $#LineItems;
        for ($Temp=0; $Temp <= $SizeOfArray; $Temp++) {
            if (defined($LineItems[$Temp])) {
                $LineItems[$Temp] =~ s/^\s*//; # Remove leading whitespace
                $LineItems[$Temp] =~ s/\s*$//; # Remove trailing whitespace
                $LineItems[$Temp] = uc($LineItems[$Temp]);
            } else {
                $LineItems[$Temp] = " ";
            }
        }

        # Now that we have a clean, standardized line, verify it
        if (! defined($LineItems[4])) {
            AddNetCardError(4033,$line,"5");
            $line++;
            next;
        }

        if ($LineItems[1] =~ /^NDI/i)
        {
            if ($LineItems[2] =~ /SERVICE/i)
            {
                if ($LineItems[4] =~ /__QUOTED_STR_\(([0-9]*)\)__$/)
                {
                    $AddRegList{'SERVICE'} = ChkInf::QuotedString($1);
                }
                else
                {
                    if ($LineItems[4] =~ /%(.*?)%/) 
                    {
                        $TempVar = quotemeta($1);
                        if (!defined($StringVals{uc($TempVar)}))
                        {
                            # Error if the %string% is not defined.
                            ChkInf::AddErrorRef(1025,$line,"%$TempVar%");
                        }
                    }
                    else
                    {
                        ChkInf::AddErrorRef(1004, $line);
                    }
                }
            }

            if ($LineItems[1] =~ /INTERFACES/i)
            {
                my(@tmp3,$cnt,$i,$str); #set of interfaces can be defined..
                if ($LineItems[4] =~ /__QUOTED_STR_\(([0-9]*)\)__$/)
                {
                    $str = ChkInf::QuotedString($1);
                    @tmp3 = split(',',$str);
                    $cnt = $#tmp3;
                }
                else
                {
                    if ($LineItems[4] =~ /%(.*?)%/) 
                    {
                        $TempVar = quotemeta($1);
                        if (!defined($StringVals{uc($TempVar)}))
                        {
                            # Error if the %string% is not defined.
                            ChkInf::AddErrorRef(1025,$line,"%$TempVar%");
                        }
                    }
                    else
                    {
                        ChkInf::AddErrorRef(1004, $line);
                    }
                }
                $cnt=0 unless(defined($cnt));
                for ($i=0; $i < $cnt; $i++)
                {
                        AddNetCardError(4005,$line) if (!defined($ValidInterfaces{uc $tmp3[$i]}));
                }
                #checks that no lower interface has install section..
                #if (($LineItems[2] =~ /DefLower/i)||($LineItems[2] =~ /LowerRange/i))
                #{
                #   for ($i=0; $i < $cnt; $i++)
                #   {
                #       AddNetCardError(4006,$INFSections{$Section}) if (defined($interface_Install{uc $tmp3[$i]}));
                #   }
                #}
            }

            if ($LineItems[1] =~ /PARAMS/i)
            {
                my(@tmp3,$val);
                @tmp3 = split(/\\/,$LineItems[1]);
                $ParamsDefined = 1;
                $val = hex($ModelChar)&0x80;
                AddNetCardError(4007,$line) if (defined($ReservedWords{uc $tmp3[2]}));
                if ($LineItems[1] !~ /ENUM/i)
                {
                    AddNetCardError(4008,$line) if (!defined($ParamKeys{uc $LineItems[2]}));
                    if ($LineItems[4] !~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
                    {
                        if ($LineItems[4] =~ /%(.*?)%/) 
                        {
                            $TempVar = quotemeta($1);
                            if (!defined($StringVals{uc($TempVar)}))
                            {
                                # Error if the %string% is not defined.
                                ChkInf::AddErrorRef(1025,$line,"%$TempVar%");
                            }
                        }
                        else
                        {
                            ChkInf::AddErrorRef(1004, $line);
                        }
                    }
                }
                elsif ($LineItems[1] =~ /ENUM/i)
                {
                    if ($LineItems[4] !~ /^__QUOTED_STR_\(([0-9]*)\)__$/)
                    {
                        if ($LineItems[4] =~ /%(.*?)%/) 
                        {
                            $TempVar = quotemeta($1);
                            if (!defined($StringVals{uc($TempVar)}))
                            {
                                # Error if the %string% is not defined.
                                ChkInf::AddErrorRef(1025,$line,"%$TempVar%");
                            }
                        }
                        else
                        {
                            ChkInf::AddErrorRef(1004, $line);
                        }
                    }
                }
            }
        }   #ndi-if ends.
        else
        {
            if ($LineItems[1] =~ /^$/ )     #Defaults section...
            {
                #Also flag an error if the value-name doesnt satisfy "some" criterion..
                # This criteria is unknown for DynamicCRTMax, DynamicCRTIncrement,
                # DynamicCRTInterval, EventMessageFile, TypesSupported and TextModeFlags.

                if (($LineItems[4] !~ /__QUOTED_STR_\(([0-9]*)\)__$/)||($LineItems[4] !~ /[0x]?[0-9a-fA-F]*/))
                {
                    if ($LineItems[4] =~ /%(.*?)%/) 
                    {
                        $TempVar = quotemeta($1);
                        if (!defined($StringVals{uc($TempVar)}))
                        {
                            # Error if the %string% is not defined.
                            ChkInf::AddErrorRef(1025,$line,"%$TempVar%");
                        }
                    }
                    else
                    {
                        ChkInf::AddErrorRef(1004, $line);
                    }
                }
            }
        }

        $line++;
    }
    if ($ParamsDefined == 1)
    {
        AddNetCardError(4009,$SecRef{$Section}) if ($ParamVerify == 0);
    }
    return(TRUE);
}
#-----------------------CheckServiceName($Section)-----------------------
sub CheckServiceName {
    my($Section) =  @_;
    my($line);
    my(@lines)   =  ChkInf::GetInfLines;
    my($DONE)    =  FALSE;
    my($Directive,@Values);
    my($ucAddReg,$ucAddService);

    $line = $SecRef{$Section};

    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));
        @Values    = split(/,/,ChkInf::GetDirectiveValue($line, $lines[$line]));

        $Values[0] =~ s/^\s*//; # Remove leading whitespace
        $Values[0] =~ s/\s*$//; # Remove trailing whitespace

        $ucAddReg = $AddRegList{'SERVICE'};
		$ucAddReg = uc($ucAddReg); # Ugly statement because some versions of Perl complained! =-(
        $ucAddService = uc $Values[0];

        if ((!defined($Values[0])) or (!defined($AddRegList{'SERVICE'})) or ($ucAddService ne $ucAddReg)) {
            AddNetCardError(4011,$line);
        }

        $line++;
    }
    return(TRUE);
}
#-----------------------CheckCompatibleKey--------------------------------
sub CheckCompatibleKey {
    my($Section);
    my($line); 
    my(@lines)   =  ChkInf::GetInfLines;
    my($DONE)    =  FALSE;
    my($Directive,$Value);
    my($Signature,$CompVal,$CompPresent,$CompLine);

    $CompPresent = 0;

    $line = $SecRef{'VERSION'};
    $CompLine = $line;

    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        $Directive = uc(ChkInf::GetDirective($line,$lines[$line]));
        $Value     = uc(ChkInf::GetDirectiveValue($line, $lines[$line]));

        if ($Directive =~ /SIGNATURE/i)
        {
            $Signature = $Value;
        }
        if ($Directive =~ /COMPATIBLE/i)
        {
            $CompVal = $Value;
            $CompPresent = 1;
            $CompLine = $line;
        }

        $line++
    }

    if ($Signature =~ /CHICAGO/i)
    {
        if ($CompPresent == 1)
        {
            if ($CompVal != 1)
            {
                AddNetCardError(3046,$CompLine);
            }
        }
        else
        {
            AddNetCardError(3045,$CompLine);
        }
    }
    return(TRUE);
}




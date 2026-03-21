# Copyright 1999-2000 Microsoft Corporation
package ChkInf;

    use strict; # Keep this code clean

    # Global variables specific to each instance of the module
    my($Version) = "5.1.2199.0"; # Version of this file
    my($DEBUG)   = 0;            # Setting to 1 produces more verbose output

    # Global variables specific to each INF
    my($CLASS);
    my($ClassMod);        # Global Value of Module to Use
    my($Copyright);       # FLAG to verify copyright was detected
    my($CurrentInfDir);   # Dir to use when inf's are INCLUDE'd
    my($DCOptions);       # Device Class Specific options
    my($ErrorCount, $WarningCount); # Error & Warning counts
    my($Included);        # Tracks files already included to avoid duplicates
    my($inffile);         # Name of inf file currently being processed
    my($LastLine);        # Last Line of Main INF in @INFLines
    my($LayoutCheck)  =0; # Check Layout.inf??
    my($LAYOUTFILE_USED);
    my($LogFile);         # Name of NTLOG
    my($Models);          # Global Listing of Models Processed
    my($Module);          # Name of module we're loading
    my($QuoteNum)     =0; # Global Index for %AllQuotes
    my($StandardClass)=0; # Is this a standard class? Used to warn on ClassInstall32.

    my(%AllExcludes);
    my(%AllQuotes) =  (); # Global Hash of Quoted Strings
    my(%CheckedSections); # Hash to record section that have been validated
    my(%DestDirs);
    my(%ErrorTable)=  (); # Global Hash for Errors
    my(%FilesChecked);    # List of inf's checked for use in writing summary.htm
    my(%IncludedSections);# Array of sections referenced by NEEDS
    my(%INFFiles);        # Data about included INFs
    my(%INFSections);     # Hash of checked sections indexed by section name
    my(%InstList);        # NO USED CURRRENTLY BUT NEEDED FOR LATER MODULES
    my(%LayoutFiles);
    my(%ManuList);        # NO USED CURRRENTLY BUT NEEDED FOR LATER MODULES
    my(%ModelLine);
    my(%Models);
    my(%RealSections);    # Case preserved hash
    my(%StringVals)=  (); # Stores actual value for later swapping.
    my(%TimeMarks);       # Hash shared by BeginTimer(), EndTimer()
    my(%WarningTable)=(); # Global Hash for Warnings

    # Valid DirID Locations
    my(%DirIDValues) = (-1    => 1,  # Absolute path  
                        01    => 1,  # SourceDrive:\pathname (the directory from which the INF file was installed) 
                        10    => 1,  # Windows directory
                        11    => 1,  # System directory 
                        12    => 1,  # Drivers directory
                        17    => 1,  # INF file directory 
                        18    => 1,  # Help directory 
                        20    => 1,  # Fonts directory 
                        21    => 1,  # Viewers directory 
                        23    => 1,  # Color (ICM) 
                        24    => 1,  # Applications directory 
                        25    => 1,  # Shared directory 
                        30    => 1,  # Root directory of the boot drive
                        50    => 1,  # %windir%\system directory (Windows 2000 only)  
                        51    => 1,  # Spool directory 
                        52    => 1,  # Spool drivers directory 
                        53    => 1,  # User Profile directory 
                        54    => 1,  # Directory where ntldr.exe and osloader.exe are located (Windows 2000 only)  
                        55    => 1,  # Print processors directory 
                        16406 => 1,  # All Users\Start Menu  
                        16407 => 1,  # All Users\Start Menu\Programs  
                        16408 => 1,  # All Users\Start Menu\Programs\Startup 
                        16409 => 1,  # All Users\Desktop 
                        16415 => 1,  # All Users\Favorites 
                        16419 => 1,  # All Users\Application Data 
                        16422 => 1,  # Program Files 
                        16427 => 1,  # Program Files\Common 
                        16429 => 1,  # All Users\Templates 
                        16430 => 1); # All Users\Documents 

    my(@INFlines);        # Array of processed lines from INF

    # Some simple utility subroutines.
    sub FALSE        {return(0);}      # For BOOLEAN testing
    sub FAILURE      {return(FALSE());}# Indicate Sub Failure
    sub NUMERIC      { $a <=> $b; }    # To sort numerically
    sub SUCCESS      {return(TRUE());} # Indicate Sub Success
    sub TRUE         {return(1);}      # For BOOLEAN testing

    # Subroutines to share out read-only access to global variables.
    sub ClassMod     {return($ClassMod);}
    sub GetInfLines  {return(@INFlines);}
    sub InfLength    {return(@INFlines);}
    sub INFSections  {return(%INFSections);}
    sub InstList     {return(%InstList);}
    sub ManuList     {return(%ManuList);}
    sub QuotedString {return($AllQuotes{$_[0]});}
    sub StringVals   {return(%StringVals);}
    sub SecChecked   {$CheckedSections{$_[0]} = TRUE; return(TRUE);}
    my($CreateLog);  # Create an NTLOG?

    #---------------------------------------------------
    # Allow $CreateLog to be set from MAIN
    #---------------------------------------------------
    sub CreateLog {
        $CreateLog = TRUE;
        $LogFile   = $_[0];
        return(TRUE);
    }
    #---------------------------------------------------
    # Allow a device module to mark a section as checked
    #---------------------------------------------------
    sub MarkChecked {
        $CheckedSections{$_[0]} = TRUE;
        return(TRUE);
    }

    #---------------------------------------------------
    # Return a list of all sections referenced
    #---------------------------------------------------
    sub SectionRef   {
        my($Returns) = ";";
        foreach (keys(%INFSections)) {
                $Returns=join(";",$Returns,$_) if ($INFSections{$_} == TRUE);
        }
        return($Returns);
    }
    #---------------------------------------------------
    return(1); # module's return value

#-------------------------------------------------------------------------------------------------#
# This is called by Device Class Modules to add a new error to the list of errors for the file.   #
#-- AddDeviceSpecificError($LineNum, $ErrorNum, $Error) ------------------------------------------#
sub AddDeviceSpecificError {
    my($LineNum) = $_[0];
    my($ErrorNum)= $_[1];
    my($Error)   = $_[2];
    my($str, $this_str);

    if (defined($ErrorTable{$LineNum})) {
        # Don't add the same error twice.
        return(FALSE) if ($ErrorTable{$LineNum} =~ /\(DE$ErrorNum\)/);
    }

    $this_str = ("(DE$ErrorNum) $Error");
    if (defined($ErrorTable{$LineNum})) {
        # If this line has errors already, append the new one.
        $str = $ErrorTable{$LineNum};
        $str = join("; ", $str, $this_str);
    } else {
        # If it doesn't have errors, nothing to append to.
        $str = "; $this_str";
    }

    # Incrment the global error count
    $ErrorCount++;
    $ErrorTable{$LineNum} = $str;
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# This is called by Device Class Modules to add a new warning to the list of errors for the file. #
#-- AddDeviceSpecificWarning($LineNum, $ErrorNum, $Error) ----------------------------------------#
sub AddDeviceSpecificWarning {
    my($LineNum) = $_[0];
    my($WarnNum) = $_[1];
    my($Warn)    = $_[2];
    my($str, $this_str);

	die(@_) if (! defined($LineNum));

    if (defined($WarningTable{$LineNum})) {
    # Don't add the same warning twice.
        return(FALSE) if ($WarningTable{$LineNum} =~ /\(DE$WarnNum\)/);
    }

    $this_str = ("(DE$WarnNum) $Warn");
    if (defined($WarningTable{$LineNum})) {
        # If this line has warnings, append to new one to them
        $str = $WarningTable{$LineNum};
        $str = join("; ", $str, $this_str);
    } else {
        # Else nothing to append to.
        $str = "; $this_str";
    }

    # Increment the global warning count
    $WarningCount++;
    $WarningTable{$LineNum} = $str;
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# This sub adds a new error to the list of errors for the file.                                   #
#-- AddErrorRef($ErrorNum, $LineNum, @ErrorArgs) -------------------------------------------------#
sub AddErrorRef {
    my($ErrorNum) = $_[0], shift;
    my($LineNum)  = $_[0], shift;
    my(@ErrorArgs)= @_;

    # For neatness, don't leave this undefined
    $ErrorArgs[0] = " " if (! defined($ErrorArgs[0]));
    my($str, $this_str, $info_err, $format_err);

    if (! defined($LineNum)) {
        # Can't error a non-existant line, die() becuase something is really wrong
        die("Error: \$LineNum not defined for ErrorRef $ErrorNum\n");
    }

    # Get the error
    $info_err = GetError($ErrorNum);

    if (defined($ErrorTable{$LineNum})) {
        # Don't repeat the same error!
        return(FALSE) if (($ErrorTable{$LineNum} =~ /\(E$ErrorNum\)/) && ($ErrorNum != 1095));
    }

    if ( !defined($info_err) ) {
        # If we're not defined, keep the error but add this text to it.
        $this_str = "Unknown error $ErrorNum.";
    } else {
        # We are defined, so format the error
        $format_err = $$info_err[0];
        $this_str = sprintf($format_err, @ErrorArgs);
    }

    if (defined($ErrorTable{$LineNum})) {
        # If we have errors already, append to them.
        $str = $ErrorTable{$LineNum};
        $str = join("; ", $str, "(E$ErrorNum) ".$this_str);
    } else {
        # Otherwise, create the list anew.
        $str = "; (E$ErrorNum) ".$this_str;
    }

    # Increment the global error count.
    $ErrorCount++;
    $ErrorTable{$LineNum} = $str;

    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# Adds a file to the included file list                                                           #
# WARNING: This routine generates the following warning: "Illegal hex digit ignored" if the file  #
#   being included begins with a numeric character.  Warnings are disabled to prevent this from   #
#   showing up as it is a known issue.                                                            #
#-- AddIncFile($FileName) ------------------------------------------------------------------------#
sub AddIncFile {
    # Make sure we don't add the same file a second time

    # save warning value, then turn warnings off explicitly
    my($File) = "$_[0]";
    my($SAVE) = $^W;
    $^W = 0;

    if ($Included !~ /\Q$File\E/i) {
        if ($Included ne "") {
            # If we've included files already.. append to the list.
            $Included = join("; ", ($Included, $File));
        } else {
            # Otherwise start a clean list.
            $Included = $File;
        }
    }
    # reset saved warning value
    $^W = $SAVE;
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# This sub adds a warning to the list of warnings for the file.                                   #
#-- AddWarnRef($WarnNum, $LineNum, @WarnArgs) ----------------------------------------------------#
sub AddWarnRef {
    my($WarnNum) = $_[0], shift;
    my($LineNum) = $_[0], shift;
    my(@WarnArgs)= @_;

    my($str, $this_str, $info_wrn, $format_wrn);

    # Get the warning text
    $info_wrn = GetWarning($WarnNum);
    if (defined($WarningTable{$LineNum})) {
        # Don't repeat the same warning!
        return(FALSE) if ($WarningTable{$LineNum} =~ /\(W$WarnNum\)/);
    }

    if ( !defined($info_wrn) ) {
        # If there is not warning text, keep the warning but add this text.
        $this_str = "Unknown warning $WarnNum.";
    } else {
        # OTherwise format the warning.
        $format_wrn = $$info_wrn[0];
        $this_str = sprintf($format_wrn, @WarnArgs);
    }

    if (defined($WarningTable{$LineNum}))    {
        # If this line has warnings already, append the new one
        $str = $WarningTable{$LineNum};
        $str = join("; ", $str, "(W$WarnNum) ".$this_str);
    } else {
        # Otherwise start a new list.
        $str = ("; (W$WarnNum) ".$this_str);
    }

    # Increment the global warning count
    $WarningCount++;
    $WarningTable{$LineNum} = $str;

    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#  Starts a timer for process $ProcName                                                           #
#-- BeginTimer($ProcName) ------------------------------------------------------------------------#
sub BeginTimer {
    my($ProcName) = @_;
    my(@time);
    if (defined($TimeMarks{$ProcName})) {
        # Warn if this $ProcName exists already.
        print STDERR "Procedure $ProcName called recursively.\n";
    }
    # Record the time
    $TimeMarks{$ProcName} = time();

    return(SUCCESS);
}

#-------------------------------------------------------------------------------------------------#
# Checks for 32-bit value                                                                         #
#-- Check32bitvalue($value, $line)----------------------------------------------------------------#
sub Check32bitvalue {
    my($num,$Lnum)=@_;
    my($len);
    $num =~ s/^[0]*//;  #Remove Starting 0s
    $len = length $num;
    AddErrorRef(1103,$Lnum) if ($len > 4);
}

#-------------------------------------------------------------------------------------------------#
# Checks for 64-bit value                                                                         #
#-- Check64bitvalue($value, $line)----------------------------------------------------------------#
sub Check64bitvalue {
    my($num,$Lnum)=@_;
    my($len);
    $num =~ s/^[0]*//;  #Remove Starting 0s
    $len = length $num;
    AddErrorRef(1103,$Lnum) if ($len > 8);
}

#-------------------------------------------------------------------------------------------------#
# Matches a CLASS with a corresponding CLASSGUID                                                  #
#-- CheckClass($ClassName, $ClassGUID, $Line) ----------------------------------------------------#
sub CheckClass {
    my($ClassName) = $_[0];
    my($ClassGUID) = $_[1];
    my($line)      = $_[2];

    my(%ClassGUIDTable) = (
        "1394",           "{6BDD1FC1-810F-11D0-BEC7-08002BE2092F}",
        "APMSUPPORT",     "{D45B1C18-C8FA-11D1-9F77-0000F805F530}",
        "BATTERY",        "{72631E54-78A4-11D0-BCF7-00AA00B7B32A}",
        "CDROM",          "{4D36E965-E325-11CE-BFC1-08002BE10318}",
        "DISKDRIVE",      "{4D36E967-E325-11CE-BFC1-08002BE10318}",
        "DISPLAY",        "{4D36E968-E325-11CE-BFC1-08002BE10318}",
        "FDC",            "{4D36E969-E325-11CE-BFC1-08002BE10318}",
        "FLOPPYDISK",     "{4D36E980-E325-11CE-BFC1-08002BE10318}",
        "HDC",            "{4D36E96A-E325-11CE-BFC1-08002BE10318}",
        "HIDCLASS",       "{745A17A0-74D3-11D0-B6FE-00A0C90F57DA}",
        "IMAGE",          "{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}",
        "INFRARED",       "{6BDD1FC5-810F-11D0-BEC7-08002BE2092F}",
        "KEYBOARD",       "{4D36E96B-E325-11CE-BFC1-08002BE10318}",
        "MEDIUMCHANGER",  "{CE5939AE-EBDE-11D0-B181-0000F8753EC4}",
        "MEDIA",          "{4D36E96C-E325-11CE-BFC1-08002BE10318}",
        "MODEM",          "{4D36E96D-E325-11CE-BFC1-08002BE10318}",
        "MONITOR",        "{4D36E96E-E325-11CE-BFC1-08002BE10318}",
        "MOUSE",          "{4D36E96F-E325-11CE-BFC1-08002BE10318}",
        "MTD",            "{4D36E970-E325-11CE-BFC1-08002BE10318}",
        "MULTIFUNCTION",  "{4D36E971-E325-11CE-BFC1-08002BE10318}",
        "MULTIPORTSERIAL","{50906CB8-BA12-11D1-BF5D-0000F805F530}",
        "NET",            "{4D36E972-E325-11CE-BFC1-08002BE10318}",
        "NETCLIENT",      "{4D36E973-E325-11CE-BFC1-08002BE10318}",
        "NETSERVICE",     "{4D36E974-E325-11CE-BFC1-08002BE10318}",
        "NETTRANS",       "{4D36E975-E325-11CE-BFC1-08002BE10318}",
        "PCMCIA",         "{4D36E977-E325-11CE-BFC1-08002BE10318}",
        "PORTS",          "{4D36E978-E325-11CE-BFC1-08002BE10318}",
        "PRINTER",        "{4D36E979-E325-11CE-BFC1-08002BE10318}",
        "SCSIADAPTER",    "{4D36E97B-E325-11CE-BFC1-08002BE10318}",
        "SMARTCARDREADER","{50DD5230-BA8A-11D1-BF5D-0000F805F530}",
        "SYSTEM",         "{4D36E97D-E325-11CE-BFC1-08002BE10318}",
        "TAPEDRIVE",      "{6D807884-7D21-11CF-801C-08002BE10318}",
        "USB",            "{36FC9E60-C465-11CF-8056-444553540000}",
        "VOLUME",         "{71A27CDD-812A-11D0-BEC7-08002BE2092F}" );

    my(%BadClassGUIDTable) = (
        "ADAPTER",        "{4D36E964-E325-11CE-BFC1-08002BE10318}", #obsolete.
        "COMPUTER",       "{4D36E966-E325-11CE-BFC1-08002BE10318}", #reserved for system use.
        "DECODER",        "{6BDD1FC2-810F-11D0-BEC7-08002BE2092F}", #reserved for future use.
        "GPS",            "{6BDD1FC3-810F-11D0-BEC7-08002BE2092F}", #reserved for future use.
        "LEGACYDRIVER",   "{8ECC055D-047F-11D1-A537-0000F8753ED1}", #reserved for system use.
        "NTAPM",          "{D45B1C18-C8FA-11D1-9F77-0000F805F530}", #reserved for system use.
        "NODRIVER",       "{4D36E976-E325-11CE-BFC1-08002BE10318}", #obsolete.
        "OTHER DEVICES",  "{4D36E97E-E325-11CE-BFC1-08002BE10318}", #reserved for system use.
        "PRINTERUPGRADE", "{4D36E97A-E325-11CE-BFC1-08002BE10318}", #reserved for system use.
        "SOUND",          "{4D36E97C-E325-11CE-BFC1-08002BE10318}", #obsolete.
        "UNKNOWN",        "{4D36E97E-E325-11CE-BFC1-08002BE10318}");#reserved for system use.

    if (defined($ClassGUIDTable{$ClassName})) {
        if ($ClassGUIDTable{$ClassName} eq (uc($ClassGUID))) {
             # Return true if Class matches ClassGUID
             $StandardClass = TRUE;
             return(TRUE);
        } else {
            # Return false if they don't BUT the class is defined.
            $StandardClass = FALSE;
            AddErrorRef(1010, $line, $ClassGUIDTable{$ClassName});
            return(FALSE);
        }

    } else {
        if (defined ($BadClassGUIDTable{$ClassName})) {
            # Flag an error if the CLass is on the Bad CLass List
            $StandardClass = FALSE;
            AddErrorRef(1059, $line, $ClassName);
            return(FALSE);
         }

         if ($ClassGUID !~ /\{[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}\}/i) {
            # Class isn't defined (anywhere), and ClassGUID is in the wrong format.
            $StandardClass = FALSE;
            AddErrorRef(1055, $line, "");
            return(FALSE);
         } else {
            # Class isn't known but ClasssGUID looks like it should be okay
            $StandardClass = FALSE;
            AddWarnRef (2019, $line, $ClassName, $ClassGUID);
            AddErrorRef(1073, $line)
                unless ((defined($INFSections{'CLASSINSTALL32'}) && ($INFSections{'CLASSINSTALL32'} <= $LastLine))
                     or (defined($INFSections{'CLASSINSTALL32.NT'}) && ($INFSections{'CLASSINSTALL32.NT'} <= $LastLine))
                     or (defined($INFSections{'CLASSINSTALL32.NTX86'}) && ($INFSections{'CLASSINSTALL32.NTX86'} <= $LastLine)));
            return(FALSE);
         }
        # We should _NEVER_ hit this error!
        AddErrorRef(9999, $line);

        return(FALSE);
    }
}

#--- Verifies each [STRINGS.LangID] Section ---------------------------#
#--- CheckStringsWithLangID() -----------------------------------------#
sub CheckStringsWithLangID {
    my($secs);

    print(STDERR "\tEntering CheckStringsWithLangID...\n") if ($DEBUG);
    #  A language ID is a 16 bit value which is the combination of a
    #  primary language ID and a secondary language ID.  The bits are
    #  allocated as follows:
    #
    #       +-----------------------+-------------------------+
    #       |     Sublanguage ID    |   Primary Language ID   |
    #       +-----------------------+-------------------------+
    #        15                   10 9                       0   bit
    #
    # See %DDK_ROOT%\inc\winnt.h for more information

    my(%ValidStringDecorations) = ( # Pregenerated list of valid LanguageID values
    "0000" => 1,"0001" => 1,"0002" => 1,"0003" => 1,"0004" => 1,"0005" => 1,"0006" => 1,
    "0007" => 1,"0008" => 1,"0009" => 1,"000A" => 1,"000B" => 1,"000C" => 1,"000D" => 1,
    "000E" => 1,"000F" => 1,"0010" => 1,"0011" => 1,"0012" => 1,"0013" => 1,"0014" => 1,
    "0015" => 1,"0016" => 1,"0018" => 1,"0019" => 1,"001A" => 1,"001A" => 1,"001B" => 1,
    "001C" => 1,"001D" => 1,"001E" => 1,"001F" => 1,"0020" => 1,"0021" => 1,"0022" => 1,
    "0023" => 1,"0024" => 1,"0025" => 1,"0026" => 1,"0027" => 1,"0029" => 1,"002A" => 1,
    "002B" => 1,"002C" => 1,"002D" => 1,"002F" => 1,"0036" => 1,"0037" => 1,"0038" => 1,
    "0039" => 1,"003E" => 1,"003F" => 1,"0041" => 1,"0043" => 1,"0044" => 1,"0045" => 1,
    "0046" => 1,"0047" => 1,"0048" => 1,"0049" => 1,"004A" => 1,"004B" => 1,"004C" => 1,
    "004D" => 1,"004E" => 1,"004F" => 1,"0057" => 1,"0058" => 1,"0059" => 1,"0060" => 1,
    "0061" => 1,"0400" => 1,"0401" => 1,"0404" => 1,"0407" => 1,"0409" => 1,"040A" => 1,
    "040C" => 1,"0410" => 1,"0412" => 1,"0413" => 1,"0414" => 1,"0416" => 1,"041D" => 1,
    "0420" => 1,"0427" => 1,"042C" => 1,"043E" => 1,"0443" => 1,"0800" => 1,"0801" => 1,
    "0804" => 1,"0807" => 1,"0809" => 1,"080A" => 1,"080C" => 1,"0810" => 1,"0813" => 1,
    "0814" => 1,"0816" => 1,"081A" => 1,"081D" => 1,"0820" => 1,"082C" => 1,"083E" => 1,
    "0843" => 1,"0860" => 1,"0861" => 1,"0C01" => 1,"0C04" => 1,"0C07" => 1,"0C09" => 1,
    "0C0A" => 1,"0C0C" => 1,"0C1A" => 1,"1001" => 1,"1004" => 1,"1007" => 1,"1009" => 1,
    "100A" => 1,"100C" => 1,"1401" => 1,"1404" => 1,"1407" => 1,"1409" => 1,"140A" => 1,
    "140C" => 1,"1801" => 1,"1809" => 1,"180A" => 1,"180C" => 1,"1C01" => 1,"1C09" => 1,
    "1C0A" => 1,"2001" => 1,"2009" => 1,"200A" => 1,"2401" => 1,"2409" => 1,"240A" => 1,
    "2801" => 1,"2809" => 1,"280A" => 1,"2C01" => 1,"2C09" => 1,"2C0A" => 1,"3001" => 1,
    "3009" => 1,"300A" => 1,"3401" => 1,"3409" => 1,"340A" => 1,"3801" => 1,"380A" => 1,
    "3C01" => 1,"3C0A" => 1,"4001" => 1,"400A" => 1,"440A" => 1,"480A" => 1,"4C0A" => 1,
    "500A" => 1);

    foreach $secs (sort(keys(%INFSections))) {
        if ($secs=~ /^STRINGS\.(.*)$/i) {
            if (! defined($ValidStringDecorations{$1})) {
                # We've defined all valid LanguageID's so flag the rest as errors
                AddErrorRef(1099,$INFSections{$secs}-1, $1);
                $CheckedSections{$secs} = TRUE;
            } else {
                # CODE TO CROSS VALIDATE STRINGs.* SECTIONS GOES HERE!
                CrossCheckStrings('STRINGS',$secs);
                # Mark the section os valid
                $CheckedSections{$secs} = TRUE;
            }

        }
    }
    print(STDERR "\tExiting CheckStringsWithLangID...\n") if ($DEBUG);
    return(TRUE);
}

#-----------------------------------------------------------------------------------------------------#
#   Compiles list of destdirs - no validation (that comes later)                                      #
#   Globals changed: %DestDirs                                                                        #
#-- CompileDestDirs($line, $Section) -----------------------------------------------------------------#
sub CompileDestDirs {

    my($line, $Section) = @_;
    my(@lines)          = @INFlines;
    my($Directive, $Value);
    my($DONE)           = FALSE;

    print(STDERR "\tBeginning CompileDestDirsSection (@_)...\n") if ($DEBUG);
    while (! $DONE) {

        $line++, next if ($lines[$line] =~ /^\s$/); # Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {           # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        GetLine($line) if ($DEBUG);
        $Directive = uc(GetDirective($line,$lines[$line]));
        $Value     = uc(GetDirectiveValue($line,$lines[$line]));

        $Directive =~ s/^\s//; # Remove leading/trailing whitespace
        $Directive =~ s/\s$//;

        if ($Directive !~ /^DefaultDestDir$/i) {
            $DestDirs{$Directive}=$Value;
        } else {
            $DestDirs{'DEFAULT'}=$Value;
        }
        $line++;
    }
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);
    print(STDERR "\tExiting CompileDestDirsSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#  Verifies that two strings sections carry the same information.                                 #
#-- CrossCheckStrings($Section1, $Section2) ------------------------------------------------------#
sub CrossCheckStrings {
    my($Section1) = $_[0];
    my($Section2) = $_[1];
    my($string);

    print(STDERR "\tEntering CrossCheckStrings (@_)...\n") if ($DEBUG);
    my(%SectionOne) = ReadInSection($INFSections{$Section1});
    my(%SectionTwo) = ReadInSection($INFSections{$Section2});

    foreach $string (sort(keys(%SectionOne))) {
        AddErrorRef(1095,$INFSections{$Section2}-1,$string) unless (defined($SectionTwo{$string}));
    }

    print(STDERR "\tExiting CrossCheckStrings...\n") if ($DEBUG);
    #-----------------------------------------------------------------------------------------#
    # Returns a hash of all strings defined in a section                                      #
    #-- ReadInSection($Section) --------------------------------------------------------------#
    sub ReadInSection {
        my($line)     = $_[0];
        my(@lines)    = @INFlines;
        my($CurrentString, $String, $StringValue, %FStr, $UString);
        my($DONE)      = FALSE;

        print(STDERR "\t\tEntering ReadInSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if (! defined($lines[$line]));
            $line++, next if ($lines[$line] =~ /^\s*$/); # Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {
                # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);

            $String      = uc(GetDirective($line,$lines[$line]));
            $StringValue = uc(GetDirectiveValue($line,$lines[$line]));

            #   right side should be in quotes or a numeric value
            if($StringValue !~ /^__QUOTED_STR_\(([0-9]*)\)__$/) {
                # Swap out QuotedString if need be
                if ($StringValue !~ /^(0x)??[0-9a-f]*$/i) {
                    $FStr{$String} = $StringValue;
                }
            } else {
                $FStr{$String} = $AllQuotes{$1};
            }

            $line++;

        }
        print(STDERR "\t\tExiting ReadInSection...\n") if ($DEBUG);
        return(%FStr);
    }
}

#-------------------------------------------------------------------------------------------------#
#  Stops the $ProcName Timer and reports $Elpsed Time                                             #
#-- EndTimer($ProcName) --------------------------------------------------------------------------#
sub EndTimer {
    my($ProcName) = @_;
    my($EndTime, @time);

    if (! defined($TimeMarks{$ProcName})) {
        # There's an error if BeginTimer wasn't called.
        print(STDERR "BeginTimer for procedure $ProcName not called.\n");
        return(FALSE);
    }
    $EndTime = time();

    # Calculate and print total runtime
    my($Elapsed) = $EndTime-$TimeMarks{$ProcName};
    printf(STDERR  "Elapsed time : %02dh %02dm %02ds\n",
                    int($Elapsed / 3600), int($Elapsed / 60) % 60, $Elapsed % 60);
    undef ($TimeMarks{$ProcName});

    return(SUCCESS);
}

#-------------------------------------------------------------------------------------------------#
#  Returns the directive part (L-Side) of an INF line                                             #
#-- GetDirective($line, $INFLine) ----------------------------------------------------------------#
sub GetDirective {
    my($line)    = $_[0];
    my($INFLine) = $_[1];
    my(@Count);
    my($string);

    die("INFline ($line) not defined!\n") if (! defined($INFLine));
    return($INFLine) unless ($INFLine =~ /=/);# There is note directive if there isn't an '='
    $INFLine =~ s/=.*$/=/;                    # eat right part
    $INFLine =~ s/^\s*//;                     # eat leading spaces
    $INFLine =~ s/\s*=$//;                    # eat trailing spaces & =

    if ($INFLine =~ /\s/) { # Only parse this if we contain spaces on the directive side
        @Count = ($INFLine =~ m/(\w+)/g)     # Count the number of words remaining
            unless ($INFLine =~ m/^".*"$/);  # unless the whole thing is quoted
        AddErrorRef(1013, $line) if ($#Count > 1);
    }

    # Swap out QuotedString if necessary.
    $INFLine = $AllQuotes{$1} if($INFLine =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);

    # Make sure any strings used are defined
    my($DONE)    = FALSE;
    my($TempVar);
    my($UnalteredVar);

    # Swap out any %string% identifiers for their literal values.
    while (! $DONE) {
        if ($INFLine =~ /%(.*?)%/) {
            $TempVar = $1;
            $UnalteredVar = $TempVar;
            # A few directives allow a '%' as valid inline characters.
            # Move around these by testing for value seperator ',' within
            #   the theoretical string.
            if ($TempVar !~ /,/) {
                $TempVar = quotemeta($TempVar);
                if (defined($StringVals{uc($TempVar)})) {
                    $INFLine =~ s/%$TempVar%/$StringVals{uc($TempVar)}/;
                    # To avoid self-referencing loops
                    #$DONE = TRUE if (uc("%$TempVar%") eq uc("$StringVals{uc($TempVar)}"));
                } else {
                    # Error if the %string% is not defined.
                    AddErrorRef(1025,$line,"%$UnalteredVar%");
                    $INFLine =~ s/%$TempVar%/$TempVar/;
                }
            } else {
                $DONE = TRUE;
            }
        } else {
            $DONE = TRUE;
        }
    }

    # Swap out QuotedString if necessary.
    $INFLine = $AllQuotes{$1} if($INFLine =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);

    return($INFLine);
}

#-------------------------------------------------------------------------------------------------#
#  Returns the directive part (L-Side) of an INF line                                             #
#  Seperate routine for [Model] sections due to mutliword permissions on L-Side                   #
#-- GetDirectiveModel($line, $INFLine) -----------------------------------------------------------#
sub GetDirectiveModel {
    my($line)    = $_[0];
    my($INFLine) = $_[1];
    my(@Count);
    my($string);

    die("INFline not defined!\n") if (! defined($INFLine));
    return($INFLine) unless ($INFLine =~ /=/);# There is note directive if there isn't an '='
    $INFLine =~ s/=.*$/=/;                    # eat right part
    $INFLine =~ s/^\s*//;                     # eat leading spaces
    $INFLine =~ s/\s*=$//;                    # eat trailing spaces & =

    # Swap out any %string% identifiers for their literal values.
    my($DONE)    = FALSE;
    my($TempVar);
    my($UnalteredVar);

    while (! $DONE) {
        if ($INFLine =~ /%(.*?)%/) {
            $TempVar = $1;
            $UnalteredVar = $TempVar;
            # A few directives allow a '%' as valid inline characters.
            # Move around these by testing for value seperator ',' within
            #   the theoretical string.
            if ($TempVar !~ /,/) {
                $TempVar = quotemeta($TempVar);
                if (defined($StringVals{uc($TempVar)})) {
                    $INFLine =~ s/%$TempVar%/$StringVals{uc($TempVar)}/;
                    # To avoid self-referencing loops
                    #$DONE = TRUE if (uc("%$TempVar%") eq uc("$StringVals{uc($TempVar)}"));
                } else {
                    # Error if the %string% is not defined.
                    AddErrorRef(1025,$line,"%$UnalteredVar%");
                    $INFLine =~ s/%$TempVar%/$TempVar/;
                }
            } else {
                $DONE = TRUE;
            }
        } else {
            $DONE = TRUE;
        }
    }

    # Swap out QuotedString if necessary.
    $INFLine = $AllQuotes{$1} if($INFLine =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
    $INFLine =~ s/%(.*)%/$StringVals{uc($1)}/ if (($INFLine =~ /%(.*)%/) and defined($StringVals{uc($1)}));
    $INFLine =~ s/__QUOTED_STR_\(([0-9]*)\)__/$AllQuotes{$1}/;
    return($INFLine);
}

#-------------------------------------------------------------------------------------------------#
#  Returns the directive value part (R-Side) of an INF line                                       #
#-- GetDirectiveValue($line, $INFLine) -----------------------------------------------------------#
sub GetDirectiveValue {
    my($line)    = $_[0], shift;
    my($INFLine) = @_;
    my($DONE)    = FALSE;
    my($TempVar);
    my($UnalteredVar);

    AddErrorRef(1087, $line) if (($INFLine =~ m/=/) && ($INFLine !~ m/=\w*/));

    $INFLine =~ s/^.*=/=/;          # eat left part
    $INFLine =~ s/\s*$//;           # eat trailing spaces
    $INFLine =~ s/^=\s*//;          # eat leading spaces & =

    # Swap out any %string% identifiers for their literal values.
    while (! $DONE) {
        if ($INFLine =~ /%(.*?)%/) {
            $TempVar = $1;
            $UnalteredVar = $TempVar;
            # A few directives allow a '%' as valid inline characters.
            # Move around these by testing for value seperator ',' within
            #   the theoretical string.
            if ($TempVar !~ /,/) {
                $TempVar = quotemeta($TempVar);
                if (defined($StringVals{uc($TempVar)})) {
                    $INFLine =~ s/%$TempVar%/$StringVals{uc($TempVar)}/;
                    # To avoid self-referencing loops
                    #$DONE = TRUE if (uc("%$TempVar%") eq uc("$StringVals{uc($TempVar)}"));
                } else {
                    # Error if the %string% is not defined.
                    AddErrorRef(1025,$line,"%$UnalteredVar%");
                    $INFLine =~ s/%$TempVar%/$TempVar/;
                }
            } else {
                $DONE = TRUE;
            }
        } else {
            $DONE = TRUE;
        }
    }
    # Swap out QuotedString if necessary.
    $INFLine = $AllQuotes{$1} if($INFLine =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
    $INFLine =~ s/%(.*)%/$StringVals{uc($1)}/ if (($INFLine =~ /%(.*)%/) and defined($StringVals{uc($1)}));
    $INFLine =~ s/__QUOTED_STR_\(([0-9]*)\)__/$AllQuotes{$1}/;

    if ($INFLine =~ /^$/) { AddErrorRef(1087, $line);}
    return($INFLine); #return value
}


#-------------------------------------------------------------------------------------------------#
# Returns the string associated with ErrorNum                                                     #
#-- GetError($ErrorNum) --------------------------------------------------------------------------#
sub GetError {
    my($ErrorNum) = $_[0];

    my(%ErrorTable) = (
        1001 => ["Incorrect section definition."],
        1002 => ["Section redefined. [%s] already defined at line %d. Not processing this section."],
        1003 => ["Section [%s] not defined."],
        1004 => ["Entry value should be in quotes."],
        1005 => ["String has more than %d characters."],
        1006 => ["Key %s redefined"],
        1007 => ["Provider %s not found in [STRINGS] section."],
        1008 => ["Signature is expected to be \"\$CHICAGO\$\" or \"\$WINDOWS NT\$\""],
        1009 => ["Class is expected to be \"%s\"."],
        1010 => ["ClassGUID is expected to be \"%s\"."],
		1011 => ["There can not be more than one controlling service - use SPSVCINST_ASSOCSERVICE for one AddService entry only."],
		1012 => ["Version must be in the format w[.x.y.z] where w, x, y, and z are each decimal values of one word in size."],
        1013 => ["Not more than one token on the left of \"=\"."],
		1014 => ["The first field of version must be greater than 0."],
		1015 => ["Each segment of version mus be between 0 and 65535."],
		1016 => ["Version can be at most x.y.w.z where each letter represents a decimal which is one word in size."],

        1022 => ["%s should not have a value."],

        1024 => ["Only one section allowed for %s."],
        1025 => ["No value for %s."],

        1028 => ["This line is missing the double quote (\") at the end of the line. Adding one to continue processing."],
        1029 => ["\"CopyFiles\" directive should not copy files of type .inf."],

        1031 => ["Not more than one word or token on the left of \"=\"."],

        1034 => ["File too big, maximum file size is 62K."],

        1039 => ["No manufacturer defined in section [%s]."],

        1041 => ["Last value is empty, remove trailing commas."],
        1042 => ["File_section keyword redefined."],
        1043 => ["DirID value is missing."],
        1044 => ["Wrong value for DirID."],

        1047 => ["Section not referenced by any copyfiles, renfiles or delfiles keywords."],
        1048 => ["String entry already defined."],

        1053 => ["[VERSION] must contain a CLASS directive."],
        1054 => ["[VERSION] must contain a CLASSGUID directive. Unable to validate CLASS directive."],
        1055 => ["ClassGUID should be in the format {nnnnnnn-nnnn-nnnn-nnnn-nnnnnnnnnnnn}"],
        1056 => ["Section [%s] not found.\n  Microsoft internal inf's using the LayoutFile directive can ignore (E1056)"],
        1057 => ["Unrecognized directive: %s (may be device class specific)"],
        1058 => ["Directive: %s required in section [%s]"],
        1059 => ["Class %s is either reserved or obsolete"],
        1060 => ["No value for %s"],
        1061 => ["Value for EventLogType should be 'System', 'Application', or 'Security'"],
        1062 => ["The value of %s is expected to be %s"],
        1063 => ["Invalid RegRoot Value: %s"],
        1064 => ["CatalogFile name should be like 'filename.cat' (8.3 format)."],
        1065 => ["InterfaceGUID should be in the format {nnnnnnnn-nnnn-nnnn-nnnn-nnnnnnnnnnnn}"],
        1066 => ["Bad value for %s in %s."],
        1067 => ["DriverVer date is in the future %s"],
        1068 => ["Illegal Value for %s"],
        1069 => ["Year must be in four digit format"],
        1070 => ["Unable to open file: %s\n"],
        1071 => ["Bad value %s for %s. Consult the DDK Documentation."],
        1072 => ["%s not a defined model."],
        1073 => ["Custom defined Classes must have a [ClassInstall32] section."],
        1074 => ["Section names must be less than 255 characters in length."],
        1075 => ["[Version] should always be the first section"],
        1076 => ["New-File-Name and Old-File-Name should be different."],

        1078 => ["Description length is too long. Limit is 1024 characters."],
        1079 => ["Directive: %s required in section [%s]"],
        1080 => ["Directive: %s required in section [%s]"],
        1081 => ["Directive: %s required in section [%s] for WHQL digital signature."],
        1082 => ["Date (%s-%s-%s) is invalid"],
        1083 => ["Section [%s] not referenced"],
        1084 => ["The MIPS & PPC platform extensions are not supported"],
        1085 => ["%s is an invalid decoration for a %s section."],
        1086 => ["Bad value for %s in %s."],
        1087 => ["Value side of directive cannot be blank."],
        1088 => ["%s parameter for %s is not optional"],
        1089 => ["A disabled service canot be installed (StartType=0x4)"],

        1092 => ["Neither a section specific nor a default destination dir has not been specified for this section.\n\t Correct the [DestinationDirs] section."],
        1093 => ["The system will automatically copy the inf file, it should not be listed in a copyfiles section."],
        1094 => ["CopyFiles directives should not list multiple filenames or mix filenames and file-list-sections."],
        1095 => ["%s is not defined in this section."],
        1096 => ["File is listed in unmatching SourceDisksFiles sections between this inf and layout.inf."],
        1097 => ["Source filename ends in \".inf\"- file should not be renamed."],
        1098 => ["Destination filename ends in \".inf\"- file should not be renamed."],
        1099 => ["%s is not a valid LanguageID."],
        1100 => ["Invalid Priority Value for ConfigPriority"],
        1101 => ["Invalid Config Type for ConfigPriority"],
        1102 => ["Invalid IRQ value for IRQconfig"],
        1103 => ["The hexadecimal integer value exceeds range"],
        1104 => ["IRQConfig not assigned according to DDK specification"],
        1105 => ["IOConfig not assigned according to DDK specification"],
        1106 => ["Invalid DMA value for DMAconfig"],
        1107 => ["DMAConfig not assigned according to DDK specification"],
        1108 => ["MEMConfig not assigned according to DDK specification"],
        1109 => ["\"DelFiles\" directive should not delete files of type .inf."],
        1110 => ["File \"%s\" is not listed in the [SourceDisksFiles]."],
        1111 => ["[SourceDisksFiles] section not defined.  Can't check CopyFiles sections."],
        1112 => ["Flag FLG_ADDREG_TYPE_MULTI_SZ must be specified for UpperFilter and LowerFilter Registry entries."],
        1113 => ["Section names with leading or trailing spaces must be enclosed in quotes."],
        1114 => ["INF files should not be listed in SourceDiskFiles."],

        1199 => ["Model %s should be listed in an ExcludeFromSelect directive"],

        9998 => ["%s checked!"],
        9999 => ["I don't know how to process this line"]
    );

    return($ErrorTable{"$ErrorNum"});
}

#-------------------------------------------------------------------------------------------------#
# Creates hash of all files in layout.inf                                                         #
#-- GetLayout($File) -----------------------------------------------------------------------------#
sub GetLayout {
    my($File) = $_[0];
    my($Handle);
    my(@Lines, $line, $Section, @Temp);
    my($DONE) = FALSE;

    # Try to open the file
    if (open(Handle, "$File")) {

        # Store contents then close
        my(@Lines) = <Handle>;
        close(Handle);

        # Make sure flag is true
        $LayoutCheck = TRUE;

        # Run thru each line
        foreach ($line=0;$line<=$#Lines;$line++) {
            $DONE=FALSE;
            if ($Lines[$line]=~/\[(sourcedisksfiles.*)\]/i) {
                $Section=uc($1);
                $line++;

                while (! $DONE) {
                    $Lines[$line] = "." if (!defined($Lines[$line]));
                    chomp($Lines[$line]);
                    $Lines[$line] =~ s/^[\s\t]*//;
                    $Lines[$line] =~ s/[\s\t]*$//;
                    if ($Lines[$line] =~ /=/) {
                        @Temp = split(/=/, $Lines[$line]);
                        $Temp[0] =~ s/^[\s\t]*//;
                        $Temp[0] =~ s/[\s\t]*$//;
                        $LayoutFiles{uc($Temp[0])} = $Section;
                        $line++;
                    } elsif ($Lines[$line] =~ /\[.*\]/) {
                        $line--;
                        $DONE = TRUE;
                    } else {
                        $line++;
                    }
                }
            }
        }

    } else {
        # Couldn't open- error and continue w/o check
        $LayoutCheck = FALSE;
        print(STDERR "ERROR: Cannot open $File. Skipping layout check.\n");
    }
    return($LayoutCheck);
}

#-------------------------------------------------------------------------------------------------#
# this sub extracts a list of elements seperated by ',' from the given line. It strips leading and#
# ending whitespaces. It flags error for whitespaces in elements.                                 #
#                                                                                                 #
# This is slightly more complicated. you can just use split function call to accomplish the same  #
# without the whitespace elimination.                                                             #
#-- GetList($line, $line_num, $check_empty) ------------------------------------------------------#
sub GetList {

    my($line, $Lnum, $check_empty) = @_;
    my(@list, $element);

    if (!defined($check_empty)) {
        $check_empty    = 0;
    }
    if ($check_empty) {
        AddErrorRef(1041, $Lnum) if ($line =~ /,$/);
    }
    @list = split(',', $line);
    foreach $element (@list) {
        $element =~ s/^[\s\t]*//; # remove blanks at ends.
        $element =~ s/[\s\t]*$//; # remove blanks at ends.

        # add_ref_wrn("White spaces in identifier \"$element\".", $Lnum)
        AddWarnRef(2011, $Lnum, $element) if ($element =~ /[\s\t]+/);
    }
    return(@list);  #return value.
}

#-------------------------------------------------------------------------------------------------#
# Returns the string associated with WarnNum                                                      #
#-- GetWarning($WarnNum) -------------------------------------------------------------------------#
sub GetWarning {
    my($WarnNum) = $_[0];

    my(%WarningTable) = (
        2001 => ["Line has more than %d characters."],

        2008 => ["Line has an odd number of \"s. Correct this to avoid erroneous processing."],
        2009 => ["TAB inside quoted string."],
		2010 => ["Possible deep recursion on [%s]."],
        2011 => ["White spaces in identifier \"%s\"."],

        2013 => ["Remove LayoutFile entry. LayoutFile entry used ONLY by internal Microsoft engineering."],
        2014 => ["String tokens inside quotes are interpreted as straight text."],

        2018 => ["A class specific check module does not exist for this device class. Defaulting to generic module."],
        2019 => ["Class %s (ClassGUID %s) is unrecognized."],
        2020 => ["Directive %s should be defined in section %s"],
        2021 => ["String tokens in a [STRINGS] section are interpreted literally."],
        2022 => ["InteractiveInstall should be used only when necessary. See the DDK Documentation."],
        2023 => ["Use a string token, and put localizable text in the [Strings] section."],
        2024 => ["Use of the RunOnce Registry key is not recommended."],
        2025 => ["No value for %s."],
        2026 => ["Line continuation character preceeding section definition."],
        2027 => ["Service_Start_Type %x %s"],

        2029 => ["String %s not referenced."],

        2033 => ["Drivers should be copied to the drivers directory (DirID 12). Check your [DestinationDirs] section."],
        2034 => ["No destination dir was defined for this section.  Your driver is being copied to the DefaultDestDir location specified (%s)."],

        2073 => ["No Copyright information found"],

        2077 => ["System defined classes should not use a [ClassInstall32] section."],

        2083 => ["Section [%s] not referenced"],

        2100 => ["No undecorated [STRINGS] section found. Using [%s] instead."],
        2101 => ["Only system critical devices/drivers should set ErrorControl to %d"],

        9999 => ["The above line was looked at"]
    );

    return($WarningTable{"$WarnNum"});
}

#-------------------------------------------------------------------------------------------------#
# Clears all global variables in order to start a new inf                                         #
#-- InitGlobals($inffile, $DCOptions, [$IncInfDir]) ----------------------------------------------#
sub InitGlobals {
    $ErrorCount      =  0;    # Number of errors in this INF
    $Copyright       =  0;    # Copyright flag (TRUE if found)
    $DCOptions       = $_[1]; # Device Class Specific Options.
    $Included        = "";    # Start with no includes.
    $inffile         = $_[0]; # Name of file we're checking.
    $LAYOUTFILE_USED=FALSE;
    $Models          =";";    # List of models defined starts empty
    $QuoteNum        =  0;    # Count of QuotedStrings
    $WarningCount    =  0;    # Number of Warnings in this INF

    @INFlines        = ();    # No lines read yet.

    %AllQuotes       = ();    # All of these tables start blank
    %CheckedSections = ();
    %ErrorTable      = ();
    %DestDirs        = ();
    %IncludedSections= ();    # No sections NEEDed yet.
    %INFFiles        = ();
    %INFSections     = ();
    %WarningTable    = ();

    %Models          = ();
    %ModelLine       = ();
    %AllExcludes     = ();
    %RealSections    = ();

    $INFFiles{$inffile}= 1;

    # Auto-include system defined strings
    %StringVals      = (
                        "55","PrintProcDir",
                        "54","\$OSLOADER_DIR\$",
                        "53","UserProfileDir",
                        "52","SpoolDriversDir",
                        "51","SpoolDir",
                        "50","\$WinDir\$\\system",
                        "30","\$BootDrive\$",
                        "25","SharedDir",
                        "24","AppsDir",
                        "23","ICMDir",
                        "21","ViewersDir",
                        "20","\$FontsDir\$",
                        "18","HelpDir",
                        "17","\$InfDir\$",
                        "12","\$WinDir\$\\system32\\drivers",
                        "11","\$WINNT\$\\system32",
                        "10","\$Windir\$",
                        "01","\$InfSrcDriveAndPath\$",
                        "-1","AbsPath");

    if (defined($_[2])) {
        $CurrentInfDir = $_[2];
    } else {
        $inffile =~ m/^(.*\\).*/;
        $CurrentInfDir = $1;
    }
    return(SUCCESS);
}

#-------------------------------------------------------------------------------------------------#
# "Learns" all of the INF Sections & creates the global %INFSections                              #
#-- LearnSections(@INFLines) ---------------------------------------------------------------------#
sub LearnSections {
    my(@INF)    =   @_;
    my($CurLine, $LineNum, $Section);
    my($FIRST) = TRUE; # Flag so we know we found the first section already.
    $LineNum = 1;

    foreach $CurLine (@INF) {
        if (defined($CurLine)) {
            # True is this line is a section header.
            if ($CurLine =~ m!^\[(.*)\]$!) {

                # Add to @INFSections if this section isn't defined yet.
                if (! defined($INFSections{uc($1)})) {
                    $Section = $1;
                    if (($Section =~ /^[\s\t]+/) or ($Section =~ /[\s\t]+$/)) {
                        AddErrorRef(1113, $LineNum-1);
                    }
                    $INFSections{uc($Section)}     = $LineNum;
                    $RealSections{uc($Section)}    = $Section; #Hash to preserve case
                    # Set a flag to show that we haven't checked this section yet.
                    $CheckedSections{uc($Section)} = 0;

                    # Flag an error if the first section is not [Version]
                    if ($FIRST) {
                        $FIRST = FALSE;
                        AddErrorRef(1075, $LineNum) unless ($Section =~ /Version/i);
                    }
                } else {
                    # Flag an error if we're redefining a section.
                    AddErrorRef(1002,($LineNum-1),$1,($INFSections{uc($1)}-1));
                }

                # Error if section title is longer than 255 characters
                AddErrorRef(1074,($LineNum-1)) if (length($Section) > 255);

            }
        }
        $LineNum++;
    }

    # Add an end of file marker to the array.
    $INFSections{'EOF'} = $LineNum++;

    # Return the $INFSections array.
    return(%INFSections);
}
#-------------------------------------------------------------------------------------------------#
#-- LearnStrings($line, $lines) ------------------------------------------------------------------#
sub LearnStrings {
    my($line)     = $_[0];
    shift(@_);
    my(@lines)    = @_;
    my($DONE)     = FALSE;
    my($String, $StringValue);

    while (! $DONE) {
        $line++, next if (! defined($lines[$line]));
        $line++, next if ($lines[$line] =~ /^\s$/); # Skip if it's a blank line
        if (($lines[$line] =~ /^\[.*\]/) or ($line > $#INFlines)) { # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        # Start with the INF line
        $String=$lines[$line];

        $String =~ s/=.*$//;      # left part removed
        $String =~ s/^\s*//;      # extract leading spaces
        $String =~ s/\s*$//;      # extract trailing spaces
        $String = $AllQuotes{$1}      if ($String =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
        $String = quotemeta($String);
        $StringValue = $lines[$line];
        $StringValue =~ s/^.*=//; # right part removed
        $StringValue =~ s/^\s*//; # extract leading spaces
        $StringValue =~ s/\s*$//; # extract trailing spaces

        if (defined($StringVals{$String})) {
            $line++;
            next;
        }
        $StringVals{uc($String)} = $StringValue; #Store the value for later use.
        if ($StringValue =~ /%.*%/) {
            $StringVals{uc($String)} = $String;
            AddWarnRef(2021, $line);
        }
        $line++;
    }
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#   This sub does the following:                                                                  #
#       - flags warnings for length > MaxLineLen.                                                 #
#       - joins lines to next line if ending in '\'                                               #
#       - replace quoted strings by __QUOTED_STR_XXXX__ and put the                               #
#           unqouted string in an array. (use XXXX for indexing it.)                              #
#       - flags warnings for tabs in quoted strings.                                              #
#       - gets rid of comments, preceeding and ending whitespaces,                                #
#           end of line.                                                                          #
#       - adds non null strings to the array %INFline - indexing                                  #
#           with the line number. (NOTE: joined lines will be                                     #
#           indexed by the line joined line's line number.  )                                     #
#                                                                                                 #
#-- Preprocess($InfFilename) ---------------------------------------------------------------------#
sub Preprocess {
    my($infile) = $_[0];
    my($prev_line, $line, $CLnum, $poscomma, $posquote1, $posquote2, $bRemoveComment, $comment);
    my($TotalLines, $FileSize);
    my($MaxLineLen) = 128;
    my($path);
    my($CopyFiles) = FALSE;

    $prev_line = "";
    open (INFILE, "$infile")    || die "Can't open $infile for input\n";

    while(<INFILE>) {
        $Copyright = 1 if (/CopyRight/i);
        $CLnum = $.;
        if (/^\;/) {
            $INFlines[$CLnum] = " ";
            next;
        }
        chop if /\n$/;

        if (($prev_line ne "") and ($_ =~ /\[.*\]/)) {
            $line=$_;
            AddWarnRef(2026,$CLnum-1);
            # Handle the previous line first

            #process for quotes.
            $INFlines[$CLnum-1] = ProcQuotes($prev_line, ($CLnum-1));

            #$line =~ tr/\s//;  # Yank out all remaining whitespace
            $INFlines[$CLnum-1] = " "  if ($INFlines[$CLnum-1] =~ /^$/); # $line is empty.
            if (! defined($INFlines[$CLnum-1])) {
                $INFlines[$CLnum-1] = " ";
            }
            $prev_line="";
        } else {
            $line = join("", $prev_line, $_);
            $prev_line = "";
        }

        #process for comments.
        $posquote1  = index($line, '"');
        $posquote2  = ($posquote1 >= 0) ? index($line, '"', $posquote1+1) : -1;
        $poscomma   = index($line, ';');
        $bRemoveComment = 0;

        while ($poscomma >= 0) {
            if ($poscomma < $posquote1 || $posquote1 < 0) {  # ';' before '"' => is comment
                $bRemoveComment = 1;
                last;
            }

            if ($posquote2 >= 0 && $posquote1 >= 0) {
                if ($poscomma < $posquote2) {        # ';' between '"' => string first
                    $poscomma = index($line, ';', $posquote2 + 1);
                }
                # find next string
                $posquote1  = index($line, '"', $posquote2 + 1);
                $posquote2  = ($posquote1 >= 0) ? index($line, '"', $posquote1+1) : -1;
                $INFlines[$CLnum] = " ";
                next;
            }
            last;    # ';' after '"' => sting not terminated yet, string first
        }

        if ($bRemoveComment == 1) {
            $comment = substr($line, $poscomma+1);
            $line = substr($line, 0, $poscomma);
        }

        $line =~ s/^[\s\t]*//; #strip leading white spaces.
        $line =~ s/[\s\t]*$//; #strip ending white spaces and eol.
        $INFlines[$CLnum] = " ", next if ($line =~ /^$/);

        if ($line =~ /(.)*\\$/) {
            # need to join this line to the next.
            $line =~ s/\\$//;
            $prev_line = $line;
            $INFlines[$CLnum] = " ";
            next; # process joined line together for quotes.
        }

        #process for quotes.
        $line = ProcQuotes($line, $CLnum);

        #$line =~ tr/\s//;  # Yank out all remaining whitespace
        $line = " "  if ($line =~ /^$/); # $line is empty.
        if (! defined($line)) {
            $line = " ";
        }
        #put the preprocessed line in assoc array indexed by the linenum.
        $INFlines[$CLnum] = $line;
    }

    $TotalLines = $.;

    my($lpVAR);
    for ($lpVAR=0;$lpVAR <= ($#INFlines);$lpVAR++) {
        if (! defined($INFlines[$lpVAR])) {
            $INFlines[$lpVAR] = " ";
        }
        # Check to see if we have any copyfiles directives
        if (! $CopyFiles) {
            if ($INFlines[$lpVAR] =~ /copyfiles.*=/i) {
                $CopyFiles = TRUE;
            }
        }
    }

    $TotalLines = $.;
    $TotalLines++;

# Win9x specific
#    $FileSize = -s INFILE;
#    AddErrorRef(1034, $TotalLines) if ($FileSize > 62*1024);


    $INFlines[$TotalLines++] = " ";
    $INFlines[$TotalLines] = "[EOF]";
    $LastLine = $TotalLines;

    close (INFILE);
    %INFSections = LearnSections(@INFlines);

    # Let's verify that a few required sections actually exist!
    unless (defined($INFSections{'VERSION'})) {
        AddErrorRef(1003, $TotalLines, "Version");
    }

    unless (defined($INFSections{'MANUFACTURER'})) {
        AddErrorRef(1003, $TotalLines, "Manufacturer");
    }

    if ($CopyFiles) {
        unless (defined($INFSections{'SOURCEDISKSNAMES'}) || defined($INFSections{'SOURCEDISKSNAMES.X86'})) {
            AddErrorRef(1056, $TotalLines, 'SourceDisksNames');
        }

        unless (defined($INFSections{'SOURCEDISKSFILES'}) || defined($INFSections{'SOURCEDISKSFILES.X86'})) {
            AddErrorRef(1003, $TotalLines, 'SourceDisksFiles') if (defined($INFSections{'SOURCEDISKSNAMES'}) ||
                                                                   defined($INFSections{'SOURCEDISKSNAMES.X86'}));
        }

        unless (defined($INFSections{'DESTINATIONDIRS'})) {
            AddErrorRef(1003, $TotalLines, 'DestinationDirs');
        }
    }

    unless (defined($INFSections{'STRINGS'})) {
        AddErrorRef(1003, $TotalLines, 'Strings');
    }

    return(@INFlines);
}

#-------------------------------------------------------------------------------------------------#
#-- PrintResults($InfFile, $OutFile) -------------------------------------------------------------#
sub PrintResults {
    my($InfFile)    =   $_[0];
    my($OutFile)    =   $_[1];
    my($IN, $OUT, $NTLOG)   =   "";
    my($err, @errs);
    my($lp, $count);

    open(IN,   "$InfFile") || die("Couldn't open $InfFile: $!\n");
    open(OUT, ">$OutFile") || die("Couldn't open $OutFile: $!\n");
    if ($CreateLog) {
        open(NTLOG, ">>$LogFile") || die("Couldn't open $LogFile: $!\n");
        print(NTLOG "$InfFile: ") if ($CreateLog);
        ($ErrorCount == 0) ?    print(NTLOG "PASSED\n") : print(NTLOG "FAILED\n");
        print( NTLOG "NTLOG REPORT--------------\n");
        printf(NTLOG "Total Lines: %10i  |\n", $#INFlines);
        printf(NTLOG "Total Errors: %9i  |\n", $ErrorCount);
        printf(NTLOG "Total Warnings: %7i  |\n", $WarningCount);
        print( NTLOG "--------------------------\n\n");

    }

    print(OUT "<HTML><HEAD><TITLE>Summary of ChkInf results for $InfFile</TITLE></HEAD>\n<BODY VLINK=#000000>\n");

    print(OUT "<H1>Summary of \"<FONT COLOR=#990099>$InfFile</FONT>\"</H1>\n");
    print(OUT "<TABLE CELLSPACING=0 CELLPADDING=0 BORDER=0>\n");

    if ($ErrorCount >= 1) {
       print(OUT "\t<TR><TD><H3><A HREF=\"#ERRORS\">Total Errors:</A></TD>\n");
       print(OUT "\t    <TD>&nbsp;&nbsp;&nbsp;&nbsp;</TD>\n");
       print(OUT "\t    <TD ALIGN=RIGHT><H3><FONT COLOR=RED>$ErrorCount</FONT></TD></TR>\n");
    }

    if ($WarningCount >= 1) {
        print(OUT "\t<TR><TD><H3><A HREF=\"#WARNINGS\">Total Warnings:</A></TD>\n");
        print(OUT "\t    <TD>&nbsp;&nbsp;&nbsp;&nbsp;</TD>\n");
        print(OUT "\t    <TD ALIGN=RIGHT><H3><FONT COLOR=BLUE>$WarningCount</FONT></TD></TR>\n");
    }

    if (($ErrorCount == 0) and ($WarningCount == 0)) {
       print(OUT "\t<TR><TD><H3><FONT COLOR=#990099><B><I>INF Passed with no warnings or errors!</I></B></FONT></H3></TD></TR>\n");
    }


    if (keys(%IncludedSections) >= 1) {
        print(OUT "\t<TR><TD><H3><A HREF=\"#NEEDS\">Included Sections:</A></TD>\n");
        print(OUT "\t    <TD>&nbsp;&nbsp;&nbsp;&nbsp;</TD>\n");
        printf(OUT "\t<TD ALIGN=RIGHT><H3><FONT COLOR=GREEN>%i</FONT></TD></TR>\n", $count=keys(%IncludedSections));
    }


    print(OUT "</TABLE><BR>\n");
    if (keys(%INFFiles) > 1) {
        print(OUT "<TABLE>\n");
        print(OUT "<TR><TD><H3>Inf file(s) parsed:</TD><TD><TT>");
        foreach (sort(keys(%INFFiles))) {
            print OUT "<LI>$_</TT></TD></TR>\n<TR><TD></TD><TD><TT>";
        }
        print(OUT "</TT></TD></TR></TABLE>\n");
    }

    print(OUT "\t<HR>\n") if ($ErrorCount >= 1);
    print(OUT "\t<FONT COLOR=RED><H3><A NAME=\"ERRORS\">Errors:</A></H3>\n")
        if ($ErrorCount >= 1);
    for ($lp=0;$lp<=($#INFlines+2);$lp++) { # Because we write past the number of INFLines for general errors
        if (defined ($err = $ErrorTable{$lp})) {
            @errs = split ('; ', $err);
            foreach $err (@errs) {
                next if ($err=~ /^$/);
                print(OUT   "\t\t<LI><A HREF=\"#Lnum$lp\">Line $lp</A>: $err \n");
                print(NTLOG "Line $lp: ERROR: $err \n") if ($CreateLog);
            }
        }
    }

    print(OUT "\t</FONT>\n");

    print(OUT "\t<HR>\n") if ($WarningCount >= 1);
    print(OUT "\t<FONT COLOR=BLUE><H3><A NAME=\"WARNINGS\">Warnings:</A></H3>\n")
        if ($WarningCount >= 1);
    for ($lp=0;$lp<=($#INFlines+2);$lp++) {# Because we write past the number of INFLines for general warnings
        if (defined ($err = $WarningTable{$lp})) {
            @errs = split ('; ', $err);
            foreach $err (@errs) {
                next if ($err=~ /^$/);
                print(OUT   "\t\t<LI><A HREF=\"#Lnum$lp\">Line $lp</A>: $err \n");
                print(NTLOG "Line $lp: WARNING: $err \n") if ($CreateLog);
            }
        }
    }
    print(OUT "\t</FONT>\n");

    print(OUT "\t<HR>\n") if (keys(%IncludedSections) >= 1);
    print(OUT "\t<FONT COLOR=GREEN><H3><A NAME=\"NEEDS\">Sections included from other infs:</A></H3>\n")
        if (keys(%IncludedSections) >= 1);
    foreach (sort(keys(%IncludedSections))) {
        print(OUT "\t\t<LI><A HREF=\"#$_\">$_</A>\n");
    }
    print(OUT "\t</FONT>\n");

    if (defined($Module)) {
        $Module->PrintHeader();
    }

    print(OUT "\t<BR><HR>\n<H3>Annotated INF:</H3>\n<PRE>\n");
    while(<IN>) {
        print(OUT "<A NAME=\"Lnum", $., "\">");
        chop if /\n$/;           # removes \n
        s/</&lt\;/g;             # so that htm interprets them right
        s/>/&gt\;/g;
        print(OUT);
        print(OUT "</A>\n");

        if (defined ($err = $ErrorTable{$.})) {
            print(OUT "<FONT COLOR=RED>");
            @errs = split ('; ', $err);
            foreach $err (@errs) {
                next if ($err=~ /^$/);
                print(OUT "; $err \n");
            }
            print(OUT "</FONT>");
        }

        if (defined ($err = $WarningTable{$.})) {
            print(OUT "<FONT COLOR=BLUE>");
            @errs = split ('; ', $err);
            foreach $err (@errs) {
                next if ($err=~ /^$/);
                print(OUT "; $err \n");
            }
            print(OUT "</FONT>");
        }
    }

    $lp = $.; # Last line of file
    $lp++;

    print(OUT "<A NAME=\"Lnum", $lp, "\">"); # Print out any warnings or errors
    print(OUT "</A>\n");                     # that have been flagged for the end
                                             # of the file.
    if (defined ($err = $ErrorTable{$lp})) { # Putting them on the actual last line
        print(OUT "<FONT COLOR=RED>");       # can make output very ugly if another
        @errs = split ('; ', $err);          # error or warning is also flagged
        foreach $err (@errs) {               # for that line by a later subroutine!
            next if ($err=~ /^$/);
            print(OUT "; ERROR: $err \n");
        }
        print(OUT "</FONT>");
    }
    if (defined ($err = $WarningTable{$lp})) {
        print(OUT "<FONT COLOR=BLUE>");
        @errs = split ('; ', $err);
        foreach $err (@errs) {
            next if ($err=~ /^$/);
            print(OUT "; $err \n");
        }
        print(OUT "</FONT>");
    }

    $lp++;    # Step past it by one

    print(OUT "<A NAME=\"Lnum", $lp, "\">"); # Print out any warnings or errors
    print(OUT "</A>\n");                     # that have been flagged for the end
                                             # of the file.
    if (defined ($err = $ErrorTable{$lp})) { # Putting them on the actual last line
        print(OUT "<FONT COLOR=RED>");       # can make output very ugly if another
        @errs = split ('; ', $err);          # error or warning is also flagged
        foreach $err (@errs) {               # for that line by a later subroutine!
            next if ($err=~ /^$/);
            print(OUT "; ERROR: $err \n");
        }
        print(OUT "</FONT>");
    }
    if (defined ($err = $WarningTable{$lp})) {
        print(OUT "<FONT COLOR=BLUE>");
        @errs = split ('; ', $err);
        foreach $err (@errs) {
            next if ($err=~ /^$/);
            print(OUT "; $err \n");
        }
        print(OUT "</FONT>");
    }

    print(OUT "</PRE>\n");

    print(OUT "<HR>\n<H3><A NAME=\"INCS\">Sections included from other infs:</A></H3>\n<PRE>\n")
        if (keys(%IncludedSections) >= 1);

    close(IN);

    my($ThisSec, $ThisFile, $RelLineNum, @TheseLines, $Continue, $AbsLineNum);
    foreach $ThisSec (sort(keys(%IncludedSections))) {
        $ThisFile = $inffile; # To init the array.
        
        foreach (keys(%INFFiles)) {

            # Record File Section Is From By Referencing Line Nums Recorded For Each File

            if ( ($INFFiles{$_} <= $INFSections{$ThisSec}) and
                 ($INFFiles{$_} >= $INFFiles{$ThisFile}) ) {
                    $ThisFile = $_;
            }

        } # $ThisFile Now Has the Name of the File the $ThisSec is From
        # What Line In the File Does the Section Start On?
        # Header line not included in earlier count
        $RelLineNum = $INFSections{$ThisSec} - $INFFiles{$ThisFile} - 2;
        $AbsLineNum = $INFSections{$ThisSec} -1;

        open(IN, "$ThisFile") || die("Couldn't open $ThisFile: $!\n");
        @TheseLines = <IN>;
        close(IN);
        # Add [EOF] to end of file
        $TheseLines[++$#TheseLines]="[EOF]";

        print(OUT "</PRE>\n<A NAME=\"$ThisSec\"><FONT COLOR=GREEN>From $ThisFile:</FONT></A>\n<PRE>\n");
        # $TheseLines[$RelLineNum] = " " unless (defined($TheseLines[$RelLineNum]));
        # 25 lines to print the full section header
        print(OUT "<A NAME=\"Lnum", $AbsLineNum, "\">");

        next if (! defined($TheseLines[$RelLineNum]));

        chomp($TheseLines[$RelLineNum]); # removes \n
        $TheseLines[$RelLineNum] =~ s/</&lt\;/g;# so that htm interprets them right
        $TheseLines[$RelLineNum] =~ s/>/&gt\;/g;
        print(OUT "$TheseLines[$RelLineNum]");
        print(OUT "</A>\n");
        if (defined ($err = $ErrorTable{$AbsLineNum})) {
            print(OUT "<FONT COLOR=RED>");
            @errs = split ('; ', $err);
            foreach $err (@errs) {
                next if ($err=~ /^$/);
                print(OUT "; ERROR: $err \n");
            }
            print(OUT "</FONT>");
        }
        if (defined ($err = $WarningTable{$AbsLineNum})) {
            print(OUT "<FONT COLOR=BLUE>");
            @errs = split ('; ', $err);
            foreach $err (@errs) {
                next if ($err=~ /^$/);
                print(OUT "; $err \n");
            }
            print(OUT "</FONT>");
        }
        $AbsLineNum++;
        $RelLineNum++;
        $Continue = TRUE;
        while ($Continue) {
            # Stop at next section header.
            # $Continue = FALSE, next if (! defined($TheseLines[$RelLineNum]));
            if ($TheseLines[$RelLineNum] =~ /^\[.*\]/) {
                $Continue = FALSE;
                next;
            }

            print(OUT "<A NAME=\"Lnum", $AbsLineNum, "\">");
            chomp($TheseLines[$RelLineNum]); # removes \n
            $TheseLines[$RelLineNum] =~ s/</&lt\;/g; # so that htm interprets them right
            $TheseLines[$RelLineNum] =~ s/>/&gt\;/g;
            print(OUT $TheseLines[$RelLineNum]);
            print(OUT "</A>\n");

            if (defined ($err = $ErrorTable{$AbsLineNum})) {
                print(OUT "<FONT COLOR=RED>");
                @errs = split ('; ', $err);
                foreach $err (@errs) {
                    next if ($err=~ /^$/);
                    print(OUT "; ERROR: $err \n");
                }
                print(OUT "</FONT>");
            }
            if (defined ($err = $WarningTable{$AbsLineNum})) {
                print(OUT "<FONT COLOR=BLUE>");
                @errs = split ('; ', $err);
                foreach $err (@errs) {
                    next if ($err=~ /^$/);
                    print(OUT "; $err \n");
                }
                print(OUT "</FONT>");
            }
            $AbsLineNum++;
            $RelLineNum++;
        }
    }
    print(OUT "</PRE>\n");
    print(NTLOG "\n\n") if ($CreateLog);
    close(NTLOG) if ($CreateLog);
    close(IN);
    close(OUT);

    $FilesChecked{$InfFile} = "$WarningCount:$ErrorCount:$OutFile:$#INFlines";

}

#-------------------------------------------------------------------------------------------------#
#-- PrintSummary() -------------------------------------------------------------------------------#
sub PrintSummary {

    my($File,@Values,$Link);
    my(@Keys) = sort(keys(%FilesChecked));
    my($TotalErrors)  =  0;

    open(OUT, ">$_[0]") || die("Couldn't open $_[0]: $!\n");

    print(OUT "<HTML><HEAD><TITLE>Summary of ChkInf Results</TITLE></HEAD><BODY LINK=#000000 VLINK=#000000>\n");
    print(OUT "\tChkInf v.$Version: &copy; Copyright 1999 Microsoft Corp.<BR>\n");
    print(OUT "\t<HR SIZE=10>\n");
    print(OUT "\t<H4>Files processed: ");
    print(OUT (@Keys+0));
    print(OUT "</H4>\n\t<BR>\n");
    print(OUT "\t<TABLE CELLPADDING=3 BORDER=0>\n");
    foreach $File (@Keys) {
        @Values = split(/:/,$FilesChecked{$File});
        $Values[2] =~ s/^\.\\htm\\//;
        print(OUT "\t\t<TR><TD><FONT SIZE=+1><TT><A HREF=\"$Values[2]\">$File:</A></TT></FONT></TD>\n");
        print(OUT "\t\t<TD><FONT COLOR=#FF0000>Errors  : $Values[1]</FONT></TD></TR>\n");
        print(OUT "\t\t<TR><TD></TD><TD><FONT COLOR=#0000FF>Warnings: $Values[0]</FONT></TD></TR>\n");
        print(OUT "\t\t<TR><TD ROWSPAN=2><FONT COLOR=#FFFFFF>&nbsp;&nbsp;&nbsp;</FONT></TD></TR>\n");
        print(OUT "\t\t<TR><TD> </TD></TR>\n\n");
        $TotalErrors += $Values[1] if (defined($Values[1]));
    }
    print(OUT "\t</TABLE>\n");
    print(OUT "</BODY></HTML>\n");

    close(OUT);

    return($TotalErrors);
}

#-----------------------------------------------------------------------------------------------------#
#-- ProcIncludedStrings() ----------------------------------------------------------------------------#
sub ProcIncludedStrings {
    my($line)  = $_[0];
    my(@lines) = @INFlines;
    my($CurrentString, $String, $StringValue, %FStr, $UString, $Str);
    my($DONE)      = FALSE;
    my($MaxLength) = 1024;

    print(STDERR "\tInvoking ProcIncludedStrings...\n") if ($DEBUG);
    while (! $DONE) {

        $line++, next if (! defined($lines[$line]));
        $line++, next if ($lines[$line] =~ /^\s*$/); # Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {
            # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        GetLine($line) if ($DEBUG);

        $String      = uc(GetDirective($line,$lines[$line]));
        $StringValue = uc(GetDirectiveValue($line,$lines[$line]));

        if (defined($FStr{$String})) {
            # Error if this string is already defined
            $line++;
            next;
        }

        #   right side should be in quotes or a numeric value
        if($StringValue !~ /^__QUOTED_STR_\(([0-9]*)\)__$/) {
            # Swap out QuotedString if need be
            if ($StringValue !~ /^(0x)??[0-9a-f]*$/i) {
                # AddErrorRef(1004, $line);  # "Entry value should be in quotes."
                $FStr{$String} = $StringValue;
            }
        } else {
            $FStr{$String} = $AllQuotes{$1};
            # "String has more than $MaxStrLen characters."
        }

        $line++;

    }

    foreach $Str (sort(keys(%FStr))) {
        if (!defined($StringVals{$Str})) {
            $StringVals{$Str} = $FStr{$Str};
        }
    }

    print(STDERR "\tExiting ProcIncludedStrings...\n") if ($DEBUG);
    return(TRUE);
}

#-----------------------------------------------------------------------------------------------------#
#-- ProcInf() ----------------------------------------------------------------------------------------#
sub ProcInf {
        if (! $Copyright) {
            # Must add to line 1, not 0.
            AddWarnRef(2073, 1);
        }

        # If we have a [STRINGS] section Read it Into Memory
        if (defined($INFSections{'STRINGS'})) {
            print(STDERR "\tInvoking LearnStrings...\n") if ($DEBUG);
            LearnStrings($INFSections{'STRINGS'}, @INFlines);
            print(STDERR "\tExiting LearnStrings...\n") if ($DEBUG);
        } else {
            my($sec);
            foreach $sec (sort(keys(%INFSections))) {
                if (($sec =~ /^STRINGS\.\w{4}$/i) && (! defined($INFSections{'STRINGS'}))) {
                    AddWarnRef(2100,$INFSections{$sec},$RealSections{$sec});
                    $INFSections{'STRINGS'}=$INFSections{$sec};
                    LearnStrings($INFSections{'STRINGS'}, @INFlines);
                }
            }
        }

        # If we have a [VERSION] section (and we should!) verify it
        if (defined($INFSections{'VERSION'})) {
            VerifyVersion($INFSections{'VERSION'}, "VERSION");
        }

        # Create hashes needed for INF checking
        if (defined($INFSections{'DESTINATIONDIRS'}) && ($INFSections{'DESTINATIONDIRS'} <= $LastLine)) {
            CompileDestDirs($INFSections{'DESTINATIONDIRS'}, "DESTINATIONDIRS");
        }

        #--------------------------------------------------------#
        # Check all the standard sections                        #
        #--------------------------------------------------------#
        if (defined($INFSections{'MANUFACTURER'})) {
            ProcManu($INFSections{'MANUFACTURER'}, @INFlines);
        }

        if (defined($INFSections{'CONTROLFLAGS'}) && ($INFSections{'CONTROLFLAGS'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifyCFlagsSection...\n") if ($DEBUG);
            VerifyCFlagsSection($INFSections{'CONTROLFLAGS'}, "CONTROLFLAGS");
        }

        my($device, $hw_id, $current, %Spew, $exclude, $match);

        foreach $device (sort(keys(%Models))) {
            $match=FALSE;
            print("\tChecking $device for ExcludeFromSelect\n") if ($DEBUG);
            $current=$Models{$device}[0];
            # Don't check for *DEVICE if BUS\DEVICE is excluded
            next if (defined($AllExcludes{uc($current)}));
            foreach $exclude (sort(keys(%AllExcludes))) {
                $exclude =~ s/\*/\.\*/;
                $exclude =~ s/\\/\\\\/;
                if ($current =~ /$exclude/i) {
                    $match = TRUE;
                }
            }

            if ($match){
                print("Excluded: $current\n") if ($DEBUG);
            } else {
                print(".".$current.".\n") if (!defined $ModelLine{uc($current)});
                AddErrorRef(1199, $ModelLine{uc($current)}, $current)
                        unless (defined($Spew{$current}) or
                                (($current !~ m/^\*/) and ($current !~ /\\/)));
                $Spew{$current} = TRUE;
            }

        }

        if (defined($INFSections{'CLASSINSTALL32'}) && ($INFSections{'CLASSINSTALL32'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifyClassInstall32Section...\n") if ($DEBUG);
            VerifyClassInstall32Section($INFSections{'CLASSINSTALL32'}, "CLASSINSTALL32");
        }

        if (defined($INFSections{'CLASSINSTALL32.NT'}) && ($INFSections{'CLASSINSTALL32.NT'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifyClassInstall32Section...\n") if ($DEBUG);
            VerifyClassInstall32Section($INFSections{'CLASSINSTALL32.NT'}, "CLASSINSTALL32.NT");
        }

        if (defined($INFSections{'CLASSINSTALL32.NTX86'}) && ($INFSections{'CLASSINSTALL32.NTX86'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifyClassInstall32Section...\n") if ($DEBUG);
            VerifyClassInstall32Section($INFSections{'CLASSINSTALL32.NTX86'}, "CLASSINSTALL32.NTX86");
        }

        if (defined($INFSections{'SOURCEDISKSNAMES'}) && ($INFSections{'SOURCEDISKSNAMES'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifySourceDisksNames...\n") if ($DEBUG);
            VerifySDNames($INFSections{'SOURCEDISKSNAMES'}, "SOURCEDISKSNAMES");
        }

        if (defined($INFSections{'SOURCEDISKSNAMES.X86'}) && ($INFSections{'SOURCEDISKSNAMES.X86'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifySourceDisksNames...\n") if ($DEBUG);
            VerifySDNames($INFSections{'SOURCEDISKSNAMES.X86'}, "SOURCEDISKSNAMES.X86");
        }

        # Commonly mis-decorated section
        if (defined($INFSections{'SOURCEDISKSNAMES.NTX86'}) && ($INFSections{'SOURCEDISKSNAMES.NTX86'} <= $LastLine)) {
           AddErrorRef(1085,$INFSections{'SOURCEDISKSNAMES.NTX86'},"NTX86","SourceDiskNames");
        }

        # Commonly mis-decorated section
        if (defined($INFSections{'SOURCEDISKSNAMES.NTALPHA'}) && ($INFSections{'SOURCEDISKSNAMES.NTALPHA'} <= $LastLine)) {
           AddErrorRef(1085,$INFSections{'SOURCEDISKSNAMES.NTALPHA'},"NTALPHA","SourceDiskNames");
        }

        # Commonly mis-decorated section - ALPHA no longer suppported
        if (defined($INFSections{'SOURCEDISKSNAMES.ALPHA'}) && ($INFSections{'SOURCEDISKSNAMES.ALPHA'} <= $LastLine)) {
           AddErrorRef(1085,$INFSections{'SOURCEDISKSNAMES.ALPHA'},"ALPHA","SourceDiskNames");
        }

        if (defined($INFSections{'SOURCEDISKSFILES'}) && ($INFSections{'SOURCEDISKSFILES'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifySourceDisksFiles...\n") if ($DEBUG);
            VerifySDFiles($INFSections{'SOURCEDISKSFILES'}, "SOURCEDISKSFILES");
        }

        if (defined($INFSections{'SOURCEDISKSFILES.X86'}) && ($INFSections{'SOURCEDISKSFILES.X86'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifySourceDisksFiles...\n") if ($DEBUG);
            VerifySDFiles($INFSections{'SOURCEDISKSFILES.X86'}, "SOURCEDISKSFILES.X86");
        }

        # Commonly mis-decorated section
        if (defined($INFSections{'SOURCEDISKSFILES.NTX86'}) && ($INFSections{'SOURCEDISKSFILES.NTX86'} <= $LastLine)) {
           AddErrorRef(1085,$INFSections{'SOURCEDISKSFILES.NTX86'},"NTX86","SourceDiskFiles");
        }

        # Commonly mis-decorated section
        if (defined($INFSections{'SOURCEDISKSFILES.NTALPHA'}) && ($INFSections{'SOURCEDISKSFILES.NTALPHA'} <= $LastLine)) {
           AddErrorRef(1085 ,$INFSections{'SOURCEDISKSFILES.NTALPHA'},"NTALPHA","SourceDiskFiles");
        }

        # Commonly mis-decorated section - ALPHA no longer supported
        if (defined($INFSections{'SOURCEDISKSFILES.ALPHA'}) && ($INFSections{'SOURCEDISKSFILES.ALPHA'} <= $LastLine)) {
           AddErrorRef(1085 ,$INFSections{'SOURCEDISKSFILES.ALPHA'},"ALPHA","SourceDiskFiles");
        }

        if (defined($INFSections{'INTERFACEINSTALL32'}) && ($INFSections{'INTERFACEINSTALL32'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifyInterfaceInstall32...\n") if ($DEBUG);
            VerifyInterfaceInstall32Section($INFSections{'INTERFACEINSTALL32'}, "INTERFACEINSTALL32");
        }
        # We need to check this last so we can verify that the sections are refernced
        #       by a CopyFiles, RenFiles, or DelFiles
        if (defined($INFSections{'DESTINATIONDIRS'}) && ($INFSections{'DESTINATIONDIRS'} <= $LastLine)) {
            print(STDERR "\tInvoking VerifySourceDestinationDirs...\n") if ($DEBUG);
            VerifyDestDirsSection($INFSections{'DESTINATIONDIRS'}, "DESTINATIONDIRS");
        }
        # Now that all else is checked, we can see which strings were & were not used.
        if (defined($INFSections{'STRINGS'}) && ($INFSections{'STRINGS'} <= $LastLine)) {
            ProcStrings($INFSections{'STRINGS'}, @INFlines) || die("Couldn't ProcStrings!\n");
        } else {
            my($sec);
            foreach $sec (sort(keys(%INFSections))) {
                if (($sec =~ /^STRINGS\.\w{4}$/i) && (! defined($INFSections{'STRINGS'}))) {
                    AddWarnRef(2100,$INFSections{$sec},$RealSections{$sec});
                    $INFSections{'STRINGS'}=$INFSections{$sec};
                    ProcStrings($INFSections{'STRINGS'}, @INFlines);
                }
            }
        }

        if (! defined($Module) ) {
            # warn("The variable \$Module is undefined!\n");
            return(FALSE);
        } else {
            $Module->InfCheck();
        }

        # Need to find unreferenced sections & invalid section extensions
        CheckStringsWithLangID();

        foreach (sort(keys(%INFSections))) {
            AddWarnRef(2083, ($INFSections{$_}-1),$RealSections{$_} )
                    unless (( $CheckedSections{$_}==TRUE ) or ( m/^EOF$/i));# or ( m/^STRINGS\.\d*$/i));
            AddErrorRef(1084, $INFSections{$_},$RealSections{$_} ) if (m/PPC$/i || m/MIPS$/i);
        }
        return(TRUE);
}
#-------------------------------------------------------------------------------------------------#
# This section looks for the Manufacurer section                                                  #
#                                                                                                 #
# Globals changed:                                                                                #
#   ManuList                                                                                      #
#                                                                                                 #
#-- ProcManu($line, @lines) ----------------------------------------------------------------------#
sub ProcManu {
my($line, @lines) = @_;
my($DONE)         = FALSE;
my($Directive, $Value);
my($count)        = 0;

    $CheckedSections{"MANUFACTURER"} = TRUE;

    print(STDERR "\tInvoking ProcManu...\n") if ($DEBUG);

    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/); # Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {           # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        GetLine($line) if ($DEBUG);

        # [Manufacturers] may not have both a directive & a value
        if ($lines[$line] =~ /=/) {
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));
            $ManuList{uc($Directive)} = "$Value";
       } else {
            $Directive=  'x';                # If just a mfg name, define directive to avoid errors.
            $Value    =   uc($lines[$line]); # Set Value to the name
            $Value    =~  s/^\"(.*)\"/$1/;   # And Strip the Quotes!
            $ManuList{uc($Value)} = "x";
        }

        # Replace with stored string if necessary!!
        $Value = $AllQuotes{$1} if ($Value =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);

        if (defined($INFSections{uc($Value)})) {
            $count++; # Count models successfully being checked.
            VerifyModelSection($INFSections{uc($Value)}, uc($Value));
        } else {
            # INF Section for this model isn't defined.
            AddErrorRef(1003,$line, $Value);
        }
        $line++;
    }

    if (keys(%ManuList) == 0) {
       # Must have atleast one manufacturer.
       AddErrorRef(1039, $INFSections{"MANUFACTURER"}, $RealSections{"MANUFACTURER"});
    }
    print(STDERR "\tLeaving ProcManu...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# this sub extracts quoted strings from the line and puts them (unquoted) in assoc_array AllQuotes#
# index by a number and replaces the quoted string by __QUOTED_STR_XXX__ where XXX is the index   #
# number.                                                                                         #
#                                                                                                 #
#-- ProcQuotes($line, $linenum) ------------------------------------------------------------------#
sub ProcQuotes {
    my($line, $Lnum) = @_;
    my($cnt, $quote);
    $_ = $line;

    # Count the number of ", and tack on extra " if we have an odd number;
    # otherwise perl faults!
    $cnt = tr/"/"/;

    if ($cnt % 2) {
        # check if this odd quote is after a semicolon.
        if ($_ =~ /(.(".*")*)*;.*"(.*".*")*/) {
            AddWarnRef(2008, $Lnum);
        } else {
            AddErrorRef(1028, $Lnum);
        }
        $_ = "$_\"";
    }

    # Process compound quoted strings one-at-a-time, saving the quoted string
    # when we find it and doing some further processing to replace "" by ", and
    # to remove the enclosing quotes.
    while(/"/) {
        s/"([^"]*("")*)*"/__QUOTED_STR_($QuoteNum)__/;
        $quote = $&;
        $quote =~ s/^"(.*)"$/$1/;
        $quote =~ s/""/"/g;
        # Ttest for TABs inside quoted strings here
        AddWarnRef(2009, $Lnum) if ($quote =~ /\t/);
        $AllQuotes{$QuoteNum}=$quote;
        AddWarnRef(2014, $Lnum) if ($quote =~ /%.+%/);
        $QuoteNum++;
    }
    return($_);
}
#-------------------------------------------------------------------------------------------------#
#-- ProcStrings($line, @lines) -------------------------------------------------------------------#
sub ProcStrings {
    my($line)     = $_[0];
    shift(@_);
    my(@lines)    = @_;
    my($CurrentString, $String, $StringValue, %FStr, $UString);
    my($DONE)      = FALSE;
    my($MaxLength) = 1024;

    if (! defined ($INFSections{'STRINGS'})) {
        AddErrorRef(1003, $#lines, 'STRINGS'); # "Section [$sec] not defined."
        return(FAILURE);
    }

    $CheckedSections{"STRINGS"} = TRUE;

    print(STDERR "\tInvoking ProcStrings...\n") if ($DEBUG);

    my($SINF,@stringlines);  # Because %STRING% could
    open(SINF, "$inffile");  # be buried in "STRING"
    @stringlines=<SINF>;     # we need a fresh array
    close(SINF);             # for testing.

    while (! $DONE) {

        $line++, next if (! defined($lines[$line]));
        $line++, next if ($lines[$line] =~ /^\s*$/); # Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {
            # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        GetLine($line) if ($DEBUG);

        $String      = uc(GetDirective($line,$lines[$line]));
        $StringValue = uc(GetDirectiveValue($line,$lines[$line]));

        if (defined($FStr{$String})) {
            # Error if this string is already defined
            AddErrorRef(1048, $line); # "String entry redefined."
            $line++;
            next;
        }

        #   right side should be in quotes or a numeric value
        if($StringValue !~ /^__QUOTED_STR_\(([0-9]*)\)__$/) {
            # Swap out QuotedString if need be
            if ($StringValue !~ /^(0x)??[0-9a-f]*$/i) {
                # AddErrorRef(1004, $line);  # "Entry value should be in quotes."
                $FStr{$String} = $StringValue;
            }
        } else {
            $FStr{$String} = $AllQuotes{$1};
            # "String has more than $MaxStrLen characters."
            AddErrorRef(1005, $line, $MaxLength) if (length($FStr{$String}) > $MaxLength);
        }

        my($CCC) = 0;
        foreach (@lines) {
            $CCC++;
            if (! defined($_) ) {
                print("ERROR: LINE $CCC Not Defined!\n");
            }
        }

        $UString=$String;
        $String=quotemeta($String);

        if (defined($String) and defined(@stringlines)) {
            # Look for use of %string% in the INF lines
            unless ((grep(/%$String%/i,@stringlines)) > 0) {
                AddWarnRef(2029, $line, $UString);
            }
        }
        $line++;

    }
    undef(@stringlines);
    print(STDERR "\tExiting ProcStrings...\n") if ($DEBUG);
    return(TRUE);
}


#-------------------------------------------------------------------------------------------------#
# Our smart globbing function.  Accepts an array of file spec's & returns an array of every file  #
#  that list expands to. Directories in the argument list are interpreted as Dirname\*.*          #
#                                                                                                 #
#   parameter: an array of file path specifiers including wildcards                               #
#   returns: the list of files that match those file specifiers with duplicates removed           #
#                                                                                                 #
#-- @array SmartGlob(@FileSpecs) -----------------------------------------------------------------#
sub SmartGlob {
    my (@FileSpecs) = @_;
    my ($filespec, $dir, $filepattern, @DirectoryFiles, @MatchedFiles, $file);
    my (@AllFiles);
    foreach $filespec (@FileSpecs) {

        @MatchedFiles = ();

        if (($filespec =~ /\~/) and (! -d($filespec)) and (-e($filespec))) {
            if ($filespec !~ /(.*)\\(.*)/) {
                $filepattern = $filespec;
                $dir = ".";
            } else {
                $filepattern = $2;
                $dir = (($1 =~ /^$/) ? "." : $1) ;
            }
            push(@MatchedFiles, (($dir =~ /^\.$/) ? ".\\" : $dir."\\") .$filepattern);
            @AllFiles = sort((@AllFiles, @MatchedFiles));
            next;
        }

        if (!defined($filespec) or $filespec =~ /^$/) {
            $filespec = ".";  
        }   #   current directory

        #   is it a directory name ?
        if ( opendir(LOCALDIR,  $filespec) ) {
            $dir = $filespec;
            $filepattern = "*.*";
        } else  {
            # extract the directory path
            # find the last \
            if ($filespec !~ /(.*)\\(.*)/) {
                $filepattern = $filespec;
                $dir = ".";
            } else {
                $filepattern = $2;
                $dir = (($1 =~ /^$/) ? "." : $1) ;
            }

            next if (!opendir(LOCALDIR,  $dir));
        }

        $filepattern = quotemeta($filepattern);
        $filepattern =~ s/\\\*/.*/g;
        $filepattern =~ s/\\\?/./g;

        @DirectoryFiles = readdir LOCALDIR;
        chop($dir) if ($dir =~ /\\$/);

        foreach $file (sort @DirectoryFiles) {
            # remove . and .. entries as well as entries that are directories
            next if (($file =~ /^\.$/) or ($file =~ /^\.\.$/) or (-d "$dir\\$file"));

           #! Uncomment to Validate Files as Being 8.3 Compliant!
           # if ($file !~ /^(.{1,8}\..{0,3})$/) {
           #     print ("Skipping \"$file\": Not a valid 8.3 name!\n") if ($file =~ /\.inf$/);
           #     next;
           # }

            #   add a dot at the end if not one present
            $file .= "." if ($file !~ /\./);

            push(@MatchedFiles, (($dir =~ /^\.$/) ? ".\\" : $dir."\\") .$file)
                if (($file =~ /^$filepattern$/i) && (! -d "$dir\\$file") && ($file =~ /\.inf$/i));
        }
        closedir LOCALDIR;

        # Concatenate the new results with the old
        @AllFiles = sort((@AllFiles, @MatchedFiles));
    }
    my($i);
    # Remove Duplicates!
    for ($i=1;$i<=$#AllFiles;$i++) {

        # Is there a duplicate?
        if ($AllFiles[$i] eq $AllFiles[$i-1]) {
            splice(@AllFiles, ($i), 1); # Remove the duplicate
            $i--; # We just shortened the array by one, so we need to decrement
        }
    }
    # Return the sorted glob =-)
    return(@AllFiles);
}

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#| Subroutines to verify directives                                                              |#
#| All Logic Comes Directly From DDK Setup Documentation                                         |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################
#-------------------------------------------------------------------------------------------------#
#-- ADDINTERFACE($line, @Values) -----------------------------------------------------------------#
sub ADDINTERFACE {
    my($line)    = $_[0], shift;
    my($IntGUID, $RefStr, $AISec, $Flags)  = split(/,/, $_[0]);

    print(STDERR "\tBeginning ADDINTERFACE (@_)...\n") if ($DEBUG);
    if ($IntGUID =~ /^__QUOTED_STR_\(([0-9]*)\)__$/) {
        $IntGUID = $AllQuotes{$1};
    }
    if ($IntGUID !~ /\{.{8}-.{4}-.{4}-.{4}-.{12}\}/) {
        AddErrorRef(1065, $line);
    }

    if (defined($AISec)) {
        $AISec =~ s/^\s*//; # Remove leading spaces
        $AISec =~ s/\s*$//; # Remove trailing spaces
        if (defined($INFSections{$AISec})) {
            VerifyAddInterfaceSection($INFSections{$AISec}, $AISec);
        } else {
            AddErrorRef(1003,$line, $RealSections{$AISec});
        }
    }

    if ((defined($Flags)) && ($Flags ne 0)) {
        AddErrorRef(1062, $line, "Flags", "0") unless ($Flags =~ /^\s*$/);
    }
    print(STDERR "\tExiting ADDINTERFACE...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- ADDREG($line, @Values) -----------------------------------------------------------------------#
sub ADDREG {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning ADDREG (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s*//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s*$//;

        if (defined($INFSections{$RegSection})) {
            VerifyAddRegistrySection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting ADDREG...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- ADDSERVICE($line, @Values) -------------------------------------------------------------------#
sub ADDSERVICE {
    my($line)    = $_[0];
    my($ServiceName, $Flags, $SISec, $ELSec, $ELType, $EName)  = split(/,/, $_[1]);

    my(%ELTypeValues) = ("SYSTEM",      1,
                         "SECRURITY",   1,
                         "APPLICATION", 1);

    my($SWAPDONE) = FALSE;
    my($NO)       = FALSE;
    my($TempVar)  = FALSE;

    print(STDERR "\tBeginning ADDSERVICE (@_)...\n") if ($DEBUG);
    # 251 = OR of all possible values for $Flags;
    # 251 = 0x0000001|0x00000002|0x00000004.... (See DDK Docs for possible values
    if (defined($Flags) && ($Flags !~ /^\s*$/)) {
        # Control to indicate we're done exchanging string values.
        $SWAPDONE = FALSE;

        # Just here to avoid extraneous warnings below.
        $NO = FALSE;

        if ($Flags =~ /^__QUOTED_STR_\(([0-9]*)\)__$/) {
            # Was this line qouted when we propagated our array
            $Flags = $AllQuotes{$1};
        }

        while (! $SWAPDONE) {
            # If we have a string value
            if ($Flags =~ /%(.*?)%/) {
                $TempVar = quotemeta($1);
                if (defined($StringVals{uc($TempVar)})) {
                    # Swap it if defined.
                    $Flags =~ s/%$TempVar%/$StringVals{uc($TempVar)}/;
                } else {
                    # Or flag some errors. Also, set $NO to avoid error in performing
                    #    math ops on strings.
                    AddErrorRef(1025,$line,"%$TempVar%");
                    AddErrorRef(1066,$line,"flags ($Flags)","AddService"); # If we can't swap, then value isn't legit
                    $NO = TRUE; # Since the value wasn't legit, the OR below will generate a non-numeric error
                    $Flags =~ s/%$TempVar%/$TempVar/;
                }
            } else {
                # No more strings to swap.
                $SWAPDONE = TRUE;
            }
        }

        $Flags =~ s/^\s*//; # Kill leading whitespace.
        $Flags =~ s/\s*$//; # Kill trailing whitespace.
        # Convert to hex num if string is hex

        if ($Flags =~ /^0x/i) {
            $Flags =~ s/^0X//i;
            $Flags = hex($Flags);
        } elsif ((($Flags !~ /^[0-9]*$/) && ($Flags =~ /^[0-9A-F]*$/i)) || ($Flags =~ /^0/)) {
            $Flags = hex($Flags);
        }


        if ($Flags !~ /^\d*$/) {
            # Error out if we're not numeric now.
            AddErrorRef(1066,$line,"flags ($Flags)","AddService");
            $NO = TRUE;
        }
        if (! $NO) {
            # Actually perform the OR to verify the value we have.
            AddErrorRef(1066,$line,"flags ($Flags)","AddService") if (($Flags | 0xFB) != 0xFB);
        }

    }

    undef($Flags) if (($Flags =~ /^[\s\t]*$/) or ($Flags =~ /^$/) or ($Flags=~ /[G-Zg-z]/));

    if (((defined($Flags)) && ($Flags != 2)) or (defined($SISec))) {
        unless (defined($ServiceName) && ($ServiceName !~ /^$/)) {
            AddErrorRef(1060, $line, "ServiceName");
        }

        if (defined($SISec) && ($SISec !~ /^$/)) {
            $SISec =~ s/^[\s\t]*//; #strip leading white spaces.
            $SISec =~ s/[\s\t]*$//; #strip ending white spaces and eol.

            if (defined($INFSections{uc($SISec)})) {
                VerifyServiceInstallSection($INFSections{uc($SISec)},uc($SISec));
            } else {
                AddErrorRef(1003,$line, $SISec);
            }
        } else {
            AddErrorRef(1060, $line, "ServiceInstallSection");
        }
        if (defined($ELSec) && ($ELSec !~ /^$/)) {
            $ELSec =~ s/^[\s\t]*//; #strip leading white spaces.
            $ELSec =~ s/[\s\t]*$//; #strip ending white spaces and eol.
            if (defined($INFSections{uc($ELSec)})) {
                VerifyEventLogSection($INFSections{uc($ELSec)}, $ELSec);
            } else {
                AddErrorRef(1003,$line, $ELSec);
            }
        }
        if (defined($ELType)) {
            unless (defined($ELTypeValues{$ELType})) {
                AddErrorRef(1061, $line);
            }
        }
    }
    # To avoid later complications.
    undef($Flags);


    print(STDERR "\tExiting ADDSERVICE...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- BITREG($line, @Values) -----------------------------------------------------------------------#
sub BITREG {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning BITREG (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s$//;

        if (defined($INFSections{$RegSection})) {
            VerifyBitRegSection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting BITREG...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- COPYFILES($line, @Values) --------------------------------------------------------------------#
sub COPYFILES {
    my($line)    = $_[0];
    my($File);

    print(STDERR "\tBeginning COPYFILES (@_)...\n") if ($DEBUG);
    #If the next identifier begins w/ an @, it specifies only one file
    if ($_[1] =~ /^@/) {
        $File = $_[1];
        $File =~ s/^@//;

		if (! $LAYOUTFILE_USED) {
			if (! defined $INFSections{"SOURCEDISKSFILES"}) {
				AddErrorRef(1111, $line);
			    print(STDERR "\tExiting VerifyCopyFilesSection...\n") if ($DEBUG);
				return(FALSE);
			} else {
				my($sdline)=$INFSections{"SOURCEDISKSFILES"};
				my($SDDONE) = FALSE;
				my(@sdfiles);
			    my(@lines)   =  @INFlines; #Local copy =-)

		        while (! $SDDONE) {
		            $sdline++, next if ($lines[$sdline] =~ /^\s$/);# Skip if it's a blank line
		            if ($lines[$sdline] =~ /^\[.*\]/) {          # Quit once we get to the next section
		                $SDDONE = TRUE;
		                next;
					}
	
			        $sdfiles[++$#sdfiles] = uc(GetDirective($sdline,$lines[$sdline]));
					$sdline++;
				}
				AddErrorRef(1110, $line, $File)if (grep(/$File/i,@sdfiles) < 1);
			}
		}
        AddErrorRef(1093, $line) if (uc($File) eq uc($inffile));
        AddErrorRef(1094, $line) if (uc($File) =~ /,/);
    # otherwise, it specifies a copyfiles-section
    } else {
        my(@Values)  = split(/,/, $_[1]);
        my($RegSection);

        foreach $RegSection (@Values) {
            $RegSection =~ s/^\s//; # Remove leading/trailing whitespace
            $RegSection =~ s/\s$//;

            if (defined($INFSections{$RegSection})) {
                VerifyCopyFilesSection($INFSections{$RegSection}, $RegSection);
            } else {
                AddErrorRef(1003,$line, $RegSection);
            }
        }
    }

    print(STDERR "\tExiting COPYFILES...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- DELFILES($line, @Values) ---------------------------------------------------------------------#
sub DELFILES {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning DELFILES (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s$//;
        if (defined($INFSections{$RegSection})) {
            VerifyDelFilesSection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting DELFILES...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- DELREG($line, @Values) -----------------------------------------------------------------------#
sub DELREG {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning DELREG (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s$//;

        if (defined($INFSections{$RegSection})) {
            VerifyDelRegistrySection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting DELREG...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- DELSERVICE($line, @Values) -------------------------------------------------------------------#
sub DELSERVICE {
    my($line)    = $_[0];
    my($ServiceName, $Flags, $EventLogType, $EName) = split(/,/, $_[1]);

    my(%ELTypeValues) = ("SYSTEM",      1,
                         "SECRURITY",   1,
                         "APPLICATION", 1);

    print(STDERR "\tBeginning DELSERVICE (@_)...\n") if ($DEBUG);
    unless (defined($ServiceName)) {
        AddErrorRef(1060, $line, "ServiceName");
    }
    if (defined($EventLogType)) {
        unless (defined($ELTypeValues{$EventLogType})) {
            AddErrorRef(1061, $line);
        }
    }
    if ((defined($Flags)) && (uc($Flags) ne "SPSVCINST_DELETEEVENTLOGENTRY")) {
        AddErrorRef(1062, $line, "flags", "\'SPSVCINST_DELETEEVENTLOGENTRY\' or empty");
    }
    print(STDERR "\tExiting DELSERVICE...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- DRIVERVER($line, @Values) --------------------------------------------------------------------#
sub DRIVERVER {
    my($line)    = $_[0];
    #(Month)/(Day)/(Year,Version)
    my($Line)    = $_[1];
    $Line = $AllQuotes{$1} if($_[1] =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
    my(@Values)  = split(/\//, $Line);
	my($Revision, @Versions); #Holds indiviual elements in version string.
    my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    $mon++;

    print(STDERR "\tBeginning DRIVERVER (@_)...\n") if ($DEBUG);
    $year += 1900 if ($year < 1000); # Make sure year is 4 digits.
    if (defined($Values[2])) {
        # (Year),(Version)
        ($Values[2],$Values[3]) = split(/,/,$Values[2]);
    }

    $Values[0] = 0 unless (defined $Values[0]);
    $Values[1] = 0 unless (defined $Values[1]);
    $Values[2] = 0 unless (defined $Values[2]);
    AddErrorRef(1066, $line, "Month","DriverVer") if (($Values[0] =~ /\D/) || ($Values[0] < 1) || ($Values[0] > 12));
    AddErrorRef(1086, $line, "Day","DriverVer")   if (($Values[1] =~ /\D/) || ($Values[1] < 1) || ($Values[1] > 31));

    AddErrorRef(1069, $line)                      if (($Values[2] =~ /\D/) || ($Values[2] < 1900));

    if ($Values[0] !~ /\D/ && $Values[1] !~ /\D/ && $Values[2] !~ /\D/) {
        # These months have 31 days
        if (($Values[0] == 1) || ($Values[0] == 3) || ($Values[0] == 5) ||
            ($Values[0] == 7) || ($Values[0] == 8) || ($Values[0] ==10) ||
            ($Values[0] ==12)) {
            AddErrorRef(1082, $line, $Values[0], $Values[1], $Values[2]) if ($Values[1] > 31);

        # February follows leap year rules
        } elsif ($Values[0] == 2) {
            if (($Values[2] % 4) == 0) {
                if (($Values[2] % 100) == 0) {
                    if (($Values[2] % 400) == 0) {
                        AddErrorRef(1082, $line, $Values[0], $Values[1], $Values[2]) if ($Values[1] > 29);
                    } else {
                        AddErrorRef(1082, $line, $Values[0], $Values[1], $Values[2]) if ($Values[1] > 28);
                    }
                } else {
                    AddErrorRef(1082, $line, $Values[0], $Values[1], $Values[2]) if ($Values[1] > 29);
                }
            } else {
                AddErrorRef(1082, $line, $Values[0], $Values[1], $Values[2]) if ($Values[1] > 28);
            }

        # Which leaves us with months that only have 30 days
        } else {
            AddErrorRef(1082, $line, $Values[0], $Values[1], $Values[2]) if ($Values[1] > 30);
        }

        # Make sure the date isn't in the future!
        if ($Values[2] > $year) {
            AddErrorRef(1067, $line, "");
        } elsif ($Values[2] == $year) {
             if ($Values[0] > $mon) {
                AddErrorRef(1067, $line, "");
             } elsif ($Values[0] == $mon) {
                if ($Values[1] > $mday) {
                    AddErrorRef(1067, $line, "");
                }
            }
        }
    }

    AddWarnRef(2025, $line, "Version (DriverVer=mm/dd/yyyy,version)") if (! defined($Values[3]));

	if (defined($Values[3])) {

		@Versions=split(/\./,$Values[3]);

		foreach $Revision (@Versions) {

            $Revision =~ s/^[\s\t]*//;
            $Revision =~ s/[\s\t]*$//;

			if ($Revision !~ /^\d*$/) { # If this field contains a non-digit character.
				AddErrorRef(1012, $line);
				next;
			}

			if (($Revision < 0) || ($Revision > 65535)) {
				AddErrorRef(1015, $line);
			}
		}

		if (defined($Versions[4])) { # Max version is w.x.y.z
			AddErrorRef(1016, $line);
		}

		if (defined($Versions[0])) {
			if (($Versions[0] =~ /^\d*$/)&&($Versions[0] == 0)) {    # w must be > 0
				AddErrorRef(1014, $line); 
			}
		} else {
		    AddWarnRef(2025, $line, "Version (DriverVer=mm/dd/yyyy,version)");
		}
	}

    print(STDERR "\tExiting DRIVERVER...\n") if ($DEBUG);
    return(TRUE);
}


#-------------------------------------------------------------------------------------------------#
#-- INCLUDE($line, @Values) ----------------------------------------------------------------------#
sub INCLUDE {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($IncFile);
    my($MaxLineLen) = 128;
    my($IncInfLine);
    my($BASENUM);
    my($prev_line, $CLnum, $poscomma, $posquote1, $posquote2, $bRemoveComment, $comment);
    my($HadStrings, $StringsLine) = (FALSE, 0);

    $INFlines[@INFlines] = " "; # Line one
    print(STDERR "\tBeginning INCLUDE (@_)...\n") if ($DEBUG);
    foreach $IncFile (@Values) {
        $IncFile =~ s/^\s*//; # Strip leading space
        $IncFile =~ s/\s*$//; # Strip trailing space

        if ($Included =~ /\\$IncFile/i) {
            next; # Already Included!
        } else {
            $IncFile = $CurrentInfDir . $IncFile;
            # Mark it as Included!
            print(STDERR "\t\t$IncFile is being read into memory...\n") if ($DEBUG);
        }

        if (!open(INCLUDEFILE, "$IncFile")) {
            print("\tCan't open $IncFile for input\n");
            AddErrorRef(1070, $line, $IncFile);
            return(FALSE);
        }


        # Don't add to the array until we know we can read it =-)
        AddIncFile($IncFile);
        $BASENUM = @INFlines + 0; # Explicitly force numeric by adding 0
        $INFlines[$BASENUM] = " "; #
        $INFFiles{$IncFile} = $BASENUM; # Record the base number of the INF
                                        # This allows of determine relative lines
                                        # inside the INF as well as determining
                                        # what INF a section is in.
        while(<INCLUDEFILE>) {
            $CLnum = $. + $BASENUM;
            $INFlines[$CLnum] = " ";  # For line not to be undef!
            chop if /\n$/;            # removes \n
            if (defined($prev_line)) {
                $IncInfLine = join("", $prev_line, $_);
            } else {
                $IncInfLine = $_;
            }
            if ($IncInfLine =~ /^;/) {
                AddErrorRef(9999, ($CLnum-1)) if (defined($prev_line)); #Reference point, no real error.
                $INFlines[$CLnum] = " ";
                next;
            }

            undef($prev_line); # = "";
            #process for comments.
            $posquote1  = index($IncInfLine, '"');
            $posquote2  = ($posquote1 >= 0) ? index($IncInfLine, '"', $posquote1+1) : -1;
            $poscomma   = index($IncInfLine, ';');
            $bRemoveComment = 0;

            while ($poscomma >= 0) {
                if ($poscomma < $posquote1 || $posquote1 < 0)  {    # ';' before '"' => is comment
                    $bRemoveComment = 1;
                    last;
                }

                if ($posquote2 >= 0 && $posquote1 >= 0) {
                    if ($poscomma < $posquote2)  {  # ';' between '"' => sting first
                        $poscomma = index($IncInfLine, ';', $posquote2 + 1);
                    }
                    # find next string
                    $posquote1  = index($IncInfLine, '"', $posquote2 + 1);
                    $posquote2  = ($posquote1 >= 0) ? index($IncInfLine, '"', $posquote1+1) : -1;
                    $INFlines[$CLnum] = " ";
                    next;
                }
                last;  # ';' after '"' => sting not terminated yet, string first
            }

            if ($bRemoveComment == 1) {
                $comment = substr($IncInfLine, $poscomma+1);
                $IncInfLine = substr($IncInfLine, 0, $poscomma);
            }

            $IncInfLine =~ s/^[\s]*//; #strip leading white spaces.
            $IncInfLine =~ s/[\s]*$//; #strip ending white spaces and eol.
            $IncInfLine = " ", next if ($IncInfLine =~ /^$/);

            if ($IncInfLine =~ /(.)*\\$/) {
                # need to join this line to the next.
                $IncInfLine =~ s/\\$//;
                $prev_line = $IncInfLine;
                $INFlines[$CLnum] = " ";
                next; # process joined line together for quotes.
            }

            #process for quotes.
            $IncInfLine = ProcQuotes($IncInfLine, $CLnum);

            $IncInfLine = " "  if ($IncInfLine =~ /^$/); # $line is empty.
            if (! defined($IncInfLine)) {
                $IncInfLine = " ";
                $INFlines[$CLnum] = " ";
            }

            #put the preprocessed line in assoc array indexed by the linenum.
            $INFlines[$CLnum] = $IncInfLine;
            $INFlines[$CLnum] = " " if ($IncInfLine =~ /^\;/);
            if ($IncInfLine =~ /\[(.*)\]/) {
                $IncInfLine =~ /\[(.*)?\]/;
                if (!defined($INFSections{uc($1)})) {
                    $INFSections{uc($1)}     = $CLnum + 1;
                    $CheckedSections{uc($1)} = TRUE;
                } else {
                    if (uc($1) eq "STRINGS") {
                        $HadStrings = TRUE;
                        $StringsLine= $CLnum + 1;
                    }
                    1;
                    # We could add an error here, but why?
                }
            }
        }

        $INFlines[$CLnum] = " " if (!defined($INFlines[$CLnum]));
        $INFlines[++$CLnum] = " ";
        # Add and EOF marker.. also serves to allow proc on last section to
        # correctly terminate.
        $INFlines[$CLnum++] = "[EOF]";
        close (INCLUDEFILE);
    }

    if ($HadStrings) {
        ProcIncludedStrings($StringsLine);
    }
    print(STDERR "\tExiting INCLUDE...\n") if ($DEBUG);

    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- INI2REG($line, @Values) ----------------------------------------------------------------------#
sub INI2REG {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning INI2REG (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s$//;

        if (defined($INFSections{$RegSection})) {
            VerifyIni2RegSection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting INI2REG...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- LOGCONFIG($line, @Values) --------------------------------------------------------------------#
sub LOGCONFIG {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning LOGCONFIG (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s*//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s*$//;

        if (defined($INFSections{$RegSection})) {
            VerifyLogConfigSection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting LOGCONFIG...\n") if ($DEBUG);
    return(TRUE);
}

my(@WasNeeded);
#-------------------------------------------------------------------------------------------------#
#-- NEEDS($line, @Values) ------------------------------------------------------------------------#
sub NEEDS {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($type)    = $_[2];
    my($RegSection);

    print(STDERR "\tBeginning NEEDS (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
		$RegSection = uc($RegSection);
        $RegSection =~ s/^\s//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s$//;

        if (defined($INFSections{$RegSection})) {
           if (defined(@WasNeeded)) {
				#AddWarnRef(2010, $line, $RegSection);
				next if (grep(/$RegSection/i,@WasNeeded) > 0);
			}

			$WasNeeded[++$#WasNeeded] = $RegSection;
            NEEDSCHK($INFSections{$RegSection}, $RegSection);
            $IncludedSections{$RegSection} = "included" unless ($INFSections{$RegSection} <= $LastLine);
        } else {
            AddErrorRef(1003, $line, $RegSection);
        }
    }
    print(STDERR "\tExiting NEEDS...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
# The problem with the NEEDS section is that we really have no idea what kind of section is being #
#     pointed to.  It could be registry keys (though bare keys would be ugly in practive) or      #
#     a standard section.  We have to, thus, make this section as robust as possible.             #
#-- NEEDSCHK($line, $SectionName) ----------------------------------------------------------------#
sub NEEDSCHK {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value, %PreChecked);
    my(%Directives)       = ("ADDINTERFACE",     1,
                             "ADDREG",           1,
                             "DELSERVICE",       1,
                             "DRIVERVER",        1,
                             "ADDSERVICE",       1,
                             "BITREG",           1,
                             "COPYFILES",        1,
                             "DELFILES",         1,
                             "DELREG",           1,
                             "INCLUDE",          1,
                             "INI2REG",          1,
                             "LOGCONFIG",        1,
                             "NEEDS",            1,
                             "RENFILES",         1,
                             "UPDATEINIS",       1,
                             "UPDATEINIFIELDS",  1);


    #$CheckedSections{$Section} = FALSE;  # Set to false to cheat here
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning NEEDSCHK (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^$/); # Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {         # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            if ($lines[$line] =~ /=/) {  # Then we have a directive/value pair.
                $Directive = uc(GetDirective($line,$lines[$line]));
                $Value     = uc(GetDirectiveValue($line,$lines[$line]));
            } else { # We have a bare key.
                # Do nothing for now. May need to re-evaluate later.
                $Directive=  'x';                # Define directive to avoid errors.
                $Value    =   uc($lines[$line]); # Set Value to the line
                AddErrorRef(1057, $line, $Value) unless (($Value =~ /^\s*$/) || $Module->DCDirective($Value));
            }
            if (defined($Directives{uc($Directive)})) {
                # Have to wrap this outside of strict since it's not normally allowed.
                %PreChecked = %CheckedSections; # Record Sections Checked Before We Did This
                no strict;
                &$Directive($line, $Value);# || die("Couldn't process $Directive in NEEDSCHK\n");;
                use strict;
                # Now, what sections just became checked?
            }
            GetLine($line) if ($DEBUG);
            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);
    print(STDERR "\tExiting NEEDSCHK...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- PROFILEITEMS($line, @Values) -----------------------------------------------------------------#
sub PROFILEITEMS {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning PROFILEITMES (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s*//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s*$//;

        if (defined($INFSections{$RegSection})) {
            VerifyProfileItemsSection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting PROFILEITEMS...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- RENFILES($line, @Values) ---------------------------------------------------------------------#
sub RENFILES {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning RENFILES (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s$//;

        if (defined($INFSections{$RegSection})) {
            VerifyRenFilesSection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting RENFILES...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- UPDATEINIFIELDS($line, @Values) --------------------------------------------------------------#
sub UPDATEINIFIELDS {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning UPDATEINIFIELDS (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s$//;

        if (defined($INFSections{$RegSection})) {
            VerifyUpdateIniFieldsSection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting UPDATEINIFILES...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- UPDATEINIS($line, @Values) -------------------------------------------------------------------#
sub UPDATEINIS {
    my($line)    = $_[0];
    my(@Values)  = split(/,/, $_[1]);
    my($RegSection);

    print(STDERR "\tBeginning UPDATEINIS (@_)...\n") if ($DEBUG);
    foreach $RegSection (@Values) {
        $RegSection =~ s/^\s//; # Remove leading/trailing whitespace
        $RegSection =~ s/\s$//;

        if (defined($INFSections{$RegSection})) {
            VerifyUpdateInisSection($INFSections{$RegSection}, $RegSection);
        } else {
            AddErrorRef(1003,$line, $RegSection);
        }
    }
    print(STDERR "\tExiting UPDATEINIS...\n") if ($DEBUG);
    return(TRUE);
}

###################################################################################################
#+-----------------------------------------------------------------------------------------------+#
#| Subroutines to verify common sections                                                         |#
#+-----------------------------------------------------------------------------------------------+#
###################################################################################################
#-------------------------------------------------------------------------------------------------#
#-- VerifyAddInterfaceSection($line, $Section) ---------------------------------------------------#
sub VerifyAddInterfaceSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);

    #Valid AddInterface Directives
    my(%AddInterfaceDirective) = ("ADDREG",             1,
                                  "DELREG",             1,
                                  "BITREG",             1,
                                  "COPYFILES",          1,
                                  "DELFILES",           1,
                                  "RENFILES",           1,
                                  "UPDATEINIS",         1,
                                  "INI2REG",            1,
                                  "UPDATEINIFIELDS",    1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section})  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyAddInterfaceSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/); # Skip if it's a blank line
            if (($lines[$line] =~ /^\[.*\]/)) { # or ($line > $INFSections{'EOF'})) { # Quit once we hit next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($AddInterfaceDirective{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));;
                $line++;
                next;
            }

            # Have to wrap this outside of strict since it's not normally allowed.
            no strict;
            &$Directive($line, $Value);
            use strict;

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyAddInterfaceSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyAddRegistrySection($line, $Section) ----------------------------------------------------#
sub VerifyAddRegistrySection {
    my($line)    =  $_[0];     # Line num array to start with
    my($Section) =  $_[1];     # Section name (Just for reference)
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;     # Control Flag
    my($RegRoot, $SubKey, $Name, $Flags, $Value, $TempVar, $SWAPDONE, $NO, $SWAP);

    # Valid RegRoot Values
    my(%RegRootValues) = ("HKCR",   1,
                          "HKCU",   1,
                          "HKLM",   1,
                          "HKU",    1,
                          "HKR",    1);


    $CheckedSections{$Section} = FALSE  # Set to false if in included inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        # Don't check a section twice.
        return(FALSE);
    } else {
        # Being verbose
        print(STDERR "\tBeginning VerifyAddRegistrySection (@_)...\n") if ($DEBUG);

        # Start checking this section.
        while (! $DONE) {
            $lines[$line] =~ s/^\s*//; # Kill leading whitespace
            $lines[$line] =~ s/\s*$//; # Kill trailing whitespace

            # Undef'd lines are just blank, ignore and move on.
            if (! defined($lines[$line])) {
                $line++;
                next;
            }

            #Skip blank lines even though this should never be true.
            if ($lines[$line] =~ /^\s*$/) {
                $line++;
                next;
            }

            # We've hit the next section so fall out of the loop.
            $DONE=TRUE, next if ($lines[$line] =~ /^\[.*\]$/);

            # Propogate the values in the line.
            ($RegRoot, $SubKey, $Name, $Flags, $Value) = split(/,/,$lines[$line]);
            $SubKey = " " unless(defined($SubKey));
            $SubKey =~ s/^[\s\t]*//;
            $SubKey =~ s/[\s\t]$//;

            $SWAP = TRUE;   # Swap out all string tokens
            while ($SWAP) {
                if ($SubKey =~ /^\%(.*?)\%/) {
                    $SubKey = $StringVals{uc(quotemeta($1))}
                        unless (! defined($StringVals{uc(quotemeta($1))}));
                } else {
                    $SWAP = FALSE;
                }
            }

            $SWAP = TRUE;   # Swap out all quoteed strings
            while ($SWAP) {
                if ($SubKey =~ /__QUOTED_STR_\(([0-9]*)\)__/) {
                    $SubKey =~ s/__QUOTED_STR_\(([0-9]*)\)__/\"$AllQuotes{$1}\"/;
                } else {
                    $SWAP = FALSE;
                }
            }

            # Make sure $SubKey is defined
            $SubKey = " " unless(defined($SubKey));

            $Value = " " unless (defined($Value));

            $SWAP = TRUE;   # Swap out all quoteed strings
            while ($SWAP) {
                if ($Value =~ /__QUOTED_STR_\(([0-9]*)\)__/) {
                    $Value =~ s/__QUOTED_STR_\(([0-9]*)\)__/\"$AllQuotes{$1}\"/;
                } else {
                    $SWAP = FALSE;
                }
            }

            if ($SubKey =~ /CurrentVersion\\RunOnce/) {
                AddWarnRef(2024,$line) unless ($Value =~ /Run32DLL/);
            }

            if (uc($Name) eq "NOINSTALLCLASS") {
            #   $NoInstall = $Value;
            }

            # More verboseness for debug execution.
            GetLine($line) if ($DEBUG);

            unless (defined($RegRootValues{uc($RegRoot)}))  {
                # Invalid RegRoot, error.
                AddErrorRef(1063, $line, $RegRoot);
            }

            # 196671 = OR of all possible values for $Flags;
            # 196671 = 0x0000001|0x00000002|0x00000004.... (See DDK Docs for possible values)
            if (defined($Flags) && ($Flags !~ /^\s*$/)) {
                # Control to indicate we're done exchanging string values.
                $SWAPDONE = FALSE;

                # Just here to avoid extraneous warnings below.
                $NO = FALSE;

                if ($Flags =~ /^__QUOTED_STR_\(([0-9]*)\)__$/) {
                    # Was this line qouted when we propagated our array
                    $Flags = $AllQuotes{$1};
                }

                while (! $SWAPDONE) {
                    # If we have a string value
                    if ($Flags =~ /%(.*?)%/) {
                        $TempVar = quotemeta($1);
                        if (defined($StringVals{uc($TempVar)})) {
                            # Swap it if defined.
                            $Flags =~ s/%$TempVar%/$StringVals{uc($TempVar)}/;
                        } else {
                            # Or flag some errors. Also, set $NO to avoid error in performing
                            #    math ops on strings.
                            AddErrorRef(1025,$line,"%$TempVar%");
                            AddErrorRef(1066,$line,"flags ($Flags)",$Section); # If we can't swap, then value isn't legit
                            $NO = TRUE; # Since the value wasn't legit, the OR below will generate a non-numeric error
                            $Flags =~ s/%$TempVar%/$TempVar/;
                        }
                    } elsif  ($Flags =~ /__QUOTED_STR_\(([0-9]*)\)__/) {
                        # Swap out QuotedString if need be
                        $Flags =~ s/__QUOTED_STR_\(([0-9]*)\)__/\"$AllQuotes{$1}\"/;
                    } else {
                        # No more strings to swap.
                        $SWAPDONE = TRUE;
                    }
                }

                $Flags =~ s/^\s*//; # Kill leading whitespace.
                $Flags =~ s/\s*$//; # Kill trailing whitespace.
                # Convert to hex num if string is hex
                $Flags = hex($Flags) if ($Flags =~ /^0x[0-9A-F]*$/i);

                if ($Flags !~ /^\d*$/) {
                    # Error out if we're not numeric now.
                    AddErrorRef(1066,$line,"flags ($Flags)",$Section);
                    $NO = TRUE;
                }
                if (! $NO) {
                    # Actually perform the OR to verify the value we have.
                    AddErrorRef(1066,$line,"flags ($Flags)",$Section) if (($Flags | 0x0003003f) != 0x0003003f);
                    if ($Name =~ /^__QUOTED_STR_\(([0-9]*)\)__$/) {
                        # Was this line qouted when we propagated our array
                        $Name = $AllQuotes{$1};
                    }

                    if ($Name =~ /[upper|lower]filters/i) {
                        AddErrorRef(1112,$line) unless (($Flags & 0x00010000) == 0x00010000);
                    }
                }
            }

            # To avoid later complications.
            undef($Flags);

            # Move on to next line.
            $line++;
        }

        # Mark this section as checked.
        $CheckedSections{$Section} = TRUE;
    }
    # Still being verbose.
    print(STDERR "\tExiting VerifyAddRegistrySection...\n") if ($DEBUG);

    # Exit out of this sub.
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyBitRegSection($line, $Section) ---------------------------------------------------------#
sub VerifyBitRegSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($RegRoot, $SubKey, $Name, $Flags, $Mask, $Modify);

    #Valid RegRoot Values
    my(%RegRootValues) = ("HKCR",   1,
                          "HKCU",   1,
                          "HKLM",   1,
                          "HKU",    1,
                          "HKR",    1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyBitRegSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            ($RegRoot, $SubKey, $Name, $Flags, $Mask, $Modify) = split(/,/,$lines[$line]);

            unless (defined($RegRootValues{uc($RegRoot)}))  {
                AddErrorRef(1063, $line, $RegRoot."BitReg");
                $line++;
                next;
            }

            if (defined($Flags) && ($Flags != 0) && ($Flags != 1)) {
                AddErrorRef(1068, $line, "flag. (Expected null, 0, or 1)");
            }

            unless (defined($RegRoot)) {
                AddErrorRef(1060, $line, "reg-root");
            }

            unless (defined($Name)) {
                AddErrorRef(1060, $line, "value-entry-name");
            }

            unless (defined($Mask)) {
                AddErrorRef(1060, $line, "byte-mask");
            }

            unless (defined($Modify)) {
                AddErrorRef(1060, $line, "byte-to-modify");
            }

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }

    print(STDERR "\tExiting VerifyBitRegSection...\n") if ($DEBUG);
    return(TRUE);
}

#-----------------------------------------------------------------------------------------------------#
#  This sub processes the ControlFlags section and adds the                                           #
#  IDs defined here to the respective lists of Gen/NT by using                                        #
#  ExSel.                                                                                             #
#                                                                                                     #
#  Globals changed:                                                                                   #
#                                                                                                     #
#-- VerifyCFlagsSection($line, $Section) -------------------------------------------------------------#
sub VerifyCFlagsSection {

    my($line, $Section)= @_;
    my(@lines)         = @INFlines;
    my($Directive, $Value, @Values, $Saved, $Check);
    my($DONE);
    my(%CFlagKeys)=('EXCLUDEFROMSELECT',        'ExSel',
                    'EXCLUDEFROMSELECT.NT',     'ExSel',
                    'EXCLUDEFROMSELECT.NTX86',  'ExSel',
                    'EXCLUDEFROMSELECT.NTALPHA','ExSel',
                    'EXCLUDEFROMSELECT.NTMIPS', 'ExSel',
                    'EXCLUDEFROMSELECT.NTPPC',  'ExSel',
                    'INTERACTIVEINSTALL',       ' ',
                    'COPYFILESONLY',            ' ');

    print(STDERR "\tBeginning VerifyCFlagsSection (@_)...\n") if ($DEBUG);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/); # Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {         # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }
        GetLine($line) if ($DEBUG);
        $Directive = uc(GetDirective($line,$lines[$line]));
        $Value     = uc(GetDirectiveValue($line,$lines[$line]));

        # $Directive should be one of the CFlagKeys.
        if (!defined ($CFlagKeys{$Directive})) {
            AddErrorRef(1057, $line, $Directive) unless (($Directive =~ /^\s*$/) || $Module->DCDirective($Directive));
            $line++;
            next;
        }

        $Check = FALSE;
        if ($Directive =~ /^EXCLUDEFROMSELECT/i) {
            @Values = split(/,/, $Value);

            if ((uc($Directive) eq "EXCLUDEFROMSELECT.NTPPC")  or
                (uc($Directive) eq "EXCLUDEFROMSELECT.NTMIPS") or
                (uc($Directive) eq "EXCLUDEFROMSELECT.NTALPHA")) {
                AddErrorRef(1084, $line);
                $Check = FALSE;
            } else {
                $Check = TRUE;
            }

            foreach $Value (@Values) {
                $Value = $AllQuotes{$1} if ($Value =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
                $Value =~ s/^\s*//; # Kill Leading  Spaces
                $Value =~ s/\s*$//; # Kill Trailing Spaces
                $Saved = $Value;
                $AllExcludes{uc($Saved)} = 1 if ($Check);
                if ($Saved =~ /\*/) {
                    $Value =~ s/\*/\.\*/;
                    $AllExcludes{uc($Saved)} = 1 if ($Check);
                    AddErrorRef(1072, $line, $Saved) unless ($Models =~ /$Value/i);
                } else {
                    $Value=quotemeta($Value);
                    AddErrorRef(1072, $line, $Saved) unless ($Models =~ /$Value/i);
                }
           }

        }
        if ($Directive =~ /^INTERACTIVEINSTALL$/i) {
            AddWarnRef(2022, $line);
            @Values = split(/,/, $Value);
            foreach $Value (@Values) {
                $Value = $AllQuotes{$1} if ($Value =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
                $Value =~ s/^\s*//; # Kill Leading  Spaces
                $Value =~ s/\s*$//; # Kill Trailing Spaces
                $Saved = $Value;
                $Value=quotemeta($Value);
                AddErrorRef(1072, $line, $Saved) unless (($Saved eq "*") || ($Models =~ /$Value/i));
           }
        }
    $line++;
    }

    $CheckedSections{$Section} = TRUE;

    print(STDERR "\tExiting VerifyCFlagsSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyClassInstall32Section($line, $Section) -------------------------------------------------#
sub VerifyClassInstall32Section {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);
    my($AddReg) = 0;
    my(%ClassInstall32Directive) = ("ADDREG",           1,
                                    "BITREG",           1,
                                    "COPYFILES",        1,
                                    "DELFILES",         1,
                                    "DELREG",           1,
                                    "INI2REG",          1,
                                    "RENFILES",         1,
                                    "UPDATEINIS",       1,
                                    "UPDATEINIFIELDS",  1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE) {
        return(FALSE);
    } else {

        print(STDERR "\tBeginning VerifyClassInstall32Section (@_)...\n") if ($DEBUG);
        if ($StandardClass) {
            AddWarnRef(2077, ($line-1));
        }
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($ClassInstall32Directive{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

            # Have to wrap this outside of strict since it's not normally allowed.
            no strict;
            &$Directive($line, $Value);
            use strict;

            $AddReg++ if ($Directive eq "ADDREG");

            $line++;
        }

        AddErrorRef(1058,$line,"AddReg",$RealSections{$Section}) unless ($AddReg);

        $CheckedSections{$Section} = TRUE;
    }

    print(STDERR "\tExiting VerifyClassInstall32Section...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyCopyFilesSection($line, $Section) ------------------------------------------------------#
sub VerifyCopyFilesSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Destination, $Source, $Temp, $Flags);
    my($NO,$SWAPDONE,$TempVar);
	my(@sdfiles);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));

    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyCopyFilesSection (@_)...\n") if ($DEBUG);

		if (! $LAYOUTFILE_USED) {
			# If SourceDIsksFiles is not defined, error & exit routine
			if (! defined $INFSections{"SOURCEDISKSFILES"}) {
				AddErrorRef(1111, $line);
			    print(STDERR "\tExiting VerifyCopyFilesSection...\n") if ($DEBUG);
				return(FALSE);
			} else {
				# Create a list of files in the SourceDisksFiles section
				my($sdline)=$INFSections{"SOURCEDISKSFILES"};
				my($SDDONE) = FALSE;
		        while (! $SDDONE) {
		            $sdline++, next if ($lines[$sdline] =~ /^\s$/);# Skip if it's a blank line
		            if ($lines[$sdline] =~ /^\[.*\]/) {          # Quit once we get to the next section
		                $SDDONE = TRUE;
		                next;
					}
			        $sdfiles[++$#sdfiles] = uc(GetDirective($sdline,$lines[$sdline]));
					$sdline++;
				}
			}
		}
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            ($Destination, $Source, $Temp, $Flags) = split(/,/,$lines[$line]);

            unless (defined($Destination)) {
                AddErrorRef(1060, $line, "destination-file-name");
                $line++;
                next;
            }

            $Destination =~ s/^[\s\t]*//;
            $Destination =~ s/[\s\t]*$//;
            if ($Destination =~ /.inf$/i) {
                AddErrorRef(1029, $line);
            }

			if (! $LAYOUTFILE_USED) {
				AddErrorRef(1110, $line, $Destination) if (grep(/$Destination/i,@sdfiles) < 1);
			}

            if (($Destination =~ /.sys$/i) and (defined $DestDirs{uc($Section)})){
                AddWarnRef(2033, $line) unless (($DestDirs{uc($Section)} =~ /^12/) or
                                                (($DestDirs{uc($Section)} =~ /^10/) and
                                                 ($DestDirs{uc($Section)} =~ /system32\\drivers/i)));
            } elsif ($Destination =~ /.sys$/i) {
                if (defined $DestDirs{'DEFAULT'}) {
                    my($Temp) = $DestDirs{'DEFAULT'};
                    $Temp =~ s/^12/$StringVals{"12"}/;
                    AddWarnRef(2034, $line, $Temp) unless ($DestDirs{'DEFAULT'} =~ /^12/);
                } else {
                    AddErrorRef(1092, $line);
                }
            }

            AddWarnRef (2021, $line) if (uc($Source) eq uc($inffile));
            # 3199 = OR of all possible values for $Flags;
            # 3199 = 0x0000001|0x00000002|0x00000004.... (See DDK Docs for possible values
            if (defined($Flags) && ($Flags !~ /^\s*$/)) {
                $SWAPDONE = FALSE;
                $NO = FALSE;
                while (! $SWAPDONE) {
                    if ($Flags =~ /%(.*?)%/) {
                        $TempVar = quotemeta($1);
                        if (defined($StringVals{uc($TempVar)})) {
                            $Flags =~ s/%$TempVar%/$StringVals{uc($TempVar)}/;
                        } else {
                            AddErrorRef(1025,$line,"%$TempVar%");
                            AddErrorRef(1066,$line,"flags ($Flags)",$Section); # If we can't swap, then value isn't legit
                            $NO = TRUE; # Since the value wasn't legit, the OR below will generate a non-numeric error
                            $Flags =~ s/%$TempVar%/$TempVar/;
                        }
                    } else {
                        $SWAPDONE = TRUE;
                    }
                }

                if ($Flags =~ /^__QUOTED_STR_\(([0-9]*)\)__$/) {
                    $Flags = $AllQuotes{$1};
                }
                $Flags =~ s/^\s*//;
                $Flags =~ s/\s*$//;
                # Convert to hex num if string is hex
                $Flags = hex(lc($Flags)) if (($Flags =~ /^0x[0-9A-F]*$/i) || ($Flags =~ /^[0-9A-F]*$/i));
                if ($Flags !~ /^\d*$/) {
                    AddErrorRef(1066,$line,"flags ($Flags)",$Section);
                    $NO = TRUE;
                }
                if (! $NO) {
                    AddErrorRef(1066,$line,"flags ($Flags)",$Section) if (($Flags | 0xC7F) != 0xC7F);
                }
            }
            undef($Flags);
            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);
    print(STDERR "\tExiting VerifyCopyFilesSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyDelFilesSection($line, $Section) -------------------------------------------------------#
sub VerifyDelFilesSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Destination, $Source, $Temp, $Flag);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);
    if ($CheckedSections{$Section})  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyDelFilesSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            ($Destination, $Source, $Temp, $Flag) = split(/,/,$lines[$line]);


            $Flag=hex($Flag) if defined($Flag);

            if (defined($Flag) && (($Flag != 0x00000001) && ($Flag != 0x00010000))) {
                AddErrorRef(1068, $line, "flag (Expected null, 0x00000001, or 0x00010000).");
            }
            unless (defined($Destination)) {
                AddErrorRef(1060, $line, "destination-file-name");
            }

            $Destination =~ s/^[\s\t]*//;
            $Destination =~ s/[\s\t]*$//;

            AddErrorRef(1109,$line) if ((defined($Destination)) and ($Destination =~ /\.inf$/));

            if (defined($Source) ) {
                AddErrorRef(1062, $line, "source-file-name", "empty") unless($Source =~ /^$/);
            }

            if (defined($Temp)) {
                AddErrorRef(1062, $line, "temporary-file-name", "empty")  unless($Source =~ /^$/);
            }
            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyDelFilesSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyDelRegistrySection($line, $Section) ----------------------------------------------------#
sub VerifyDelRegistrySection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($RegRoot, $SubKey, $Name);

    #Valid RegRoot Values
    my(%RegRootValues) = ("HKCR",   1,
                          "HKCU",   1,
                          "HKLM",   1,
                          "HKU",    1,
                          "HKR",    1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);
    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyDelRegistrySection(@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            ($RegRoot, $SubKey, $Name) = split(/,/,$lines[$line]);

            unless (defined($RegRootValues{uc($RegRoot)}))  {
                AddErrorRef(1063, $line, $RegRoot."DelReg");
                $line++;
                next;
            }

            unless (defined($SubKey)) {
                AddErrorRef(1025, $line, "subkey");
            }

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyDelRegistrySection...\n") if ($DEBUG);
    return(TRUE);
}

#-----------------------------------------------------------------------------------------------------#
#   This sub processes the DestinationDirs section and retains                                        #
#   the list of file sections defined                                                                 #
#                                                                                                     #
#   Globals changed: %DestinationDirs                                                                 #
#-- VerifyDestDirsSection($line, $Section) -----------------------------------------------------------#
sub VerifyDestDirsSection {

    my($line, $Section) = @_;
    my(@lines)          = @INFlines;
    my($Directive, $Value);
    my($DONE)           = FALSE;
    my(%DestinationDirs);
    my($DDDirs)         = FALSE;
    my(@FileSectValue);

    print(STDERR "\tBeginning VerifyDestDirsSection (@_)...\n") if ($DEBUG);
    while (! $DONE) {

        $line++, next if ($lines[$line] =~ /^\s$/); # Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {           # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        GetLine($line) if ($DEBUG);
        $Directive = uc(GetDirective($line,$lines[$line]));
        $Value     = uc(GetDirectiveValue($line,$lines[$line]));

        if (defined($DestinationDirs{uc($Directive)}))  {
            AddErrorRef(1042, $line);
            $line++;
            next;
        }
        $Directive =~ s/^\s//; # Remove leading/trailing whitespace
        $Directive =~ s/\s$//;

        $DestinationDirs{uc($Directive)} = $Value;

        if ($Directive !~ /^DefaultDestDir$/i) {
            if (defined($INFSections{$Directive})) {
                $DestDirs{$Directive}=$Value;
                unless ($CheckedSections{$Directive}) {
                    AddErrorRef(1047, $line);
                }
            } else {
                AddErrorRef(1003,$line, $Directive);
            }
        } else {
            $DDDirs = TRUE;
            $DestDirs{'DEFAULT'}=$Value;
        }

        @FileSectValue = split(",", $Value);
        if (!defined($FileSectValue[0])) {
            AddErrorRef(1043, $line);
        } elsif (!defined($DirIDValues{$FileSectValue[0]})) {
            if ($FileSectValue[0] =~ /^\d*$/) {
                AddErrorRef(1044, $line) unless (($FileSectValue[0] >= 66000) and
                                                 ($FileSectValue[0] <= 66004) and
                                                 ($CLASS eq "PRINTER"));
            } else {
                AddErrorRef(1071, $line, "\"$FileSectValue[0]\"", "DirID");
            }
        }
        $line++;
    }
    AddWarnRef(2020, ($INFSections{$Section} -1), "DefaultDestDir", $RealSections{"DESTINATIONDIRS"}) unless ($DDDirs);
    $CheckedSections{$Section} = TRUE;
    print(STDERR "\tExiting VerifyDestDirsSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyEventLogSection($line, $Section) -------------------------------------------------------#
sub VerifyEventLogSection {
    my($line)    =  $_[0];
    my($Section) =  uc($_[1]);
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);

    #Valid AddInterface Directives
    my(%EventLogDirectives) =    ("ADDREG", 1,
                                  "DELREG", 1,
                                  "BITREG", 1);


    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);
    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyEventLogSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($EventLogDirectives{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

            # Have to wrap this outside of strict since it's not normally allowed.
            no strict;
            &$Directive($line, $Value);
            use strict;

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyEventLogSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyIni2RegSection($line, $Section) --------------------------------------------------------#
sub VerifyIni2RegSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($IniFile, $IniSection, $IniKey, $RegRoot, $SubKey, $Flags);

    my(%RegRootValues) = ("HKCR",   1,
                          "HKCU",   1,
                          "HKLM",   1,
                          "HKU",    1,
                          "HKR",    1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyIni2RegSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            ($IniFile, $IniSection, $IniKey, $RegRoot, $SubKey, $Flags) = split(/,/,$lines[$line]);

            unless (defined($RegRootValues{uc($RegRoot)}))  {
                AddErrorRef(1063, $line, $RegRoot);
                $line++;
                next;
            }

            if (defined($Flags) && (($Flags < 0) && ($Flags > 3))) {
                AddErrorRef(1068, $line, "flag (Expected null, 0, 1, 2, or 3).");
            }

            unless (defined($IniFile)) {
                AddErrorRef(1060, $line, "ini-file");
            }

            unless (defined($IniSection)) {
                AddErrorRef(1060, $line, "ini-section");
            }

            unless (defined($RegRoot)) {
                AddErrorRef(1060, $line, "reg-root");
            }

            unless (defined($SubKey)) {
                AddErrorRef(1060, $line, "subkey");
            }

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyIni2RegSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyInstallCoInstallersSection($line, $Section) --------------------------------------------#
sub VerifyInstallCoInstallersSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);

    my(%InstallCoInstallersDirective) =("ADDREG",          0,
                                        "BITREG",          0,
                                        "COPYFILES",       0,
                                        "DELFILES",        0,
                                        "DELREG",          0,
                                        "INI2REG",         0,
                                        "RENFILES",        0,
                                        "UPDATEINIS",      0,
                                        "UPDATEINIFIELDS", 0,
                                        "INCLUDE",         0,
                                        "NEEDS",           0);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyInstallCoInstallersSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($InstallCoInstallersDirective{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

            # Have to wrap this outside of strict since it's not normally allowed.
            $InstallCoInstallersDirective{uc($Directive)} = TRUE;
            no strict;
            &$Directive($line, $Value);
            use strict;

            $line++;
        }
        AddErrorRef(1058,($_[0]-1),"AddReg",$RealSections{$Section})    unless ($InstallCoInstallersDirective{"ADDREG"});
        AddErrorRef(1058,($_[0]-1),"CopyFiles",$RealSections{$Section}) unless ($InstallCoInstallersDirective{"COPYFILES"});
        $CheckedSections{$Section} = TRUE;
    }

    print(STDERR "\tExiting VerifyInstallCoInstallersSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyInstallFactDefSection($line, $Section) -------------------------------------------------#
sub VerifyInstallFactDefSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value, $Attribs, $Num);
    my($ConfigPriority) = 0;
    my(%FactDefDirectives) = ("CONFIGPRIORITY",0,
                              "DMACONFIG",     0,
                              "IOCONFIG",      0,
                              "MEMCONFIG",     0,
                              "IRQCONFIG",     0
                             );

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyInstallFactDefSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            if ($Directive eq "CONFIGPRIORITY") {
                if ($FactDefDirectives{$Directive} eq 0) {
                    $FactDefDirectives{$Directive} = $Value;
                    if ($Value ne "HARDRECONFIG") {
                        AddErrorRef(1062,$line,"ConfigPriority","HardReconfig");
                    }
                } else {
                    AddErrorRef(1006,$line,"ConfigPriority");
                }
            }

            if ($Directive eq "DMACONFIG") {
                if ($Value =~ /:/) {
                    ($Attribs,$Num)=split(/:/,$Value);
                    if ($Attribs !~ /^[D|W|M|N]/) {
                        AddErrorRef(1068,$line,"DMAattrs");
                    }
                } else {
                    $Num = $Value;
                }
            }

            if ($Directive eq "IOCONFIG") {
                unless(uc($Value) =~ /^[0-9A-F]{3}-[0-9A-F]{3}/) {
                        AddErrorRef(1066, $line, "io-range", "IOConfig");
                }
            }

            if ($Directive eq "IRQCONFIG") {
                if ($Value =~ /:/) {
                    ($Attribs,$Num)=split(/:/,$Value);
                    if ($Attribs !~ /^[L|LS]$/) {
                        AddErrorRef(1068,$line,"IRQConfig");
                    }
                } else {
                    $Num = $Value;
                }
            }
            $ConfigPriority++ if ($Directive eq "CONFIGPRIORITY");
            $line++;
        }
        AddErrorRef(1058, $INFSections{$Section}, "ConfigPriority", $RealSections{$Section}) if (! $ConfigPriority);

        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyInstallFactDefSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyInstallHWSection($line, $Section) ------------------------------------------------------#
sub VerifyInstallHWSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);

    my(%InstallHWDirective) = ("ADDREG",  1,
                               "BITREG",  1,
                               "DELREG",  1,
                               "INCLUDE", 1,
                               "NEEDS",   1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyInstallHWSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            if (! defined($line)) {
                $line=$INFSections{$Section};
            }
            $line++, next if (!defined($lines[$line]));
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($InstallHWDirective{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

            # Have to wrap this outside of strict since it's not normally allowed.
            no strict;
            &$Directive($line, $Value);
            use strict;

            $line++;
        }

        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyInstallHWSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyInstallInterfacesSection($line, $Section) ----------------------------------------------#
sub VerifyInstallInterfacesSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);
    my($AddInterface) = 0;
    my(%InstallInterfacesDirective)=("ADDINTERFACE",1,
                                     "INCLUDE",     1,
                                     "NEEDS",       1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyInstallInterfaceSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($InstallInterfacesDirective{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

            # Have to wrap this outside of strict since it's not normally allowed.
            no strict;
            &$Directive($line, $Value);
            use strict;

            $AddInterface++ if ($Directive eq "ADDINTERFACE");

            $line++;
        }
        AddErrorRef(1058, $INFSections{$Section}, "AddInterface", $RealSections{$Section}) if (! $AddInterface);

        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyInstallInterfaceSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyInstallSection($line, $Section) --------------------------------------------------------#
sub VerifyInstallSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);
    my($AddReg, $CopyFiles)  =  (0,0);
    $InstList{$Section}   = 1;
    my(%InstallDirective) = ("ADDREG",           1,
                             "BITREG",           1,
                             "COPYFILES",        1,
                             "DELFILES",         1,
                             "DELREG",           1,
                             "INCLUDE",          1,
                             "INI2REG",          1,
                             "LOGCONFIG",        1,
                             "NEEDS",            1,
                             "RENFILES",         1,
                             "UPDATEINIS",       1,
                             "UPDATEINIFIELDS",  1,
                             "DRIVERVER",        1,
                             "PROFILEITEMS",     1,
                             "REBOOT",           1,
                             "RESTART",          1);

    if (! defined($CheckedSections{$Section})) {die("Cannot validate $Section!\n");}
    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tInvoking VerifyInstallSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if (!defined($lines[$line]));
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }


            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($InstallDirective{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

            $AddReg++ if ($Directive eq "ADDREG");
            $CopyFiles++ if ($Directive eq "COPYFILES");
            # Have to wrap this outside of strict since it's not normally allowed.
            no strict;
            &$Directive($line, $Value) unless (($Directive eq "RESTART") or ($Directive eq "REBOOT"));
            use strict;
            $line++;
        }

        AddErrorRef(1058, $INFSections{$Section}-1, "\"AddReg\" or \"CopyFiles\"", $RealSections{$Section})
            unless ($CopyFiles||$AddReg);

        if (defined($INFSections{$Section . ".SERVICES"})) {
            VerifyInstallServicesSection($INFSections{$Section . ".SERVICES"},($Section . ".SERVICES"));
        }
        if (defined($INFSections{$Section . ".HW"})) {
            VerifyInstallHWSection($INFSections{($Section . ".HW")},($Section . ".HW"));
        }
        if (defined($INFSections{$Section . ".INTERFACES"})) {
            VerifyInstallInterfacesSection($INFSections{$Section . ".INTERFACES"},($Section . ".INTERFACES"));
        }
        if (defined($INFSections{$Section . ".COINSTALLERS"})) {
            VerifyInstallCoInstallersSection($INFSections{$Section . ".COINSTALLERS"},($Section . ".COINSTALLERS"));
        }
        if (defined($INFSections{$Section . ".FACTDEF"})) {
            VerifyInstallFactDefSection($INFSections{$Section . ".FACTDEF"},($Section . ".FACTDEF"));
        }

        if (defined($INFSections{$Section . ".LOGCONFIGOVERRIDE"})) {
            VerifyInstallOverrideSection($INFSections{$Section . ".LOGCONFIGOVERRIDE"},($Section . ".LOGCONFIGOVERRIDE"));
        }

        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyInstallSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyInstallOverrideSection($line, $Section) ------------------------------------------------#
sub VerifyInstallOverrideSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);
    my(%InstallOverrideDirective) = ("LOGCONFIG",  1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyInstallOverrideSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($InstallOverrideDirective{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

            # Have to wrap this outside of strict since it's not normally allowed.
            no strict;
            &$Directive($line, $Value);
            use strict;
            $line++;
        }

        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyInstallOverrideSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyInstallServicesSection($line, $Section) ------------------------------------------------#
sub VerifyInstallServicesSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);
    my($AddService) = 0;
	my($SPSVCINST_ASSOCSERVICE)=0;
	my(@Values);
    my(%InstallServicesDirective) = ("ADDSERVICE",  1,
                                     "DELSERVICE",  1,
                                     "INCLUDE",     1,
                                     "NEEDS",       1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyInstallServicesSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = (GetDirectiveValue($line,$lines[$line]));
			
            unless (defined($InstallServicesDirective{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

			# Check for multiple services setting SPSVCINST_ASSOCSERVICE (0x2)
			if ($Directive eq "ADDSERVICE") {
				@Values=split(/,/,$Value);
				if (defined($Values[1])) {
					$Values[1] =~ s/^[\s\t]*//;
					$Values[1] =~ s/[\s\t]*$//;

					# Handle the blank case
					if ($Values[1] =~ /^[\s\t]*$/) {
						$Values[1]=0;
					}

					if ($Values[1] =~ /^0x/i) {
						$Values[1] =~ s/^0x//i;
						$Values[1] = hex($Values[1]);
					} elsif ($Values[1] =~ /^0/) {
						$Values[1] =~ s/^0//i;
						$Values[1] = oct($Values[1]);
					} # Otherwise, assume it's already in decimal
						
					$SPSVCINST_ASSOCSERVICE++ if (($Values[1] & 0x2) == 0x2);
				}
			}

            # Have to wrap this outside of strict since it's not normally allowed.
            no strict;
            &$Directive($line, $Value);
            use strict;
                                                              # Assume the NEEDS has the AddService
            $AddService++ if (($Directive eq "ADDSERVICE") || ($Directive eq "NEEDS"));
            $line++;
        }

        AddErrorRef(1058, $INFSections{$Section}-1, "AddService", $RealSections{$Section}) if (! $AddService);
        AddErrorRef(1011, $INFSections{$Section}-1) if ($SPSVCINST_ASSOCSERVICE>1);
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyInstallServicesSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyInterfaceInstall32Section($line, $Section) ---------------------------------------------#
sub VerifyInterfaceInstall32Section {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, @Values);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyInterfaceInstall32Section (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            @Values     = split(/,/,uc(GetDirectiveValue($line,$lines[$line])));

            if (defined($Values[0]))  {
                VerifyInterfaceInstallSection($INFSections{$Values[0]}, $Values[0]);
            } else {
                AddErrorRef(1060, $line, "interface-install-section");
                $line++;
                next;
            }

            if (defined($Values[1]) && ($Values[1] ne 0)) {
                AddErrorRef(1068,$line,"flags. (Expected null or 0)");
            }

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyInterfaceInstall32Section...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyInterfaceInstallSection($line, $Section) -----------------------------------------------#
sub VerifyInterfaceInstallSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($MIN)     =  FALSE;
    my($Directive, $Value);

    #Valid InterfaceInstall Directives
    my(%InterfaceInstallDirective)=("ADDREG",             1,
                                    "DELREG",             1,
                                    "BITREG",             1,
                                    "COPYFILES",          1,
                                    "DELFILES",           1,
                                    "RENFILES",           1,
                                    "UPDATEINIS",         1,
                                    "INI2REG",            1,
                                    "UPDATEINIFIELDS",    1);


    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyInterfaceInstallSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($InterfaceInstallDirective{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

            if (($Directive eq "ADDREG") && defined($Value)){ # Must have atleast one AddReg!
                $MIN = TRUE;
            }

            # Have to wrap this outside of strict since it's not normally allowed.
            no strict;
            &$Directive($line, $Value);
            use strict;

            unless ($MIN) {
                AddErrorRef(1058,$INFSections{$Section}-1,"AddReg",$RealSections{$Section});
            }

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyInterfaceInstallSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyLogConfigSection($line, $Section) ------------------------------------------------------#
sub VerifyLogConfigSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value);

    #Valid RegRoot Values
    my($expect_RHS);
    my($cnt,$i);
    my(@IRQList,@IOList,@DMAList,@MEMList);
    my(%LCDirectives) = ( "CONFIGPRIORITY", "",
                          "DMACONFIG",      "",
                          "PCCARDCONFIG",   "",
                          "IOCONFIG",       "",
                          "MEMCONFIG",      "",
                          "IRQCONFIG",      "",
                          "MFCARDCONFIG",   "",);

    my(%Priority_Value)=( 'HARDWIRED'    => 1,
                          'DESIRED'      => 1,
                          'NORMAL'       => 1,
                          'SUBOPTIMAL'   => 1,
                          'DISABLED'     => 1,
                          'RESTART'      => 1,
                          'REBOOT'       => 1,
                          'POWEROFF'     => 1,
                          'HARDRECONFIG' => 1);

    my(%ConfigType)    =( 'FORCED'       => 1,
                          'BASIC'        => 1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyLogConfigSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));

            unless (defined($LCDirectives{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }
            $LCDirectives{$Directive} = $Value;

            # Check if ConfigPriority key.
            if ($Directive eq "CONFIGPRIORITY") {

                if ($Value =~  /^([A-Z]*)[,]?([A-Z]*)$/) {

                    if (! defined($expect_RHS=$Priority_Value{$1}) ) {
                        AddErrorRef(1100,$line);
                    }
                    if ($2 !~ /^$/) {
                        AddErrorRef(1101,$line) if (!defined($ConfigType{$2}));
                    }
                }
            }

            # Check if IRQCONFIG key.
            if ($Directive =~ /^IRQCONFIG$/) {

                if ($Value =~  /^[SL]?[:]?([0-9\s,]*)$/) {
                    @IRQList=GetList($1,$line,1);
                    $cnt=@IRQList;
                    for($i = 0; $i < $cnt; $i++) {
                        AddErrorRef(1102,$line) if (($IRQList[$i]>15)||($IRQList[$i]<0));
                    }
                } else {
                    AddErrorRef(1104,$line);
                }
            }

            # Check if IOCONFIG key.
            if ($Directive =~ /^IOCONFIG$/) {

                @IOList=GetList($Value,$line,1);
                $cnt=@IOList;
                for($i = 0; $i < $cnt; $i++) {

                    #start-end[([decode-mask]:[alias-offset]:[attr])]
                    if ($IOList[$i] =~  /^([0-9a-fA-F]*)-([0-9a-fA-F]*)[\(]?([0-9a-fA-F]*)[:]?([0-9a-fA-F]*)[:]?[M]?[\)]?$/) {
                        #Type 1 I/O Range..
                        Check64bitvalue($1,$line); #start value
                        Check64bitvalue($2,$line); #end value
                        Check64bitvalue($3,$line); #decode-mask value
                        Check32bitvalue($4,$line); #alias-offset value

                    #size@min-max[%align-mask][([decode-mask]:[alias-offset]:[attr])]
                    #RHS has to be of the form: $RHS='10@200-39f%fff0(3ff::)'

                    } elsif ($IOList[$i] =~ /^([0-9a-fA-F]*)@([0-9a-fA-F]*)-([0-9a-fA-F]*)[%]?([0-9a-fA-F]*)[\(]?([0-9a-fA-F]*)[:]?([0-9a-fA-F]*)[:]?[M]?[\)]?$/) {
                        Check32bitvalue($1,$line);  #size value
                        Check64bitvalue($2,$line);  #min value
                        Check64bitvalue($3,$line);  #max value
                        Check64bitvalue($4,$line);  #align-mask value
                        Check64bitvalue($5,$line);  #decode-mask value
                        Check32bitvalue($6,$line);  #alias-offset value
                    } else {
                        AddErrorRef(1105,$line);
                    }
                }   #for ends..
            }

            # Check if DMACONFIG key.
            if ($Directive =~ /^DMACONFIG$/) {

                if ($Value =~  /^[DW]?[:]?([0-9\s,]*)$/) {

                    @DMAList=GetList($1,$line,1);
                    $cnt=@DMAList;
                    for($i = 0; $i < $cnt; $i++) {
                        AddErrorRef(1106,$line) if (($DMAList[$i]>7)||($DMAList[$i]<0));
                    }
                } else {
                    AddErrorRef(1107,$line);
                }
            }

            # Check if MEMCONFIG key.
            if ($Directive =~ /^MEMCONFIG$/) {

                @MEMList= GetList($Value,$line,1);
                $cnt=@MEMList;
                for($i = 0; $i < $cnt; $i++) {
                    #start-end[(attr)]
                    if ($MEMList[$i] =~  /^([0-9a-fA-F]*)-([0-9a-fA-F]*)[\(][CDFRW]*[\)]?$/) {
                        #Type 1 I/O Range..
                        Check64bitvalue($1,$line);  #start value
                        Check64bitvalue($2,$line);  #end value

                        # size@min-max[%align-mask][(attr)]
                        # Value has to be of the form: $Value='10@200-39f%fff0(3ff::)'

                    } elsif ($MEMList[$i] =~ /^([0-9a-fA-F]*)@([0-9a-fA-F]*)-([0-9a-fA-F]*)[%]?([0-9a-fA-F]*)[\(]?[CDFRW]*[\)]?$/) {
                        Check32bitvalue($1,$line);  #size value
                        Check64bitvalue($2,$line);  #min value
                        Check64bitvalue($3,$line);  #max value
                        Check64bitvalue($4,$line);  #align-mask value
                    } elsif ($MEMList[$i] =~ /^([0-9a-fA-F]*)-([0-9a-fA-F]*)?$/) {
                        Check64bitvalue($1,$line);  #start value
                        Check64bitvalue($2,$line);  #end value
                    } else {
                        AddErrorRef(1108,$line);
                    }
                } #for ends..
            }

            $line++;
        }

        if ($LCDirectives{"CONFIGPRIORITY"} eq "") {
            AddErrorRef(1058, ($_[0] - 1), "ConfigPriority", $RealSections{$Section});
        }

        $CheckedSections{$Section} = TRUE;
    }
   print(STDERR "\tExiting VerifyLogConfigSection...\n") if ($DEBUG);
   return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyModelSection($line, $Section) ----------------------------------------------------------#
sub VerifyModelSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;

    my($Directive, @Values);
    my($Destination, $Source, $Temp, $Flag);
    my($ExcludeID, $ExcludeRef);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyModelSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {

            $line++, next if (!defined($lines[$line]));
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirectiveModel($line,$lines[$line]));
            @Values     = split(/,/, uc(GetDirectiveValue($line,$lines[$line])));

            if (defined($Values[0])) {
                if ($Values[0] =~ /__QUOTED_STR_\((\d*)\)__/) {
                    $Values[0] = $AllQuotes{$1};
                }
                $Values[0] =~ s/^[\s\t]*//;
                $Values[0] =~ s/[\s\t]*$//;

                if (defined($INFSections{$Values[0]})) {
                   VerifyInstallSection($INFSections{$Values[0]}, $Values[0]);
                }
                if (defined($INFSections{$Values[0].".NT"})) {
                    VerifyInstallSection($INFSections{$Values[0].".NT"}, $Values[0].".NT");
                }
                if (defined($INFSections{$Values[0].".NTX86"})) {
                   VerifyInstallSection($INFSections{$Values[0].".NTX86"}, $Values[0].".NTX86");
                }
                if ((!defined ($INFSections{$Values[0]}))            &&
                    (!defined ($INFSections{$Values[0].".NT"}))      &&
                    (!defined ($INFSections{$Values[0].".NTALPHA"})) &&
                    (!defined ($INFSections{$Values[0].".NTX86"})))  {
                    AddErrorRef(1003,$line,$Values[0]);
                }
            } else {
                AddErrorRef(1060, $line, "install-section-name");
            }

            if (!defined($Values[1])) {
                AddErrorRef(1060, $line, "hw-id");
            } else {
                if ($Values[1] =~ /__QUOTED_STR_\((\d*)\)__/) {
                    $Values[1] = $AllQuotes{$1};
                }
                $Values[1] =~ s/^[\s\t]*//;
                $Values[1] =~ s/[\s\t]*$//;
                $ModelLine{uc($Values[1])} = $line;
                shift(@Values);
                CreateArray($Directive, @Values);
            }
            $line++;
        }

        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyModelSection...\n") if ($DEBUG);
    return(TRUE);

    #--------------------------------------------------------------#
    # Create an anonymous array to hold model data                 #
    #-- CreateArray -----------------------------------------------#
    sub CreateArray {
        my($hw_id);
        my($directive) = shift;
        my(@list)      = @_;
        my(@newarray)  = ();

        foreach $hw_id (@list) {
            $hw_id =~ s/^\s*//;
            $hw_id =~ s/\s*$//;
            $hw_id = $AllQuotes{$1} if ($hw_id =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
            $Models = join(";", $Models, $hw_id);
            push( @newarray, $hw_id);
        }
        $Models{$directive} = \@newarray;
        return(TRUE);
    }

}

#-------------------------------------------------------------------------------------------------#
#-- VerifyProfileItemsSection($line, $Section) ---------------------------------------------------#
sub VerifyProfileItemsSection {
    my($line)    =  $_[0];     # Line num array to start with
    my($Section) =  $_[1];     # Section name (Just for reference)
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;     # Control Flag
    my($Directive, $DirectiveValue, @DirectiveValues);
    my($CMDLINE, $NAME) = (FALSE, FALSE);

    # Valid RegRoot Values
    my(%PIDirectives) = ( "NAME",       1,
                          "CMDLINE",    1,
                          "SUBDIR",     1,
                          "WORKINGDIR", 1,
                          "ICONPATH",   1,
                          "ICONINDEX",  1,
                          "HOTKEY",     1,
                          "INFOTIP",    1);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        # Don't check a section twice.
        return(FALSE);
    } else {
        # Being verbose
        print(STDERR "\tBeginning VerifyProfileItemsSection (@_)...\n") if ($DEBUG);

        # Start checking this section.
        while (! $DONE) {
            $lines[$line] =~ s/^\s*//; # Kill leading whitespace
            $lines[$line] =~ s/\s*$//; # Kill trailing whitespace

            # Undef'd lines are just blank, ignore and move on.
            if (! defined($lines[$line])) {
                $line++;
                next;
            }

            #Skip blank lines even though this should never be true.
            if ($lines[$line] =~ /^\s*$/) {
                $line++;
                next;
            }

            # We've hit the next section so fall out of the loop.
            $DONE=TRUE, next if ($lines[$line] =~ /^\[.*\]$/);

            # Propogate the values in the line.
            $Directive = uc(GetDirective($line, $lines[$line]));

            unless (defined($PIDirectives{$Directive})) {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));;
                $line++;
                next;
            }

            if      ($Directive eq "NAME") {
                @DirectiveValues=split(/,/,GetDirectiveValue($line,$lines[$line]));
                AddErrorRef(1088, $line, "First", "Name") unless (defined($DirectiveValues[0]));
                $NAME=TRUE;
            } elsif ($Directive eq "CMDLINE") {
                @DirectiveValues=split(/,/,GetDirectiveValue($line,$lines[$line]));
                unless (($DirectiveValues[0] >= 66000) and
                        ($DirectiveValues[0] <= 66004) and
                        ($CLASS eq "PRINTER")) {
                    AddErrorRef(1044, $line, $DirectiveValues[0]) unless (defined($DirIDValues{$DirectiveValues[0]}));
                }
                AddErrorRef(1088, $line, "Third", "CmdLine") unless (defined($DirectiveValues[2]));
                $CMDLINE=TRUE;
            } elsif ($Directive eq "SUBDIR") {
                $DirectiveValue=GetDirectiveValue($line,$lines[$line]);
            } elsif ($Directive eq "WORKINGDIR") {
                @DirectiveValues=split(/,/,GetDirectiveValue($line,$lines[$line]));
                unless (($DirectiveValues[0] >= 66000) and
                        ($DirectiveValues[0] <= 66004) and
                        ($CLASS eq "PRINTER")) {
                    AddErrorRef(1044, $line, $DirectiveValues[0]) unless (defined($DirIDValues{$DirectiveValues[0]}));
                }
            } elsif ($Directive eq "ICONPATH") {
                @DirectiveValues=split(/,/,GetDirectiveValue($line,$lines[$line]));
                unless (($DirectiveValues[0] >= 66000) and
                        ($DirectiveValues[0] <= 66004) and
                        ($CLASS eq "PRINTER")) {
                    AddErrorRef(1044, $line, $DirectiveValues[0]) unless (defined($DirIDValues{$DirectiveValues[0]}));
                }
                AddErrorRef(1088, $line, "Third", "IconPath") unless (defined($DirectiveValues[2]));
            } elsif ($Directive eq "ICONINDEX") {
                $DirectiveValue=GetDirectiveValue($line,$lines[$line]);
            } elsif ($Directive eq "HOTKEY") {
                $DirectiveValue=GetDirectiveValue($line,$lines[$line]);
            } elsif ($Directive eq "INFOTIP") {
                $DirectiveValue=GetDirectiveValue($line,$lines[$line]);
            }
            # More verboseness for debug execution.
            GetLine($line) if ($DEBUG);

            # Move on to next line.
            $line++;
        }

        AddErrorRef(1079, $line, "CmdLine", "$Section") unless ($CMDLINE);
        AddErrorRef(1080, $line, "Name", "$Section")    unless ($NAME);
        # Mark this section as checked.
        $CheckedSections{$Section} = TRUE;
    }
    # Still being verbose.
    print(STDERR "\tExiting VerifyProfileItemsSection...\n") if ($DEBUG);

    # Exit out of this sub.
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyRenFilesSection($line, $Section) -------------------------------------------------------#
sub VerifyRenFilesSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($OldName, $NewName);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyRenFilesSection (@_)...\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            ($NewName, $OldName) = split(/,/,$lines[$line]);

            if (defined($OldName)) {
                $OldName =~ s/^[\s\t]*//;
                $OldName =~ s/[\s\t]*$//;
                AddErrorRef(1097, $line) if ($OldName =~ /\.inf$/);
            } else {
                AddErrorRef(1060, $line, "old-source-file-name");
            }

            if (defined($NewName)) {
                $NewName =~ s/^[\s\t]*//;
                $NewName =~ s/[\s\t]*$//;
                AddErrorRef(1098, $line) if ($NewName =~ /\.inf$/);
            } else {
                AddErrorRef(1060, $line, "new-dest-file-name");
            }

            if ((defined($OldName)) and (defined($NewName))) {
                AddErrorRef(1076, $line) if (uc($OldName) eq uc($NewName));
            }

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyRenFilesSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#  Verifies the SourceDisksFiles Section of the INF                                               #
#-- VerifySDFiles($line, $Section) ---------------------------------------------------------------#
sub VerifySDFiles {
    my($line, $Section)  = @_;
    my(@lines)   =  @INFlines; #Local copy =-)
    my($Directive,$DiskID,$SubDir,$Size);
    my($DONE)          = FALSE;


    $CheckedSections{$Section} = TRUE;

    print(STDERR "\tBeginning VerifySDFiles (@_)...\n") if ($DEBUG);
    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if (($lines[$line] =~ /^\[.*\]/)) { # or ($line > $INFSections{'EOF'})) {# Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        GetLine($line) if ($DEBUG);
        $Directive = uc(GetDirective($line,$lines[$line]));
        ($DiskID,$SubDir,$Size) = split(/,/,uc(GetDirectiveValue($line,$lines[$line])));

        if (($LayoutCheck) and (defined($LayoutFiles{uc($Directive)})) and
            ($LayoutFiles{uc($Directive)} ne "SOURCEDISKSFILES")) {

            unless (uc($Section) eq uc($LayoutFiles{uc($Directive)})) {
                AddErrorRef(1096,$line,$Section,$LayoutFiles{uc($Directive)});
            }
        }

        AddErrorRef(1114,$line)                    if ((defined($Directive)) and ($Directive =~ /\.inf$/i));
        AddErrorRef(1060,$line,"diskid")           if (!defined($DiskID));
        AddErrorRef(1062,$line,"diskid","numeric") if ($DiskID !~ /^\d*$/);
        $line++;
    }
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);
    print(STDERR "\tExiting VerifySDFiles...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#  Verifies the SourceDisksNames Section of the INF                                               #
#-- VerifySDNames($line, $Section) ---------------------------------------------------------------#
sub VerifySDNames {
    my($line, $Section)  = @_;
    my(@lines)           =  @INFlines; #Local copy =-)
    my($DONE)            = FALSE;
    my($TestForNonToken, $Directive, $DiskDesc, $Tagfile,$UNUSED,$Path);

    $CheckedSections{$Section} = TRUE;

    print(STDERR "\tBeginning VerifySDNames (@_)...\n") if ($DEBUG);
    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if (($lines[$line] =~ /^\[.*\]/)) {
            # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        GetLine($line) if ($DEBUG);

        $TestForNonToken=$lines[$line];
        $TestForNonToken=~s/.*=[\s\t]*//;
        $TestForNonToken=~s/[\s\t]*$//;
        ($DiskDesc, $Tagfile,$UNUSED,$Path) = split(/,/,$TestForNonToken);
        AddWarnRef(2023, $line) unless ($DiskDesc =~ /%.*%/);

        $Directive = uc(GetDirective($line,$lines[$line]));
        ($DiskDesc, $Tagfile, $UNUSED, $Path) = split(/,/,uc(GetDirectiveValue($line,$lines[$line])));
        $DiskDesc = $AllQuotes{$1} if ($DiskDesc =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);# replace with stored string
        AddErrorRef(1062,$line,"diskid","numeric") if ($Directive !~ /^\d*$/);
        AddErrorRef(1060,$line,"disk-description") if (!defined($DiskDesc));
        AddErrorRef(1022,$line,"\"unused\" (field 3)") if (defined($UNUSED) and ($UNUSED ne ""));
        $line++;
    }
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);
    print(STDERR "\tExiting VerifySDNames..\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyServiceInstallSection($line, $Section) -------------------------------------------------#
sub VerifyServiceInstallSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($Directive, $Value, %SILine);

    #Valid RegRoot Values
    my(%SIDirectives) = ( "DISPLAYNAME",     "UNDEF",
                          "SERVICETYPE",     "UNDEF",
                          "STARTTYPE",       "UNDEF",
                          "ERRORCONTROL",    "UNDEF",
                          "SERVICEBINARY",   "UNDEF",
                          "STARTNAME",       "UNDEF",
                          "ADDREG",          "VERIFY", # Handled but subroutine
                          "DELREG",          "VERIFY", # Handled but subroutine
                          "BITREG",          "VERIFY", # Handled but subroutine
                          "LOADORDERGROUP",  "UNDEF",
                          "DEPENDENCIES",    "UNDEF",
                          "DESCRIPTION",     "UNDEF");

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyServiceInstallSection (@_)...\n") if ($DEBUG);
        $CheckedSections{$Section} = TRUE;
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/); # Skip if it's a blank line
            if (($lines[$line] =~ /^\[.*\]/)) {
                # Quit once we hit next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            $Directive = uc(GetDirective($line,$lines[$line]));
            $Value     = uc(GetDirectiveValue($line,$lines[$line]));
            $Value     =~ s/^[\t\s]*//; # Kill leading  whitespace
            $Value     =~ s/[\t\s]*$//; # Kill trailing whitespace
            unless (defined($SIDirectives{uc($Directive)}))  {
                AddErrorRef(1057, $line, $Directive) unless ($Module->DCDirective($Directive));
                $line++;
                next;
            }

            if ($SIDirectives{$Directive} eq "VERIFY") {
                # Have to wrap this outside of strict since it's not normally allowed.
                no strict;
                &$Directive($line, $Value);
                use strict;
            } elsif ($Directive eq "DESCRIPTION") {
                $Value = $AllQuotes{$1} if ($Value =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);
                AddErrorRef(1078, $line) if (length($Value) > 1024);
            } elsif ($SIDirectives{$Directive} ne "UNDEF") {
                AddErrRef(1006, $line, $Directive);
            } else {
                $SIDirectives{$Directive} = $Value;
                $SILine{$Directive}       = $line;
            }

            $line++;
        }
        # Valid flag values for $Flags
		my($Valid)=(0x01|0x02|0x08|0x10|0x20|0x40|0x80|0x100);
        if ($SIDirectives{"SERVICETYPE"} ne "UNDEF") {
            $SIDirectives{"SERVICETYPE"} = hex(lc($SIDirectives{"SERVICETYPE"}))
                if ($SIDirectives{"SERVICETYPE"} =~ /^0x[a-f0-9]*$/i);
            AddErrorRef(1071, $SILine{"SERVICETYPE"}, $SIDirectives{"SERVICETYPE"}, "ServiceType")
                unless (($SIDirectives{"SERVICETYPE"} =~ /^(0x){0,1}[a-fA-F0-9]*$/i) &&
                        (($SIDirectives{"SERVICETYPE"}|$Valid)==$Valid));

        } else {
            AddErrorRef(1058, $INFSections{$Section}-1, "ServiceType", $RealSections{$Section});
        }

        if ($SIDirectives{"ERRORCONTROL"} ne "UNDEF") {
            $SIDirectives{"ERRORCONTROL"} = hex(lc($SIDirectives{"ERRORCONTROL"}))
                if ($SIDirectives{"ERRORCONTROL"} =~ /^0x[a-f0-9]*$/i);
            AddErrorRef(1071, $SILine{"ERRORCONTROL"}, $SIDirectives{"ERRORCONTROL"}, "ErrorControl")
                unless (($SIDirectives{"ERRORCONTROL"} =~ /^[0-9]*$/) &&
                       (($SIDirectives{"ERRORCONTROL"} == 0x1) ||
                        ($SIDirectives{"ERRORCONTROL"} == 0x2) ||
                        ($SIDirectives{"ERRORCONTROL"} == 0x3) ||
                        ($SIDirectives{"ERRORCONTROL"} == 0x0)));
            AddWarnRef(2101, $SILine{"ERRORCONTROL"}, $SIDirectives{"ERRORCONTROL"})
                if (($SIDirectives{"ERRORCONTROL"} =~ /^[0[xX]][a-fA-F0-9]*$/) &&
                    ($SIDirectives{"ERRORCONTROL"} == 0x3));
        } else {
            AddErrorRef(1058, $INFSections{$Section}-1, "ServiceType", $RealSections{$Section});
        }

        if ($SIDirectives{"SERVICEBINARY"} eq "UNDEF") {
            AddErrorRef(1058, $INFSections{$Section}-1, "ServiceType", $RealSections{$Section});
        }

        if ($SIDirectives{"STARTTYPE"} ne "UNDEF") {
            $SIDirectives{"STARTTYPE"} = hex(lc($SIDirectives{"STARTTYPE"}))
                if ($SIDirectives{"STARTTYPE"} =~ /^0x[a-f0-9]*$/i);
            if ($SIDirectives{"STARTTYPE"} =~ /^\d*$/) {
                if ($SIDirectives{"STARTTYPE"} eq "UNDEF") {
                    # This should be much cleaner, but will catch many of the errors popular here.
                } else {
                    AddErrorRef(1068, $SILine{"STARTTYPE"}, "StartType", $Section)
                        unless ((hex(lc($SIDirectives{"STARTTYPE"})) | 0x7) == 0x7);
                    AddErrorRef(1089, $SILine{"STARTTYPE"}) if (hex(lc($SIDirectives{"STARTTYPE"})) == 0x4);
                    AddWarnRef (2027, $SILine{"STARTTYPE"}, hex(lc($SIDirectives{"STARTTYPE"})),
                         "should only be specified by drivers that are required to boot.")
                         if (hex(lc($SIDirectives{"STARTTYPE"})) == 0x2);
                    AddWarnRef (2027, $SILine{"STARTTYPE"}, hex(lc($SIDirectives{"STARTTYPE"})),
                        "should only be specified by drivers that may detect legacy hardware.")
                         if (hex(lc($SIDirectives{"STARTTYPE"})) == 0x1);
                    AddErrorRef(1066, $SILine{"STARTTYPE"}, "StartType", "[$Section]")
                        if (hex(lc($SIDirectives{"STARTTYPE"})) > 4);
                }
            } else {
                AddErrorRef(1071,$SILine{"STARTTYPE"},$SIDirectives{'STARTTYPE'},"StartType");
            }
        } else {
            AddErrorRef(1058, $INFSections{$Section}-1, "StartType", $RealSections{$Section});
        }
    }

    print(STDERR "\tExiting VerifyServiceInstallSection...\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyUpdateIniFieldsSection($line, $Section) ------------------------------------------------#
sub VerifyUpdateIniFieldsSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($IniFile, $IniSection, $Profile, $OldField, $NewField, $Flags);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyUpdateIniFieldsSection (@_)....\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }
            GetLine($line) if ($DEBUG);
            ($IniFile, $IniSection, $Profile, $OldField, $NewField, $Flags) = split(/,/,$lines[$line]);

            if (defined($Flags) && (($Flags < 0) && ($Flags > 3))) {
                AddErrorRef(1068, $line, "flag (Expected null, 0, 1, 2, or 3).");
            }

            unless (defined($IniFile)) {
                AddErrorRef(1060, $line, "ini-file");
            }

            unless (defined($IniSection)) {
                AddErrorRef(1060, $line, "ini-section");
            }

            unless (defined($Profile)) {
                AddErrorRef(1060, $line, "profile-name");
            }

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyUpdateIniFieldsSection (@_)....\n") if ($DEBUG);
    return(TRUE);
}

#-------------------------------------------------------------------------------------------------#
#-- VerifyUpdateInisSection($line, $Section) -----------------------------------------------------#
sub VerifyUpdateInisSection {
    my($line)    =  $_[0];
    my($Section) =  $_[1];
    my(@lines)   =  @INFlines; #Local copy =-)
    my($DONE)    =  FALSE;
    my($IniFile, $IniSection, $Old, $New, $Flags);

    $CheckedSections{$Section} = FALSE  # Set to false if in inluded inf and not already read
        if (($INFSections{$Section} <= $LastLine) and (!defined($IncludedSections{$Section})));
    $IncludedSections{$Section} = "included" unless ($INFSections{$Section} <= $LastLine);

    if ($CheckedSections{$Section} == TRUE)  {
        return(FALSE);
    } else {
        print(STDERR "\tBeginning VerifyUpdateInisSection (@_)....\n") if ($DEBUG);
        while (! $DONE) {
            $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
            if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
                $DONE = TRUE;
                next;
            }

            GetLine($line) if ($DEBUG);
            ($IniFile, $IniSection, $Old, $New, $Flags) = split(/,/,$lines[$line]);
            $Flags = 0 if (! defined($Flags));
            $Flags = 0 if (($Flags =~ /^$/)||($Flags =~ /^\s$/));

            if (defined($Flags) && (($Flags < 0) && ($Flags > 3))) {
                AddErrorRef(1068, $line, "flag (Expected null, 0, 1, 2, or 3).");
            }

            unless (defined($IniFile)) {
                AddErrorRef(1060, $line, "ini-file");
            }

            unless (defined($IniSection)) {
                AddErrorRef(1060, $line, "ini-section");
            }

            $line++;
        }
        $CheckedSections{$Section} = TRUE;
    }
    print(STDERR "\tExiting VerifyUpdateInisSection...\n") if ($DEBUG);
    return(TRUE);
}
#-------------------------------------------------------------------------------------------------#
# Verifies the [VERSION] section of the INF                                                       #
#-- VerifyVersion($line, @lines) -----------------------------------------------------------------#
sub VerifyVersion {
    my($line)     = $_[0];
    my($SecStart) = $line - 1;
    shift(@_);      # Remove the 1st param
    my(@lines)    = @INFlines;
    my($DONE)     = FALSE;
    my($Directive, $Value);
    my($CGLine, %BadFirstPass);

    my(%VerVal)=('SIGNATURE'          , '',   # Should be $CHICAGO$
                 'CLASS'              , '',   # Should match ClassGUID
                 'CLASSGUID'          , '',   # Should match Class
                 'PROVIDER'           , '',   #
                 'LAYOUTFILE'         , '',   # Used for MS Inf's Only
                 'CATALOGFILE'        , '',   # Used only if LayoutFile isn't defined
                 'CATALOGFILE.NT'     , '',   # Optional
                 'CATALOGFILE.NTX86'  , '',   # Optional
                 'DRIVERVER'          , '');  # Must be here or in each model section
                 # SetupClass may be a valid option here?

    print(STDERR "\tBeginning VerifyVersion (@_)....\n") if ($DEBUG);
    $CheckedSections{"VERSION"} = TRUE;
    while (! $DONE) {
        $line++, next if ($lines[$line] =~ /^\s$/);# Skip if it's a blank line
        if ($lines[$line] =~ /^\[.*\]/) {          # Quit once we get to the next section
            $DONE = TRUE;
            next;
        }

        GetLine($line) if ($DEBUG);
        $Directive = uc(GetDirective($line,$lines[$line]));
        $Value     = uc(GetDirectiveValue($line,$lines[$line]));
        $Value     = $AllQuotes{$1} if ($Value =~ /^__QUOTED_STR_\(([0-9]*)\)__$/);# replace with stored string
        if (defined($VerVal{$Directive})) {
            $VerVal{$Directive} = $Value;
        } else {
            # Create hash of unknown directives to verify against class module
            $BadFirstPass{$Directive} = $line;
        }

        if (uc($Directive) eq "CLASS") {
               $CLASS  = $Value;
               $CGLine = $line;
        }

        if ((uc($Directive) eq "SIGNATURE") && !((uc($Value) eq "\$CHICAGO\$") ||
            (uc($Value) eq "\$WINDOWS 95\$") || (uc($Value) eq "\$WINDOWS NT\$"))) {
                AddErrorRef(1008, $line);
        }

        if (uc($Directive) =~ /^CATALOG/) {
                $Value =~ s/^\s*//; # Chop leading space
                $Value =~ s/\s*$//; # Chop tailing space
                unless ($Value =~ /.{1,8}\.cat/i) {
                    AddErrorRef(1064,$line);
                }
        }

        if (uc($Directive) eq "LAYOUTFILE") {
                AddWarnRef(2013, $line);
				$LAYOUTFILE_USED=TRUE;
        }

        if (uc($Directive) eq "DRIVERVER") {
                DRIVERVER($line, $Value);
        }
        $line++;
    }

    CheckClass($VerVal{'CLASS'}, $VerVal{'CLASSGUID'}, $CGLine)
        unless (($VerVal{'CLASSGUID'} eq '') || ($VerVal{'CLASS'} eq ''));

    unless ($VerVal{'CLASS'}       ne '')    { AddErrorRef(1053, $SecStart, "");}
    unless ($VerVal{'CLASSGUID'}   ne '')    { AddErrorRef(1054, $SecStart, "");}
    unless ($VerVal{'PROVIDER'}    ne '')    { AddErrorRef(1079, $SecStart, "Provider",  "Version");}
    unless ((grep(/DriverVer/i,@lines)) > 0) { AddErrorRef(1080, $SecStart, "DriverVer", "Version");}

    # If we aren't looking at a system supplied inf (i.e. no LayoutFile directive) we must have a
    #   CatalogFile directive.
    unless (($VerVal{'CATALOGFILE'} ne '')        || ($VerVal{'CATALOGFILE.NT'} ne '') || ($VerVal{'CATALOGFILE.NTX86'} ne '')) {
        AddErrorRef(1081, $SecStart, "CatalogFile", "Version") unless ($VerVal{'LAYOUTFILE'} ne '');
    }

    if (($VerVal{'CLASS'} ne '')) { # && (-e "${VerVal{'CLASS'}}.pm")) {
        $ClassMod = $VerVal{'CLASS'};
    } else {
        $ClassMod = "GENERIC";
        AddWarnRef(2018, $CGLine) unless ($VerVal{'CLASS'} eq '');
    }

            # Find out the Class this INF is for
            $Module = uc(ClassMod());
            #
            # Pull in DeviceClass Module $Module. If it doesn't exist,
            #   default to using the generic methods in the ChkInf Module
            #
            if (-e "${Module}.pm") {
                require "${Module}.pm";
            } else {
                $Module = "GENERIC";
                require "${Module}.pm";
            }
            print(STDERR "\t");
            # Package names can't begin with numerics, so
			# refer to 1394 as FIREWIRE
			$Module="FIREWIRE" if ($Module eq "1394");
            $Module->Exists();
            print(STDERR "\tIniting $Module Globals...\n") if ($DEBUG);
            $Module->InitGlobals($inffile, $DCOptions);

    # Verify unknown directives against Class Module before flagging as bad
    foreach (sort(keys(%BadFirstPass))) {
            AddErrorRef(1057,$BadFirstPass{$_},$_) unless ($Module->DCDirective($_));
    }

    print(STDERR "\tExiting VerifyVersion...\n") if ($DEBUG);
    return(TRUE);
}



#--- Accepts a line number, flags that line as read, and returns it ---#
#--- Put here for testing purposes ------------------------------------#
sub GetLine {
    AddWarnRef(9999, $_[0]);
    return($INFlines[$_[0]]);
}


__END__

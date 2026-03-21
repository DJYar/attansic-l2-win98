MessageId=1
SymbolicName=MSG_CANNOT_ENUM_ITEM
Language=English
Failed to enumerate stored item "%1" (err=%2!u!).
.

MessageId=
SymbolicName=MSG_CANNOT_GET_DESCRIPTION
Language=English
Could not get item description of ID:%1!05x! (err=%2!u!).
.

MessageId=
SymbolicName=MSG_CANNOT_ALLOC_MEMORY
Language=English
Could not allocate memory in %1, size=%2!u! (err=%3!u!).
.

MessageId=
SymbolicName=MSG_CANNOT_ENUM_TYPE
Language=English
Failed to enumerate type "%1" (err=%2!u!).
.

MessageId=
SymbolicName=MSG_CANNOT_GET_TYPEINFO
Language=English
Could not get type information of Handle:%1!08x! (err=%2!u!).
.

MessageId=
SymbolicName=MSG_CANNOT_GET_ITEMINFO
Language=English
Could not get item information of ID:%1!05x! (err=%2!u!).
.

MessageId=
SymbolicName=MSG_CANNOT_ENUM_LINK
Language=English
Failed to enumerate linked item with ID:%1!05x! (err=%2!u!).
.

MessageId=
SymbolicName=MSG_MISC_ROUTINE_FAIL
Language=English
%1 failed (err=%2!u!).
.

MessageId=
SymbolicName=MSG_FAIL_PARSE_COMMAND_LINE
Language=English
Failed in CommandLineToArgvW, %1 (err=%2!u!).
.




MessageId=
SymbolicName=MSG_TITLE_TYPE_CMD
Language=English
Handle   Type Name                                          Support Plug-in
---------------------------------------------------------------------------
.

MessageId=
SymbolicName=MSG_TYPE_INFO
Language=English
%1!08x! %2!-50s! %3!-s!
.

MessageId=
SymbolicName=MSG_ALLOWED_CONTAINER_TYPES
Language=English
Allowed container types
.

MessageId=
SymbolicName=MSG_TITLE_VIEW_ITEM_CMD
Language=English
Reported Item
---------------------------------------------------------------------------
.

MessageId=
SymbolicName=MSG_ITEM_INFO
Language=English
@%1!08x!:%2!05x! (ID:%3!05x!) %4!-30s! by %5
.

MessageId=
SymbolicName=MSG_TITLE_DESC_ITEM_CMD
Language=English

.

MessageId=
SymbolicName=MSG_TITLE_DESC_TYPE
Language=English
Description of Type
---------------------------------------------------------------------------
.

MessageId=
SymbolicName=MSG_LINE
Language=English
---------------------------------------------------------------------------
.

MessageId=
SymbolicName=MSG_ITEM_START_LINK
Language=English
%1 @%2!08x! (ID:%3!05x!, Reported by %4)
.

MessageId=
SymbolicName=MSG_LINK_ITEM
Language=English
%1 @%2!08x! (ID:%3!05x!, Linked by %4)
.

MessageId=
SymbolicName=MSG_HELP
Language=English

type [type_name] [-h type_handle]
   - display registered types.

     type_name   Specify type name string. You can use '*(asterisk)' as a wild
                 card at the end of string. If you use a wild card, every
                 matched type information is enumerated in short form. If full
                 type_name is specified, display type information in detail.
     -h          Specify a hexadecimal type handle.

find [type_name] [start_addr [end_addr] [-o]] [-v]
   - find located item by type_name or  memory range.

     type_name   Specify type name string. You can use '*(asterisk)' as a wild
                 card at the end of string.
     start_addr  Hexadecimal start address of memory range. Default is 0. If
                 you omit both start_addr and end_addr, memory range becomes
                 whole virtual address.
     end_addr    Hexadecimal end address of memory range. If you specify
                 start_addr only, end_addr becomes the same as start_addr.
     -o          Display all items that overlap with specified memory range.
     -v          Display formatted description of each item.

view item_id
   - display item information and formatted description.

     item_id     Hexadecimal value of item ID.

link item_id [depth] [-{b|f|m}]
   - display linked items.

     item_id     Hexadecimal value of item ID.
     depth       Specify nesting level of link. Default nesting level is 1.
     -b          Display items from which an item links.
     -f          Display items to which an item links.
     -m          Display items with which an item links (forward and backward).

d{b|d} [address] [length]
   - hexadecimal dump of given range.

     address     Hexadecimal start address of memory range.
     length      Hexadecimal length of memory range.

help, ?
   - show this help.

exit
   - exit from command mode.
.


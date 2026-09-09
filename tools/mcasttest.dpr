program McastTest;

{$APPTYPE CONSOLE}

uses
  Windows, WinSock;

const
  GroupAddress = '239.255.66.66';
  InterfaceAddress = '192.168.0.121';
  ListenPort = 50000;

type
  TIPMreq = packed record
    Group: u_long;
    InterfaceAddr: u_long;
  end;

var
  WsaData: TWSAData;
  Sock: TSocket;
  BindAddress: TSockAddrIn;
  SourceAddress: TSockAddrIn;
  Membership: TIPMreq;
  ReadSet: TFDSet;
  Timeout: TTimeVal;
  SourceLength: Integer;
  Received: Integer;
  Buffer: array[0..511] of Char;
begin
  if WSAStartup($0101, WsaData) <> 0 then
  begin
    Writeln('FAIL WSAStartup');
    Halt(1);
  end;

  Sock := socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if Sock = INVALID_SOCKET then
  begin
    Writeln('FAIL socket ', WSAGetLastError);
    WSACleanup;
    Halt(1);
  end;

  FillChar(BindAddress, SizeOf(BindAddress), 0);
  BindAddress.sin_family := AF_INET;
  BindAddress.sin_port := htons(ListenPort);
  BindAddress.sin_addr.S_addr := INADDR_ANY;
  if bind(Sock, TSockAddr(BindAddress), SizeOf(BindAddress)) = SOCKET_ERROR then
  begin
    Writeln('FAIL bind ', WSAGetLastError);
    closesocket(Sock);
    WSACleanup;
    Halt(1);
  end;

  Membership.Group := inet_addr(GroupAddress);
  Membership.InterfaceAddr := inet_addr(InterfaceAddress);
  if setsockopt(Sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                PChar(@Membership), SizeOf(Membership)) = SOCKET_ERROR then
  begin
    Writeln('FAIL membership ', WSAGetLastError);
    closesocket(Sock);
    WSACleanup;
    Halt(1);
  end;

  Writeln('READY ', GroupAddress, ':', ListenPort);
  FD_ZERO(ReadSet);
  FD_SET(Sock, ReadSet);
  Timeout.tv_sec := 20;
  Timeout.tv_usec := 0;
  if select(0, @ReadSet, nil, nil, @Timeout) > 0 then
  begin
    SourceLength := SizeOf(SourceAddress);
    Received := recvfrom(Sock, Buffer, SizeOf(Buffer) - 1, 0,
                         TSockAddr(SourceAddress), SourceLength);
    if Received >= 0 then
    begin
      Buffer[Received] := #0;
      Writeln('PASS bytes=', Received, ' from=',
              inet_ntoa(SourceAddress.sin_addr), ':',
              ntohs(SourceAddress.sin_port), ' data=', PChar(@Buffer));
    end
    else
      Writeln('FAIL recv ', WSAGetLastError);
  end
  else
    Writeln('FAIL timeout');

  setsockopt(Sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
             PChar(@Membership), SizeOf(Membership));
  closesocket(Sock);
  WSACleanup;
end.

// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// docs.h is a part of Blitzping.
// ---------------------------------------------------------------------


// NOTE: All of the command-line outputs herein conform to the RFC 678
// plaintext document standard it is good readability practice to limit
// a line of code's columns to 72 characters (which include only the
// printable characters, not line endings or cursors).
//     I specifically chose 72 (and not some other limit like 80/132)
// to ensure maximal compatibility with older technology, terminals,
// paper hardcopies, and e-mails.  While some other guidelines permit
// more than just 72 characters, it is still important to note that
// American teletypewriters could sometimes write upto only 72, and
// older code (e.g., FORTRAN, Ada, COBOL, Assembler, etc.) used to
// be hand-written on a "code form" in corporations like IBM; said
// code form typically reserved the first 72 columns for statements,
// 8 for serial numbers, and the remainder for comments, which was
// finally turned into a physical punch card with 80 columns.
//     Even in modern times, the 72 limit can still be beneficial:
// you can easily quote a 72-character line over e-mail without
// requiring word-wrapping or horizontal scrolling.
//     As a sidenote, the reason that some guidelines, like PEP 8
// (Style Guide for Python Code), recommended 79 characters (i.e.,
// not 80) was that the 80th character in a 80x24 terminal might
// have been a bit hard to read.



_Pragma ("once")
#ifndef DOCS_H
#define DOCS_H


// TODO: Do I use \r\n here, or continue relying on libc?
// TODO: Create a man.1 (man page) file using this
//
// TODO: Add an option to control memory alignment?
// TODO: a 'Count' option instead of infinite sends
//
// NOTE: Unfortunately, C preprocessor is unable to include actual .txt
// files (which could otherwise be holding this text) into strings.
// (C23 is apparently able to do this using the new #embed directive.)
static const char HELP_TEXT_OVERVIEW[] = "\
Usage: blitzping [options]\n\
\n\
Options may use either -U (unix style), --gnu-style, or /dos-style\n\
conventions, and the \"=\" sign may be omitted.\n\
\n\
Example:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\
  blitzping --num-threads=4 --proto=tcp --dest-ip=10.10.10.10\n\
  blitzping --help=proto\n\
\n\
General:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\
  -? --help=<command>     Display this help message or more info.\n\
  -! --about              Display information about the Program.\n\
  -V --version            Display the Program version.\n\
  -Q --quiet              Suppress all output except errors.\n\
Advanced::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\
  -$ --bypass-checks      Ignore system compatibility issues (e.g., \n\
                          wrong endianness) if startup checks fail.\n\
                          (May result in unexpected behavior.)\n\
     --logger-level=<n>   Set the verbosity level of the logger.\n\
                          (-1: off, 0: critical, 1: error, 2: warn\n\
                          3: info, 4: debug; default: 3.)\n\
     --no-log-timestamp   Don't prefix log messages with timestamps.\n\
  -# --num-threads=<0-n>  Number of threads to poll the socket with.\n\
                          (Default: system thread count; 0: disable\n\
                          threading, run everything in main thread.)\n\
     --native-threads     If both C11 libc <threads.h> and native\n\
                          (i.e., POSIX/Win32) threads are available,\n\
                          will prefer the native implementation.\n\
     --buffer-size=<0-n>  Number of packets to pre-craft and pass to\n\
                          the kernel in batches. (default: as many\n\
                          as possible; 0: disable buffering.)\n\
     --no-async-sock      Use blocking (not asynchronous) sockets.\n\
                          (Will severely hinder performance.)\n\
     --no-mem-lock        Don't lock memory pages; allow disk swap.\n\
                          (May reduce performance.)\n\
     --no-prefetch        Don't prefetch packet buffer to CPU cache.\n\
                          (May reduce performance.)\n\
";

// TODO: Have a "raw (no protocol) layer 3 option.

static const char HELP_TEXT_IPV4[] =
"L3. IPv4 Header:::::::::::::::::::::::::::::::::::::::::::::::::::::\n\
    0                   1                   2                   3\n\
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |Version|  IHL  |    DSCP   |ECN|          Total Length         |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |         Identification        |Flags|      Fragment Offset    |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |  Time to Live |    Protocol   |         Header Checksum       |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |                       Source Address                          |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |                    Destination Address                        |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |                    Options                    |    Padding    |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
  -4 --ipv4               IPv4 layer 3 indicator.\n\
     --src-ip=<addr>      [Override] IPv4 source address to spoof.\n\
     --dest-ip=<addr>     IPv4 destination address.\n\
     --ver=<4|0-15>       [Override] IP version to spoof.\n\
     --ihl=<5|0-15>       [Override] IPv4 header length in 32-bit\n\
                          increments; minimum \"should\" be 5 (i.e.,\n\
                          5x32 = 160 bits = 20 bytes) by standard.\n\
     --tos=<0-255>        Type of Service; obsolete by DSCP+ECN.\n\
     |                    ToS itself is divided into precedence,\n\
     |                    throughput, reliability, cost, and mbz:\n\
     | --prec=<...|0-7>     RFC 791 IP Precedence/priority\n\
     |                      (\"--help=prec\" for textual entries);\n\
     | --min-delay          Minimize delay;\n\
     | --max-tput           Maximize throughput;\n\
     | --max-rely           Maximize reliability;\n\
     | --min-cost           Minimize monetary cost (RFC 1349); and\n\
     | --mbz-one            Set the MBZ (\"Must-be-Zero\") bit to 1.\n\
     --dscp=<...|0-64>    Differentiated Services Code Point\n\
                          (\"--help=dscp\" for textual entries.)\n\
     --ecn=<...|0-3>      Explicit Congestion Notification\n\
                          (\"--help=ecn\" for textual entries.)\n\
     --len=<0-65535>      [Override] total packet (+data) length.\n\
     --ident=<0-65535>    Packet identification (in fragmentation).\n\
     --flags=<0-7>        Bitfield for IPv4 flags:\n\
     | --evil-bit           RFC 3514 Evil/Reserved bit;\n\
     | --dont-frag          Don't Fragment (DF); and\n\
     | --more-frag          More Fragments (MF).\n\
     --frag-ofs=<0-8191>  Fragment Offset\n\
     --ttl=<0-255>        Time-to-live (hop-limit) for the packet.\n\
     --proto=<...|0-255>  [Override] protocol number (e.g., tcp, 6)\n\
                          (\"--help=proto\" for textual entries.)\n\
     --chksum=<0-65535>   [Override] IPv4 header checksum.\n\
     --options=<>         [[UNFINISHED]]\n\
";

static const char HELP_TEXT_IPV6[] = "\
L3. IPv6 Header:::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\
  -6 --ipv6               IPv6 layer 3 indicator.\n\
     --src-ip=<addr6>     [Override] IPv6 source address to spoof.\n\
     --dest-ip=<addr6>    IPv6 destination address.\n\
     --next-header=       Similar to IPv4's \"proto\" field.\n\
       <...|0-255>        (\"--help=next-header\" for text entries.)\n\
     --hop-limit=<0-255>  Similar to IPv4's \"ttl\" field.\n\
     --flow-label=        Flow Label (experimental; RFC 2460).\n\
       <0-1048575>\n\
  [[UNIMPLEMENTED]]\n\
";

// TODO: Make the bitfield also take one-letter flags
static const char HELP_TEXT_TCP[] = "\
::::::::::::::::::::::::::::L4.  TCP Header:::::::::::::::::::::::::::::\n\
   0                   1                   2                   3\n\
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |          Source Port          |       Destination Port        |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |                        Sequence Number                        |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |                    Acknowledgment Number                      |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |  Data |       |C|E|U|A|P|R|S|F|                               |\n\
   | Offset| Rsrvd |W|C|R|C|S|S|Y|I|            Window             |\n\
   |       |       |R|E|G|K|H|T|N|N|                               |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |           Checksum            |         Urgent Pointer        |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |                           [Options]                           |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
   |                                                               :\n\
   :                             Data                              :\n\
   :                                                               |\n\
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\
  -T --tcp                TCP layer 4 indicator.\n\
     --src-port=          [Override] Source port.\n\
       <0-65535>\n\
     --dest-port=         Destination port.\n\
       <0-65535>\n\
     --seq-num=           Sequence number.\n\
       <0-4294967295>\n\
     --ack-num=           Acknowledgement number.\n\
       <0-4294967295>\n\
     --data-ofs=<0-15>    Data Offset (in 32-bit words).\n\
     --reserved=<0-15>    TCP Reserved bits (unused).\n\
     --flags=<0-255>      Bitfield for TCP flags:\n\
     | --cwr                Congestion Window Reduced (RFC 3168);\n\
     | --ece                ECN-Echo (RFC 3168);\n\
     | --urg                Urgent;\n\
     | --ack                Acknowledgment;\n\
     | --psh                Push;\n\
     | --rst                Reset;\n\
     | --syn                Synchronize; and\n\
     | --fin                Finish.\n\
     --window=<0-65535>   Window Size\n\
     --urg-ptr=<0-65535>  Urgent Pointer\n\
     --options=<>         [[UNFINISHED]]\n\
";

static const char HELP_TEXT_UDP[] = "\
L4. UDP Header::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\
  -U --udp                UDP layer 4 indicator.\n\
  [[UNIMPLEMENTED]]\n\
";

static const char HELP_TEXT_ICMP[] = "\
L4. ICMP Header:::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\
  -I --icmp               ICMP layer 4 indicator.\n\
  [[UNIMPLEMENTED]]\n\
";

// NOTE: These were "divided into sections because C99+ compilers
// might not support string literals exceeding 4095 characters.
//
// NOTE: The help texts alone take up a few kilobytes of space; you
// could change this to an empty string to save on that, if need be.
#define HELP_TEXT "%s%s%s%s%s%s", \
    HELP_TEXT_OVERVIEW, HELP_TEXT_IPV4, HELP_TEXT_IPV6, \
    HELP_TEXT_TCP, HELP_TEXT_UDP, HELP_TEXT_ICMP


static const char HELP_PAGE_PROTO[] = "\
IPv4/IPv6 Protocols:\n\
\n\
  icmp   1    Internet Control Message Protocol\n\
  tcp    6    Transmission Control Protocol\n\
  udp    17   User Datagram Protocol\n\
";

static const char HELP_PAGE_PREC[] = "\
IPv4 Precedence Codes:\n\
\n\
  RFC 791\n\
    routine      0 Routine\n\
    priority     1 Priority\n\
    immediate    2 Immediate\n\
    flash        3 Flash\n\
    override     4 Flash-Override\n\
    critical     5 Critic/Critical\n\
    internetwork 6 Internetwork Control\n\
    network      7 Network Control\n\
";

static const char HELP_PAGE_DSCP[] = "\
IPv4 Differentiated Services Code Points:\n\
\n\
  DSCP Pool 1 Codepoints\n\
    RFC 2474 (Class Selector PHBs)\n\
      df    0  Default Forwarding (DF) PHB\n\
      cs0   0  CS0 (standard)\n\
      cs1   8  CS1 (low-priority data)\n\
      cs2   16 CS2 (network operations/OAM)\n\
      cs3   24 CS3 (broadcast video)\n\
      cs4   32 CS4 (real-time interactive)\n\
      cs5   40 CS5 (signaling)\n\
      cs6   48 CS6 (network control)\n\
      cs7   56 CS7 (reserved)\n\
    RFC 2597 (Assured Forwarding [AF] PHB)\n\
      af11  10 AF11 (high-throughput)\n\
      af12  12 AF12 (high-throughput)\n\
      af13  14 AF13 (high-throughput)\n\
      af21  18 AF21 (low-latency)\n\
      af22  20 AF22 (low-latency)\n\
      af23  22 AF23 (low-latency)\n\
      af31  26 AF31 (multimedia stream)\n\
      af32  28 AF32 (multimedia stream)\n\
      af33  30 AF33 (multimedia stream)\n\
      af41  34 AF41 (multimedia conference)\n\
      af42  36 AF42 (multimedia conference)\n\
      af43  38 AF43 (multimedia conference)\n\
    RFC 3246 (Expedited Forwarding [EF] PHB)\n\
      ef    46 EF (telephony)\n\
    RFC 5865 (Voice-Admit)\n\
      va    44 Voice-Admit\n\
  DSCP Pool 2 Codepoints\n\
    <none exist yet>\n\
  DSCP Pool 3 Codepoints\n\
    RFC 8622\n\
      le    1  Lower-Effort PHB\n\
";

static const char HELP_PAGE_ECN[] = "\
IPv4 Explicit Congestion Notifications:\n\
\n\
  RFC 3168\n\
    not  0 Not ECN-Capable Transport\n\
  RFC 8311 / RFC Errata 5399 / RFC 9331\n\
    ect1 1 ECN-Capable Transport(1)\n\
  RFC 3168\n\
    ect0 2 ECN-Capable Transport(0)\n\
    ce   3 Congestion Experienced\n\
";


static const char ABOUT_TEXT[] = "\
Blitzping  Copyright (C) 2024  Fereydoun Memarzanjany\n\
This program comes with ABSOLUTELY NO WARRANTY.\n\
\n\
This is free software, and you are welcome to redistribute it\n\
under certain conditions; see the GNU General Public License v3.0.\n\
If you wish to report a bug or contribute to this project,\n\
visit this repository: https://github.com/Thraetaona/Blitzping\n\
";

static const char VERSION_TEXT[] = "Blitzping v1.7.0";

#endif // DOCS_H

// ---------------------------------------------------------------------
// END OF FILE: docs.h
// ---------------------------------------------------------------------

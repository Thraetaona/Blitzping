<div align="center">

<p align="center">
  <img src="docs/blitzping_logo.png?raw=true" alt="Blitzping's logo" width="500">
</p>

  <h1><code>Blitzping</code></h1>

  <p>
    <strong>A very high-speed, configurable, and portable packet-crafting utility optimized for embedded devices</strong>
  </p>

  
</div>

***

# Background

I found [hping3](https://linux.die.net/man/8/hping3) and nmap's [nping](https://linux.die.net/man/1/nping) to be far too slow in terms of sending individual, bare-minimum (40-byte) TCP packets; other than inefficient socket I/O, they were also attempting to do far too much unnecessary processing in what should have otherwise been a tight execution loop.  Furthermore, none of them were able to handle [CIDR notations](https://en.wikipedia.org/wiki/Classless_Inter-Domain_Routing) (i.e., a range of IP addresses) as their source IP parameter.  Being intended for embedded devices (e.g., low-power MIPS/Arm-based routers), Blitzping only depends on standard POSIX headers and C11's libc (whether musl or gnu).  To that end, even when supporting CIDR prefixes, Blitzping is significantly faster compared to hping3, nping, and whatever else that was hosted on GitHub (benchmarks below).

Here are some of the performance optimizations specifically done on Blitzping:
* **Pre-Generation:** All the static parts of the packet buffer get generated once, outside of the `sendto()` tightloop;
* **Asynchronous:** Configuring raw sockets to be non-blocking by default;
* **Socket Binding:** Using `connect()` to bind a raw socket to its destination only once, replacing `sendto()`/`sendmmsg()` with `write()`/`writev()`;
* **Queueing:** Queues many packets to get processed *inside* the kernelspace (i.e., `writev()`), rather repeating userspace->kernelspace syscalls;
* **Memory:** Locking memory pages and allocating the packet buffer in an *aligned* manner;
* **Multithreading:** Polling the same socket in `sendto()` from multiple threads; and
* **Compiler Flags:** Compiling with `-Ofast`, `-flto`, and `-march=native` (these actually had little effect; by this point, the entire bottleneck lays on the Kernel's own `sendto()` routine).

## License

Blitzping's source code is licensed under the GNU General Public License v3.0 or later ("GPLv3+"), Copyright (C) 2024 Fereydoun Memarzanjany.  Blitzping comes with ABSOLUTELY NO WARRANTY.  Blitzping is free software, and you are welcome to redistribute it under certain conditions; see the GPLv3+.  If you wish to report a bug or contribute to this project, visit this repository: https://github.com/Thraetaona/Blitzping

Usage: 
`blitzping <num. threads> <source IP/CIDR> <dest. IP:Port>` \
Example: `./blitzping 4 192.168.123.123/19 10.10.10.10:80` (this would send TCP SYN packets to `10.10.10.10`'s port `80` from a randomly chosen source IP within an entire range of `192.168.96.0` to `192.168.127.255`, using `4` threads.)

## Benchmarks

I tested Blitzping against both hpign3 and nping on two different routers, both running OpenWRT 23.05.03 (Linux Kernel v5.15.150) with the "masquerading" option (i.e., NAT) turned off in firewall; one device was a single-core 32-bit MIPS SoC, and another was a 64-bit quad-core ARMv8 CPU.  On the quad-core CPU, because both hping3 and nping were designed without multithreading capabilities (unlike Blitzping), I made the competition "fairer" by launching  them as four individual processes, as opposed to Blitzping only using one.  Across all runs and on both devices, CPU usage remained at 100%, entirely dedicated to the currently running program.  Finally, the network interface cards ("NICs") themselves were not bottlenecks: the MIPS SoC had a 100 Mbps (~95.3674 MiB/s) NIC and the ARMv8 a 1000 Mbps (~953.674 MiB/s).

It is important to note that Blitzping was not doing any less than hping3 and nping; in fact, *it was doing more.*  While hping3 and nping only randomized the source IP and port of each packet to a fixed address, Blitzping randomized not only the source port but also the IP *within an CIDR range*---a capability that is more computionally intensive and a feature that both hping3 and nping lacked in the first place.

All of the sent packets were bare-minimum TCP SYN packets; each of them was only 40 bytes in length.  Lastly, hping3 and nping were both launched with the "best-case" command-line parameters as to maximize their speed and to disable their runtime stdio logging:

```
hping3 --flood --spoof 192.168.123.123 --syn 10.10.10.10
```
```
nping --count 0 --rate 1000000 --hide-sent --no-capture --privileged --send-eth --source-ip 192.168.123.123 --source-port random --dest-ip 10.10.10.10 --tcp --flags syn
```
```
./blitzping 4 192.168.123.123/19 10.10.10.10:80
```

### Quad-Core "Rockchip RK3328" CPU @ 1.3 GHz. (ARMv8-A)
| ARM (4 x 1.3 GHz) | nping | hping3 | Blitzping |
|:-|:-|:-|:-|
| Num. Instances | 4 (1 thread) | 4 (1 thread) | 1 (4 threads) |
| Pkts. per Second | ~65,000 | ~80,000 |~3,150,000 |
| Bandwidth (MiB/s) | ~2.50 | ~3.00 | ~120 |

### Single-Core "Qualcomm Atheros QCA9533" SoC @ 650 MHz. (MIPS32r2)
| MIPS (1 x 650 MHz) | nping | hping3 | Blitzping |
|:-|:-|:-|:-|
| Num. Instances | 1 (1 thread) | 1 (1 thread) | 1 (1 thread) |
| Pkts. per Second | ~5,000 | ~10,000 | ~420,000 |
| Bandwidth (MiB/s) | ~0.20 | ~0.40 | ~16 |

# Compilation


Blitzping uses C11 syntax (e.g., anonymous structs and list commas) but without any hard dependencies on actual C11 headers, so **it can compile under C99 just fine.[^1]**  Blitzping's usage of libc is mostly [freestanding](https://en.cppreference.com/w/cpp/freestanding), and it only uses standard headers, mainly Berkley sockets, from the 2001 edition of POSIX.1 (IEEE Standard 1003.1-2001), without any BSD-, XSI-, SysV-, or GNU-specific additions.  Blitzping can be compiled by both LLVM and GCC; because of LLVM being slightly [easier](https://clang.llvm.org/docs/CrossCompilation.html) in cross-compiling to other architectures with its target triplets, I configured the makefile to use that toolchain (i.e., clang, lld, and llvm-strip) by default.  Also, Blitzping's makefile has additional workarounds for [LLVM/Clang's bug with soft-core targets.](https://github.com/llvm/llvm-project/issues/102259)

[^1]: To compile under C99, supply `C_STD=c99` to the makefile; this will disable C11 threads but POSIX threads will continue to remain usable, independently.

### Install the LLVM toolchain (if you do not already have it):
(LLVM/Clang, LLVM Linker, and LLVM-strip)
```
apt install llvm clang
apt install lld
apt install llvm-binutils
```

## A) If you wish to only compile for your own machine (i.e., host and target are the same), you can run `make` without any additional options:

```
make
```

The compiler will then create an executable for your target device in the `./out` directory. 

As a final and optional post-processing step, you could strip the debuginfo symbols out of the compiled program and reduce its size:
```
make strip
```

## B) Cross-Compilation (sample for a Debian 12 host and MIPS32r2 target)

#### 1. Install your host's compiler runtime (`compiler-rt` *OR* `libgcc`) for the target machine's architecture:

```
apt install libclang-rt-dev:mips
```
*OR*
```
apt install libgcc1-mips-cross
```

While packages of common architectures, such as x86_64 and arm64, are widely supported on desktop-based Linux distros, Debian (for example) does not provide packages for older embedded targets like 32-bit MIPS\[eb\].  In those cases, if you are not able to manually acquire LLVM's `compile-rt:mips` for that architecture, you could always `apt install libgcc1-mips-cross` for libgcc.

### 2. Then, simply specify your ["target triplet"](https://wiki.osdev.org/Target_Triplet) in make; for example, a soft-float big-endian MIPS running Linux (OpenWRT) with musl libc would be as follows:
```
make TARGET=mips-openwrt-linux-muslsf
```
**Make sure that you specify the correct libc (e.g., `musl`, `gnu`, `uclibc`) _and_ whether or not it lacks an FPU (i.e., if it is soft-float and requires an `sf` suffix to libc).**

Optionally, you can specify the target's sub-architecture to optimize specifically for it:
```
make TARGET=mips-linux-muslsf SUBARCH=mips32r2
```

(As mentioned earlier, you could also `apt install gcc-mips-linux-gnu` and skip LLVM/Clang altogether, if you really want to.)

NOTE: If your router uses LibreCMC, be aware that the system's libc might be too old to run C programs like this; to fix that, you could either take the risk and unflag that specific package via `opkg` in order to upgrade it, or you could flash the more modern OpenWRT onto your router.
NOTE: Sometimes, your target might have a different name for its libgcc, such as `libgcc-12-dev-m68k-cross` (Debian example).

# FAQs

### Why rewrite hping3 and not just fork it?

At first, I was planning to fork hping3 as to maintain it, but **its code just had too many questionable design choices;** there were [global variables](https://github.com/antirez/hping/blob/master/globals.h) and unnecessary function calls all over the place.  In any case, the performance was abysmal because the entirety of the packet&mdash;even parts that were not supposed to change&mdash;was getting re-crafted each time.  Do you want to flood the destination with only TCP packets?  The code will continiously go through function within function and nested if-else branches to ["decide" at runtime](https://github.com/antirez/hping/blob/master/main.c#L375) whether it should be using UDP, TCP, etc. routines for this otherwise-TCP-only send; this is not how you write performant code!  Among other things, **the code was [rather unportable](https://github.com/antirez/hping/blob/master/Makefile.in);** you have to [manually patch](https://bugs.gentoo.org/706566) many areas just to get it to compile on modern compilers, due to its noncompliance with the C standard and its dependency on libpcap.

Also, both nping and hping3 use non-standard BSD/SystemV-specific extensions to the POSIX "standard" library; for some strange reason or due to mere oversight, the actual POSIX standard defines a barebones `<netinet/tcp.h>` without actually defining a `tcphdr` *per se,* making it entirely pointless to use in the first place, and `<netinet/ip.h>` does not even exist as part of the POSIX standard.  In any case, *those headers are not only nonstandard but are also very ancient;* they lag behind newer RFCs by decades.  This warranted writing my own `"netinet.h"`, which I think is (by far) the most feature-complete and cleanest TCP/IP stack implementation in C.  Also, despite supporting new fields, I still made sure to remain backward-compatible with old RFCs (for example, in IPv4, I support both the newer DSCP/ECN codepoint pools *and* the older ToS/Precedence values).

### Why not rewrite it in Rust (or C++)?

Blitzping is intended to support low-power routers and embedded devices; **Rust is a terrible choice for those places.**  Had Blitzping been a low-level system firmware *or* a high-level application, both `#![no_std]` and `std` Rust would have been suitable choices, respectively; however, Blitzping exists in a "middle" position, where you are still *required* to use `libc` or other syscalls and yet cannot expect to have the full Rust `std` library on your target.  For one, you would be interacting with low-level syscalls (e.g., Linux's `sendto()` or `sendmmsg()`); because *there are currently no raw socket wrappers for Rust,* you would be forced to use `unsafe {...}` at every single turn there.  Also, Rust's std library is not available on some of said embedded targets, such as [`mips64-openwrt-linux-musl`](https://doc.rust-lang.org/nightly/rustc/platform-support/mips64-openwrt-linux-musl.html); what will you do in such cases?  **You would have to use libc anyway,** forcing you to painstakingly wrap every single C function around `unsafe {...}` and deal with its lack of interoperability; this literally defeats the entire goal of writing your program in Rust in the first place.  **If anything, Rust's safety features would only tie your own hands.**

People don't know how to write safe C code; that is their own fault&mdash;not C's.  C might not be object-oriented like C++ and it might not have the number of compile-time constraining that Ada/Pascal offer, but it at least lets you do what you want to do with relative ease; you just have to know what you want to do with it.  Also, there are plenty of very nice but often overlooked features (e.g., unions, bool, _Static_assert(), threads.h, etc.) in the C99/C11 standards that many people just appear oblivious about.  Finally, compiling your code with `-Wall -Wextra -Werror -pedantic-errors` helps you avoid GNU-specific extensions that might have been making your code less portable or more suspectible to deprecation in future compiler releases; again, people just don't make use of it in the first place.

C++ remains largely backward-compatible with C and actually has some really useful features (such as enum underlying types or namespaces) that are either only available in C23 (I use C11) or have not made their way into C yet.  While I appreciate the syntax and wish I could type something like `TCP::FLAGS::SYN`, as opposed to `TCP_FLAGS_SYN` in C, I also cannot ignore the fact that these gains would be minoscule in practice; libc is still far more portable than libc++/libstdc++.

### Why was CIDR support important?

The reason I specifically included CIDR support&mdash;something that both nping and hping3 lacked&mdash;for spoofing source IP addresses was that ISPs or mid-route routers can block "nonsense" traffic (e.g., a source IP outside of the ISP or even country's allocated range) coming from you as a client; this is called "egress filtering." However, in a DHCP-assigned scenario, you still have thousands of IP addresses in your subnet/range to spoof, which should not get caught by this filtering.  Ultimately, because the source IP would be coming from an entire range of addresses, firewalls (and CDNs) will have a harder time detecting a pattern as to block it.  Unfortunately, in a residential ISP scenario where you only have one "real" IP address, your spoofed source IP would be chosen from a range of otherwise-legitimate addresses; those addresses (upon unexpectedly receiving SYN-ACK from your targeted server) would usually respond back to the original server with a RST packet as to terminate the connection, making it so that your "half-open" connections (the goal of SYN flooding) do not get to last as long.

Lastly, this tool only really lets you flood packets as quickly as possible.  Those underpowered processors were only used as an optimization benchmark; if it runs good enough there, you could always throw more computational power and cores at it.  In fact, if you have (or rent) multiple datacenter-grade bandwidth, powerful x86_64 CPUs, and lots of "real" IP addresses at your disposal, then there would really be nothing preventing you from DDoS'ing the destination address; at the very least, you could still saturate their download line's bandwidth.

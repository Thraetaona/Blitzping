<div align="center">

  <h1><code>Blitzping</code></h1>

  <p>
    <strong>Sending IP packets as fast as possible in userland.</strong>
  </p>

  
</div>

***

# Background

I found [hping3](https://linux.die.net/man/8/hping3) and nmap's [nping](https://linux.die.net/man/1/nping) to be far too slow in terms of sending individual, bare-minimum (40-byte) TCP packets; other than inefficient socket I/O, they were also attempting to do far too much uunnecessary processing in what should have otherwise been a tight execution loop.  Furthermore, none of them were able to handle [CIDR notations](https://en.wikipedia.org/wiki/Classless_Inter-Domain_Routing) (i.e., a range of IP addresses) as their source IP parameter.  Being intended for embedded devices (e.g., low-power MIPS/Arm-based routers), Blitzping only depends on standard POSIX headers and C11's libc (whether musl or gnu).  To that end, even when supporting CIDR prefixes, Blitzping is significantly faster compared to hping3, nping, and whatever else that was hosted on GitHub (benchmarks below).

Here are some of the performance optimizations specifically done on Blitzping:
* **Pre-Generation:** All the static parts of the packet buffer get generated once, outside of the `sendto()` tightloop;
* **Asynchronous:** Configuring raw sockets to be non-blocking by default;
* **Multithreading:** Polling the same socket in `sendto()` from multiple threads; and
* **Compiler Flags:** Compiling with `-Ofast`, `-flto`, and `-march=native` (these actually had little effect; by this point, the entire bottleneck lays on the Kernel's own `sendto()` routine).

Usage: 
`blitzping <source IP/CIDR> <source port (ignored)> <destination IP> <destination port>` \
Example: `./decimator 192.168.123.123/19 1234 10.10.10.10 80` (this would send TCP SYN packets to `10.10.10.10`'s port `80` from a randomly chosen source IP within an entire range of `192.168.96.0` to `192.168.127.255`; source ports are randomly chosen, for now.)

## Benchmarks

I tested Blitzping against both hpign3 and nping on two different routers, both running OpenWRT 23.05.03 (Linux Kernel v5.15.150) with the "masquerading" option (i.e., NAT) turned off in firewall; one device was a single-core 32-bit MIPS SoC, and another was a 64-bit quad-core ARMv8 CPU.  On the quad-core CPU, because both hping3 and nping were designed without multithreading capabilities (unlike Blitzping), I made the competition "fairer" by launching  them as four individual processes, as opposed to Blitzping only using one.  Across all runs and on both devices, CPU usage remained at 100%, entirely dedicated to the currently running program.  Finally, the connection speed itself was not a bottleneck: both devices were connected to an otherwise-unused 200 Mb/s (23.8419 MiB/s) download/upload line through a WAN ethernet interface.

It is important to note that Blitzping was not doing any less than hping3 and nping; in fact, *it was doing more.*  While hping3 and nping only randomized the source IP and port of each packet to a fixed address, Blitzping randomized not only the source port but also the IP *within an CIDR range*---a capability that is more computionally intensive and a feature that both hping3 and nping lacked in the first place.

All of the sent packets were bare-minimum TCP SYN packets; each of them was only 40 bytes in length.  Lastly, hping3 and nping were both launched with the "best-case" command-line parameters as to maximize their speed and to disable their runtime stdio logging:

```
hping3 --flood --spoof 192.168.123.123 --syn 10.10.10.10
```
```
nping --count 0 --rate 1000000 --hide-sent --no-capture --privileged --send-eth --source-ip 192.168.123.123 --source-port random --dest-ip 10.10.10.10 --tcp --flags syn
```
```
./decimator 192.168.123.123/19 1234 10.10.10.10 80
```


### Quad-Core "Rockchip RK3328" CPU @ 1.3 GHz. (ARMv8-A)
| ARM (4 x 1.3 GHz) | nping | hping3 | Blitzping |
|:-|:-|:-|:-|
| Num. Instances | 4 (1 thread) | 4 (1 thread) | 1 (4 threads) |
| Pkts. per Second | ~65,000 | ~80,000 | ~275,000 |
| Bandwidth (MiB/s) | ~2.50 | ~3.00 | ~10.50 |

### Single-Core "Qualcomm Atheros QCA9533" SoC @ 650 MHz. (MIPS32r2)
| MIPS (1 x 650 MHz) | nping | hping3 | Blitzping |
|:-|:-|:-|:-|
| Num. Instances | 1 (1 thread) | 1 (1 thread) | 1 (1 thread) |
| Pkts. per Second | ~5,000 | ~10,000 | ~25,000 |
| Bandwidth (MiB/s) | ~0.20 | ~0.40 | ~1.00 |

# Compilation

In [`./Makefile`](https://github.com/Thraetaona/Blitzping/blob/main/Makefile), you can set your target device's "target triplet" (e.g., `aarch64-openwrt-linux-musl`, `mipseb-openwrt-linux-musl`, `x86_64-unknown-linux-gnu`, etc.) and an optional architecture (e.g., `-march=armv8-a`); `make` will then create an executable for that specific device in the `./out` directory.

Blitzping only uses the standard C11 libraries (`-std=c11`) and some POSIX networking headers (`-D_DEFAULT_SOURCE`), without any GNU-specific extensions; it can be compiled by both LLVM and GCC with no performance penalty.  However, because of LLVM being much more straightforward in crosscompiling to other architectures with its ["target triplets,"](https://clang.llvm.org/docs/CrossCompilation.html) I configured the makefile to use that toolchain (i.e., clang, lld, and llvm-strip) by default.

For example, in case of ARMv8 (aarch64), you will need the following Debian packages:
* **LLVM and compiler toolchain:** `apt install llvm clang`
* **LLVM linker:** `apt install lld`
* **LLVM-strip (optional for `make strip`):** `apt install llvm-binutils`
* **LLVM libc and libunwind runtimes:** `apt install libclang-rt-dev:arm64 libunwind-dev:arm64`

While aarch64's packages are widely supported on desktop-based Linux distros, Debian (for example) does not provide packages for older embedded targets like 32-bit MIPSeb.  In those cases, if you are not able to manually acquire and compile LLVM's `compile-rt:mips` and `libunwind:mips` to that architecture, you can `apt install libgcc1-mips-cross` and replace `--rtlib=compiler-rt --unwindlib=libunwind` in the makefile with `--rtlib=libgcc --unwindlib=libgcc -static`; this statically links it against the final executable.  You could also `apt install gcc-mips-linux-gnu` and skip LLVM altogether.

The makefile is configured with `-Wall -Wextra -Werror -pedantic-errors` by default; it should compile with no warnings.

NOTE: If your router uses LibreCMC, be aware that the system's libc might be too old to run C programs like this; to fix that, you could either take the risk and unflag that specific package via `opkg` in order to upgrade it, or you could flash the more modern OpenWRT onto your router.

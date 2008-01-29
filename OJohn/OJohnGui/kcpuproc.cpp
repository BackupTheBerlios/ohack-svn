
/***************************************************************************
 *                                                                         *
 *   KCPULoad is copyright (c) 1999-2000, Markus Gustavsson                *
 *                         (c) 2002, Ben Burton                            *
 *                         (c) 2005, Johannes Sixt                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kcpuproc.h"

#include <cstdlib>
#include <cstring>

// BSD-specific includes.
#ifdef Q_OS_BSD4
#include <sys/param.h>
#ifdef Q_OS_FREEBSD
#if __FreeBSD_version < 500101
#include <sys/dkstat.h>
#else
#include <sys/resource.h>
#endif
#else
#include <sys/dkstat.h>
#endif
#include <sys/sysctl.h>
#include <string.h>
#include <kvm.h>
#ifdef Q_OS_NETBSD
#include <sys/sched.h>
#endif
#endif

/**
 * Linux
 * -----
 *
 * System information is read from /proc/stat.
 *
 * We assume /proc/stat is in one of the following formats:
 *
 *   cpu %d %d %d %d
 *   ....
 *
 *   cpu %d %d %d %d
 *   cpu0 %d %d %d %d
 *   ....
 *
 *   cpu %d %d %d %d
 *   cpu0 %d %d %d %d
 *   cpu1 %d %d %d %d
 *   ....
 *
 * where each set of four numbers is of the form:
 *
 *   user_ticks nice_ticks system_ticks idle_ticks
 *
 * We also allow sets of seven numbers instead of four (as provided by
 * the 2.6 kernel); the last three of these numbers are ignored.
 *
 * BSD
 * ---
 *
 * Currently only uni-processor mode is supported for BSD.
 *
 * BSD code by Andy Fawcett <andy@athame.co.uk> and
 * Robbie Ward <linuxphreak@gmx.co.uk>, licensed under GPL2.
 *
 * Other Operating Systems
 * -----------------------
 *
 * Please, send in a patch!
 *
 * The KCPULoad maintainer is currently Ben Burton <bab@debian.org>, or
 * you could submit a patch through the KDE bug tracking system
 * (bugs.kde.org).
 *
 * Support for more than two CPUs was implemented by
 * Johannes Sixt <johannes.sixt@telecom.at>.
 */

KCPUProc::CPU::Ticks::Ticks() :
    U(0), S(0), N(0), I(0)
{ }

KCPUProc::KCPUProc()
{
    smp = false;

    // Look for SMP support and take a current tick reading.

    // ========== BSD-specific (begin) ==========
#ifdef Q_OS_BSD4
    readLoad();
    return;
#endif
    // ========== BSD-specific (end) ==========

    // ========== Linux-specific (begin) ==========
#ifdef Q_OS_LINUX
    if ((fd = fopen("/proc/stat", "r")) == 0)
        return;

    if (!all.parse(fd))
        return;
    CPU c;
    while (c.parse(fd))
        cpu.push_back(c);
    smp = cpu.size() > 1;
#endif
    // ========== Linux-specific (end) ==========
}

void KCPUProc::readLoad() {
    // OS-specific local variables.

    // ========== Linux-specific (begin) ==========
#ifdef Q_OS_LINUX
    // Prepare to take the readings.

    if ((fd = fopen("/proc/stat", "r")) == 0)
        return;

    // Take a fresh set of current readings (SMP mode).

    all.parse(fd);
    for (int i = 0; i < cpu.size(); i++) {
        if (!cpu[i].parse(fd))
            break;
    }

    // Clean up after taking readings.

    fclose(fd);
#endif
    // ========== Linux-specific (end) ==========

    // ========== BSD-specific (begin) ==========
#ifdef Q_OS_BSD4
    static int name2oid[2] = { 0, 3 };
    static int oidCpuTime[CTL_MAXNAME + 2];
    static size_t oidCpuTimeLen = sizeof(oidCpuTime);
    long cpuTime[CPUSTATES];
    size_t cpuTimeLen = sizeof(cpuTime);
    static char *name = "kern.cp_time";
    static int initialized = 0;

    if(smp) {
        // Take a fresh set of current readings (SMP mode).
        // TODO: Add SMP support for BSD.
    } else {
        // Take a fresh set of current readings (uni-processor mode).

        // The current readings must now become the previous readings.
        all.p = all.c;

        if (! initialized) {
            if (sysctl(name2oid, 2, oidCpuTime, &oidCpuTimeLen, name,
                    strlen(name)) < 0)
                return;

            oidCpuTimeLen /= sizeof(int);
            initialized = 1;
        }

        if (sysctl(oidCpuTime, oidCpuTimeLen, cpuTime, &cpuTimeLen, 0, 0) < 0)
            return;

        all.c.U = cpuTime[CP_USER];
        all.c.N = cpuTime[CP_NICE];
        all.c.S = cpuTime[CP_SYS];
        all.c.I = cpuTime[CP_IDLE];
    }
#endif
    // ========== BSD-specific (end) ==========
}

// ========== Linux-specific (begin) ==========
#ifdef Q_OS_LINUX
bool KCPUProc::CPU::parse(FILE* fd) {
    char tagbuffer[32];
    Ticks n;	// new ticks
    if (fscanf(fd, "%32s%d%d%d%d", tagbuffer, &n.U, &n.N, &n.S, &n.I) != 5) {
        return false;	// failure
    }

    if (strncmp(tagbuffer, "cpu", 3) != 0) {
        return false;	// tag mismatch
    }

    // shift readings
    p = c;
    c = n;

    // ignore the rest of the line
    int ch;
    do {
        ch = getc(fd);
    } while (ch != '\n' && ch != EOF);
    return true;
}
#endif
// ========== Linux-specific (end) ==========

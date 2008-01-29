
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

#ifndef __KCPUPROC_H
#define __KCPUPROC_H

#include <cstdio>
#include <qglobal.h>
#include <vector>
/**
 * A class used to read raw CPU load details from the system.
 *
 * See kcpuproc.cpp for details of supported operating systems.
 */
class KCPUProc
{
public:
    /**
     * Constructor.
     *
     * In the constructor, a set of initial CPU tick readings are taken
     * and SMP support is determined.
     */
    KCPUProc();
    
    /**
     * Does this system appear to have SMP?
     */
    bool hasSMP() const;

    /**
     * Takes a fresh set of CPU tick readings.  The numerical statistics
     * returned refer to the time period between this reading and the
     * previous (or between this reading and the object's construction
     * if there was no previous reading).
     */
    void readLoad();

    /**
     * Contains tick differences for a CPU.
     */
    struct CPU {
        /**
         * Contains user/system/nice/idle tick readings.
         */
        struct Ticks {
            int U, S, N, I;
            Ticks();
            /** Returns user ticks */
            int u() const { return U; }
            /** Returns system ticks */
            int s() const { return S + N; }
            /** Returns total (non-idle) ticks */
            int t() const { return U + S + N; }
            /** Returns elapsed ticks */
            int e() const { return U + S + N + I; }
        };

        Ticks p;        /**< The previous tick readings */
        Ticks c;        /**< The last (most recent) tick readings */

        /**
         * The percentage of ticks between the last reading and the previous
         * reading used by the user loop (compared to the nice, system and
         * idle loops).
         *
         * This routine involves a short arithmetical calculation.
         * If you're paranoid about running time, you might want to cache
         * the result.
         */
        int userPercent() const;

        /**
         * The percentage of ticks between the last reading and the previous
         * reading used by the system and nice loops (compared to the user and
         * idle loops).
         *
         * This routine involves a short arithmetical calculation.
         * If you're paranoid about running time, you might want to cache
         * the result.
         */
        int systemPercent() const;

        /**
         * The percentage of ticks between the last reading and the previous
         * reading used by the user, system and nice loops (compared to the
         * idle loop).
         *
         * This routine involves a short arithmetical calculation.
         * If you're paranoid about running time, you might want to cache
         * the result.
         */
        int totalPercent() const;

        /**
         * OS-specific helper routines.
         */

        // ========== Linux-specific (begin) ==========
#ifdef Q_OS_LINUX
        /**
         * Parses the tick readings of the current line in #fd.
         * The return value is \c false if the line does not begin with
         * the tag "cpu" (optionally followed by a number).
         * If the line can be parsed successfully, the last reading becomes
         * the previous reading and the newly parsed reading becomes
         * the last reading.
         */
        bool parse(FILE* fd);
#endif
        // ========== Linux-specific (end) ==========
    };

    /*
     * Variables used in all modes.
     */
    CPU all;    /**< Tick readings for all CPUs. */

    /*
     * Variables used only with SMP.
     */
    std::vector<CPU> cpu;   /**< Tick readings for CPUs. */

private:
    /*
     * SMP support.
     */
    bool smp;
        /**< Does this system appear to have SMP? */

    /**
     * OS-specific data members.
     */

    // ========== Linux-specific (begin) ==========
#ifdef Q_OS_LINUX
    FILE *fd;
        /**< The file /proc/stat. */
#endif
    // ========== Linux-specific (end) ==========
};

inline bool KCPUProc::hasSMP() const {
    return smp;
}

inline int KCPUProc::CPU::userPercent() const {
    int tot = c.e() - p.e();
    return (tot > 0 ? (100 * (c.u() - p.u())) / tot : 0);
}
inline int KCPUProc::CPU::systemPercent() const {
    int tot = c.e() - p.e();
    return (tot > 0 ? (100 * (c.s() - p.s())) / tot : 0);
}
inline int KCPUProc::CPU::totalPercent() const {
    int tot = c.e() - p.e();
    return (tot > 0 ? (100 * (c.t() - p.t())) / tot : 0);
}

#endif

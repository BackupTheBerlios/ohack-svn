/*
 * Medusa Parallel Login Auditor
 *
 *    Copyright (C) 2006 Joe Mondloch
 *    JoMo-Kun / jmk@foofus.net
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License version 2,
 *    as published by the Free Software Foundation
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    http://www.gnu.org/licenses/gpl.txt
 *
 *    This program is released under the GPL with the additional exemption
 *    that compiling, linking, and/or using OpenSSL is allowed.
 *
*/

#ifndef _MEDUSA_H
#define _MEDUSA_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "medusa-trace.h"
#include "medusa-net.h"

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#define PROGRAM   "Medusa"
#ifndef VERSION
  #define VERSION   "1.0"
#endif
#define AUTHOR    "JoMo-Kun / Foofus Networks"
#define EMAIL     "<jmk@foofus.net>"
#define WWW       "http://www.foofus.net"

#define SUCCESS 0
#define FAILURE -1

#define FALSE 0
#define TRUE 1

/* GLOBAL VARIABLES */
FILE *pOutputFile;
pthread_mutex_t ptmMutex;
int iVerboseLevel;      // Global control over general message verbosity
int iErrorLevel;        // Global control over error debugging verbosity


//#define MAX_BUF (16 * 1024)
#define MAX_BUF 16384 
#define NUM_THREADS 6

#define L_UNSET 0
#define L_SINGLE 1
#define L_FILE 2
#define L_COMBO 3
#define L_PWDUMP 4

typedef struct __sPass {
  struct __sPass *psPassNext;
  char *pPass;
} sPass;

/* Used in __sUser to defined progress of an individual username audit */
#define PL_NULL 1
#define PL_USERNAME 2
#define PL_LOCAL 3
#define PL_GLOBAL 4
#define PL_DONE 5

typedef struct __sUser {
  struct __sUser *psUserNext;
  char *pUser;
  struct __sPass *psPass;
  struct __sPass *psPassCurrent;
  struct __sPass *psPassPrevTmp;
  char *pPass;
  char *pPassIndex;
  char *pValidPass;
  int iPassCnt;
  int iLoginsDone;
  int iPassStatus;
  int iId;

  pthread_mutex_t ptmMutex;
} sUser;

/* Used in __sHost to defined progress of an individual host audit */
#define UL_NULL 1
#define UL_ACTIVE 2
#define UL_DONE 3
#define UL_SUCCESS 4

/* Used in __sHost to defined progress of host audit */
#define HL_NULL 1;
#define HL_ACTIVE 2;
#define HL_DONE 3;

typedef struct __sHost {
  struct __sHost *psHostNext;
  char *pHost;
  int iUseSSL;            // use SSL
  int iPortOverride;      // use this port instead of the module's default port
  int iTimeout;           // Number of seconds to wait before a connection times out
  int iRetryWait;         // Number of seconds to wait between retries
  int iRetries;           // Number of retries to attempt
  char *pUserIndex;
  sUser *psUser;
  sUser *psUserPrevTmp;
  int iUserCnt;
  int iUserPassCnt;
  int iUsersDone;        // number of users tested
  int iUserStatus;
  int iHostStatus;
} sHost;

#define SRV_IDLE 1
#define SRV_ACTIVE 2
#define SRV_DONE 3

#define SRV_LOGIN_IN_PROGRESS 4
#define SRV_LOGIN_DONE 5

typedef struct __sServer {
  struct __sAudit *psAudit;
  struct __sHost *psHost;

  char *pHostIP;
  
  char *pUserSuccess;
  char *pPassSuccess;

  //int iExitOnFound;
  int iValidPairFound;
  //int iStartTime;
  //int iEndTime;
  //int iAccountsTested;
  int iStatus;

  int iId;
  int iLoginsDone;       // number of logins performed by all threads under this server
  pthread_mutex_t ptmMutex;
} sServer;

#define LOGIN_IDLE 1
#define LOGIN_ACTIVE 2
#define LOGIN_DONE 3
#define LOGIN_FAILED 4
#define LOGIN_DISABLED 5

#define LOGIN_RESULT_UNKNOWN 1
#define LOGIN_RESULT_SUCCESS 2
#define LOGIN_RESULT_FAIL 3
#define LOGIN_RESULT_ERROR 4

typedef struct __sLogin {
  struct __sServer *psServer;
  struct __sUser *psUser;
  int iStatus;
  int iResult;
  char *pErrorMsg;
  int iId;
  int iLoginsDone;       // number of logins performed by this thread
} sLogin;


#define AUDIT_IN_PROGRESS 0 
#define AUDIT_COMPLETE 1
#define LIST_IN_PROGRESS 0 
#define LIST_COMPLETE 1

#define FOUND_PAIR_EXIT_HOST 1
#define FOUND_PAIR_EXIT_AUDIT 2

#define PARALLEL_LOGINS_USER 1
#define PARALLEL_LOGINS_PASSWORD 2

typedef struct __sAudit {
  char *pOptHost;         // user specified host or host file
  char *pOptUser;         // user specified username or username file
  char *pOptPass;         // user specified password or password file
  char *pOptCombo;        // user specified combo host/username/password file
  char *pOptOutput;       // user specified output file

  char *pModuleName;      // current module name

  char *pGlobalHost; 
  char *pGlobalUser;
  char *pGlobalPass;
  char *pGlobalCombo;
  char *pHostFile; 
  char *pUserFile;
  char *pPassFile;
  char *pComboFile;

  int iHostCnt;           // total number of hosts supplied for testing
  int iUserCnt;           // total number of users supplied for testing
  int iPassCnt;           // total number of passwords supplied for testing
  int iComboCnt;          // total number of entries in combo file
  int iServerCnt;         // total number of hosts scanned concurrently
  int iLoginCnt;          // total number of logins performed concurrently

  int iHostsDone;         // number of hosts tested

  int iPortOverride;      // use this port instead of the module's default port
  int iUseSSL;            // enable SSL
  int iTimeout;           // Number of seconds to wait before a connection times out
  int iRetryWait;         // Number of seconds to wait between retries
  int iRetries;           // Number of retries to attempt
  int HostType;
  int UserType;
  int PassType;
  int iShowModuleHelp;    // Flag used to show individual module help

  char *pComboEntryTmp;   // used to managed processing of user supplied files
  int iHostListFlag;
  int iUserListFlag;

  int iAuditFlag;             /* Tracks loading of user supplied information */
  
  int iPasswordBlankFlag;     /* Submit a blank password for each user account */
  int iPasswordUsernameFlag;  /* Submit a password matching the username for each user account */
  int iFoundPairExitFlag;     /* When a valid login pair is found, end scan of host or of complete audit */
  int iParallelLoginFlag;     /* Parallel logins by user or password */
  int iValidPairFound;
  
  sHost *psHostRoot;
} sAudit;

typedef struct __sModuleStart
{
  char*   szModuleName;
  sLogin* pLogin;
  int     argc;
  char**  argv;  
} sModuleStart;

char* getNextPass(sAudit *_psAudit, sUser *_psUser);

#define _MEDUSA_H
#endif

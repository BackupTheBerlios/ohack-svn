/*
**   VMware Authentication Daemon Password Checking Medusa Module
**
**   ------------------------------------------------------------------------
**    Copyright (C) 2006 Joe Mondloch
**    JoMo-Kun / jmk@foofus.net
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License version 2,
**    as published by the Free Software Foundation
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    http://www.gnu.org/licenses/gpl.txt
**
**    This program is released under the GPL with the additional exemption
**    that compiling, linking, and/or using OpenSSL is allowed.
**
**   ------------------------------------------------------------------------
**
**    Based on code from:
**      Hydra 5.2 [van Hauser <vh@thc.org>/ David Maciejak <david.maciejak@kyxar.fr>]
**
*/

#include <sys/types.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "module.h"

#define MODULE_NAME    "vmauthd.mod"
#define MODULE_AUTHOR  "JoMo-Kun <jmk@foofus.net>"
#define MODULE_SUMMARY_USAGE  "Brute force module for the VMware Authentication Daemon"
#define MODULE_VERSION    "1.0.0"
#define MODULE_VERSION_SVN "$Id: vmauthd.c 569 2006-07-31 21:01:06Z jmk $"
#define MODULE_SUMMARY_FORMAT  "%s : version %s"
#define MODULE_SUMMARY_FORMAT_WARN  "%s : version %s (%s)"

#define PORT_VMAUTHD 902

typedef struct __VMAUTHD_DATA {
  char *szDB;
} _VMAUTHD_DATA;


// Tells us whether we are to continue processing or not
enum MODULE_STATE
{
  MSTATE_NEW,
  MSTATE_RUNNING,
  MSTATE_EXITING
};

// Forward declarations
int initConnection(int hSocket, sConnectParams *params);
int tryLogin(int hSocket, sLogin** login, _VMAUTHD_DATA* _psSessionData, char* szLogin, char* szPassword);
int initModule(sLogin* login, _VMAUTHD_DATA *_psSessionData);

// Tell medusa how many parameters this module allows
int getParamNumber()
{
  return 0;    // we don't need no stinking parameters
}

// Displays information about the module and how it must be used
void summaryUsage(char **ppszSummary)
{
  // Memory for ppszSummary will be allocated here - caller is responsible for freeing it
  int  iLength = 0;

  if (*ppszSummary == NULL)
  {
    iLength = strlen(MODULE_SUMMARY_USAGE) + strlen(MODULE_VERSION) + strlen(MODULE_SUMMARY_FORMAT) + 1;
    *ppszSummary = (char*)malloc(iLength);
    memset(*ppszSummary, 0, iLength);
    snprintf(*ppszSummary, iLength, MODULE_SUMMARY_FORMAT, MODULE_SUMMARY_USAGE, MODULE_VERSION);
  } 
  else 
  {
    writeError(ERR_ERROR, "%s reports an error in summaryUsage() : ppszSummary must be NULL when called", MODULE_NAME);
  }

}

/* Display module usage information */
void showUsage()
{
  writeVerbose(VB_NONE, "%s (%s) %s :: %s\n", MODULE_NAME, MODULE_VERSION, MODULE_AUTHOR, MODULE_SUMMARY_USAGE);
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "The VMware Authentication Daemon listens on TCP port 902 and may or may not");
  writeVerbose(VB_NONE, "require a SSL-encrypted connection. This module connects to the service using");
  writeVerbose(VB_NONE, "non-SSL and will automatically switch to SSL if required.");
  writeVerbose(VB_NONE, "");
}

// The "main" of the medusa module world - this is what gets called to actually do the work
int go(sLogin* logins, int argc, char *argv[])
{
  int i;
  char *strtok_ptr, *pOpt, *pOptTmp;
  _VMAUTHD_DATA *psSessionData;
  
  psSessionData = malloc(sizeof(_VMAUTHD_DATA));
  memset(psSessionData, 0, sizeof(_VMAUTHD_DATA));

  if ( !(0 <= argc <= 3) )
  {
    // Show usage information
    writeError(ERR_ERROR, "%s is expecting 0 parameters, but it was passed %d", MODULE_NAME, argc);
  } 
  else 
  {
    writeError(ERR_DEBUG_MODULE, "OMG teh %s module has been called!!", MODULE_NAME);

    initModule(logins, psSessionData);
  }  

  return 0;
}

int initModule(sLogin* psLogin, _VMAUTHD_DATA *_psSessionData)
{
  int hSocket = -1;
  enum MODULE_STATE nState = MSTATE_NEW;
  char* bufReceive;
  int nReceiveBufferSize = 0, nFirstPass = 0, nFoundPrompt = 0;
  int i = 0;
  char *pPass;
  sUser* user = psLogin->psUser;
  sConnectParams params;

  memset(&params, 0, sizeof(sConnectParams));
  if (psLogin->psServer->psAudit->iPortOverride > 0)
    params.nPort = psLogin->psServer->psAudit->iPortOverride;
  else
    params.nPort = PORT_VMAUTHD;
  params.nSSLVersion = 3; /* VMware Authentication Daemon requires SSLv3 */
  initConnectionParams(psLogin, &params);

  if (user != NULL) 
  {
    writeError(ERR_DEBUG_MODULE, "[%s] module started for host: %s user: '%s'", MODULE_NAME, psLogin->psServer->pHostIP, user->pUser);
  }
  else 
  {
    writeError(ERR_DEBUG_MODULE, "[%s] module started for host: %s", MODULE_NAME, psLogin->psServer->pHostIP);
  }

  pPass = getNextPass(psLogin->psServer->psAudit, user);
  if (pPass == NULL)
  {
    writeVerbose(VB_GENERAL, "[%s] out of passwords for user '%s' at host '%s', bailing", MODULE_NAME, user->pUser, psLogin->psServer->pHostIP);
  }

  while(NULL != pPass)
  {  
    switch(nState)
    {
      case MSTATE_NEW:
        if (hSocket > 0)
          medusaDisconnect(hSocket);
  
        hSocket = medusaConnect(&params);
        if (hSocket < 0) 
        {
          writeError(ERR_NOTICE, "[%s] failed to connect, port %d was not open on %s", MODULE_NAME, params.nPort, psLogin->psServer->pHostIP);
          psLogin->iResult = LOGIN_RESULT_UNKNOWN;
          psLogin->iStatus = LOGIN_FAILED;
          return FAILURE;
        }

        if (initConnection(hSocket, &params) == FAILURE)
        {
          psLogin->iResult = LOGIN_RESULT_UNKNOWN;
          psLogin->iStatus = LOGIN_FAILED;
          return FAILURE;
        }

        writeError(ERR_DEBUG_MODULE, "Connected");

        nState = MSTATE_RUNNING;
        break;
      case MSTATE_RUNNING:
        nState = tryLogin(hSocket, &psLogin, _psSessionData, user->pUser, pPass);
        if (psLogin->iResult != LOGIN_RESULT_UNKNOWN)
          pPass = getNextPass(psLogin->psServer->psAudit, user);
        break;
      case MSTATE_EXITING:
        if (hSocket > 0)
          medusaDisconnect(hSocket);
        hSocket = -1;
        pPass = NULL;
        break;
      default:
        writeError(ERR_CRITICAL, "Unknown %s module state %d", MODULE_NAME, nState);
        if (hSocket > 0)
          medusaDisconnect(hSocket);
        hSocket = -1;
        psLogin->iResult = LOGIN_RESULT_UNKNOWN;
        psLogin->iStatus = LOGIN_FAILED;
        return FAILURE;
    }  
  }

  psLogin->iStatus = LOGIN_DONE;
  return SUCCESS;
}
       
/* Module specific functions */

/*
    220 VMware Authentication Daemon Version 1.00
    220 VMware Authentication Daemon Version 1.10: SSL Required
*/
int initConnection(int hSocket, sConnectParams *params)
{ 
  int iRet;
  unsigned char* bufReceive;
  int nReceiveBufferSize;
  
  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }
  else if (strncmp(bufReceive, "220 VMware Authentication Daemon Version", 4) != 0)
  {
    writeError(ERR_ERROR, "[%s] Incorrect VMware authentication protocol or service shutdown: %s.", bufReceive);
    return FAILURE;
  }
  
  if (strstr(bufReceive, "SSL Required"))
  {
    writeError(ERR_INFO, "[%s] SSL Required for VMware authentication. Proceeding using SSL.", MODULE_NAME);
          
    if (medusaConnectSocketSSL(params, hSocket) < 0)
    {
      writeError(ERR_ERROR, "[%s] Failed to establish SSLv3 connection.", MODULE_NAME);
      return FAILURE;
    }
  }

  return SUCCESS;
}

int tryLogin(int hSocket, sLogin** psLogin, _VMAUTHD_DATA* _psSessionData, char* szLogin, char* szPassword)
{
  int iRet;
  unsigned char* bufSend;
  unsigned char* bufReceive;
  int nReceiveBufferSize = 0;

  bufSend = malloc(strlen(szLogin) + 7 + 1);
  memset(bufSend, 0, strlen(szLogin) + 7 + 1);
  sprintf(bufSend, "USER %s\r\n", szLogin);
 
  if (medusaSend(hSocket, bufSend, strlen(bufSend), 0) < 0)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaSend was not successful", MODULE_NAME);
    free(bufSend);
    return FAILURE;
  }
  free(bufSend);

  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }
  /* 331 Password required for USERNAME.[0D][0A] */
  else if (strncmp(bufReceive, "331 ", 4) != 0)
  {
    writeError(ERR_ERROR, "[%s] VMware authentication protocol error or service shutdown: %s.\n", bufReceive);
    free(bufReceive);
    return FAILURE;
  }
  free(bufReceive);

  bufSend = malloc(strlen(szLogin) + 7 + 1);
  memset(bufSend, 0, strlen(szLogin) + 7 + 1);
  sprintf(bufSend, "PASS %s\r\n", szPassword);
  
  if (medusaSend(hSocket, bufSend, strlen(bufSend), 0) < 0)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaSend was not successful", MODULE_NAME);
    free(bufSend);
    return FAILURE;
  }
  free(bufSend);

  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }
  /* 230 User USERNAME logged in.[0D][0A] */
  else if (strncmp(bufReceive, "230 ", 4) == 0)
  {
    writeError(ERR_DEBUG_MODULE, "[%s] Login attempt successful.", MODULE_NAME);
    (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
    iRet = MSTATE_EXITING;
  }
  /* 530 Login incorrect.[0D][0A] */
  else if (strncmp(bufReceive, "530 ", 4) == 0)
  {
    writeError(ERR_DEBUG_MODULE, "[%s] Login attempt failed.", MODULE_NAME);
    (*psLogin)->iResult = LOGIN_RESULT_FAIL;
    iRet = MSTATE_NEW;
  }
  else
  {
    writeError(ERR_ERROR, "[%s] Unknown error occurred during authentication: %s", MODULE_NAME, bufReceive);
    (*psLogin)->iResult = LOGIN_RESULT_ERROR;
    iRet = MSTATE_EXITING;
  }
  free(bufReceive);
  
  setPassResult((*psLogin), szPassword);

  return(iRet);
}

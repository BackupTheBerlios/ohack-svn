/*
**   FTP Password Checking Medusa Module
**
**   ------------------------------------------------------------------------
**    Copyright (C) 2006 pMonkey
**    pMonkey <pmonkey@foofus.net>
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
**
*/

#include <sys/types.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "module.h"

#define MODULE_NAME    "ftp.mod"
#define MODULE_AUTHOR  "pMonkey <pmonkey@foofus.net>"
#define MODULE_SUMMARY_USAGE  "Brute force module for FTP sessions"
#define MODULE_VERSION    "1.2.0"
#define MODULE_VERSION_SVN "$Id: ftp.c 606 2006-10-19 20:22:41Z jmk $"
#define MODULE_SUMMARY_FORMAT  "%s : version %s"

#define BUF_SIZE 300

#define FREE(x) \
        if (x != NULL) { \
           free(x); \
           x = NULL; \
        }

#define PORT_FTP  21
#define PORT_FTPS 990

// Tells us whether we are to continue processing or not
enum MODULE_STATE
{
  MSTATE_NEW,
  MSTATE_RUNNING,
  MSTATE_EXITING
};

// Forward declarations
int tryLogin(int hSocket, sLogin** login, char* szLogin, char* szPassword);
int initModule(sLogin* login);

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
}

// The "main" of the medusa module world - this is what gets called to actually do the work
int go(sLogin* logins, int argc, char *argv[])
{
  int i;

  if ( !(0 <= argc <= 3) )
  {
    // Show usage information
    writeError(ERR_ERROR, "%s is expecting 0 parameters, but it was passed %d", MODULE_NAME, argc);
  } 
  else 
  {
    writeError(ERR_DEBUG_MODULE, "OMG teh %s module has been called!!", MODULE_NAME);
 
    initModule(logins);
  }  

  return 0;
}

int initModule(sLogin* psLogin)
{
  int hSocket = -1;
  enum MODULE_STATE nState = MSTATE_NEW;
  char* bufReceive;
  int nReceiveBufferSize = 0, nFirstPass = 0, nFoundPrompt = 0;
  int i = 0;
  int nStatus = FAILURE;
  char *pPass;
  sUser* user = psLogin->psUser;
  sConnectParams params;

  memset(&params, 0, sizeof(sConnectParams));
  if (psLogin->psServer->psAudit->iPortOverride > 0)
    params.nPort = psLogin->psServer->psAudit->iPortOverride;
  else if (psLogin->psServer->psHost->iUseSSL > 0)
    params.nPort = PORT_FTPS;
  else
    params.nPort = PORT_FTP;
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
        // Already have an open socket - close it
        if (hSocket > 0)
          medusaDisconnect(hSocket);

        if (psLogin->psServer->psHost->iUseSSL > 0)
          hSocket = medusaConnectSSL(&params);
        else
          hSocket = medusaConnect(&params);
        
        if (hSocket < 0) 
        {
          writeError(ERR_NOTICE, "%s: failed to connect, port %d was not open on %s", MODULE_NAME, params.nPort, psLogin->psServer->pHostIP);
          psLogin->iResult = LOGIN_RESULT_UNKNOWN;
          psLogin->iStatus = LOGIN_FAILED;
          return FAILURE;
        }

        /* establish initial connection */
        nReceiveBufferSize = 0;
        nStatus = FAILURE;
        while (bufReceive = medusaReceiveLine(hSocket, &nReceiveBufferSize))
        {
          if ((strncmp(bufReceive,"220 ", 4) == 0) || (strncmp(bufReceive,"220-", 4) == 0))
          {
            writeError(ERR_DEBUG_MODULE, "[%s] Server sent '220' code.", MODULE_NAME);
            nStatus = SUCCESS;
            break;
          }

          free(bufReceive);
        }

        if (nStatus == FAILURE)
        {
          writeError(ERR_DEBUG_MODULE, "[%s] Server did not respond with '220' code. Exiting...", MODULE_NAME);
          psLogin->iResult = LOGIN_RESULT_UNKNOWN;
          psLogin->iStatus = LOGIN_FAILED;
          nState = MSTATE_EXITING;
        }
        else
        {
          writeError(ERR_DEBUG_MODULE, "Connected");
          nState = MSTATE_RUNNING;
        }
        
        break;
      case MSTATE_RUNNING:
        nState = tryLogin(hSocket, &psLogin, user->pUser, pPass);
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

/* Module Specific Functions */

int tryLogin(int hSocket, sLogin** psLogin, char* szLogin, char* szPassword)
{
  int iRet;
  unsigned char bufSend[BUF_SIZE];
  unsigned char* bufReceive;
  int nReceiveBufferSize = 0;

  /* send username */
  memset(bufSend, 0, sizeof(bufSend));
  sprintf(bufSend, "USER %.250s\r\n", szLogin);

  if (medusaSend(hSocket, bufSend, strlen(bufSend), 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
  }
 
  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "%s failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }
  else if (strncmp(bufReceive,"331",3)) 
  {
    writeError(ERR_ERROR, "%s failed: Server did not respond with a '331'.", MODULE_NAME);
    return FAILURE;
  }
 
  /* send password */
  memset(bufSend, 0, sizeof(bufSend));
  sprintf(bufSend, "PASS %.250s\r\n", szPassword);

  if (medusaSend(hSocket, bufSend, strlen(bufSend), 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
  }

  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "%s failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }
  else if (bufReceive[0] == '2')
  {
    writeError(ERR_DEBUG_MODULE, "%s : Login attempt successful.", MODULE_NAME);
    (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
    iRet = MSTATE_EXITING;
  }
  else
  {
    writeError(ERR_DEBUG_MODULE, "%s : Login attempt failed.", MODULE_NAME);
    (*psLogin)->iResult = LOGIN_RESULT_FAIL;
    iRet = MSTATE_NEW;
  }
 
  FREE(bufReceive);
  setPassResult((*psLogin), szPassword);

  return(iRet);
}

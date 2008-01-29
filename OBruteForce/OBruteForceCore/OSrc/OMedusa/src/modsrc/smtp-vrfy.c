/*
**   SMTP VRFY Account Enumeration Medusa Module
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
**
*/

#include <sys/types.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "module.h"

#define MODULE_NAME    "smtp-vrfy.mod"
#define MODULE_AUTHOR  "JoMo-Kun <jmk@foofus.net>"
#define MODULE_SUMMARY_USAGE  "Brute force module for enumerating accounts via SMTP VRFY"
#define MODULE_VERSION    "0.9.1"
#define MODULE_VERSION_SVN "$Id: smtp-vrfy.c 569 2006-07-31 21:01:06Z jmk $"
#define MODULE_SUMMARY_FORMAT  "%s : version %s"
#define MODULE_SUMMARY_FORMAT_WARN  "%s : version %s (%s)"

#define PORT_SMTP 25
#define BUF_SIZE 300
#define RECEIVE_DELAY_1 20 * 1000000
#define RECEIVE_DELAY_2 0.5 * 1000000

typedef struct __MODULE_DATA {
  char *szEHLO;
} _MODULE_DATA;


// Tells us whether we are to continue processing or not
enum MODULE_STATE
{
  MSTATE_NEW,
  MSTATE_RUNNING,
  MSTATE_EXITING
};

// Forward declarations
int sayEHLO(int hSocket, _MODULE_DATA* _psSessionData);
int sayQUIT(int hSocket);
int tryLogin(int hSocket, sLogin** login, _MODULE_DATA* _psSessionData, char* szLogin, char* szPassword);
int initModule(sLogin* login, _MODULE_DATA *_psSessionData);

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
  writeVerbose(VB_NONE, "Available module options:");
  writeVerbose(VB_NONE, "  EHLO:? (optional)");
  writeVerbose(VB_NONE, "    Sets the name sent via the EHLO command.");
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "  Usage example: \"-M smtp-vrfy -m EHLO:g3rg3 -U accounts.txt -p domain.com\"");
  writeVerbose(VB_NONE, "");
}

// The "main" of the medusa module world - this is what gets called to actually do the work
int go(sLogin* logins, int argc, char *argv[])
{
  int i;
  char *strtok_ptr, *pOpt, *pOptTmp;
  _MODULE_DATA *psSessionData;
  
  psSessionData = malloc(sizeof(_MODULE_DATA));
  memset(psSessionData, 0, sizeof(_MODULE_DATA));

  if ( !(0 <= argc <= 3) )
  {
    // Show usage information
    writeError(ERR_ERROR, "%s is expecting 0 parameters, but it was passed %d", MODULE_NAME, argc);
  } 
  else 
  {
    writeError(ERR_DEBUG_MODULE, "OMG teh %s module has been called!!", MODULE_NAME);

    for (i=0; i<argc; i++) {
      pOptTmp = malloc( strlen(argv[i]) + 1);
      memset(pOptTmp, 0, strlen(argv[i]) + 1);
      strncpy(pOptTmp, argv[i], strlen(argv[i]));
      writeError(ERR_DEBUG_MODULE, "Processing complete option: %s", pOptTmp);
      pOpt = strtok_r(pOptTmp, ":", &strtok_ptr);
      writeError(ERR_DEBUG_MODULE, "Processing option: %s", pOpt);

      if (strcmp(pOpt, "EHLO") == 0)
      {
        pOpt = strtok_r(NULL, "\0", &strtok_ptr);
        writeError(ERR_DEBUG_MODULE, "Processing option parameter: %s", pOpt);

        if ( pOpt )
        {
          psSessionData->szEHLO = malloc(strlen(pOpt) + 1);
          memset(psSessionData->szEHLO, 0, strlen(pOpt) + 1);
          strncpy((char *)psSessionData->szEHLO, pOpt, strlen(pOpt));
        }
        else
          writeError(ERR_WARNING, "Method EHLO requires value to be set.");
      }
      else
         writeError(ERR_WARNING, "Invalid method: %s.", pOpt);

      free(pOptTmp);
    }

    initModule(logins, psSessionData);
  }  

  return 0;
}

int initModule(sLogin* psLogin, _MODULE_DATA *_psSessionData)
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
  params.nPort = PORT_SMTP;
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

  if (_psSessionData->szEHLO == NULL)
  {
    _psSessionData->szEHLO = malloc(7);
    memset(_psSessionData->szEHLO, 0, 7);
    sprintf(_psSessionData->szEHLO, "MEDUSA");
  }

  writeError(ERR_DEBUG, "[%s] Set EHLO value: %s", MODULE_NAME, _psSessionData->szEHLO);

  while(NULL != pPass)
  {  
    switch(nState)
    {
      case MSTATE_NEW:
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

        writeError(ERR_DEBUG_MODULE, "Connected");
        
        /* send EHLO */
        writeError(ERR_DEBUG_MODULE, "[%s] Sending EHLO command.", MODULE_NAME);
        if (sayEHLO(hSocket, _psSessionData) == SUCCESS)
          nState = MSTATE_RUNNING;
        else  
          nState = MSTATE_EXITING;

        break;
      case MSTATE_RUNNING:
        nState = tryLogin(hSocket, &psLogin, _psSessionData, pPass, user->pUser);
        if (psLogin->iResult != LOGIN_RESULT_UNKNOWN)
          pPass = getNextPass(psLogin->psServer->psAudit, user);
        break;
      case MSTATE_EXITING:
        sayQUIT(hSocket);
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

/*
  EHLO foo
  250-spamfirewall.domain.com
  250-PIPELINING
  250-SIZE 100000000
  250-VRFY
  250-ETRN
  250 8BITMIME
*/
int sayEHLO(int hSocket, _MODULE_DATA* _psSessionData)
{
  unsigned char bufSend[BUF_SIZE];
  unsigned char* bufReceive;
  int nReceiveBufferSize = 0;

  /* send helo string */
  memset(bufSend, 0, sizeof(bufSend));
  sprintf(bufSend, "EHLO %.250s\r\n", _psSessionData->szEHLO);

  if (medusaSend(hSocket, bufSend, strlen(bufSend), 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
  }

  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "%s failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }

  /* check if more data is waiting */
  if (medusaDataReadyTimed(hSocket, 0, 20000) > 0)
    bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
  
  writeError(ERR_DEBUG_MODULE, "[%s] Server responded to ELHO with: %s", MODULE_NAME, bufReceive);

  free(bufReceive);
  return SUCCESS;
}

int sayQUIT(int hSocket)
{
  unsigned char bufSend[BUF_SIZE];

  memset(bufSend, 0, sizeof(bufSend));
  sprintf(bufSend, "QUIT\r\n");

  if (medusaSend(hSocket, bufSend, strlen(bufSend), 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
  }

  return SUCCESS;
}

/*
  VRFY administrator@domain.com
  252 administrator@domain.com
  
  VRFY foo@domain.com
  550 <foo@domain.com>: Recipient address rejected: No such user (foo@domain.com)
  
  VRFY foo@bar.com
  554 <foo@bar.com>: Relay access denied
  
  421 Error: too many errors
*/
int tryLogin(int hSocket, sLogin** psLogin, _MODULE_DATA* _psSessionData, char* szDomain, char* szAccount)
{
  int iRet;
  unsigned char bufSend[BUF_SIZE];
  unsigned char* bufReceive;
  int nReceiveBufferSize = 0;

  /* send helo string */
  writeError(ERR_DEBUG_MODULE, "[%s] Sending: VRFY %.250s@%.250s\r\n", MODULE_NAME, szAccount, szDomain);
  memset(bufSend, 0, sizeof(bufSend));
  sprintf(bufSend, "VRFY %.250s@%.250s\r\n", szAccount, szDomain);

  if (medusaSend(hSocket, bufSend, strlen(bufSend), 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
  }

  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "%s failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }
  else if (strncmp(bufReceive, "252 ", 4) == 0) /* valid account */
  {
    writeError(ERR_DEBUG_MODULE, "[%s] Found valid account: %s", MODULE_NAME, szAccount);
    (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
    iRet = MSTATE_RUNNING;
  }
  else if (strncmp(bufReceive, "550 ", 4) == 0) /* non-existant account */
  {
    writeError(ERR_DEBUG_MODULE, "[%s] Non-existant account: %s", MODULE_NAME, szAccount);
    (*psLogin)->iResult = LOGIN_RESULT_FAIL;
    iRet = MSTATE_RUNNING;
  }
  else if (strncmp(bufReceive, "554 ", 4) == 0) /* invalid domain name */
  {
    writeError(ERR_ERROR, "[%s] Invalid domain name: %s", MODULE_NAME, szDomain);
    (*psLogin)->iResult = LOGIN_RESULT_ERROR;
    iRet = MSTATE_EXITING;
  }
  
  /* check if more data is waiting */
  if (medusaDataReadyTimed(hSocket, 0, 20000) > 0)
    bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
 
  if (strstr(bufReceive, "421 Error: too many errors"))
  {
    writeError(ERR_DEBUG_MODULE, "[%s] Too many errors. Restarting connection.", MODULE_NAME);
    iRet = MSTATE_NEW;
  }
  
  setPassResult((*psLogin), szDomain);
  return(iRet);
}

/*
**   SMTP-AUTH Authentication Daemon Password Checking Medusa Module
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
#include <math.h>
#include "module.h"

#define MODULE_NAME    "smtp-auth.mod"
#define MODULE_AUTHOR  "pmonkey@foofus.net"
#define MODULE_SUMMARY_USAGE  "Brute force module for SMTP Authentication with TLS"
#define MODULE_VERSION    "0.9.1"
#define MODULE_VERSION_SVN "$Id: smtp-auth.c 569 2006-07-31 21:01:06Z jmk $"
#define MODULE_SUMMARY_FORMAT  "%s : version %s"
#define MODULE_SUMMARY_FORMAT_WARN  "%s : version %s (%s)"

#define PORT_SMTPAUTH 25
#define BUF_SIZE 300


typedef struct __SMTPAUTH_DATA {
  char *szDB;
} _SMTPAUTH_DATA;


// Tells us whether we are to continue processing or not
enum MODULE_STATE
{
  MSTATE_NEW,
  MSTATE_RUNNING,
  MSTATE_EXITING
};

// Forward declarations
int initConnection(int hSocket, sConnectParams *params);
int tryLogin(int hSocket, sLogin** login, _SMTPAUTH_DATA* _psSessionData, char* szLogin, char* szPassword);
int initModule(sLogin* login, _SMTPAUTH_DATA *_psSessionData);

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

/* Stolen Right Out of Mutt */
void mutt_to_base64 (unsigned char *out, unsigned char *in, size_t len, size_t olen)
{
  int j;

  char B64Chars[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
  };

  /* Back to Mutt code */
  while (len >= 3 && olen > 10)
  {
    *out++ = B64Chars[in[0] >> 2];
    *out++ = B64Chars[((in[0] << 4) & 0x30) | (in[1] >> 4)];
    *out++ = B64Chars[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
    *out++ = B64Chars[in[2] & 0x3f];
    olen  -= 4;
    len   -= 3;
    in    += 3;
  }

  /* clean up remainder */
  if (len > 0 && olen > 4)
  {
    unsigned char fragment;

    *out++ = B64Chars[in[0] >> 2];
    fragment = (in[0] << 4) & 0x30;
    if (len > 1)
      fragment |= in[1] >> 4;
    *out++ = B64Chars[fragment];
    *out++ = (len < 2) ? '=' : B64Chars[(in[1] << 2) & 0x3c];
    *out++ = '=';
  }
  *out = '\0';
}

// The "main" of the medusa module world - this is what gets called to actually do the work
int go(sLogin* logins, int argc, char *argv[])
{
  int i;
  char *strtok_ptr, *pOpt, *pOptTmp;
  _SMTPAUTH_DATA *psSessionData;
  
  psSessionData = malloc(sizeof(_SMTPAUTH_DATA));
  memset(psSessionData, 0, sizeof(_SMTPAUTH_DATA));

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

int initModule(sLogin* psLogin, _SMTPAUTH_DATA *_psSessionData)
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
    params.nPort = PORT_SMTPAUTH;
   params.nSSLVersion = 9; /* Force use of TLS */
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


int initConnection(int hSocket, sConnectParams *params)
{ 
  int iRet;
  unsigned char* bufSend;
  unsigned char* bufReceive;
  int nReceiveBufferSize;
  
  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }
  else if (strncmp(bufReceive, "220", 3) != 0)
  {
    writeError(ERR_ERROR, "[%s] Is this an SMTP server?: %s.", bufReceive);
    return FAILURE;
  }


  bufSend = malloc(200);
  memset(bufSend, 0, 200);
  sprintf(bufSend, "EHLO gerg\r\n");
 
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
  else if (strstr(bufReceive, "250-AUTH PLAIN") != NULL)
    {

      


    }
  else if (strstr(bufReceive, "250-STARTTLS") != NULL)
  {
    bufSend = malloc(200);
    memset(bufSend, 0, 200);
    sprintf(bufSend, "STARTTLS\r\n");
    
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
    else if (strstr(bufReceive, "220") != NULL)
      {

    
        if (medusaConnectSocketSSL(params, hSocket) < 0)
          {
            writeError(ERR_ERROR, "[%s] Failed to establish SSLv3 connection.", MODULE_NAME);
            return FAILURE;
          }
      }

    
  }

  free(bufReceive);
  
  return SUCCESS;
}

int tryLogin(int hSocket, sLogin** psLogin, _SMTPAUTH_DATA* _psSessionData, char* szLogin, char* szPassword)
{
  int iRet;
  unsigned char bufSend[BUF_SIZE];
  unsigned char* bufReceive;
  unsigned char b64[BUF_SIZE],b642[BUF_SIZE];
  int m;
  int nReceiveBufferSize = 0;


  
  memset(bufSend, 0, strlen(bufSend));
  memset(b64, 0, sizeof(b64));
  memset(b642, 0, sizeof(b642));
  memcpy(b64, szLogin, strlen(szLogin) );
  memcpy(b64 + strlen(szLogin) + 1 , szLogin, strlen(szLogin) );
  memcpy(b64 + strlen(szLogin) + strlen(szLogin) +2 , szPassword, strlen(szPassword) );


  m=strlen(szLogin) + strlen(szLogin) + strlen(szPassword) + 2;

  mutt_to_base64(b642,b64,m,sizeof(b642));


  sprintf(bufSend, "AUTH PLAIN\r\n");

 
  if (medusaSend(hSocket, bufSend, strlen(bufSend), 0) < 0)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaSend was not successful", MODULE_NAME);
    return FAILURE;
  }

  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }
  else if (strncmp(bufReceive, "334", 3) == 0)
  {
    
    sprintf(bufSend, "%s\r\n",b642);
    if (medusaSend(hSocket, bufSend, strlen(bufSend), 0) < 0)
      {
        writeError(ERR_ERROR, "[%s] failed: medusaSend was not successful", MODULE_NAME);
        return FAILURE;
      }

  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }
  else if (strncmp(bufReceive, "535", 3) == 0)
  {
    writeError(ERR_DEBUG_MODULE, "[%s] Login attempt failed.", MODULE_NAME);
    (*psLogin)->iResult = LOGIN_RESULT_FAIL;
    iRet = MSTATE_NEW;
  }

  else if (strncmp(bufReceive, "235", 3) == 0)
    {
      writeError(ERR_DEBUG_MODULE, "[%s] Login attempt success.", MODULE_NAME);
      (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
      iRet = MSTATE_EXITING;
    }
  else
    {
      writeError(ERR_ERROR, "[%s] Unknown error occurred during authentication: %s", MODULE_NAME, bufReceive);
      (*psLogin)->iResult = LOGIN_RESULT_ERROR;
      iRet = MSTATE_EXITING;
    }
  }

  setPassResult((*psLogin), szPassword);
  return(iRet);
}

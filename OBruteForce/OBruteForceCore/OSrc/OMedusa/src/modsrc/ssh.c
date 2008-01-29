/*
**   SSH v2 Password Checking Medusa Module
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
**    This module requires libssh2 (www.libssh2.org).
**
*/

#include <sys/types.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "module.h"

#define MODULE_NAME    "ssh.mod"
#define MODULE_AUTHOR  "JoMo-Kun <jmk@foofus.net>"
#define MODULE_SUMMARY_USAGE  "Brute force module for SSH v2 sessions"
#define MODULE_VERSION    "1.0.2"
#define MODULE_VERSION_SVN "$Id: ssh.c 606 2006-10-19 20:22:41Z jmk $"
#define MODULE_SUMMARY_FORMAT  "%s : version %s"
#define MODULE_SUMMARY_FORMAT_WARN  "%s : version %s (%s)"
#define LIBSSH2_WARNING "No usable LIBSSH2. Module disabled."

#ifdef HAVE_LIBSSH2

#include <libssh2.h>

#define PORT_SSH 22
#define SSH_AUTH_UNDEFINED 1
#define SSH_AUTH_KBDINT 2
#define SSH_AUTH_PASSWORD 3
#define SSH_AUTH_ERROR 4

char *szBannerMsg = NULL;

typedef struct __ssh2_data {
  char *pPass;
  int iAnswerCount;
} _ssh2_data;

// Tells us whether we are to continue processing or not
enum MODULE_STATE
{
  MSTATE_NEW,
  MSTATE_RUNNING,
  MSTATE_EXITING
};

// Forward declarations
int tryLogin(int hSocket, LIBSSH2_SESSION *session, sLogin** login, char* szLogin, char* szPassword);
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
  writeVerbose(VB_NONE, "Available module options:");
  writeVerbose(VB_NONE, "  BANNER:? (Libssh client banner. Default SSH-2.0-MEDUSA.)");
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "Usage example: \"-M ssh -m BANNER:SSH-2.0-FOOBAR\"");
}

// The "main" of the medusa module world - this is what gets called to actually do the work
int go(sLogin* logins, int argc, char *argv[])
{
  int i;
  char *strtok_ptr, *pOpt, *pOptTmp;

  if ( !(0 <= argc <= 1) )
  {
    // Show usage information
    writeError(ERR_ERROR, "%s is expecting 0/1 parameters, but it was passed %d", MODULE_NAME, argc);
  } 
  else 
  {
    writeError(ERR_DEBUG_MODULE, "OMG teh %s module has been called!!", MODULE_NAME);

    for (i=0; i<argc; i++) {
      writeError(ERR_DEBUG_MODULE, "Processing option: %s", argv[i]);
      pOptTmp = malloc( strlen(argv[i]) + 1);
      memset(pOptTmp, 0, strlen(argv[i]) + 1);
      strncpy(pOptTmp, argv[i], strlen(argv[i]));
      pOpt = strtok_r(pOptTmp, ":", &strtok_ptr);

      if (strcmp(pOpt, "BANNER") == 0)
      {
        pOpt = strtok_r(NULL, "\0", &strtok_ptr);

        if ( pOpt )
        {
          szBannerMsg = malloc(strlen(pOpt));
          strncpy((char *) szBannerMsg, pOpt, strlen(pOpt));
        }
        else
        {
          writeError(ERR_WARNING, "Method BANNER requires value to be set.");
        }
      }
      else 
      {
        writeError(ERR_WARNING, "Invalid method: %s.", pOpt);
      }
      
      free(pOptTmp);
    }

    initModule(logins);
  }  

  return 0;
}

// void *name(size_t count, void **abstract)
static LIBSSH2_ALLOC_FUNC(ssh2_init_alloc_cb)
{
  return malloc(count);
}

// void name(void *ptr, void **abstract)
static LIBSSH2_FREE_FUNC(ssh2_init_free_cb)
{
  free(ptr);
}

// void *name(void *ptr, size_t count, void **abstract)
static LIBSSH2_REALLOC_FUNC(ssh2_init_realloc_cb)
{
  return realloc(ptr, count);
}

int initModule(sLogin* psLogin)
{
  int hSocket = -1;
  enum MODULE_STATE nState = MSTATE_NEW;
  char* bufReceive;
  int nReceiveBufferSize = 0, nFirstPass = 0, nFoundPrompt = 0;
  int i = 0;
  char *pPass;
  sUser* user = psLogin->psUser;
  sConnectParams params;
  LIBSSH2_SESSION *session = NULL;
  char *pErrorMsg;
  int iErrorMsg;

  _ssh2_data ssh2_data;
  ssh2_data.pPass = NULL;
  ssh2_data.iAnswerCount = 0;

  memset(&params, 0, sizeof(sConnectParams));
 
  if (psLogin->psServer->psAudit->iPortOverride > 0)
    params.nPort = psLogin->psServer->psAudit->iPortOverride;
  else
    params.nPort = PORT_SSH;
 
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
        /*
          Create a session instance and start it up
          This will trade welcome banners, exchange keys, and setup crypto, compression, and MAC layers
        */
        if (session)
        {
          writeError(ERR_DEBUG_MODULE, "%s: Destroying previous SSH session structure: Host: %s User: %s", MODULE_NAME, psLogin->psServer->pHostIP, user->pUser);
          libssh2_session_free(session);
        }
        
        session = libssh2_session_init_ex(ssh2_init_alloc_cb, ssh2_init_free_cb, ssh2_init_realloc_cb, &ssh2_data);
        if (!session)
        {
          writeError(ERR_ERROR, "%s: Failed initiating SSH session: Host: %s User: %s Pass: %s", MODULE_NAME, psLogin->psServer->pHostIP, user->pUser, pPass);
          psLogin->iResult = LOGIN_RESULT_UNKNOWN;
          psLogin->iStatus = LOGIN_FAILED;
          return FAILURE;
        }        

        /* Set client SSH banner */
        if (!szBannerMsg) { 
          szBannerMsg = malloc(20);
          memset(szBannerMsg, 0, 20);
          sprintf(szBannerMsg, "SSH-2.0-MEDUSA_1.0");
        }
        
        writeError(ERR_DEBUG_MODULE, "Attempting to set banner: %s", szBannerMsg);
        if ( libssh2_banner_set(session, szBannerMsg) ) {
           writeError(ERR_DEBUG_MODULE, "Failed to set libssh banner.");
        }
        
        writeError(ERR_DEBUG_MODULE, "Attempting to initiate SSH session.");
        for (i = 1; i <= 5; i++) {
          if (hSocket > 0) {
            medusaDisconnect(hSocket);
          }
        
          hSocket = medusaConnect(&params);
          if ( hSocket < 0 ) {
            writeError(ERR_NOTICE, "%s: failed to connect, port %d was not open on %s", MODULE_NAME, params.nPort, psLogin->psServer->pHostIP);
            libssh2_session_free(session);
            psLogin->iResult = LOGIN_RESULT_UNKNOWN;
            psLogin->iStatus = LOGIN_FAILED;
            return FAILURE;
          }
          
          if (libssh2_session_startup(session, hSocket)) {
            writeError(ERR_ERROR, "%s: Failed establishing SSH session (%d/5): Host: %s User: %s Pass: %s", MODULE_NAME, i, psLogin->psServer->pHostIP, user->pUser, pPass);
          
            libssh2_session_last_error(session, &pErrorMsg, &iErrorMsg, 1);
            if (pErrorMsg) {
              writeError(ERR_DEBUG_MODULE, "libssh2 Error Message: %s", pErrorMsg);
            }

            if (i == 5) {
              writeError(ERR_ERROR, "%s: Failed establishing SSH session. The following credentials were NOT tested: Host: %s User: %s Pass: %s", MODULE_NAME, psLogin->psServer->pHostIP, user->pUser, pPass);
              libssh2_session_disconnect(session, "");
              libssh2_session_free(session);
              if (hSocket > 0)
                medusaDisconnect(hSocket);
              hSocket = -1;
              psLogin->iResult = LOGIN_RESULT_UNKNOWN;
              psLogin->iStatus = LOGIN_FAILED;
              return FAILURE;
            }
        
            sleep(psLogin->psServer->psHost->iRetryWait);
          }
          else { break; }
        }
        
        writeError(ERR_DEBUG_MODULE, "Id: %d successfully established connection.", psLogin->iId);
        nState = MSTATE_RUNNING;
        break;
      case MSTATE_RUNNING:
        ssh2_data.pPass = pPass;
        ssh2_data.iAnswerCount = 0;
        nState = tryLogin(hSocket, session, &psLogin, user->pUser, pPass);
        if (psLogin->iResult != LOGIN_RESULT_UNKNOWN)
          pPass = getNextPass(psLogin->psServer->psAudit, user);
        break;
      case MSTATE_EXITING:
        pPass = NULL;
        break;
      default:
        writeError(ERR_CRITICAL, "Unknown %s module state %d", MODULE_NAME, nState);
        libssh2_session_disconnect(session, "");
        libssh2_session_free(session);
        if (hSocket > 0)
          medusaDisconnect(hSocket);
        hSocket = -1;
        psLogin->iResult = LOGIN_RESULT_UNKNOWN;
        psLogin->iStatus = LOGIN_FAILED;
        return FAILURE;
    }  
  }
  
  if (session)
  {
    libssh2_session_disconnect(session, "");
    libssh2_session_free(session);
  }
  
  if (hSocket > 0)
  {
    medusaDisconnect(hSocket);
    hSocket = -1;
  }
 
  if (psLogin->iStatus != LOGIN_FAILED)
    psLogin->iStatus = LOGIN_DONE;

  return SUCCESS;
}

void response_callback(const char* name, int name_len, const char* instruction, int instruction_len, int num_prompts, const LIBSSH2_USERAUTH_KBDINT_PROMPT* prompts, LIBSSH2_USERAUTH_KBDINT_RESPONSE* responses, void **abstract)
{
    int i;
    char *pPass = ((_ssh2_data*)(*abstract))->pPass;
  
    if (((_ssh2_data*)(*abstract))->iAnswerCount > 0)
    {
      writeError(ERR_DEBUG_MODULE, "libssh2 response_callback: sshd asked a question, but we've already given out answer.");
    }
    else {
      for (i=0; i<num_prompts; i++)
      {
        writeError(ERR_DEBUG_MODULE, "libssh2 response_callback: prompt[%d/%d]: %s %d", i + 1, num_prompts, prompts[i].text, prompts[i].length);
    
        if ( strncmp(prompts[i].text, "Password: ", 9) == 0 ) {
          responses[i].text = malloc( strlen(pPass) );
          memset(responses[i].text, 0, strlen(pPass));
          strncpy(responses[i].text, pPass, strlen(pPass));
          responses[i].length = strlen(pPass);
          writeError(ERR_DEBUG_MODULE, "libssh2 response_callback set password response: %s", pPass);
          ((_ssh2_data*)(*abstract))->iAnswerCount++;
        }
        else
        {
          writeError(ERR_ERROR, "%s received an unknown SSH prompt: %s", MODULE_NAME, prompts[i].text);
        }
      }
    }
}

int tryLogin(int hSocket, LIBSSH2_SESSION *session, sLogin** psLogin, char* szLogin, char* szPassword)
{
  char *pErrorMsg = NULL;
  int iErrorMsg, iAuthMode, iRet;
  void (*pResponseCallback) ();
  char *strtok_ptr = NULL;
  char *pAuth = NULL;
  pResponseCallback = response_callback;

  /*
    Password authentication failure delay: 2
    Password authentication maximum tries: 3
    Keyboard-interactive authentication failure delay: 2
    Keyboard-interactive authentication maximum tries: 3
  */

  /* libssh2 supports: none, password, publickey, hostbased, keyboard-interactive */
  iAuthMode = SSH_AUTH_UNDEFINED;
  pErrorMsg = libssh2_userauth_list(session, szLogin, strlen(szLogin));
  if (pErrorMsg)
  {
    writeError(ERR_DEBUG_MODULE, "Supported user-auth modes: %s.", pErrorMsg);
    pAuth = strtok_r(pErrorMsg, ",", &strtok_ptr);

    while (pAuth) {
      if (strcmp(pAuth, "password") == 0) {
        writeError(ERR_DEBUG_MODULE, "Server support user-auth type: password");
        iAuthMode = SSH_AUTH_PASSWORD;
        break;
      }
      else if (strcmp(pAuth, "keyboard-interactive") == 0) {
        writeError(ERR_DEBUG_MODULE, "Server support user-auth type: keyboard-interactive");
        iAuthMode = SSH_AUTH_KBDINT;
        break;
      }

      pAuth = strtok_r(NULL, ",", &strtok_ptr);
    }
  
    free(pErrorMsg); 
  }
  else {
    writeError(ERR_ERROR, "Failed to retrieve supported authentication modes. Aborting...");
    (*psLogin)->iResult = LOGIN_RESULT_UNKNOWN;
    (*psLogin)->iStatus = LOGIN_FAILED;
    iRet = MSTATE_EXITING;
  }
 
  switch (iAuthMode)
  {
    case SSH_AUTH_KBDINT:
      if (libssh2_userauth_keyboard_interactive(session, szLogin, pResponseCallback) ) 
      {
        writeError(ERR_DEBUG_MODULE, "Keyboard-Interactive authentication failed: Host: %s User: %s Pass: %s", (*psLogin)->psServer->pHostIP, szLogin, szPassword);
        (*psLogin)->iResult = LOGIN_RESULT_FAIL;
        iRet = MSTATE_NEW;
      }
      else {
        writeError(ERR_DEBUG_MODULE, "Keyboard-Interactive authentication succeeded: Host: %s User: %s Pass: %s", (*psLogin)->psServer->pHostIP, szLogin, szPassword);
        (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
        iRet = MSTATE_EXITING;
      }
      break;
      
    case SSH_AUTH_PASSWORD:
      if (libssh2_userauth_password(session, szLogin, szPassword))
      {
        libssh2_session_last_error(session, &pErrorMsg, &iErrorMsg, 1);
        writeError(ERR_DEBUG_MODULE, "Password-based authentication failed: %s: Host: %s User: %s Pass: %s", pErrorMsg, (*psLogin)->psServer->pHostIP, szLogin, szPassword);
        (*psLogin)->iResult = LOGIN_RESULT_FAIL;
        iRet = MSTATE_RUNNING;
      }
      else
      {
        writeError(ERR_DEBUG_MODULE, "Password-based authentication succeeded: Host: %s User: %s Pass: %s", (*psLogin)->psServer->pHostIP, szLogin, szPassword);
        (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
        iRet = MSTATE_EXITING;
      }
      break;

    default:
      writeError(ERR_ERROR, "No supported authentication methods located.");
      (*psLogin)->iResult = LOGIN_RESULT_UNKNOWN;
      (*psLogin)->iStatus = LOGIN_FAILED;
      iRet = MSTATE_EXITING;
      break;
  }
 
  setPassResult((*psLogin), szPassword);

  return(iRet);
}

#else

void summaryUsage(char **ppszSummary)
{
  // Memory for ppszSummary will be allocated here - caller is responsible for freeing it
  int  iLength = 0;
  

  if (*ppszSummary == NULL)
  {
    iLength = strlen(MODULE_SUMMARY_USAGE) + strlen(MODULE_VERSION) + strlen(MODULE_SUMMARY_FORMAT) + strlen(LIBSSH2_WARNING) + 1;
    *ppszSummary = (char*)malloc(iLength);
    memset(*ppszSummary, 0, iLength);
    snprintf(*ppszSummary, iLength, MODULE_SUMMARY_FORMAT_WARN, MODULE_SUMMARY_USAGE, MODULE_VERSION, LIBSSH2_WARNING);
  } 
  else 
  {
    writeError(ERR_ERROR, "%s reports an error in summaryUsage() : ppszSummary must be NULL when called", MODULE_NAME);
  }
}

void showUsage()
{
  writeVerbose(VB_NONE, "%s (%s) %s :: %s\n", MODULE_NAME, MODULE_VERSION, MODULE_AUTHOR, MODULE_SUMMARY_USAGE);
  writeVerbose(VB_NONE, "** Module was not properly built. Is libssh2 (www.libssh2.org) installed correctly? **");
  writeVerbose(VB_NONE, "");
}

int go(sLogin* logins, int argc, char *argv[])
{
  writeVerbose(VB_NONE, "%s (%s) %s :: %s\n", MODULE_NAME, MODULE_VERSION, MODULE_AUTHOR, MODULE_SUMMARY_USAGE);
  writeVerbose(VB_NONE, "** Module was not properly built. Is libssh2 (www.libssh2.org) installed correctly? **");
  writeVerbose(VB_NONE, "");
  return FAILURE;
}

#endif

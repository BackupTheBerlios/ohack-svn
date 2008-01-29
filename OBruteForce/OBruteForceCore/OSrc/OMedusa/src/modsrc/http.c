/***************************************************************************
 *   http.c                                                                *
 *   Copyright (C) 2006 by fizzgig                                         *
 *   fizzgig@foofus.net                                                    *
 *                                                                         *
 *   Implementation of a http brute force module for                       *
 *   medusa. Module concept by the one-and-only Foofus.                    *
 *   Protocol stuff based on the original hydra telnet code by             *
 *   VanHauser and the good folks at thc (vh@thc.org)                      *
 *                                                                         *
 *                                                                         *
 *   CHANGE LOG                                                            *
 *   04/15/2005 - Created by fizzgig (fizzgig@foofus.net)                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2,       *
 *   as published by the Free Software Foundation                          *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   http://www.gnu.org/licenses/gpl.txt                                   *
 *                                                                         *
 *   This program is released under the GPL with the additional exemption  *
 *   that compiling, linking, and/or using OpenSSL is allowed.             *
 *                                                                         *
 *   Modifications: (JoMo-Kun)                                             *
 *      Support for user specified URL/User-Agent                          *
 *      Replaced Base64 function from Hydra with Wget version              *
 *        Hydra Base64 appears partially broken...                         *
 *      Added NTLM authentication using code from Wget                     *
 *      Wget came be found here: http://wget.sunsite.dk/                   *
 *                                                                         *
 ***************************************************************************/

#include "module.h"

#define	MODULE_NAME		"http.mod"
#define MODULE_AUTHOR  "fizzgig <fizzgig@foofus.net>"
#define	MODULE_SUMMARY_USAGE	"Brute force module for HTTP"
#define MODULE_VERSION		"1.3.0"
#define MODULE_VERSION_SVN "$Id: http.c 606 2006-10-19 20:22:41Z jmk $"
#define MODULE_SUMMARY_FORMAT	"%s : version %s"
#define MODULE_SUMMARY_FORMAT_WARN  "%s : version %s (%s)"
#define OPENSSL_WARNING "No usable OPENSSL. Module disabled."

#ifdef HAVE_LIBSSL

#include "../http-ntlm.h"

#define HTTP_PORT 80
#define HTTPS_PORT 443

#define AUTH_UNKNOWN 0
#define AUTH_NONE 1
#define AUTH_BASIC 2
#define AUTH_NTLM 3

typedef struct __MODULE_DATA {
  char *szDomain;
  char *szDir;
  char *szHostHeader;
  char *szUserAgent;
  int nAuthType;
} _MODULE_DATA;

// Tells us whether we are to continue processing or not
enum HTTP_STATE
{
  HSTATE_NEW,
  HSTATE_RUNNING,
  HSTATE_EXITING
};

int nOptions = 0;

// Forward declarations
int getAuthType(int hSocket, _MODULE_DATA* _psSessionData, sLogin** login, int nPort);
int tryLogin(int hSocket, _MODULE_DATA* _psSessionData, sLogin** login, int nPort, char* szLogin, char* szPassword);
int initHttp(_MODULE_DATA* _psSessionData, sLogin* login);

// Tell medusa how many parameters this module allows
int getParamNumber()
{
  return 0;		// we don't need no stinking parameters
}

// Displays information about the module and how it must be used
void summaryUsage(char **ppszSummary)
{
  // Memory for ppszSummary will be allocated here - caller is responsible for freeing it
  int	iLength = 0;

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
  writeVerbose(VB_NONE, "  USER-AGENT:? (User-Agent. Default: Teh Forest Lobster.)");
  writeVerbose(VB_NONE, "  DIR:? (Target directory. Default \"/\"");
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "Usage example: \"-M http -m USER-AGENT:\"g3rg3 gerg\" -m DIR:exchange/\"");
}

// The "main" of the medusa module world - this is what gets called to actually do the work
int go(sLogin* logins, int argc, char *argv[])
{
  int i;
  char *strtok_ptr, *pOpt, *pOptTmp;
  _MODULE_DATA *psSessionData;
  psSessionData = malloc(sizeof(_MODULE_DATA));
  memset(psSessionData, 0, sizeof(_MODULE_DATA));

  psSessionData->szDomain = malloc(strlen("FOO") + 1);
  memset(psSessionData->szDomain, 0, strlen("FOO") + 1);
  sprintf(psSessionData->szDomain, "FOO"); 
 
  if ( !( 0 <= argc <= 2) )
  {
    // Show usage information
    writeError(ERR_ERROR, "%s: Incorrect number of parameters passed to module.", MODULE_NAME);
    return FAILURE;
  }
  else
  {
    // Parameters are good - make module go now
    writeError(ERR_DEBUG, "OMG teh %s module has been called!!", MODULE_NAME);
 
    for (i=0; i<argc; i++) {
      pOptTmp = malloc( strlen(argv[i]) + 1);
      memset(pOptTmp, 0, strlen(argv[i]) + 1);
      strncpy(pOptTmp, argv[i], strlen(argv[i]));
      writeError(ERR_DEBUG_MODULE, "Processing complete option: %s", pOptTmp);
      pOpt = strtok_r(pOptTmp, ":", &strtok_ptr);
      writeError(ERR_DEBUG_MODULE, "Processing option: %s", pOpt);

      if (strcmp(pOpt, "DIR") == 0)
      {
        pOpt = strtok_r(NULL, "\0", &strtok_ptr);
        writeError(ERR_DEBUG_MODULE, "Processing option parameter: %s", pOpt);

        if ( pOpt )
        {
          psSessionData->szDir = malloc(strlen(pOpt) + 1);
          memset(psSessionData->szDir, 0, strlen(pOpt) + 1);
          strncpy(psSessionData->szDir, pOpt, strlen(pOpt) + 1);
        }
        else
          writeError(ERR_WARNING, "Method DIR requires value to be set.");
      }
      else if (strcmp(pOpt, "USER-AGENT") == 0)
      {
        pOpt = strtok_r(NULL, "\0", &strtok_ptr);
        writeError(ERR_DEBUG_MODULE, "Processing option parameter: %s", pOpt);

        if ( pOpt )
        {
          psSessionData->szUserAgent = malloc(strlen(pOpt) + 1);
          memset(psSessionData->szUserAgent, 0, strlen(pOpt) + 1);
          strncpy(psSessionData->szUserAgent, pOpt, strlen(pOpt) + 1);
        }
        else
          writeError(ERR_WARNING, "Method USER-AGENT requires value to be set.");
      }
      else
      {
        writeError(ERR_WARNING, "Invalid method: %s.", pOpt);
      }

      free(pOptTmp);
    }

    initHttp(psSessionData, logins);
  }

  return SUCCESS;
}

int initHttp(_MODULE_DATA *_psSessionData, sLogin* login)
{
  int hSocket = -1;
  enum HTTP_STATE nState = HSTATE_NEW;
  char* bufReceive;
  int nReceiveBufferSize = 0, nFirstPass = 0, nFoundPrompt = 0;
  int i = 0;
  char *pPass;
  int nPort, nBufLength = 0;
  sUser* user = login->psUser;
  sConnectParams params;
  
  memset(&params, 0, sizeof(sConnectParams));
  if (login->psServer->psAudit->iPortOverride > 0)
    params.nPort = login->psServer->psAudit->iPortOverride;
  else if (login->psServer->psHost->iUseSSL > 0)
    params.nPort = HTTPS_PORT;
  else
    params.nPort = HTTP_PORT; 
  initConnectionParams(login, &params);

  if (user != NULL)
  {
    writeError(ERR_DEBUG, "[%s] module started for host: %s:%d, user: '%s'", MODULE_NAME, login->psServer->pHostIP, params.nPort, user->pUser);
  }
  else
  {
    writeError(ERR_DEBUG, "null user");
  }
  
  pPass = getNextPass(login->psServer->psAudit, user);
  if (pPass == NULL)
  {
    writeVerbose(VB_GENERAL, "[%s] out of passwords for user '%s' at host '%s', bailing", MODULE_NAME, user->pUser, login->psServer->pHostIP);
  }

  while(NULL != pPass)
  {
    switch(nState)
    {
    case HSTATE_NEW:
      nFirstPass = 0;

      // Already have an open socket - close it
      if (hSocket > 0)
        medusaDisconnect(hSocket);

      if (login->psServer->psHost->iUseSSL > 0)
        hSocket = medusaConnectSSL(&params);
      else
        hSocket = medusaConnect(&params);

      if (hSocket < 0)
      {
        writeError(ERR_NOTICE, "%s: failed to connect, port %d was not open on %s", MODULE_NAME, params.nPort, login->psServer->pHostIP);
        login->iResult = LOGIN_RESULT_UNKNOWN;
        login->iStatus = LOGIN_DONE;
        setPassResult(login, pPass);
        return FAILURE;
      }

      /* Set request parameters */
      if (!_psSessionData->szDir) {
        _psSessionData->szDir = malloc(1);
        memset(_psSessionData->szDir, 0, 1);
      }

      if (!_psSessionData->szHostHeader) {
        
        nBufLength = strlen(login->psServer->pHostIP) + 1 + log(params.nPort) + 1;
        _psSessionData->szHostHeader = malloc(nBufLength + 1);
        memset(_psSessionData->szHostHeader, 0, nBufLength + 1);
        sprintf(_psSessionData->szHostHeader, "%s:%d", login->psServer->pHostIP, params.nPort);
      }
      
      if (!_psSessionData->szUserAgent) {
        _psSessionData->szUserAgent = malloc(19);
        memset(_psSessionData->szUserAgent, 0, 19);
        sprintf(_psSessionData->szUserAgent, "Teh Forest Lobster");
      }

      /* Get required authorization method */
      if (_psSessionData->nAuthType == AUTH_UNKNOWN)
      {
        getAuthType(hSocket, _psSessionData, &login, params.nPort);
        
        if (_psSessionData->nAuthType == AUTH_UNKNOWN)
        {
          login->iResult = LOGIN_RESULT_UNKNOWN;
          login->iStatus = LOGIN_DONE;
          return FAILURE;
        }
        
        nState = HSTATE_NEW;
      }
      else
      nState = HSTATE_RUNNING;
      
      break;
    case HSTATE_RUNNING:
      nState = tryLogin(hSocket, _psSessionData, &login, params.nPort, user->pUser, pPass);
      medusaDisconnect(hSocket);
      hSocket = -1;
      pPass = getNextPass(login->psServer->psAudit, user);
      if (pPass == NULL)
      {
        writeError(ERR_DEBUG_MODULE, "Done with passwords, exiting http");
        nState = HSTATE_EXITING;
      }
      else
      {
        nState = HSTATE_NEW;
      }
      break;
    case HSTATE_EXITING:
      if (hSocket > 0)
        medusaDisconnect(hSocket);
      hSocket = -1;
    default:
      writeError(ERR_CRITICAL, "Unknown HTTP module state (%d). Exiting...", nState);
      login->iResult = LOGIN_RESULT_UNKNOWN;
      login->iStatus = LOGIN_DONE;
      setPassResult(login, pPass);
    }
  }

  login->iStatus = LOGIN_DONE;
  return SUCCESS;
}

/* Module Specific Functions */

int getAuthType(int hSocket, _MODULE_DATA* _psSessionData, sLogin** login, int nPort)
{
  int iRet;
  unsigned char* bufSend = NULL;
  unsigned char* bufReceive = NULL;
  int nReceiveBufferSize = 0;
  int nSendBufferSize = 0;

  nSendBufferSize = 5 + strlen(_psSessionData->szDir) + 17 + strlen(_psSessionData->szHostHeader) +
                    14 + strlen(_psSessionData->szUserAgent) + 4; 
 
  bufSend = malloc(nSendBufferSize + 1);
  memset(bufSend, 0, nSendBufferSize + 1);

  sprintf(bufSend, "GET /%s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: %s\r\n\r\n", 
          _psSessionData->szDir, (*login)->psServer->pHostIP, nPort, _psSessionData->szUserAgent);

  writeError(ERR_DEBUG_MODULE, "[%s] Sending initial non-authentication request: %s", MODULE_NAME, bufSend);
  if (medusaSend(hSocket, bufSend, nSendBufferSize, 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
    free(bufSend);
    return FAILURE;
  }
  
  free(bufSend);

  bufReceive = medusaReceiveLine(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "No data received", MODULE_NAME);
    return FAILURE;
  }
  
  while (((char *)strcasestr(bufReceive, "WWW-Authenticate") == NULL) && (bufReceive[0] != '\0'))
  {
    free(bufReceive);
    bufReceive = medusaReceiveLine(hSocket, &nReceiveBufferSize);
  }

  writeError(ERR_DEBUG_MODULE, "[%s] Parsing authentication header: %s", MODULE_NAME, bufReceive);
  if (strcasestr(bufReceive, "WWW-Authenticate: Basic"))
  {
    writeError(ERR_DEBUG_MODULE, "[%s] Server requested basic authentication.", MODULE_NAME);
    _psSessionData->nAuthType = AUTH_BASIC;
  }
  else if (strcasestr(bufReceive, "WWW-Authenticate: NTLM"))
  {
    writeError(ERR_DEBUG_MODULE, "[%s] Server requested integrated windows authentication.", MODULE_NAME);
    _psSessionData->nAuthType = AUTH_NTLM;
  }
  else if (strcasestr(bufReceive, "WWW-Authenticate:"))
  {
    writeError(ERR_ERROR, "[%s] Server requested unknown authentication type.", MODULE_NAME);
    _psSessionData->nAuthType = AUTH_UNKNOWN;
  }
  else
  {
    writeError(ERR_DEBUG_MODULE, "[%s] No authentication header located.", MODULE_NAME);
    _psSessionData->nAuthType = AUTH_NONE;
  }

  free(bufReceive);
  return(SUCCESS);
}

int sendAuthBasic(int hSocket, _MODULE_DATA* _psSessionData, char* szLogin, char* szPassword)
{
  unsigned char* bufSend;
  unsigned char* bufReceive;
  unsigned char* szEncodedAuth;
  int nReceiveBufferSize = 0;
  int nSendBufferSize = 0;
  int nRet = SUCCESS;

  writeError(ERR_DEBUG_MODULE, "[%s] Base64 encoding: %s:%s", MODULE_NAME, szLogin, szPassword);
  szEncodedAuth = basic_authentication_encode(szLogin, szPassword);
  writeError(ERR_DEBUG_MODULE, "[%s] Base64 encoded data is: %s", MODULE_NAME, szEncodedAuth);
  
  nSendBufferSize = 5 + strlen(_psSessionData->szDir) + 17 + strlen(_psSessionData->szHostHeader) +
                    14 + strlen(_psSessionData->szUserAgent) + 23 + strlen(szEncodedAuth) + 4;
 
  bufSend = malloc(nSendBufferSize + 1);
  memset(bufSend, 0, nSendBufferSize + 1);

  sprintf(bufSend, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nAuthorization: Basic %s\r\n\r\n", 
          _psSessionData->szDir, _psSessionData->szHostHeader, _psSessionData->szUserAgent, szEncodedAuth);
  
  if (medusaSend(hSocket, bufSend, nSendBufferSize, 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
    nRet = FAILURE;  
  }
  
  free(szEncodedAuth);
  free(bufSend);
  return nRet;
}

int sendAuthNTLM(int hSocket, _MODULE_DATA* _psSessionData, char* szLogin, char* szPassword)
{
  unsigned char* bufSend;
  unsigned char* bufReceive;
  int nReceiveBufferSize = 0;
  int nSendBufferSize = 0;
  int i, nState;
  struct ntlmdata ntlm;
  unsigned char* szEncodedAuth, *szTmp;

  nState = 0;
  memset(&ntlm, 0, sizeof(ntlm));
  
  /* Send NTLM request */
  szEncodedAuth = ntlm_output(&ntlm, szLogin, szPassword, &nState);
  if (szEncodedAuth == NULL)
  {
    writeError(ERR_ERROR, "[%s] Failed to create initial NTLM request.", MODULE_NAME);
    return FAILURE;
  }
  writeError(ERR_DEBUG_MODULE, "[%s] Sending initial challenge (B64 Encoded): %s", MODULE_NAME, szEncodedAuth);
  
  nSendBufferSize = 5 + strlen(_psSessionData->szDir) + 17 + strlen(_psSessionData->szHostHeader) +
                    14 + strlen(_psSessionData->szUserAgent) + 17 + strlen(szEncodedAuth) + 28;
 
  bufSend = malloc(nSendBufferSize + 1);
  memset(bufSend, 0, nSendBufferSize + 1);

  sprintf(bufSend, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nAuthorization: %s\r\nConnection: Keep-Alive\r\n\r\n", 
          _psSessionData->szDir, _psSessionData->szHostHeader, _psSessionData->szUserAgent, szEncodedAuth);
  
  if (medusaSend(hSocket, bufSend, nSendBufferSize, 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
    return FAILURE;
  }
  
  free(szEncodedAuth);
  free(bufSend);


  /* Retrieve NTLM challenge from server */
  bufReceive = medusaReceiveLine(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "[%s] No data received", MODULE_NAME);
    return FAILURE;
  }
  
  while (((char *)strcasestr(bufReceive, "WWW-Authenticate: NTLM ") == NULL) && (bufReceive[0] != '\0'))
  {
    free(bufReceive);
    bufReceive = medusaReceiveLine(hSocket, &nReceiveBufferSize);
  }

  if (bufReceive[0] == '\0')
  {
    writeError(ERR_ERROR, "[%s] Failed to locate NTLM challenge.", MODULE_NAME);
    return FAILURE;
  }

  /* Extract NTLM challenge from response */
  szEncodedAuth = (char *)strcasestr(bufReceive, "WWW-Authenticate: NTLM ");
  szEncodedAuth += 18;
  szTmp = ((char*)index(szEncodedAuth, '\r'));
  szTmp[0] = '\0';
  writeError(ERR_DEBUG_MODULE, "[%s] NTLM Challenge (B64 Encoded): %s", MODULE_NAME, szEncodedAuth);
  
  if (ntlm_input(&ntlm, szEncodedAuth) == 0)
  {
    writeError(ERR_ERROR, "[%s] Failed to process NTLM response.", MODULE_NAME);
    return FAILURE;
  }
 
  free(bufReceive);
  
 
  /* Send NTLM response */
  nState = 1;
  szEncodedAuth = ntlm_output(&ntlm, szLogin, szPassword, &nState);
  if (szEncodedAuth == NULL)
  {
    writeError(ERR_ERROR, "[%s] Failed to create NTLM response.", MODULE_NAME);
    return FAILURE;
  }
  writeError(ERR_DEBUG_MODULE, "[%s] NTLM Response (B64 Encoded): %s", MODULE_NAME, szEncodedAuth);
  
  nSendBufferSize = 5 + strlen(_psSessionData->szDir) + 17 + strlen(_psSessionData->szHostHeader) +
                    14 + strlen(_psSessionData->szUserAgent) + 17 + strlen(szEncodedAuth) + 28;
 
  bufSend = malloc(nSendBufferSize + 1);
  memset(bufSend, 0, nSendBufferSize + 1);

  sprintf(bufSend, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nAuthorization: %s\r\nConnection: Keep-Alive\r\n\r\n", 
          _psSessionData->szDir, _psSessionData->szHostHeader, _psSessionData->szUserAgent, szEncodedAuth);
  
  if (medusaSend(hSocket, bufSend, nSendBufferSize, 0) < 0)
  {
    writeError(ERR_ERROR, "[%s] failed: medusaSend was not successful", MODULE_NAME);
    return FAILURE;
  }

  free(szEncodedAuth);
  free(bufSend);

  return SUCCESS;
}

int tryLogin(int hSocket, _MODULE_DATA* _psSessionData, sLogin** login, int nPort, char* szLogin, char* szPassword)
{
  char* pReceiveBuffer;
  int nReceiveBufferSize, nRet;
  char* pTemp;

  switch(_psSessionData->nAuthType)
  {
    case AUTH_NONE:
      writeError(ERR_DEBUG_MODULE, "[%s] No authentication required.", MODULE_NAME);
      (*login)->iResult = LOGIN_RESULT_SUCCESS;
      setPassResult(*login, szPassword);
      return HSTATE_RUNNING;  
      break;
    case AUTH_BASIC:
      writeError(ERR_DEBUG_MODULE, "[%s] Sending Basic Authentication.", MODULE_NAME);
      nRet = sendAuthBasic(hSocket, _psSessionData, szLogin, szPassword);
      break;
    case AUTH_NTLM:
      writeError(ERR_DEBUG_MODULE, "[%s] Sending Windows Integrated (NTLM) Authentication.", MODULE_NAME);
      nRet = sendAuthNTLM(hSocket, _psSessionData, szLogin, szPassword);
      break;
    default:
      break;
  }

  if (nRet == FAILURE)
  {
    writeError(ERR_ERROR, "[%s] Failed during sending of authentication data.", MODULE_NAME);
    (*login)->iResult = LOGIN_RESULT_UNKNOWN;
    setPassResult(*login, szPassword);
    return HSTATE_RUNNING;  
  }

  writeError(ERR_DEBUG_MODULE, "[%s] Retrieving server response.", MODULE_NAME);
  pReceiveBuffer = medusaReceiveLine(hSocket, &nReceiveBufferSize);
  if (pReceiveBuffer == NULL)
  {
    writeError(ERR_ERROR, "[%s] No data received", MODULE_NAME);
    (*login)->iResult = LOGIN_RESULT_UNKNOWN;
    setPassResult(*login, szPassword);
    return HSTATE_RUNNING;  
  }
    
  while (strstr(pReceiveBuffer, "HTTP/1.") == NULL && (pReceiveBuffer[0] != '\0'))
  {
    free(pReceiveBuffer);
    pReceiveBuffer = medusaReceiveLine(hSocket, &nReceiveBufferSize);
  }

  if ((pReceiveBuffer == NULL) || (pReceiveBuffer[0] == '\0'))
  {
    writeError(ERR_ERROR, "[%s] No data received", MODULE_NAME);
    (*login)->iResult = LOGIN_RESULT_UNKNOWN;
    setPassResult(*login, szPassword);
    return HSTATE_RUNNING;  
  }

  pTemp = ((char*)index(pReceiveBuffer, ' ')) + 1;
  if (*pTemp == '2' || strncmp(pTemp, "301", 3) == 0 ) 
  {
    (*login)->iResult = LOGIN_RESULT_SUCCESS;
    setPassResult(*login, szPassword);
    return HSTATE_RUNNING;
  } 
  else 
  {
    if (*pTemp != '4')
    {
      writeVerbose(VB_GENERAL, "Unusual return code for %s:%s (%.3s)", szLogin, szPassword, pTemp);
      (*login)->iResult = LOGIN_RESULT_UNKNOWN;
      setPassResult(*login, szPassword);
      return HSTATE_RUNNING;   
    }
    
    if (strncmp(pTemp, "404", 3) == 0)
    {
      writeVerbose(VB_GENERAL, "Page not found at %s:%d", (*login)->psServer->pHostIP, nPort);
      (*login)->iResult = LOGIN_RESULT_FAIL;
      setPassResult(*login, szPassword);
      return HSTATE_RUNNING;   
    }
    
    writeError(ERR_DEBUG, "Login failed: error code was %.3s", pTemp);
    (*login)->iResult = LOGIN_RESULT_FAIL;
    setPassResult(*login, szPassword);
    return HSTATE_RUNNING;   
  }
}

#else

void summaryUsage(char **ppszSummary)
{
  // Memory for ppszSummary will be allocated here - caller is responsible for freeing it
  int  iLength = 0;

  if (*ppszSummary == NULL)
  {
    iLength = strlen(MODULE_SUMMARY_USAGE) + strlen(MODULE_VERSION) + strlen(MODULE_SUMMARY_FORMAT) + strlen(OPENSSL_WARNING) + 1;
    *ppszSummary = (char*)malloc(iLength);
    memset(*ppszSummary, 0, iLength);
    snprintf(*ppszSummary, iLength, MODULE_SUMMARY_FORMAT_WARN, MODULE_SUMMARY_USAGE, MODULE_VERSION, OPENSSL_WARNING);
  }
  else
  {
    writeError(ERR_ERROR, "%s reports an error in summaryUsage() : ppszSummary must be NULL when called", MODULE_NAME);
  }
}

void showUsage()
{
  writeVerbose(VB_NONE, "%s (%s) %s :: %s\n", MODULE_NAME, MODULE_VERSION, MODULE_AUTHOR, MODULE_SUMMARY_USAGE);
  writeVerbose(VB_NONE, "** Module was not properly built. Is OPENSSL installed correctly? **");
  writeVerbose(VB_NONE, "");
  return FAILURE;
}

int go(sLogin* logins, int argc, char *argv[])
{
  writeVerbose(VB_NONE, "%s (%s) %s :: %s\n", MODULE_NAME, MODULE_VERSION, MODULE_AUTHOR, MODULE_SUMMARY_USAGE);
  writeVerbose(VB_NONE, "** Module was not properly built. Is OPENSSL installed correctly? **");
  writeVerbose(VB_NONE, "");
  return FAILURE;
}

#endif

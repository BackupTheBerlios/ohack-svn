/***************************************************************************
 *   telnet.c                                                              *
 *   Copyright (C) 2006 by fizzgig                                         *
 *   fizzgig@foofus.net                                                    *
 *                                                                         *
 *   Implementation of a telnet brute force module for                     *
 *   medusa. Module concept by the one-and-only Foofus.                    *
 *   Protocol stuff based on the original hydra telnet code by             *
 *   VanHauser and the good folks at thc (vh@thc.org)                      *
 *                                                                         *
 *                                                                         *
 *   CHANGE LOG                                                            *
 *   04/05/2005 - Created by fizzgig (fizzgig@foofus.net)                  *
 *   All other changes are in the Subversion comments                      *
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
 *   Tested on so far:                                                     *
 *       Jetdirect cards (woo!)                                            *
 *       HP switches using basic auth                                      *
 *       Cisco switches using vty auth                                     *
 *                                                                         *
 *    Support for hosts w/o username prompt added by pMonkey/JoMo-Kun      *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/telnet.h>
#include "module.h"

#define  MODULE_NAME    "telnet.mod"
#define MODULE_AUTHOR  "fizzgig <fizzgig@foofus.net>"
#define  MODULE_SUMMARY_USAGE  "Brute force module for telnet sessions"
#define MODULE_VERSION    "1.2.1"
#define MODULE_VERSION_SVN "$Id: telnet.c 606 2006-10-19 20:22:41Z jmk $"
#define MODULE_SUMMARY_FORMAT  "%s : version %s"

#define PORT_TELNET 23
#define PORT_TELNETS 992

#define PROMPT_UNKNOWN 0
#define PROMPT_LOGIN_PASSWORD 1
#define PROMPT_PASSWORD 2

#define RECEIVE_DELAY_1 20 * 1000000
#define RECEIVE_DELAY_2 0.5 * 1000000

const unsigned int BUFFER_SIZE = 300;
const char* KNOWN_PROMPTS = ">#$%/";  // Each character represents a known telnet prompt - feel free to add a new one if desired

const int KNOWN_PWD_SIZE = 3;  // Make sure to keep this in sync with the size of the array below!!
const char* KNOWN_PWD_PROMPTS[] = { "assword", "asscode", "ennwort" };  // Complete/partial lines that indicate a password request

//const int KNOWN_LOGIN_SIZE = 4;  // Make sure to keep this in sync with the size of the array below!!
//const char* KNOWN_LOGIN_PROMPTS[] = { "login:", "last login", "sername:", "User" }; // Complete/partial lines that request a user name
const int KNOWN_LOGIN_SIZE = 3;  // Make sure to keep this in sync with the size of the array below!!
const char* KNOWN_LOGIN_PROMPTS[] = { "login:", "sername:", "User" }; // Complete/partial lines that request a user name

// Tells us whether we are to continue processing or not
enum TELNET_STATE
{
  TSTATE_NEW,
  TSTATE_RUNNING,
  TSTATE_EXITING
};

// Forward declarations
int tryLogin(int hSocket, sLogin** login, char* szLogin, char* szPassword, int nFoundPrompt);
int initTelnet(sLogin* login);
void processIAC(int hSocket, char** buffer, int* nBufferSize);

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
  if (argc != 0)
  {
    // Show usage information
    writeError(ERR_ERROR, "%s is expecting 0 parameters, but it was passed %d", MODULE_NAME, argc);
    return FAILURE;
  }
  else
  {
    writeError(ERR_DEBUG_MODULE, "OMG teh %s module has been called!!", MODULE_NAME);
    initTelnet(logins);
  }

  return SUCCESS;
}

int initTelnet(sLogin* login)
{
  int hSocket = -1;
  enum TELNET_STATE nState = TSTATE_NEW;
  char* bufReceive;
  int nReceiveBufferSize = 0, nFoundPrompt = PROMPT_UNKNOWN;
  int i = 0;
  char *pPass;
  int nPort, nOptions = 0;
  sUser* user = login->psUser;
  sConnectParams params;

  memset(&params, 0, sizeof(sConnectParams));
  if (login->psServer->psAudit->iPortOverride > 0)
    params.nPort = login->psServer->psAudit->iPortOverride;
  else if (login->psServer->psHost->iUseSSL > 0)
    params.nPort = PORT_TELNETS;
  else
    params.nPort = PORT_TELNET;
  initConnectionParams(login, &params);

  if (user != NULL)
  {
    writeError(ERR_DEBUG_MODULE, "[%s] module started for host: %s:%d, user: '%s'", MODULE_NAME, login->psServer->pHostIP, params.nPort, user->pUser);
  }
  else
  {
    writeError(ERR_DEBUG_MODULE, "null user");
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
    case TSTATE_NEW:
      // Already have an open socket - close it
      if (hSocket > 0)
        medusaDisconnect(hSocket);

      if (login->psServer->psHost->iUseSSL > 0)
        hSocket = medusaConnectSSL(&params);
      else               
        hSocket = medusaConnect(&params);
      
      if (hSocket <= 0)
      {
        writeError(ERR_NOTICE, "%s: failed to connect, port %d was not open on %s", MODULE_NAME, params.nPort, login->psServer->pHostIP);
        login->iResult = LOGIN_RESULT_UNKNOWN;
        login->iStatus = LOGIN_DONE;
        setPassResult(login, pPass);
        return FAILURE;
      }

      writeError(ERR_DEBUG_MODULE, "Connected");

      // Examine the first line returned
      nReceiveBufferSize = 0;
      bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
      if (bufReceive == NULL)
        return FAILURE;

      bufReceive[nReceiveBufferSize] = 0;  // Make certain buffer is null-terminated

      if (bufReceive == NULL)
      {
        writeError(ERR_NOTICE, "null response was unexpected from a telnet server (is one running?)");
        login->iResult = LOGIN_RESULT_UNKNOWN;
        login->iStatus = LOGIN_DONE;
        setPassResult(login, pPass);
        return FAILURE;
      }

      // Telnet protocol negotiation
      do
      {
        nFoundPrompt = PROMPT_UNKNOWN;
        processIAC(hSocket, &bufReceive, &nReceiveBufferSize);

        if (bufReceive != NULL && bufReceive[0] != 0 && (unsigned char)bufReceive[0] != IAC)
          makeToLower(bufReceive);

        if (bufReceive != NULL)
        {
          writeError(ERR_DEBUG_MODULE, "Looking for login prompts");
          
          // Look for known login prompts
          for (i = 0; i < KNOWN_LOGIN_SIZE; i++)
          {
            if (strstr(bufReceive, KNOWN_LOGIN_PROMPTS[i]) != NULL)
            {
              // Do we have a prompt?
              writeError(ERR_DEBUG_MODULE, "Found login prompt...");
              nFoundPrompt = PROMPT_LOGIN_PASSWORD;
              break;
            }
          }
          
          /* Some systems do not provide a login prompt and go right to password */
          for (i = 0; i < KNOWN_PWD_SIZE; i++)
          {
            if (strstr(bufReceive, KNOWN_PWD_PROMPTS[i]) != NULL)
            {
              // Do we have a prompt?
              writeError(ERR_DEBUG_MODULE, "Found a password prompt already...");
              nFoundPrompt = PROMPT_PASSWORD;
              break;
            }
          }
          
          if (nFoundPrompt == PROMPT_UNKNOWN)
          {
            if (medusaDataReadyTimed(hSocket, 0, 20000) > 0)
            {
              // More data waiting
              bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
              if (bufReceive != NULL)
                bufReceive[nReceiveBufferSize] = 0;  // Make certain buffer is null-terminated
            } 
          }
        }
      }
      while (bufReceive != NULL && (unsigned char)bufReceive[0] == IAC && nFoundPrompt == PROMPT_UNKNOWN);

      if (bufReceive != NULL)
        free(bufReceive);

      nState = TSTATE_RUNNING;
      break;

    case TSTATE_RUNNING:
      nState = tryLogin(hSocket, &login, user->pUser, pPass, nFoundPrompt);
      if (nState == TSTATE_EXITING)
      {
        pPass = NULL;
      }
      else
      {
        medusaDisconnect(hSocket);
        hSocket = -1;
        pPass = getNextPass(login->psServer->psAudit, user);
        if (pPass == NULL)
        {
          writeVerbose(VB_GENERAL, "Done with passwords, exiting telnet");
          nState = TSTATE_EXITING;
        }
        else
        {
          nState = TSTATE_NEW;
        }
      }
      break;
    case TSTATE_EXITING:
      if (hSocket > 0)
        medusaDisconnect(hSocket);
      hSocket = -1;
    default:
      writeError(ERR_CRITICAL, "unknown telnet module state");
      login->iResult = LOGIN_RESULT_UNKNOWN;
      login->iStatus = LOGIN_DONE;
      setPassResult(login, pPass);
    }
  }

  login->iStatus = LOGIN_DONE;
  return SUCCESS;
}

int tryLogin(int hSocket, sLogin** login, char* szLogin, char* szPassword, int nFoundPrompt)
{
  // This function should return TSTATE_RUNNING to continue or TSTATE_EXITING to terminate the module
  char bufSend[BUFFER_SIZE];
  char* bufReceive;
  int nSendBufferSize = 0, nReceiveBufferSize = 0;
  int nContinue = 0, i = 0;

  // Check the socket and such
  if (hSocket <= 0)
  {
    writeError(ERR_ERROR, "%s failed: socket was invalid", MODULE_NAME);
    (*login)->iResult = LOGIN_RESULT_UNKNOWN;
    setPassResult(*login, szPassword);
    return TSTATE_EXITING;    // No good socket
  }

  if (nFoundPrompt == PROMPT_LOGIN_PASSWORD)
  {
    // Set up the send buffer
    memset(bufSend, 0, BUFFER_SIZE);
    sprintf(bufSend, "%s\r", szLogin);
    nSendBufferSize = strlen(bufSend) + 1;  // Count the null terminator

    if (medusaSend(hSocket, bufSend, nSendBufferSize, 0) < 0)
    {
      writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
      (*login)->iResult = LOGIN_RESULT_UNKNOWN;
      setPassResult(*login, szPassword);
      return TSTATE_EXITING;
    }

    do
    {
      // Look for a return
      bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
      if (bufReceive == NULL)
      {
        // Found a prompt - telnet appears to be alive, keep going
        writeError(ERR_ERROR, "%s: Telnet did not respond to the sending of the user name '%s' in a timely fashion - is it down or refusing connections?", MODULE_NAME, szLogin);
        (*login)->iResult = LOGIN_RESULT_UNKNOWN;
        setPassResult(*login, szPassword);
        return TSTATE_EXITING;
      }

      bufReceive[nReceiveBufferSize] = 0;  // Make certain buffer is null-terminated

      // Do we have a prompt?
      if (strcspn(bufReceive, KNOWN_PROMPTS) != strlen(bufReceive))
      {
        (*login)->iResult = LOGIN_RESULT_SUCCESS;
        setPassResult(*login, szPassword);
        free(bufReceive);
        return TSTATE_EXITING;
      }

      makeToLower(bufReceive);

      // Are we at a known password prompt?
      for (i = 0; i < KNOWN_PWD_SIZE; i++)
      {
        if (strstr(bufReceive, KNOWN_PWD_PROMPTS[i]) != NULL)
        {
          nContinue = 1;
          break;
        }
      }

      // Look for known login prompts
      if (nContinue == 0)
      {
        for (i = 0; i < KNOWN_LOGIN_SIZE; i++)
        {
          if (strstr(bufReceive, KNOWN_LOGIN_PROMPTS[i]) != NULL)
          {
            free(bufReceive);
            (*login)->iResult = LOGIN_RESULT_FAIL;
            setPassResult(*login, szPassword);
            return TSTATE_RUNNING;
          }
        }
      }

      free(bufReceive);
    }
    while(nContinue == 0);
  }
  else if (nFoundPrompt == PROMPT_PASSWORD)
  {
    writeError(ERR_DEBUG_MODULE, "%s we are skipping a username", MODULE_NAME);
  }
  else
  {
    writeError(ERR_ERROR, "%s: No login prompt detected.", MODULE_NAME);
    return FAILURE;
  }

  // Send the password
  memset(bufSend, 0, BUFFER_SIZE);
  sprintf(bufSend, "%s\r", szPassword);
  nSendBufferSize = strlen(bufSend) + 1;  // Count the null terminator

  if (medusaSend(hSocket, bufSend, nSendBufferSize, 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
    (*login)->iResult = LOGIN_RESULT_UNKNOWN;
    setPassResult(*login, szPassword);
    return TSTATE_EXITING;
  }

  // Look for a return
  bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "timeout waiting for response from server after sending password");
    (*login)->iResult = LOGIN_RESULT_UNKNOWN;
    setPassResult(*login, szPassword);
    return TSTATE_EXITING;
  }

  bufReceive[nReceiveBufferSize] = 0;  // Make certain buffer is null-terminated

  // It's possible that some telnet servers (like Microsoft's) may send some more IAC commands at this point
  // Take care of zem!
  processIAC(hSocket, &bufReceive, &nReceiveBufferSize);

  if (bufReceive == NULL)
    bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);

  // Do we have a prompt?
  while (bufReceive != NULL)
  {
    /* check for known failures */
    if (strstr(bufReceive, "% Authentication failed."))
    {
      writeError(ERR_DEBUG, "Server responded with Cisco \"Authentication failed.\" message.");
      (*login)->iResult = LOGIN_RESULT_FAIL;
      setPassResult(*login, szPassword);
      return TSTATE_NEW;
    }
    else if (strcspn(bufReceive, KNOWN_PROMPTS) != strlen(bufReceive))
    {
      // Found a prompt - telnet appears to be alive
      free(bufReceive);
      (*login)->iResult = LOGIN_RESULT_SUCCESS;
      setPassResult(*login, szPassword);
      return TSTATE_EXITING;
    }
    else
    {
      if (nFoundPrompt == PROMPT_LOGIN_PASSWORD) {
        // If we have a login prompt, we have failed
        for (i = 0; i < KNOWN_LOGIN_SIZE; i++)
        {
          if (strstr(bufReceive, KNOWN_LOGIN_PROMPTS[i]) != NULL)
          {
            free(bufReceive);
            writeError(ERR_DEBUG_MODULE, "unsuccessful login - user '%s' with a password of '%s'", szLogin, szPassword);
            (*login)->iResult = LOGIN_RESULT_FAIL;
            setPassResult(*login, szPassword);
            return TSTATE_NEW;
          }
        }
      } 
      else 
      {
        // We are operating on a non-login telnet setup
        for (i = 0; i < KNOWN_PWD_SIZE; i++)
        {
          if (strstr(bufReceive, KNOWN_PWD_PROMPTS[i]) != NULL)
          {
            free(bufReceive);
            writeError(ERR_DEBUG_MODULE, "unsuccessful login with a password of '%s'", szPassword);
            (*login)->iResult = LOGIN_RESULT_FAIL;
            setPassResult(*login, szPassword);
            return TSTATE_NEW;
          }
        }
      }
    }

    free(bufReceive);
    bufReceive = medusaReceiveLineDelay(hSocket, &nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
  }

  (*login)->iResult = LOGIN_RESULT_FAIL;
  setPassResult(*login, szPassword);
  return TSTATE_NEW;
}

void processIAC(int hSocket, char** buffer, int* nReceiveBufferSize)
{
  unsigned char* bufTemp = (unsigned char*)*buffer;

  while (*bufTemp == IAC)
  {
    writeError(ERR_DEBUG_MODULE, "Handling command...");

    if ((bufTemp[1] == 0xfc || bufTemp[1] == 0xfe) && bufTemp[2] == 0x22)
    {
      writeError(ERR_DEBUG_MODULE, "TELNETD peer does not like linemode");
    }
    if (bufTemp[1] == WILL || bufTemp[1] == WONT)
    {
      bufTemp[1] = DONT;
      medusaSend(hSocket, bufTemp, 3, 0);
    }
    else if (bufTemp[1] == DO || bufTemp[1] == DONT)
    {
      bufTemp[1] = WONT;
      medusaSend(hSocket, bufTemp, 3, 0);
    }
    bufTemp += 3; // Read three bytes - move on
  }

  if (*bufTemp == 0)
  {
    writeError(ERR_DEBUG_MODULE, "Getting more data");
    free(*buffer);

    *nReceiveBufferSize = 0;
    *buffer = medusaReceiveLineDelay(hSocket, nReceiveBufferSize, RECEIVE_DELAY_1, RECEIVE_DELAY_2);
    if (*buffer != NULL)
      (*buffer)[*nReceiveBufferSize] = 0;  // Make certain buffer is null-terminated
    else
    {
      // No data
      *buffer = NULL;
      return;
    }

    writeError(ERR_DEBUG_MODULE, "Next pass buffer: %s", *buffer);
    if ((unsigned char)*buffer[0] == IAC)
    {
      writeError(ERR_DEBUG_MODULE, "More commands waiting...");
    }
  }
}

/*
**   MySQL Password Checking Medusa Module
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
**    MySQL 4.1 authentication support added by: pmonkey <pmonkey@foofus.net>
**
*/

#include <sys/types.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "module.h"

#define MODULE_NAME    "mysql.mod"
#define MODULE_AUTHOR  "JoMo-Kun <jmk@foofus.net>"
#define MODULE_SUMMARY_USAGE  "Brute force module for MySQL sessions"
#define MODULE_VERSION    "1.1.2"
#define MODULE_VERSION_SVN "$Id: mysql.c 606 2006-10-19 20:22:41Z jmk $"
#define MODULE_SUMMARY_FORMAT  "%s : version %s"
#define BUF_SIZE 300

#define FREE(x) \
        if (x != NULL) { \
           free(x); \
           x = NULL; \
        }

#define PORT_MYSQL 3306
#define PROTO_NEW 0
#define PROTO_OLD 1

typedef struct __MYSQL_DATA {
  int protoFlag;
} _MYSQL_DATA;


// Tells us whether we are to continue processing or not
enum MODULE_STATE
{
  MSTATE_NEW,
  MSTATE_RUNNING,
  MSTATE_EXITING
};

// Forward declarations
int tryLogin(int hSocket, sLogin** login, _MYSQL_DATA* _psSessionData, char* szLogin, char* szPassword);
int initModule(sLogin* login, _MYSQL_DATA *_psSessionData);

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
  _MYSQL_DATA *psSessionData;

  psSessionData = malloc(sizeof(_MYSQL_DATA));
  memset(psSessionData, 0, sizeof(_MYSQL_DATA));
  psSessionData->protoFlag = PROTO_NEW;

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

int initModule(sLogin* psLogin, _MYSQL_DATA *_psSessionData)
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
    params.nPort = PORT_MYSQL;
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

/* Module Specific Functions */
int MySQLSessionQuit(int hSocket)
{
  char com_quit_packet[5] = { 0x01, 0x00, 0x00, 0x00, 0x01 };

  if (medusaSend(hSocket, com_quit_packet, 5, 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
    return FAILURE;
  }

  return SUCCESS;
}

/* 
   Code used verbatim from: 
   MySQL 3.21 [mysql_com.h / password.c]
*/
struct rand_struct {
  unsigned long seed1, seed2, max_value;
  double max_value_dbl;
};

void randominit(struct rand_struct *rand_st, unsigned long seed1, unsigned long seed2)
{                               
  /* for mysql 3.21.# */
  rand_st->max_value = 0x3FFFFFFFL;
  rand_st->max_value_dbl = (double) rand_st->max_value;
  rand_st->seed1 = seed1 % rand_st->max_value;
  rand_st->seed2 = seed2 % rand_st->max_value;
}

double rnd(struct rand_struct *rand_st)
{
  rand_st->seed1 = (rand_st->seed1 * 3 + rand_st->seed2) % rand_st->max_value;
  rand_st->seed2 = (rand_st->seed1 + rand_st->seed2 + 33) % rand_st->max_value;
  return (((double) rand_st->seed1) / rand_st->max_value_dbl);
}

/* 
   Code used verbatim from: 
   MySQL 4.1 [password.c]
*/
#define PVERSION41_CHAR '*'
typedef unsigned char    uint8;
typedef unsigned char    uchar; 
typedef unsigned long long int ulonglong;
typedef unsigned int     uint32;
typedef short    int16; 
typedef unsigned long    ulong;
#include "sha1.h"
#define SCRAMBLE_LENGTH 20
#define SCRAMBLE_LENGTH_323 8

#define char_val(X) (X >= '0' && X <= '9' ? X-'0' :\
                     X >= 'A' && X <= 'Z' ? X-'A'+10 :\
                     X >= 'a' && X <= 'z' ? X-'a'+10 :\
                     '\177')

void hash_password(ulong *result, const char *password, uint password_len)
{
  register ulong nr=1345345333L, add=7, nr2=0x12345671L;
  ulong tmp;
  const char *password_end= password + password_len;
  for (; password < password_end; password++)
  {
    if (*password == ' ' || *password == '\t')
      continue;                                 /* skip space in password */
    tmp= (ulong) (uchar) *password;
    nr^= (((nr & 63)+add)*tmp)+ (nr << 8);
    nr2+=(nr2 << 8) ^ nr;
    add+=tmp;
  }
  result[0]=nr & (((ulong) 1L << 31) -1L); /* Don't use sign bit (str2int) */;
  result[1]=nr2 & (((ulong) 1L << 31) -1L);
}

void octet2hex(char *to, const uint8 *str, unsigned int len)
{
  char _dig_vec_upper[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const uint8 *str_end= str + len;
  for (; str != str_end; ++str)
  {
    *to++= _dig_vec_upper[(*str & 0xF0) >> 4];
    *to++= _dig_vec_upper[*str & 0x0F];
  }
  *to= '\0';
}

void hex2octet(uint8 *to, const char *str, uint len)
{
  const char *str_end= str + len;
  while (str < str_end)
  {
    register char tmp= char_val(*str++);
    *to++= (tmp << 4) | char_val(*str++);
  }
}

static void my_crypt(char *to, const uchar *s1, const uchar *s2, uint len)
{
  const uint8 *s1_end= s1 + len;
  while (s1 < s1_end)
    *to++= *s1++ ^ *s2++;
}

double my_rnd(struct rand_struct *rand_st)
{
  rand_st->seed1=(rand_st->seed1*3+rand_st->seed2) % rand_st->max_value;
  rand_st->seed2=(rand_st->seed1+rand_st->seed2+33) % rand_st->max_value;
  return (((double) rand_st->seed1)/rand_st->max_value_dbl);
}

void scramble_323(char *to, const char *message, const char *password)
{
  struct rand_struct rand_st;
  ulong hash_pass[2], hash_message[2];

  if (password && password[0])
  {
    char extra, *to_start=to;
    const char *message_end= message + SCRAMBLE_LENGTH_323;
    hash_password(hash_pass,password, strlen(password));
    hash_password(hash_message, message, SCRAMBLE_LENGTH_323);
    randominit(&rand_st,hash_pass[0] ^ hash_message[0],
               hash_pass[1] ^ hash_message[1]);
    for (; message < message_end; message++)
      *to++= (char) (floor(my_rnd(&rand_st)*31)+64);
    extra=(char) (floor(my_rnd(&rand_st)*31));
    while (to_start != to)
      *(to_start++)^=extra;
  }
  *to= 0;
}

void scramble(char *to, const char *message, const char *password)
{
  SHA1_CONTEXT sha1_context;
  uint8 hash_stage1[SHA1_HASH_SIZE];
  uint8 hash_stage2[SHA1_HASH_SIZE];

  sha1_reset(&sha1_context);
  /* stage 1: hash password */
  sha1_input(&sha1_context, (uint8 *) password, strlen(password));
  sha1_result(&sha1_context, hash_stage1);
  /* stage 2: hash stage 1; note that hash_stage2 is stored in the database */
  sha1_reset(&sha1_context);
  sha1_input(&sha1_context, hash_stage1, SHA1_HASH_SIZE);
  sha1_result(&sha1_context, hash_stage2);
  /* create crypt string as sha1(message, hash_stage2) */;
  sha1_reset(&sha1_context);
  sha1_input(&sha1_context, (const uint8 *) message, SCRAMBLE_LENGTH);
  sha1_input(&sha1_context, hash_stage2, SHA1_HASH_SIZE);
  /* xor allows 'from' and 'to' overlap: lets take advantage of it */
  sha1_result(&sha1_context, (uint8 *) to);
  my_crypt(to, (const uchar *) to, hash_stage1, SCRAMBLE_LENGTH);
}

/* End of MySQL copy and paste items */ 

int MySQLPrepareAuthOld(char* szLogin, char* szPassword, char* szSessionSalt, unsigned char** szResponse, unsigned long* iResponseLength)
{
  char* szHash;
  unsigned long login_len = strlen(szLogin) > 32 ? 32 : strlen(szLogin);
  unsigned long response_len = 4 /* header */  +
                               2 /* client flags */  +
                               3 /* max packet len */  +
                               login_len + 1 + 8 /* scrambled password len */ ;
  unsigned char* response;

  *(iResponseLength) = response_len;

  /* < + 15 causes FREE issue. was 4 in original code. ??? */
  response = (unsigned char *) malloc(response_len + 15);
  if (response == NULL) {
    writeError(ERR_ERROR, "%s: Error allocating memory.", MODULE_NAME);
    return FAILURE;  
  }

  memset(response, 0, response_len + 15);

  *((unsigned long *) response) = response_len - 4;  /* packet length */
  response[3] = 0x01;           /* packet number */
  response[4] = 0x85;           /* client flag */
  response[5] = 0x24;           /* client flag */
  response[6] = 0x00;           /* max packet */
  response[7] = 0x00;           /* max packet */
  response[8] = 0x00;           /* max packet */
  strncpy(response + 9, szLogin, 32);
  response[9 + login_len] = '\0';       /* null terminate login */
  
  if ( strcmp(szPassword, "") )
    scramble_323((char *) &response[9 + login_len + 1], szSessionSalt, szPassword);

  *szResponse = response;
  return SUCCESS;
}

/* This is a weird packet format used on a rare case when the newer Mysql
   still has a password table entry in the old shorter format */
int MySQLPrepareAuthNewOld(char* szLogin, char* szPassword, char* szSessionSalt, unsigned char** szResponse, unsigned long* iResponseLength)
{
  char* szHash;
  unsigned long login_len = strlen(szLogin) > 32 ? 32 : strlen(szLogin);
  unsigned long response_len = 4 /* header */  +
                               1 + 8 /* scrambled password len */ ;
  unsigned char* response;

  *(iResponseLength) = response_len;

  /* < + 15 causes FREE issue. was 4 in original code. ??? */
  response = (unsigned char *) malloc(response_len + 15);
  if (response == NULL) {
    writeError(ERR_ERROR, "%s: Error allocating memory.", MODULE_NAME);
    return FAILURE;  
  }

  memset(response, 0, response_len + 15);

  *((unsigned long *) response) = response_len - 4;  /* packet length */
  response[3] = 0x03;           /* packet number */
  scramble_323((char *) &response[4], szSessionSalt, szPassword);

  *szResponse = response;
  return SUCCESS;
}

int MySQLPrepareAuth(char* szLogin, char* szPassword, char* szSessionSalt, unsigned char** szResponse, unsigned long* iResponseLength)
{
  char* szHash;
  unsigned long login_len = strlen(szLogin) > 32 ? 32 : strlen(szLogin);
  unsigned long response_len = 4 /* header */  +
                               2 /* client flags */  +
                               3 /* max packet len */  +
                               login_len + 1 + 48 /* scrambled password len */ ;
  unsigned char* response;

  *(iResponseLength) = response_len; 

  /* < + 15 causes FREE issue. was 4 in original code. ??? */
  response = (unsigned char *) malloc(response_len + 15);
  if (response == NULL) {
    writeError(ERR_ERROR, "%s: Error allocating memory.", MODULE_NAME);
    return FAILURE;
  }

  /* Much of this derived from 
    http://www.redferni.uklinux.net/mysql/MySQL-Protocol.html 
  */

  memset(response, 0, response_len + 15);

  *((unsigned long *) response) = response_len - 4;  /* packet length */
  response[3] = 0x01;           /* packet number */
  /* response[4] = 0x85;        */    /* client flag */
  response[4] = 0x05;           /* client flag */
  /* response[5] = 0x24; */           /* client flag */ 
  response[5] = 0xa6;           /* client flag */ 
  response[6] = 0x03;           /* max packet */
  response[7] = 0x00;           /* max packet */        
  response[8] = 0x00;           /* max packet */

  response[9] = 0x00;    /* Username is null in a 4.1 auth packet */
  response[10] = 0x00;    
  response[11] = 0x01;    
  response[12] = 0x08;    /* Charset Latin */

  /* OK, now add the username null terminated */

  strncpy(response + 36, szLogin, 32);

  if ( strcmp(szPassword, "") )
  {
/* Now add a 0x14 which is always the scrambled length of the password (20) */ 
    response[36+strlen(szLogin)+1] = 0x14;

    /* Now add the scramble SHA hash */
    scramble((char *) &response[36+strlen(szLogin)+2], szSessionSalt, szPassword); 
  }
  
  *szResponse = response;
  return SUCCESS;
}

int MySQLSessionInit(int hSocket, unsigned char** szSessionSalt)
{
  unsigned char* bufReceive;
  unsigned char* szServerVersion;
  int nReceiveBufferSize = 0;
  int newerauth = 0;

  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "%s failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }

  /* check protocol version */
  if (bufReceive[4] == 0xff)
  {
    writeError(ERR_ERROR, "%s: Failed to retrieve server version: %s", MODULE_NAME, bufReceive + 7);
    FREE(bufReceive);
    return FAILURE;
  }

  if (bufReceive[4] < 10)
  {
    writeError(ERR_ERROR, "%s: Server responded requestion protocol version (%d). Version 10 support required." MODULE_NAME, bufReceive[4]);
    FREE(bufReceive);
    return FAILURE;
  }

  if (bufReceive[4] > 10)
  {
    writeError(ERR_WARNING, "%s: Server responded requestion protocol version (%d). Support for versions >10 is unknown." MODULE_NAME, bufReceive[4]);
  }

  /* check server version */
  szServerVersion = bufReceive + 5;

  if (!(strstr(szServerVersion, "3.") || strstr(szServerVersion, "4.") || strstr(szServerVersion, "5.") ))
  {
    writeError(ERR_ERROR, "%s: Server responded requestion version (%d). Only versions 3.x 4.x 5.x currently supported." MODULE_NAME, szServerVersion);
    FREE(bufReceive);
    return FAILURE;
  }

  if ((strstr(szServerVersion, "4.1") || strstr(szServerVersion, "5.") ))
  {
     newerauth=1;    
     writeError(ERR_DEBUG_MODULE, "%s: Server version %s is using newer auth method.", MODULE_NAME, szServerVersion);
  }

  if (newerauth)
  {
    /*  retrieve session salt for newer auth */
    *szSessionSalt = malloc(22);
    memset(*szSessionSalt, 0, 22);
    memcpy(*szSessionSalt, bufReceive + strlen(szServerVersion) + 10, 9);
    memcpy(*szSessionSalt+8 , bufReceive + strlen(szServerVersion) + 37 , 12); 

    if (strlen(*szSessionSalt) != 20)
    {
      writeError(ERR_ERROR, "%s: Failed to retrieve valid session salt." MODULE_NAME);
      FREE(bufReceive);
      return FAILURE;
    }
    else
    {
      writeError(ERR_DEBUG_MODULE, "%s: Retrieved session salt: %s", MODULE_NAME, *szSessionSalt);
    }
  } 
  else 
  {
    /* use the older salt code */
    *szSessionSalt = malloc(10);
    memset(*szSessionSalt, 0, 10);
    memcpy(*szSessionSalt, bufReceive + strlen(szServerVersion) + 10, 9);

    if (strlen(*szSessionSalt) != 8) {
      writeError(ERR_ERROR, "%s: Failed to retrieve valid session salt." MODULE_NAME);
      FREE(bufReceive);
      return FAILURE;
    }
    else
    {
      writeError(ERR_DEBUG_MODULE, "%s: Retrieved session salt: %s.", MODULE_NAME, *szSessionSalt);
    }
  }

  FREE(bufReceive);
  return SUCCESS;
}

int tryLogin(int hSocket, sLogin** psLogin, _MYSQL_DATA* _psSessionData, char* szLogin, char* szPassword)
{
  int iRet, iReturnCode;
  unsigned char bufSend[BUF_SIZE];
  unsigned char* bufReceive = NULL;
  unsigned char* szSessionSalt = NULL;
  unsigned char* szResponse = NULL;
  unsigned long iResponseLength = 0;
  int nReceiveBufferSize = 0;

  /* initialize MySQL connection */
  iRet = MySQLSessionInit(hSocket, &szSessionSalt);
  if (iRet == FAILURE)
  {
    writeError(ERR_ERROR, "%s: Failed to initialize MySQL connection.", MODULE_NAME);
    return FAILURE;
  }

  /* prepare client authentication packet */
  if (strlen(szSessionSalt) == 8 || _psSessionData->protoFlag == PROTO_OLD)
  {
    if (_psSessionData->protoFlag == PROTO_OLD) {
      writeError(ERR_DEBUG, "%s: using older auth per previous attempt response.", MODULE_NAME);
    }
    
    iRet = MySQLPrepareAuthOld(szLogin, szPassword, szSessionSalt, &szResponse, &iResponseLength);
    if (iRet == FAILURE)
    {
      writeError(ERR_ERROR, "%s: Failed to create client authentication packet.", MODULE_NAME);
      return FAILURE;
    }
  } 
  else 
  {
    iRet = MySQLPrepareAuth(szLogin, szPassword, szSessionSalt, &szResponse, &iResponseLength);
    if (iRet == FAILURE)
    {
      writeError(ERR_ERROR, "%s: Failed to create client authentication packet.", MODULE_NAME);
      return FAILURE;
    }
  }

  /* send authentication attempt */
  if (medusaSend(hSocket, szResponse, iResponseLength, 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
    FREE(szResponse);
    return FAILURE;
  }

  FREE(szResponse);

  /* process authentication response */
  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
  {
    writeError(ERR_ERROR, "%s failed: medusaReceive returned no data.", MODULE_NAME);
    return FAILURE;
  }

  if (bufReceive[4] == 0x00)
  {
    (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
    iReturnCode = MSTATE_EXITING;
  }
  else if (bufReceive[4] == 0xFF)
  {
    (*psLogin)->iResult = LOGIN_RESULT_FAIL;
    
    if (bufReceive[5] == 0xe3 && bufReceive[6] == 0x04)
    {
      writeError(ERR_ERROR, "%s failed: MYSQL VERSION IS NEWER\n", MODULE_NAME);
      (*psLogin)->iResult = LOGIN_RESULT_ERROR;
      (*psLogin)->iStatus = LOGIN_FAILED;
      iReturnCode = MSTATE_EXITING;
    }
    else 
      iReturnCode = MSTATE_NEW;
  }
  else if (bufReceive[4] == 0xFE)
  {
    writeError(ERR_DEBUG, "%s failed: Server responded resend older Auth request.\n", MODULE_NAME);
    (*psLogin)->iResult = LOGIN_RESULT_FAIL;

    /* OK, so what the blank do I do now? */

    _psSessionData->protoFlag = PROTO_OLD;

    /* Well, you gotta send just the old style scramble using the existing connection */

    iRet = MySQLPrepareAuthNewOld(szLogin, szPassword, szSessionSalt, &szResponse,&iResponseLength);
    if (iRet == FAILURE)
    {
      writeError(ERR_ERROR, "%s: Failed to create client authentication packet.", MODULE_NAME);
      return FAILURE;
    }

    /* send authentication attempt */
    if (medusaSend(hSocket, szResponse, iResponseLength, 0) < 0)
    {
      writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
      FREE(szResponse);
      return FAILURE;
    }

    FREE(szResponse);

    /* process authentication response */
    nReceiveBufferSize = 0;
    bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
    if (bufReceive == NULL)
    {
      writeError(ERR_ERROR, "%s failed: medusaReceive returned no data.", MODULE_NAME);
      return FAILURE;
    }

    if (bufReceive[4] == 0x00)
    {
      (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
      iReturnCode = MSTATE_EXITING;
    }
    else if (bufReceive[4] == 0xFF)
    {
      (*psLogin)->iResult = LOGIN_RESULT_FAIL;

      if (bufReceive[5] == 0xe3 && bufReceive[6] == 0x04) {
        writeError(ERR_ERROR, "%s failed: MYSQL VERSION IS NEWER\n", MODULE_NAME);
        (*psLogin)->iResult = LOGIN_RESULT_ERROR;
        (*psLogin)->iStatus = LOGIN_FAILED;
        iReturnCode = MSTATE_EXITING;
      }
      else
        iReturnCode = MSTATE_NEW;
    }
    /* End of the weird downshift resend case */
  }
  else
  {
    writeError(ERR_ERROR, "%s: Unknown response code received from server: %X", MODULE_NAME, bufReceive[4]);
    (*psLogin)->iResult = LOGIN_RESULT_UNKNOWN;
    (*psLogin)->iStatus = LOGIN_FAILED;
    iReturnCode = MSTATE_EXITING;
  }

  /* close MySQL connection */
  iRet = MySQLSessionQuit(hSocket);
  if (iRet == FAILURE)
  {
    writeError(ERR_ERROR, "%s: Failed to terminate MySQL connection.", MODULE_NAME);
    return FAILURE;
  }

  FREE(bufReceive);
  setPassResult((*psLogin), szPassword);

  return(iReturnCode);
}

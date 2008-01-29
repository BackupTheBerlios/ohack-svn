/*
**   SMB NTLM Password/HASH Checking Medusa Module
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
**   Based on code from: SMB Auditing Tool
**   [Copyright (C) Patrik Karlsson 2001]
**
**   This code allows Medusa to directly test NTLM hashes against
**   a Windows host. This may be useful for an auditor who has aquired
**   a sam._ or pwdump file and would like to quickly determine
**   which are valid entries.
**
**   Several "-m 'METHOD:VALUE'" options can be used with this module. The
**   following are valid methods: GROUP, GROUP_OTHER, PASS and NETBIOS.
**   The following values are useful for these methods:
**
**   GROUP:?
**     LOCAL  == Check local account.
**     DOMAIN == Check credentials against this hosts primary
**               domain controller via this host.
**     BOTH   == Check both. This leaves the workgroup field set
**               blank and then attempts to check the credentials
**               against the host. If the account does not exist
**               locally on the host being tested, that host then
**               queries its domain controller.
**
**   GROUP_OTHER:?
**      Configure arbitrary domain for host to authenticate against.
**
**   PASS:?
**     PASSWORD == Use a normal password.
**     HASH     == Use a NTLM hash rather than a password.
**     MACHINE  == Use the Machine's NetBIOS name as the password.
**
**   NETBIOS
**            Force NetBIOS Mode (Disable Native Win2000 Mode)
**
**   Be careful of mass domain account lockout with this. For
**   example, assume you are checking several accounts against
**   many domain workstations. If you are not using the 'LOCAL'
**   option and these accounts do not exist locally on the
**   workstations, each workstation will in turn check their
**   respective domain controller. This could cause a bunch of
**   lockouts. Of course, it'd look like the workstations, not
**   you, were doing it. ;)
**
**   **FYI, this code is unable to test accounts on default XP
**   hosts which are not part of a domain and do not have normal
**   file sharing enabled. Default XP does not allow shares and
**   returns STATUS_LOGON_FAILED for both valid and invalid
**   credentials. XP with simple sharing enabled returns SUCCESS
**   for both valid and invalid credentials. If anyone knows a
**   way to test in these configurations...
**
**   See http://www.foofus.net/jmk/passhash.html for further
**   examples.
*/

#include <sys/types.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "module.h"

#define MODULE_NAME    "smbnt.mod"
#define MODULE_AUTHOR  "JoMo-Kun <jmk@foofus.net>"
#define MODULE_SUMMARY_USAGE  "Brute force module for SMB/NTLMv1 sessions"
#define MODULE_VERSION    "1.3.0"
#define MODULE_VERSION_SVN "$Id: smbnt.c 606 2006-10-19 20:22:41Z jmk $"
#define MODULE_SUMMARY_FORMAT  "%s : version %s"
#define MODULE_SUMMARY_FORMAT_WARN  "%s : version %s (%s)"
#define OPENSSL_WARNING "No usable OPENSSL. Module disabled."

#ifdef HAVE_LIBSSL

#include <openssl/md4.h>
#include <openssl/des.h>

#define PORT_SMB 139
#define PORT_SMBNT 445
#define WIN2000_NATIVEMODE 1
#define WIN_NETBIOSMODE 2
#define PASSWORD 3
#define HASH 4
#define MACHINE 5
#define LOCAL 6
#define DOMAIN 7
#define BOTH 8
#define OTHER 9
#define PLAINTEXT 10
#define ENCRYPTED 11

typedef struct __SMBNT_DATA {
  char challenge[8];
  unsigned char workgroup[16];
  unsigned char workgroup_other[16];
  unsigned char machine_name[16];
  int security_mode;
  int hashFlag;
  int accntFlag;
  int protoFlag;
} _SMBNT_DATA;

// Tells us whether we are to continue processing or not
enum MODULE_STATE
{
  MSTATE_NEW,
  MSTATE_RUNNING,
  MSTATE_EXITING
};

// Forward declarations
int tryLogin(int hSocket, sLogin** login, _SMBNT_DATA* _psSessionData, char* szLogin, char* szPassword);
int initModule(sLogin* login, _SMBNT_DATA *_psSessionData);

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
  writeVerbose(VB_NONE, "  GROUP:? (DOMAIN, LOCAL*, BOTH)");
  writeVerbose(VB_NONE, "    Option sets NetBIOS workgroup field.");
  writeVerbose(VB_NONE, "    DOMAIN: Check credentials against this hosts primary domain controller via this host.");
  writeVerbose(VB_NONE, "    LOCAL:  Check local account.");
  writeVerbose(VB_NONE, "    BOTH:   Check both. This leaves the workgroup field set blank and then attempts to check");
  writeVerbose(VB_NONE, "            the credentials against the host. If the account does not exist locally on the ");
  writeVerbose(VB_NONE, "            host being tested, that host then queries its domain controller.");
  writeVerbose(VB_NONE, "  GROUP_OTHER:? ");
  writeVerbose(VB_NONE, "    Option allows manual setting of domain to check against. Use instead of GROUP.");
  writeVerbose(VB_NONE, "  PASS:?  (PASSWORD*, HASH, MACHINE)");
  writeVerbose(VB_NONE, "    PASSWORD: Use normal password.");
  writeVerbose(VB_NONE, "    HASH:     Use a NTLM hash rather than a password.");
  writeVerbose(VB_NONE, "    MACHINE:  Use the machine's NetBIOS name as the password.");
  writeVerbose(VB_NONE, "  NETBIOS");
  writeVerbose(VB_NONE, "    Force NetBIOS Mode (Disable Native Win2000 Mode). Win2000 mode is the default.");
  writeVerbose(VB_NONE, "    Default mode is to test TCP/445 using Native Win2000. If this fails, module will");
  writeVerbose(VB_NONE, "    fall back to TCP/139 using NetBIOS mode. To test only TCP/139, use the following:");
  writeVerbose(VB_NONE, "    medusa -M smbnt -m NETBIOS -n 139");
  writeVerbose(VB_NONE, "\n(*) Default value");
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "Usage examples:");
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "1: Normal boring check..."); 
  writeVerbose(VB_NONE, "    medusa -M smbnt -h somehost -u someuser -p somepassword");
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "2: Testing domain credentials against a client system..."); 
  writeVerbose(VB_NONE, "    medusa -M smbnt -h somehost -U users.txt -p password -m GROUP:DOMAIN");
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "3: Testing each credential from a PwDump file against the target's domain via the target..."); 
  writeVerbose(VB_NONE, "    medusa -M smbnt -h somehost -C pwdump.txt -m PASS:HASH -m GROUP:DOMAIN");
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "4: Testing each hash from a PwDump file against a specific user local to the target..."); 
  writeVerbose(VB_NONE, "    medusa -M smbnt -H hosts.txt -C pwdump.txt -u someuser -m PASS:HASH");
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "5: Testing an individual NTLM hash..."); 
  writeVerbose(VB_NONE, "    medusa -M smbnt -H hosts.txt -u administrator -p 92D887C8010492C2944E2DF489A880E4:7A2EDE4F51BC5A03984C6BA2C239CF63::: -m PASS:HASH");
  writeVerbose(VB_NONE, "");
}

// The "main" of the medusa module world - this is what gets called to actually do the work
int go(sLogin* logins, int argc, char *argv[])
{
  int i;
  char *strtok_ptr, *pOpt, *pOptTmp;
  _SMBNT_DATA *psSessionData;
  psSessionData = malloc(sizeof(_SMBNT_DATA));  
  memset(psSessionData, 0, sizeof(_SMBNT_DATA));

  if ( !(0 <= argc <= 3) )
  {
    // Show usage information
    writeError(ERR_ERROR, "%s is expecting 0 parameters, but it was passed %d", MODULE_NAME, argc);
  } 
  else 
  {
    writeError(ERR_DEBUG, "OMG teh %s module has been called!!", MODULE_NAME);
 
    psSessionData->accntFlag = LOCAL;
    psSessionData->hashFlag = PASSWORD;
    psSessionData->protoFlag = WIN2000_NATIVEMODE;

    for (i=0; i<argc; i++) {
      pOptTmp = malloc( strlen(argv[i]) + 1);
      memset(pOptTmp, 0, strlen(argv[i]) + 1);
      strncpy(pOptTmp, argv[i], strlen(argv[i]));
      writeError(ERR_DEBUG_MODULE, "Processing complete option: %s", pOptTmp);
      pOpt = strtok_r(pOptTmp, ":", &strtok_ptr);
      writeError(ERR_DEBUG_MODULE, "Processing option: %s", pOpt);
      
      if (strcmp(pOpt, "GROUP") == 0)
      {
        pOpt = strtok_r(NULL, "\0", &strtok_ptr);
        writeError(ERR_DEBUG_MODULE, "Processing option parameter: %s", pOpt);
        
        if (strcmp(pOpt, "LOCAL") == 0)
          psSessionData->accntFlag = LOCAL;
        else if (strcmp(pOpt, "DOMAIN") == 0)
          psSessionData->accntFlag = DOMAIN;
        else if (strcmp(pOpt, "BOTH") == 0)
          psSessionData->accntFlag = BOTH;
        else
          writeError(ERR_WARNING, "Invalid value for method GROUP.");
      }
      else if (strcmp(pOpt, "GROUP_OTHER") == 0)
      {
        pOpt = strtok_r(NULL, "\0", &strtok_ptr);
        writeError(ERR_DEBUG_MODULE, "Processing option parameter: %s", pOpt);
    
        if ( pOpt )
        {
          strncpy((char *) psSessionData->workgroup_other, pOpt, 16);
          psSessionData->accntFlag = OTHER;
        }
        else
          writeError(ERR_WARNING, "Method GROUP_OTHER requires value to be set.");
      }
      else if (strcmp(pOpt, "PASS") == 0) {
        pOpt = strtok_r(NULL, "\0", &strtok_ptr);
        writeError(ERR_DEBUG_MODULE, "Processing option parameter: %s", pOpt);
        
        if (strcmp(pOpt, "PASSWORD") == 0)
          psSessionData->hashFlag = PASSWORD;
        else if (strcmp(pOpt, "HASH") == 0)
          psSessionData->hashFlag = HASH;
        else if (strcmp(pOpt, "MACHINE") == 0)
          psSessionData->hashFlag = MACHINE;
        else
          writeError(ERR_WARNING, "Invalid value for method PASS.");
      }
      else if (strcmp(pOpt, "NETBIOS") == 0)
      {
        psSessionData->protoFlag = WIN_NETBIOSMODE;
      }
      else 
      {
        writeError(ERR_WARNING, "Invalid method: %s.", pOpt);
      }
    
      free(pOptTmp);
    }
 
    initModule(logins, psSessionData);
  }  

  return 0;
}

int initModule(sLogin* psLogin, _SMBNT_DATA *_psSessionData)
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
    params.nPort = PORT_SMBNT;
  
  initConnectionParams(psLogin, &params);

  if (user != NULL) 
  {
    writeError(ERR_DEBUG, "[%s] module started for host: %s user: '%s'", MODULE_NAME, psLogin->psServer->pHostIP, user->pUser);
  }
  else 
  {
    writeError(ERR_DEBUG, "[%s] module started for host: %s", MODULE_NAME, psLogin->psServer->pHostIP);
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
  
        if (params.nPort == PORT_SMBNT) {
          hSocket = medusaConnect(&params);
          if ( hSocket < 0 ) {
            writeError(ERR_NOTICE, "%s Failed to establish WIN2000_NATIVE mode. Attempting WIN_NETBIOS mode.)", MODULE_NAME);
            params.nPort = PORT_SMB;
            _psSessionData->protoFlag = WIN_NETBIOSMODE;
            hSocket = medusaConnect(&params);
          }
        }
        else {
          hSocket = medusaConnect(&params);
        }
        
        if (hSocket < 0) 
        {
          writeError(ERR_NOTICE, "%s: failed to connect, port %d was not open on %s", MODULE_NAME, params.nPort, psLogin->psServer->pHostIP);
          psLogin->iResult = LOGIN_RESULT_UNKNOWN;
          psLogin->iStatus = LOGIN_FAILED;
          free(_psSessionData);
          return FAILURE;
        }

        writeError(ERR_DEBUG_MODULE, "Connected");
 
        if (NBSSessionRequest(hSocket, _psSessionData) < 0) {
          writeError(ERR_DEBUG_MODULE, "Session Setup Failed with host: %s. Is the server service running?", psLogin->psServer->pHostIP);
          psLogin->iResult = LOGIN_RESULT_UNKNOWN;
          psLogin->iStatus = LOGIN_FAILED;
          free(_psSessionData);
          return FAILURE;
        }
        
        if (SMBNegProt(hSocket, _psSessionData) < 0)
        {
          writeError(ERR_DEBUG_MODULE, "SMB Protocol Negotiation Failed with host: %s", psLogin->psServer->pHostIP);
          psLogin->iResult = LOGIN_RESULT_UNKNOWN;
          psLogin->iStatus = LOGIN_FAILED;
          free(_psSessionData);
          return FAILURE;
        }
        else {
          nState = MSTATE_RUNNING;
        }
        
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
        writeError(ERR_CRITICAL, "Unknown %s module (%d) state %d host: %s", MODULE_NAME, psLogin->iId, nState, psLogin->psServer->pHostIP);
        if (hSocket > 0)
          medusaDisconnect(hSocket);
        hSocket = -1;
        psLogin->iResult = LOGIN_RESULT_UNKNOWN;
        psLogin->iStatus = LOGIN_FAILED;
        free(_psSessionData);
        return FAILURE;
    }  
  }
 
  psLogin->iStatus = LOGIN_DONE;
  free(_psSessionData);
  return SUCCESS;
}

/* SMBNT Specific Functions */

static unsigned char Get7Bits(unsigned char *input, int startBit)
{
  register unsigned int word;

  word = (unsigned) input[startBit / 8] << 8;
  word |= (unsigned) input[startBit / 8 + 1];

  word >>= 15 - (startBit % 8 + 7);

  return word & 0xFE;
}

/* Make the key */
static void MakeKey(unsigned char *key, unsigned char *des_key)
{
  des_key[0] = Get7Bits(key, 0);
  des_key[1] = Get7Bits(key, 7);
  des_key[2] = Get7Bits(key, 14);
  des_key[3] = Get7Bits(key, 21);
  des_key[4] = Get7Bits(key, 28);
  des_key[5] = Get7Bits(key, 35);
  des_key[6] = Get7Bits(key, 42);
  des_key[7] = Get7Bits(key, 49);

  des_set_odd_parity((des_cblock *) des_key);
}

/* Do the DesEncryption */
void DesEncrypt(unsigned char *clear, unsigned char *key, unsigned char *cipher)
{
  des_cblock des_key;
  des_key_schedule key_schedule;

  MakeKey(key, des_key);
  des_set_key(&des_key, key_schedule);
  des_ecb_encrypt((des_cblock *) clear, (des_cblock *) cipher, key_schedule, 1);
}

/*
  HashNTLM
  Function: Create a NTLM hash from the challenge
  Variables:
        ntlmhash  = the hash created from this function
        pass      = users password
        challenge = the challenge recieved from the server
*/
int HashNTLM(_SMBNT_DATA *_psSessionData, unsigned char **ntlmhash, unsigned char *pass, unsigned char *challenge)
{
  MD4_CTX md4Context;
  unsigned char hash[16];                       /* MD4_SIGNATURE_SIZE = 16 */
  unsigned char unicodePassword[256 * 2];       /* MAX_NT_PASSWORD = 256 */
  unsigned char p21[21];
  unsigned char ntlm_response[24];
  int i = 0, j = 0;
  int mdlen;
  unsigned char *p;
  char HexChar;
  int HexValue;

  /* Use NTLM Hash instead of password */
  if (_psSessionData->hashFlag == HASH) {
    /* [OLD] 1000:D42E35E1A1E4C22BD32E2170E4857C20:5E20780DD45857A68402938C7629D3B2::: */
    /* [NEW] D42E35E1A1E4C22BD32E2170E4857C20:5E20780DD45857A68402938C7629D3B2::: */
    p = pass;
    while ((*p != '\0') && (i < 1)) {
      if (*p == ':')
        i++;
      p++;
    }
  }

  /* If "-e ns" was used, don't treat these values as hashes. */
  if ((_psSessionData->hashFlag == HASH) && (i >= 1)) {
    if (*p == '\0') {
      writeError(ERR_ERROR, "Error reading PwDump file.");
      return FAILURE;
    }
    else if (*p == 'N') {
      writeError(ERR_DEBUG_MODULE, "Found \"NO PASSWORD\" for NTLM Hash.");
      pass = "";
      
      /* Initialize the Unicode version of the secret (== password). */
      /* This implicitly supports 8-bit ISO8859/1 characters. */
      bzero(unicodePassword, sizeof(unicodePassword));
      for (i = 0; i < strlen((char *) pass); i++)
        unicodePassword[i * 2] = (unsigned char) pass[i];

      mdlen = strlen((char *) pass) * 2;    /* length in bytes */
      MD4_Init(&md4Context);
      MD4_Update(&md4Context, unicodePassword, mdlen);
      MD4_Final(hash, &md4Context);        /* Tell MD4 we're done */
    }
    else {
      writeError(ERR_DEBUG_MODULE, "Convert ASCII PwDump NTLM Hash (%s).", p);
      for (i = 0; i < 16; i++) {
        HexValue = 0x0;
        for (j = 0; j < 2; j++) {
          HexChar = (char) p[2 * i + j];

          if (HexChar > 0x39)
            HexChar = HexChar | 0x20;     /* convert upper case to lower */

          if (!(((HexChar >= 0x30) && (HexChar <= 0x39)) ||       /* 0 - 9 */
                ((HexChar >= 0x61) && (HexChar <= 0x66)))) {      /* a - f */
            
            writeError(ERR_ERROR, "Error invalid char (%c) for hash.", HexChar);
            HexChar = 0x30;
          }
  
          HexChar -= 0x30;
          if (HexChar > 0x09)     /* HexChar is "a" - "f" */
            HexChar -= 0x27;

          HexValue = (HexValue << 4) | (char) HexChar;
        }
        hash[i] = (unsigned char) HexValue;
      }
    }
  } else {
    /* Password == Machine Name */
    if (_psSessionData->hashFlag == MACHINE) {
      for (i = 0; i < 16; i++) {
        if (_psSessionData->machine_name[i] > 0x39)
          _psSessionData->machine_name[i] = _psSessionData->machine_name[i] | 0x20;     /* convert upper case to lower */
        pass = _psSessionData->machine_name;
      }
    }
   
    /* Initialize the Unicode version of the secret (== password). */
    /* This implicitly supports 8-bit ISO8859/1 characters. */
    bzero(unicodePassword, sizeof(unicodePassword));
    for (i = 0; i < strlen((char *) pass); i++)
      unicodePassword[i * 2] = (unsigned char) pass[i];

    mdlen = strlen((char *) pass) * 2;    /* length in bytes */
    MD4_Init(&md4Context);
    MD4_Update(&md4Context, unicodePassword, mdlen);
    MD4_Final(hash, &md4Context);        /* Tell MD4 we're done */
  }

  memset(p21, '\0', 21);
  memcpy(p21, hash, 16);

  DesEncrypt(challenge, p21 + 0, ntlm_response + 0);
  DesEncrypt(challenge, p21 + 7, ntlm_response + 8);
  DesEncrypt(challenge, p21 + 14, ntlm_response + 16);

  memcpy(*ntlmhash, ntlm_response, 24);

  return SUCCESS;
}

/*
   NBS Session Request
   Function: Request a new session from the server
   Returns: TRUE on success else FALSE.
*/
int NBSSessionRequest(int hSocket, _SMBNT_DATA* _psSessionData)
{
  char nb_name[32];             /* netbiosname */
  char nb_local[32];            /* netbios localredirector */
  unsigned char rqbuf[7] = { 0x81, 0x00, 0x00, 0x48, 0x20, 0x00, 0x20 };
  char *buf = NULL;
  char* bufReceive = NULL;
  int nReceiveBufferSize = 0;
  
  /* if we are running in native mode (aka port 445) don't do netbios */
  if (_psSessionData->protoFlag == WIN2000_NATIVEMODE)
    return 0;
  
  /* convert computer name to netbios name */
  memset(nb_name, 0, 32);
  memset(nb_local, 0, 32);
  memcpy(nb_name, "CKFDENECFDEFFCFGEFFCCACACACACACA", 32);      /* *SMBSERVER */
  /* smbat/netbios.c/getNetbiosName */
  /* memcpy(nb_local, "EHGFHCGHFDHFHIEMGPGCHDHEGFHCCACA", 32); */
  memcpy(nb_local, "ENEFEEFFFDEBCACACACACACACACACACA", 32);     /* MEDUSA */
  
  buf = (char *) malloc(100);
  memset(buf, 0, 100);
  memcpy(buf, (char *) rqbuf, 5);
  memcpy(buf + 5, nb_name, 32);
  memcpy(buf + 37, (char *) rqbuf + 5, 2);
  memcpy(buf + 39, nb_local, 32);
  memcpy(buf + 71, (char *) rqbuf + 5, 1);

  if (medusaSend(hSocket, (char *) buf, 76, 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
  }

  free(buf);

  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
    return FAILURE;

  if ((unsigned char)bufReceive[0] == 0x82)
    return SUCCESS;                   /* success */
  else
    return FAILURE;                  /* failed */
}


/*
   SMBNegProt
   Function: Negotiate protocol with server ...
       Actually a pseudo negotiation since the whole
       program counts on NTLM support :)

    The challenge is retrieved from the answer
    No error checking is performed i.e cross your fingers....
*/
int SMBNegProt(int hSocket, _SMBNT_DATA* _psSessionData)
{
  unsigned char buf[168] = {
    0x00, 0x00, 0x00, 0xa4, 0xff, 0x53, 0x4d, 0x42,
    0x72, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x7d,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x81, 0x00, 0x02,
    0x50, 0x43, 0x20, 0x4e, 0x45, 0x54, 0x57, 0x4f,
    0x52, 0x4b, 0x20, 0x50, 0x52, 0x4f, 0x47, 0x52,
    0x41, 0x4d, 0x20, 0x31, 0x2e, 0x30, 0x00, 0x02,
    0x4d, 0x49, 0x43, 0x52, 0x4f, 0x53, 0x4f, 0x46,
    0x54, 0x20, 0x4e, 0x45, 0x54, 0x57, 0x4f, 0x52,
    0x4b, 0x53, 0x20, 0x31, 0x2e, 0x30, 0x33, 0x00,
    0x02, 0x4d, 0x49, 0x43, 0x52, 0x4f, 0x53, 0x4f,
    0x46, 0x54, 0x20, 0x4e, 0x45, 0x54, 0x57, 0x4f,
    0x52, 0x4b, 0x53, 0x20, 0x33, 0x2e, 0x30, 0x00,
    0x02, 0x4c, 0x41, 0x4e, 0x4d, 0x41, 0x4e, 0x31,
    0x2e, 0x30, 0x00, 0x02, 0x4c, 0x4d, 0x31, 0x2e,
    0x32, 0x58, 0x30, 0x30, 0x32, 0x00, 0x02, 0x53,
    0x61, 0x6d, 0x62, 0x61, 0x00, 0x02, 0x4e, 0x54,
    0x20, 0x4c, 0x41, 0x4e, 0x4d, 0x41, 0x4e, 0x20,
    0x31, 0x2e, 0x30, 0x00, 0x02, 0x4e, 0x54, 0x20,
    0x4c, 0x4d, 0x20, 0x30, 0x2e, 0x31, 0x32, 0x00
  };

  char* bufReceive;
  int nReceiveBufferSize = 0;
  unsigned char sess_key[2];
  unsigned char userid[2] = {0xCD, 0xEF};
  int i = 0, j = 0;

  /* set session key */
  sess_key[1] = getpid() / 100;
  sess_key[0] = getpid() - (100 * sess_key[1]);
  memcpy(buf + 30, sess_key, 2);
  memcpy(buf + 32, userid, 2);
  
  if (medusaSend(hSocket, (char *) buf, 168, 0) < 0)
  {
    writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
  }

  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
    return FAILURE;

  /* retrieve the security mode */
  /*
    [0] Mode:       (0) ?                                 (1) USER security mode 
    [1] Password:   (0) PLAINTEXT password                (1) ENCRYPTED password. Use challenge/response
    [2] Signatures: (0) Security signatures NOT enabled   (1) ENABLED
    [3] Sig Req:    (0) Security signatures NOT required  (1) REQUIRED
  
    SAMBA: 0x01 (default)
    WinXP: 0x0F (default)
    WinXP: 0x07 (Windows 2003 / DC)
  */
  switch (bufReceive[39])
  {
    case 0x01:
      writeVerbose(VB_GENERAL, "%s: Server requested PLAINTEXT password. Possible SAMBA server.", MODULE_NAME);
      _psSessionData->security_mode = PLAINTEXT;
      
      if (_psSessionData->hashFlag == HASH)
      {
        writeError(ERR_ERROR, "%s: Server requested PLAINTEXT password. HASH password mode not supported for this configuration.", MODULE_NAME);
        return FAILURE;
      }
      if (_psSessionData->hashFlag == MACHINE)
      {
        writeError(ERR_ERROR, "%s: Server requested PLAINTEXT password. MACHINE password mode not supported for this configuration.", MODULE_NAME);
        return FAILURE;
      }
      
      break;
    case 0x03:
      writeVerbose(VB_GENERAL, "%s: Server requested ENCRYPTED password without security signatures. Possible SAMBA server.", MODULE_NAME);
      _psSessionData->security_mode = ENCRYPTED;
      break;
    case 0x07:
    case 0x0F:
      writeVerbose(VB_GENERAL, "%s: Server requested ENCRYPTED password.", MODULE_NAME);
      _psSessionData->security_mode = ENCRYPTED;
      break;
    default:
      writeError(ERR_ERROR, "%s: Unknown security mode request: %2.2X. Proceeding using ENCRYPTED password mode.", MODULE_NAME, bufReceive[39]);
      _psSessionData->security_mode = ENCRYPTED;
      break;
  }

  /* retrieve the challenge */
  memcpy(_psSessionData->challenge, (char *) bufReceive + 73, sizeof(_psSessionData->challenge));

  /* find the primary domain/workgroup name */
  memset(_psSessionData->workgroup, 0, 16);
  memset(_psSessionData->machine_name, 0, 16);

  while ((bufReceive[81 + i * 2] != 0) && (i < 16)) {
    _psSessionData->workgroup[i] = bufReceive[81 + i * 2];
    i++;
  }
  
  while ((bufReceive[81 + (i + j + 1) * 2] != 0) && (j < 16)) {
    _psSessionData->machine_name[j] = bufReceive[81 + (i + j + 1) * 2];
    j++;
  }

  return SUCCESS;
}


/*
  SMBSessionSetup
  Function: Send username + response to the challenge from
            the server.
       Currently we're sendin ZEROES for the LMHASH since
       NT4/2000 doesn't seem to look at this if we got a
       valid NTLM hash.
  Returns: TRUE on success else FALSE.
*/
unsigned long SMBSessionSetup(int hSocket, _SMBNT_DATA *_psSessionData, char* szLogin, char* szPassword)
{
  unsigned char b[137] = {
    0x00, 0x00, 0x00, 0x85, 0xff, 0x53, 0x4d,
    0x42, 0x73, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c,
    0x7d, 0x00, 0x00, 0x01, 0x00, 0x0d, 0xff, 0x00,
    0x00, 0x00, 0xff, 0xff, 0x02, 0x00, 0x3c, 0x7d,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x48, 0x00, 0xdf, 0x82, 0xb9, 0x2f, 0xda, 0xdb,
    0x43, 0x47, 0x09, 0xdb, 0x7f, 0xfc, 0xb0, 0xa8,
    0xf0, 0x46, 0xe2, 0xfe, 0x64, 0x6d, 0x67, 0x58,
    0xe0, 0xf9, 0xca, 0x9f, 0x5e, 0xdb, 0xe0, 0x15,
    0x80, 0xf8, 0x54, 0xe3, 0x6e, 0xe9, 0xf8, 0x88,
    0x80, 0x19, 0xb6, 0xf9, 0xae, 0xb7, 0xa6, 0x62,
    0x14, 0xfc, 0x54, 0x45, 0x53, 0x54, 0x00, 0x4d,
    0x59, 0x47, 0x52, 0x4f, 0x55, 0x50, 0x00, 0x55,
    0x6e, 0x69, 0x78, 0x00, 0x53, 0x61, 0x6d, 0x62,
    0x61, 0x00
  };

  unsigned char buf[512];
  unsigned char *NTLMhash;
  unsigned char LMhash[24];
  char* bufReceive;
  int nReceiveBufferSize = 0;
  unsigned char sess_key[2];
  int i, userlen, passlen, ret;

  if (_psSessionData->accntFlag == LOCAL) {
    strcpy((char *) _psSessionData->workgroup, "localhost");
  } else if (_psSessionData->accntFlag == BOTH) {
    memset(_psSessionData->workgroup, 0, 16);
  } else if (_psSessionData->accntFlag == OTHER) {
    strncpy((char *) _psSessionData->workgroup, _psSessionData->workgroup_other, 16);
  }

  if (_psSessionData->security_mode == ENCRYPTED)
  {
    writeError(ERR_DEBUG_MODULE, "%s: Attempting ENCRYPTED password authentication.", MODULE_NAME);
    
    userlen = strlen(szLogin);

    NTLMhash = (unsigned char *) malloc(24);
    memset(NTLMhash, 0, 24);
    memset(LMhash, 0, 24);

    ret = HashNTLM(_psSessionData, &NTLMhash, (unsigned char *) szPassword, (unsigned char *) _psSessionData->challenge);

    if (ret == FAILURE)
      return FAILURE;

    memset(buf, 0, 512);
    memcpy(buf, b, 89);

    /* set the header length */
    buf[2] = (userlen + strlen((char *) _psSessionData->workgroup) + 0x7A) / 256;
    buf[3] = (userlen + strlen((char *) _psSessionData->workgroup) + 0x7A) % 256;
    writeError(ERR_DEBUG_MODULE, "%s: Set header length: %2.2X", MODULE_NAME, buf[3]);

    /* set session key */
    sess_key[1] = getpid() / 100;
    sess_key[0] = getpid() - (100 * sess_key[1]);
    memcpy(buf + 30, sess_key, 2);
  
    /* set data length */
    buf[63] = 24 + 24 + userlen + 1 + strlen((char *) _psSessionData->workgroup) + 1 + 11;
    writeError(ERR_DEBUG_MODULE, "%s: Set data length: %2.2X", MODULE_NAME, buf[63]);

    /* set LM/NTLM hash */
    memcpy(buf + 65, LMhash, 24);
    memcpy(buf + 89, NTLMhash, 24);
    
    memcpy(buf + 113, szLogin, userlen);
    memset(buf + (113 + userlen), 0, 1);
    memcpy(buf + (114 + userlen), _psSessionData->workgroup, strlen((char *) _psSessionData->workgroup));
    memcpy(buf + (114 + userlen + strlen((char *) _psSessionData->workgroup)), b + 125, 12);

    if (medusaSend(hSocket, (char *) buf, 126 + userlen + strlen((char *) _psSessionData->workgroup), 0) < 0)
    {
      writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
    }
  }
  else if (_psSessionData->security_mode == PLAINTEXT) 
  {
    writeError(ERR_DEBUG_MODULE, "%s: Attempting PLAINTEXT password authentication.", MODULE_NAME);

    userlen = strlen(szLogin);
    passlen = strlen(szPassword);
    
    if (passlen > 256)
      passlen = 256;
    
    memset(buf, 0, 512);
    memcpy(buf, b, 89);
  
    /* set the header length */
    buf[2] = (userlen + strlen((char *) _psSessionData->workgroup) + 75) / 256;
    buf[3] = (passlen + userlen + strlen((char *) _psSessionData->workgroup) + 75) % 256;
    writeError(ERR_DEBUG_MODULE, "%s: Set header length: %2.2X", MODULE_NAME, buf[3]);

    /* set session key */
    sess_key[1] = getpid() / 100;
    sess_key[0] = getpid() - (100 * sess_key[1]);
    memcpy(buf + 30, sess_key, 2);
  
    /* set data length */
    buf[63] = 11 + 3 + passlen + userlen + strlen((char *) _psSessionData->workgroup);
    writeError(ERR_DEBUG_MODULE, "%s: Set data length: %2.2X", MODULE_NAME, buf[63]);

    /* set ANSI password length */
    memset(buf + 50, passlen + 1, 2);
    
    /* set UNICODE password length */
    memset(buf + 52, 0, 2);
    
    /* set ANSI password */
    /*
      Depending on the SAMBA server configuration, multiple passwords may be successful
      when dealing with mixed-case values. The SAMBA parameter "password level" appears
      to determine how many characters within a password are tested by the server both  
      upper and lower case. For example, assume a SAMBA account has a password of "Fred" 
      and the server is configured with "password level = 2". Medusa sends the password
      "FRED". The SAMBA server will brute-force test this value for us with values
      like: "FRed", "FrEd", "FreD", "fREd", "fReD", "frED", ... The default setting
      is "password level = 0". This results in only two attempts to being made by the 
      remote server; the password as is and the password in all-lower case.
    */
    strncpy(buf + 65, szPassword, 256);

    /* set Account */
    memcpy(buf + 65 + passlen + 1, szLogin, userlen);
    memset(buf + 65 + passlen + + 1 + userlen, 0, 1);
    
    /* set Primary Domain */
    memcpy(buf + 67 + passlen + userlen, _psSessionData->workgroup, strlen((char *) _psSessionData->workgroup));
    
    /* set client OS information */
    memcpy(buf + 67 + passlen + userlen + strlen((char *) _psSessionData->workgroup), b + 125, 12);

    if (medusaSend(hSocket, (char *) buf, 65 + 2 + 12 + passlen + userlen + strlen((char *) _psSessionData->workgroup), 0) < 0)
    {
      writeError(ERR_ERROR, "%s failed: medusaSend was not successful", MODULE_NAME);
    }
  }
  else
  {
    writeError(ERR_ERROR, "%s: security_mode was not properly set. This should not happen.", MODULE_NAME);
    return FAILURE;
  }
  
  nReceiveBufferSize = 0;
  bufReceive = medusaReceiveRaw(hSocket, &nReceiveBufferSize);
  if (bufReceive == NULL)
    return FAILURE;
 
  /* 41 - Action (Guest/Non-Guest Account) */
  /*  9 - NT Status (Error code) */
  return (((bufReceive[41] & 0x0F) << 24) | ((bufReceive[11] & 0xFF) << 16) | ((bufReceive[10] & 0xFF) << 8) | (bufReceive[9] & 0xFF));
}

int tryLogin(int hSocket, sLogin** psLogin, _SMBNT_DATA* _psSessionData, char* szLogin, char* szPassword)
{
  int SMBerr, SMBaction;
  unsigned long SMBSessionRet;
  char *pErrorMsg;
  char ErrorCode[8];
  int i, iRet;

  /* Nessus Plugins: smb_header.inc */
  /* Note: we are currently only examining the lower 2 bytes of data */
  int smbErrorCode[] = {
    0xFFFFFFFF,         /* UNKNOWN_ERROR_CODE */
    0x00000000,         /* STATUS_SUCCESS */
    0xC000000D,         /* STATUS_INVALID_PARAMETER */
    0xC000005E,         /* STATUS_NO_LOGON_SERVERS */
    0xC000006D,         /* STATUS_LOGON_FAILURE */
    0xC000006E,         /* STATUS_ACCOUNT_RESTRICTION */
    0xC000006F,         /* STATUS_INVALID_LOGON_HOURS */
    0xC0000070,         /* STATUS_INVALID_WORKSTATION */
    0xC0000071,         /* STATUS_PASSWORD_EXPIRED */
    0xC0000072,         /* STATUS_ACCOUNT_DISABLED */
    0xC000015B,         /* STATUS_LOGON_TYPE_NOT_GRANTED */
    0xC000018D,         /* STATUS_TRUSTED_RELATIONSHIP_FAILURE */
    0xC0000193,         /* STATUS_ACCOUNT_EXPIRED */
    0xC0000224,         /* STATUS_PASSWORD_MUST_CHANGE */
    0xC0000234,         /* STATUS_ACCOUNT_LOCKED_OUT */
    0x00050001          /* AS400_STATUS_LOGON_FAILURE */
  };

  char *smbErrorMsg[] = {
    "UNKNOWN_ERROR_CODE",
    "STATUS_SUCCESS",
    "STATUS_INVALID_PARAMETER",
    "STATUS_NO_LOGON_SERVERS",
    "STATUS_LOGON_FAILURE",
    "STATUS_ACCOUNT_RESTRICTION",
    "STATUS_INVALID_LOGON_HOURS",
    "STATUS_INVALID_WORKSTATION",
    "STATUS_PASSWORD_EXPIRED",
    "STATUS_ACCOUNT_DISABLED",
    "STATUS_LOGON_TYPE_NOT_GRANTED",
    "STATUS_TRUSTED_RELATIONSHIP_FAILURE",
    "STATUS_ACCOUNT_EXPIRED",
    "STATUS_PASSWORD_MUST_CHANGE",
    "STATUS_ACCOUNT_LOCKED_OUT",
    "AS400_STATUS_LOGON_FAILURE"
  };

  SMBSessionRet = SMBSessionSetup(hSocket, _psSessionData, szLogin, szPassword);
  SMBerr = (unsigned long) SMBSessionRet & 0x00FFFFFF;
  SMBaction = ((unsigned long) SMBSessionRet & 0xFF000000) >> 24;

  writeError(ERR_DEBUG, "SMBSessionRet: %8.8X SMBerr: %4.4X SMBaction: %2.2X", SMBSessionRet, SMBerr, SMBaction);
 
  /* Locate appropriate SMB code message */
  pErrorMsg = smbErrorMsg[0]; /* UNKNOWN_ERROR_CODE */
  for (i = 0; i < sizeof(smbErrorCode)/4; i++) {
    if (SMBerr == (smbErrorCode[i] & 0x00FFFFFF)) {
      pErrorMsg = smbErrorMsg[i];
      break;
    }
  }

  switch (SMBerr)
  {
    case 0x000000:
      /*
        Non-domain connected XP and 2003 hosts map non-existant accounts to
        the anonymous user and return SUCCESS during password checks. Medusa
        will check the value of the ACTION flag in the target's response to 
        determine if the account is a legitimate or anonymous success.
      */
      if (SMBaction == 0x01) {
        (*psLogin)->pErrorMsg = malloc( 40 + 1 );
        memset((*psLogin)->pErrorMsg, 0, 40 + 1 );
        sprintf((*psLogin)->pErrorMsg, "Non-existant account. Anonymous success.");
        (*psLogin)->iResult = LOGIN_RESULT_ERROR;
      }
      else
        (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
      
      iRet = MSTATE_EXITING;
      break;
    case 0x00006E:  /* Valid password, GPO Disabling Remote Connections Using NULL Passwords */
    case 0x00015B:  /* Valid password, GPO "Deny access to this computer from the network" */
    case 0x000193:  /* Valid password, account expired */ 
    case 0x000224:  /* Valid password, password expired and must be changed on next logon */
      (*psLogin)->iResult = LOGIN_RESULT_SUCCESS;
      sprintf(ErrorCode, "0x%6.6X:", SMBerr);
      (*psLogin)->pErrorMsg = malloc( strlen(ErrorCode) + strlen(pErrorMsg) + 1);
      memset((*psLogin)->pErrorMsg, 0, strlen(ErrorCode) + strlen(pErrorMsg) + 1);
      strncpy((*psLogin)->pErrorMsg, ErrorCode, strlen(ErrorCode));
      strncat((*psLogin)->pErrorMsg, pErrorMsg, strlen(pErrorMsg));
      iRet = MSTATE_EXITING;
      break;
    case 0x050001:  /* AS/400 -- Incorrect password */
      writeError(ERR_DEBUG_MODULE, "[%s] AS/400 Access is Denied. Incorrect password or disabled account.", MODULE_NAME);
      (*psLogin)->iResult = LOGIN_RESULT_FAIL;
      iRet = MSTATE_RUNNING;
      break;
    case 0x00006D:  /* Incorrect password */
      (*psLogin)->iResult = LOGIN_RESULT_FAIL;
      iRet = MSTATE_RUNNING;
      break;
    default:
      sprintf(ErrorCode, "0x%6.6X:", SMBerr);
      (*psLogin)->pErrorMsg = malloc( strlen(ErrorCode) + strlen(pErrorMsg) + 1);
      memset((*psLogin)->pErrorMsg, 0, strlen(ErrorCode) + strlen(pErrorMsg) + 1);
      strncpy((*psLogin)->pErrorMsg, ErrorCode, strlen(ErrorCode));
      strncat((*psLogin)->pErrorMsg, pErrorMsg, strlen(pErrorMsg));
      (*psLogin)->iResult = LOGIN_RESULT_ERROR;
      iRet = MSTATE_EXITING;
      break;
  }

  if (_psSessionData->hashFlag == MACHINE) { 
    setPassResult((*psLogin), _psSessionData->machine_name);
    iRet = MSTATE_EXITING;
  }
  else {
    setPassResult((*psLogin), szPassword);
  }

  return(iRet);
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


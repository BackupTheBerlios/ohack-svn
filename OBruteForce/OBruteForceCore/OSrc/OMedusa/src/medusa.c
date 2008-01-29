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
 * Based on ideas from Hydra 3.1 by VanHauser [vh@thc.org]
 * Do only use for legal purposes. Illegal purposes cost $1 each.
 *
*/

#define VERSION_SVN "$Id: medusa.c 603 2006-10-18 20:20:10Z jmk $" 

#include <dlfcn.h>
#include "medusa.h"
#include "modsrc/module.h"

char* szModuleName;
char* szTempModuleParam;
char* szModulePaths[3] = {"a", "b", "c"};         // will look at 3 different locations for modules if possible
char** arrModuleParams;    // the "argv" for the module
int nModuleParamCount;    // the "argc" for the module

void freeModuleParams()
{
  int i;

  for (i = 0; i < nModuleParamCount; i++)
  {
    free(arrModuleParams[i]);
  }

  free(arrModuleParams);
}

/*
  Display appropriate usage information for application.
*/
void usage()
{
  writeVerbose(VB_NONE, "");
  writeVerbose(VB_NONE, "Syntax: %s [-h host|-H file] [-u username|-U file] [-p password|-P file] [-C file] -M module [OPT]", PROGRAM);
  writeVerbose(VB_NONE, "  -h [TEXT]    : Target hostname or IP address");
  writeVerbose(VB_NONE, "  -H [FILE]    : File containing target hostnames or IP addresses");
  writeVerbose(VB_NONE, "  -u [TEXT]    : Username to test");
  writeVerbose(VB_NONE, "  -U [FILE]    : File containing usernames to test");
  writeVerbose(VB_NONE, "  -p [TEXT]    : Password to test");
  writeVerbose(VB_NONE, "  -P [FILE]    : File containing passwords to test");
  writeVerbose(VB_NONE, "  -C [FILE]    : File containing combo entries. See README for more information.");
  writeVerbose(VB_NONE, "  -O [FILE]    : File to append log information to");
  writeVerbose(VB_NONE, "  -e [n/s/ns]  : Additional password checks ([n] No Password, [s] Password = Username)");
  writeVerbose(VB_NONE, "  -M [TEXT]    : Name of the module to execute (without the .mod extension)");
  writeVerbose(VB_NONE, "  -m [TEXT]    : Parameter to pass to the module. This can be passed multiple times with a"); 
  writeVerbose(VB_NONE, "                 different parameter each time and they will all be sent to the module (i.e.");
  writeVerbose(VB_NONE, "                 -m Param1 -m Param2, etc.)"); 
  writeVerbose(VB_NONE, "  -d           : Dump all known modules");
  writeVerbose(VB_NONE, "  -n [NUM]     : Use for non-default TCP port number");
  writeVerbose(VB_NONE, "  -s           : Enable SSL");
  writeVerbose(VB_NONE, "  -g [NUM]     : Give up after trying to connect for NUM seconds (default 3)"); 
  writeVerbose(VB_NONE, "  -r [NUM]     : Sleep NUM seconds between retry attempts (default 3)");   
  writeVerbose(VB_NONE, "  -R [NUM]     : Attempt NUM retries before giving up. The total number of attempts will be NUM + 1.");
  writeVerbose(VB_NONE, "  -t [NUM]     : Total number of logins to be tested concurrently");
  writeVerbose(VB_NONE, "  -T [NUM]     : Total number of hosts to be tested concurrently");
  writeVerbose(VB_NONE, "  -L           : Parallelize logins using one username per thread. The default is to process ");
  writeVerbose(VB_NONE, "                 the entire username before proceeding.");
  writeVerbose(VB_NONE, "  -f           : Stop scanning host after first valid username/password found.");
  writeVerbose(VB_NONE, "  -F           : Stop audit after first valid username/password found on any host.");
  writeVerbose(VB_NONE, "  -b           : Suppress startup banner");
  writeVerbose(VB_NONE, "  -q           : Display module's usage information");
  writeVerbose(VB_NONE, "  -v [NUM]     : Verbose level [0 - 6 (more)]");
  writeVerbose(VB_NONE, "  -w [NUM]     : Error debug level [0 - 10 (more)]");
  writeVerbose(VB_NONE, "  -V           : Display version");
  writeVerbose(VB_NONE, "\n");
  return;
}

/*
  Read user options and check validity.
*/
int checkOptions(int argc, char **argv, sAudit *_psAudit)
{
  int opt;
  extern char *optarg;
  extern int   opterr;
  int ret = 0;
  int i = 0;
  int nIgnoreBanner = 0;

  /* initialize options */
  _psAudit->iServerCnt = 1;
  _psAudit->iLoginCnt = 1;
  _psAudit->iParallelLoginFlag = PARALLEL_LOGINS_PASSWORD;
  _psAudit->iPortOverride = 0;                        /* Use default port */
  _psAudit->iUseSSL = 0;                              /* No SSL */
  _psAudit->iTimeout = DEFAULT_WAIT_TIME;             /* Default wait of 3 seconds */
  _psAudit->iRetryWait = WAIT_BETWEEN_CONNECT_RETRY;  /* Default wait of 3 seconds */
  _psAudit->iRetries = MAX_CONNECT_RETRY;             /* Default of 2 retries (3 total attempts) */
  _psAudit->iShowModuleHelp = 0;
  iVerboseLevel = 5;
  iErrorLevel = 5;

  for (i =0; i < argc; i++)
  {
    if (strstr(argv[i], "-b") != NULL)
    {
      nIgnoreBanner = 1;
      break;
    }
  }

  if (nIgnoreBanner == 0)
    writeVerbose(VB_NONE, "%s v%s [%s] (C) %s %s\n", PROGRAM, VERSION, WWW, AUTHOR, EMAIL);

  while ((opt = getopt(argc, argv, "h:H:u:U:p:P:C:O:e:M:m:g:r:R:t:T:n:bqdsLfFVv:w:")) != EOF)
  {
    switch (opt)
    {
    case 'h':
      if (_psAudit->HostType)
      {
        writeError(ERR_ALERT, "Options 'h' and 'H' are exclusive.");
        ret = EXIT_FAILURE;
      }
      else
      {
        _psAudit->pGlobalHost = malloc( strlen(optarg) + 1 );
        memset(_psAudit->pGlobalHost, 0, strlen(optarg) + 1);
        strncpy(_psAudit->pGlobalHost, optarg, strlen(optarg));
        _psAudit->HostType = L_SINGLE;
      }
      break;
    case 'H':
      if (_psAudit->HostType)
      {
        writeError(ERR_ALERT, "Options 'h' and 'H' are exclusive.");
        ret = EXIT_FAILURE;
      }
      else
      {
        _psAudit->pOptHost = malloc( strlen(optarg) + 1 );
        memset(_psAudit->pOptHost, 0, strlen(optarg) + 1);
        strncpy(_psAudit->pOptHost, optarg, strlen(optarg));
        _psAudit->HostType = L_FILE;
      }
      break;
    case 'u':
      if (_psAudit->UserType)
      {
        writeError(ERR_ALERT, "Options 'u' and 'U' are exclusive.");
        ret = EXIT_FAILURE;
      }
      else
      {
        _psAudit->pGlobalUser = malloc( strlen(optarg) + 1 );
        memset(_psAudit->pGlobalUser, 0, strlen(optarg) + 1);
        strncpy(_psAudit->pGlobalUser, optarg, strlen(optarg));
        _psAudit->UserType = L_SINGLE;
        _psAudit->iUserCnt = 1;
      }
      break;
    case 'U':
      if (_psAudit->UserType)
      {
        writeError(ERR_ALERT, "Options 'u' and 'U' are exclusive.");
        ret = EXIT_FAILURE;
      }
      else
      {
        _psAudit->pOptUser = malloc( strlen(optarg) + 1 );
        memset(_psAudit->pOptUser, 0, strlen(optarg) + 1);
        strncpy(_psAudit->pOptUser, optarg, strlen(optarg));
        _psAudit->UserType = L_FILE;
      }
      break;
    case 'p':
      if (_psAudit->PassType)
      {
        writeError(ERR_ALERT, "Options 'p' and 'P' are exclusive.");
        ret = EXIT_FAILURE;
      }
      else
      {
        _psAudit->pGlobalPass = malloc( strlen(optarg) + 2 );
        memset(_psAudit->pGlobalPass, 0, strlen(optarg) + 2);
        strncpy(_psAudit->pGlobalPass, optarg, strlen(optarg));
        _psAudit->PassType = L_SINGLE;
        _psAudit->iPassCnt = 1;
      }
      break;
    case 'P':
      if (_psAudit->PassType)
      {
        writeError(ERR_ALERT, "Options 'p' and 'P' are exclusive.");
        ret = EXIT_FAILURE;
      }
      else
      {
        _psAudit->pOptPass = malloc( strlen(optarg) + 1 );
        memset(_psAudit->pOptPass, 0, strlen(optarg) + 1);
        strncpy(_psAudit->pOptPass, optarg, strlen(optarg));
        _psAudit->PassType = L_FILE;
      }
      break;
    case 'C':
      _psAudit->pOptCombo = malloc( strlen(optarg) + 1 );
      memset(_psAudit->pOptCombo, 0, strlen(optarg) + 1);
      strncpy(_psAudit->pOptCombo, optarg, strlen(optarg));
      break;
    case 'O':
      _psAudit->pOptOutput = malloc( strlen(optarg) + 1 );
      memset(_psAudit->pOptOutput, 0, strlen(optarg) + 1);
      strncpy(_psAudit->pOptOutput, optarg, strlen(optarg));
      break;
    case 'e':
      if (strcmp(optarg, "n") == 0)
      {
        _psAudit->iPasswordBlankFlag = TRUE;
        _psAudit->iPasswordUsernameFlag = FALSE;
      }
      else if (strcmp(optarg, "s") == 0)
      {
        _psAudit->iPasswordBlankFlag = FALSE;
        _psAudit->iPasswordUsernameFlag = TRUE;
      }
      else if ((strcmp(optarg, "ns") == 0) || (strcmp(optarg, "sn") == 0))
      {
        _psAudit->iPasswordBlankFlag = TRUE;
        _psAudit->iPasswordUsernameFlag = TRUE;
      }
      else
      {
        writeError(ERR_ALERT, "Option 'e' requires value of n, s, or ns.");
        ret = EXIT_FAILURE;
      }
      break;
    case 's':
      _psAudit->iUseSSL = 1;  
      break;
    case 'L':
      _psAudit->iParallelLoginFlag = PARALLEL_LOGINS_USER;
      break;
    case 'f':
      _psAudit->iFoundPairExitFlag = FOUND_PAIR_EXIT_HOST;
      break;
    case 'F':
      _psAudit->iFoundPairExitFlag = FOUND_PAIR_EXIT_AUDIT;
      break;
    case 't':
      _psAudit->iLoginCnt = atoi(optarg);
      break;
    case 'T':
      _psAudit->iServerCnt = atoi(optarg);
      break;
    case 'n':
      _psAudit->iPortOverride = atoi(optarg);
      break;
    case 'v':
      iVerboseLevel = atoi(optarg);
      break;
    case 'w':
      iErrorLevel = atoi(optarg);
      break;
    case 'V':
      writeVerbose(VB_EXIT, "");  // Terminate now
      break;
    case 'M':
      szModuleName = malloc(strlen(optarg) + 1);
      memset(szModuleName, 0, strlen(optarg) + 1);
      strncpy(szModuleName, optarg, strlen(optarg));
      _psAudit->pModuleName = szModuleName;
      break;
    case 'm':
      nModuleParamCount++;
      szTempModuleParam = malloc(strlen(optarg) + 1);
      memset(szTempModuleParam, 0, strlen(optarg) + 1);
      strncpy(szTempModuleParam, optarg, strlen(optarg));
      arrModuleParams = realloc(arrModuleParams, nModuleParamCount * sizeof(char*));
      arrModuleParams[nModuleParamCount - 1] = szTempModuleParam;
      break;
    case 'd':
      listModules(szModulePaths, 1);  // End the program after this executes by passing a 1 as the second param
      break;
    case 'b':
      // Do nothing - supression of the startup banner is handled before the switch statement
      break;
    case 'q':
      _psAudit->iShowModuleHelp = 1;
      break;
    case 'g':
      _psAudit->iTimeout = atoi(optarg);
      break;
    case 'r':
      _psAudit->iRetryWait = atoi(optarg);
      break;
    case 'R':
      _psAudit->iRetries = atoi(optarg);
      break;
    default:
      writeError(ERR_CRITICAL, "Unknown error processing command-line options.");
      ret = EXIT_FAILURE;
    }
  }

  if (argc <= 1) {
    ret = EXIT_FAILURE;
  }
  
  if (_psAudit->iShowModuleHelp)
  {
    ret = invokeModule(_psAudit->pModuleName, NULL, NULL, NULL);
    if (ret < 0)
    {
      writeError(ERR_CRITICAL, "invokeModule failed - see previous errors for an explanation");
    }
  }
  else
  {
    if ( !((_psAudit->HostType) || (_psAudit->pOptCombo)) )
    {
      writeError(ERR_ALERT, "Host information must be supplied.");
      ret = EXIT_FAILURE;
    }
    else if ( !((_psAudit->UserType) || (_psAudit->pOptCombo)) )
    {
      writeError(ERR_ALERT, "User logon information must be supplied.");
      ret = EXIT_FAILURE;
    }
    else if ( !((_psAudit->PassType) || (_psAudit->pOptCombo) || (_psAudit->iPasswordBlankFlag) || ( _psAudit->iPasswordUsernameFlag)) )
    {
      writeError(ERR_ALERT, "Password information must be supplied.");
      ret = EXIT_FAILURE;
    }
  }

  return ret;
}

int invokeModule(char* pModuleName, sLogin* pLogin, int argc, char* argv[])
{
  void    *pLibrary;
  int    iReturn;
  function_go  pGo;
  function_showUsage pUsage;
  char* modPath;
  int nPathLength;
  int i;
  int nSuccess = 0;

  iReturn   = -1;
  pLibrary  = NULL;
  pGo       = NULL;
  pUsage    = NULL;

  if (NULL == pModuleName)
  {
    listModules(szModulePaths, 0);
    writeError(ERR_CRITICAL, "invokeModule called with no name");
    return -1;
  }

  // Find the first available path to use
  for(i = 0; i < 3; i++)
  {
    if (szModulePaths[i] != NULL)
    {
      // Is the module available under here?
      writeError(ERR_DEBUG, "Trying module path of %s", szModulePaths[i]);
      nPathLength = strlen(szModulePaths[i]) + strlen(pModuleName) + strlen(MODULE_EXTENSION) + 2;  // Going to add a slash too
      modPath = malloc(nPathLength);
      memset(modPath, 0, nPathLength);
      strncpy(modPath, szModulePaths[i], strlen(szModulePaths[i]));
      strncat(modPath, "/", 1);
      strncat(modPath, pModuleName, strlen(pModuleName));
      strncat(modPath, MODULE_EXTENSION, strlen(MODULE_EXTENSION));

      // Now try the load
      writeError(ERR_DEBUG, "Attempting to load %s", modPath);
      pLibrary = dlopen(modPath, RTLD_NOW);

      if (pLibrary == NULL)
      {
        continue;
      }
      else if (!pLogin)
      {
        pUsage = (function_showUsage)dlsym(pLibrary, "showUsage");
       
        writeError(ERR_DEBUG, "Attempting to display usage information for module: %s", modPath);
        
        if (pUsage == NULL)
        {
          writeError(ERR_ALERT, "Couldn't get a pointer to \"showUsage\" for module %s [%s]", modPath, dlerror());
          return -1;
        }
        else
        {
          nSuccess = 1;
          pUsage();
        }
        dlclose(pLibrary);
        exit(EXIT_SUCCESS); // TEMP FIX
      }
      else
      {
        pGo = (function_go)dlsym(pLibrary, "go");

        if (pGo == NULL)
        {
          writeError(ERR_ALERT, "Couldn't get a pointer to \"go\" for module %s [%s]", modPath, dlerror());
          return -1;
        }
        else
        {
          nSuccess = 1;
          iReturn = pGo(pLogin, argc, argv);
          break;
        }
        dlclose(pLibrary);
      }
    }
  }

  if (!nSuccess)
  {
    writeVerbose(VB_IMPORTANT, "Couldn't load \"%s\" [%s]. Place the module in the medusa directory, set the MEDUSA_MODULE_NAME environment variable or run the configure script again using --with-default-mod-path=[path].", pModuleName, dlerror());
    iReturn = -1;
  }

  return iReturn;
}

void* startModule(void* pParams)
{
  int nRet = 0;
  sModuleStart* modParams = (sModuleStart*)pParams;
  if (NULL == modParams)
  {
    writeError(ERR_FATAL, "Bad pointer passed to invokeModule");
    nRet = -1;
    return (void*) nRet;
  }

  writeError(ERR_DEBUG, "startModule iId: %d pLogin: %X modParams->argv: %X modParams: %X", modParams->pLogin->iId, modParams->pLogin, modParams->argv, modParams);
  nRet = invokeModule(modParams->szModuleName, modParams->pLogin, modParams->argc, modParams->argv);
  if (nRet < 0)
  {
    writeVerbose(VB_EXIT, "invokeModule failed - see previous errors for an explanation");
    nRet = -1;
    return (void*) nRet;
  }

  return (void*) nRet;    // Success
}

/*
  Read the contents of a user supplied file. Store contents in memory and provide
  a count of the total file lines processed.
*/
void loadFile(char *pFile, char **pFileContent, int *iFileCnt)
{
  FILE *pfFile;
  size_t stFileSize = 0;
  char tmp[MAX_BUF];
  char *ptr;

  *iFileCnt = 0;

  if ((pfFile = fopen(pFile, "r")) == NULL)
  {
    writeError(ERR_FATAL, "Failed to open file %s - %s", pFile, strerror( errno ) );
  }
  else
  {
    /* get file stats */
    while (! feof(pfFile) )
    {
      if ( fgets(tmp, MAX_BUF, pfFile) != NULL )
      {
        if (tmp[0] != '\0')
        {
          stFileSize += strlen(tmp) + 1;
          (*iFileCnt)++;
        }
      }
    }
    rewind(pfFile);

    *pFileContent = malloc(stFileSize + 1);    /* extra end NULL */

    if (pFileContent == NULL)
    {
      writeError(ERR_FATAL, "Failed to allocate memory for file %s.", pFile);
    }

    memset(*pFileContent, 0, stFileSize + 1);
    ptr = *pFileContent;

    /* load file into mem */
    while (! feof(pfFile) )
    {
      if (fgets(tmp, MAX_BUF, pfFile) != NULL)
      {
        /* ignore blank lines */
        if ((tmp[0] == '\n') || (tmp[0] == '\r'))
        {
          (*iFileCnt)--;
          writeError(ERR_DEBUG, "Ignoring blank line in file: %s. Resetting total count: %d.", pFile, (*iFileCnt));
        }
        else if (tmp[0] != '\0')
        {
          if (tmp[strlen(tmp) - 1] == '\n') tmp[strlen(tmp) - 1] = '\0';
          if (tmp[strlen(tmp) - 1] == '\r') tmp[strlen(tmp) - 1] = '\0';
          memcpy(ptr, tmp, strlen(tmp) + 1);
          ptr += strlen(tmp) + 1;
        }
      }
    }
    *ptr = '\0';  /* extra NULL to identify end of list */
  }

  free(pFile);
  return;
}

/*
  Examine the first row of the combo file to determine information provided.
  Combo files are colon separated and in the following format: host:user:password.
  If any of the three fields are left empty, the respective information should be
  provided either as a single global value or as a list in a file.

  The following combinations are possible in the combo file:
  1. foo:bar:fud
  2. foo:bar:
  3. foo::
  4. :bar:fud
  5. :bar:
  6. ::fud
  7. foo::fud

  Medusa also supports using PwDump files as a combo file. The format of these
  files should be user:id:lm:ntlm. We look for ':::' at the end of the first line
  to determine if the file contains PwDump output.
*/
int processComboFile(sAudit **_psAudit)
{
  int ret = 0, iColonCount = 0;
  char *pComboTmp;

  writeError(ERR_DEBUG, "[processComboFile] Processing user supplied combo file.");

  pComboTmp = (*_psAudit)->pGlobalCombo;

  /* PwDump file check */
  /* USERNAME:ID:LM HASH:NTLM HASH::: */
  writeError(ERR_DEBUG, "[processComboFile] PwDump file check.");
  while (*pComboTmp != '\0')
  {
    if (strcmp(pComboTmp, ":::") == 0)
    {
      iColonCount += 3;
      pComboTmp += 3;
    }
    else if (*pComboTmp == ':')
    {
      iColonCount++;
      pComboTmp++;
    }
    else 
    {
      pComboTmp++;
    }

    if ((iColonCount == 6) && (*pComboTmp == '\0')) {
      writeError(ERR_DEBUG, "[processComboFile] Combo format scan detected PwDump file.");

      if (((*_psAudit)->HostType != L_SINGLE) && ((*_psAudit)->HostType != L_FILE))
      {
        writeError(ERR_FATAL, "Combo format used requires host information via (-h/-H).");
      }

      if (((*_psAudit)->UserType != L_SINGLE) && ((*_psAudit)->UserType != L_FILE))
      {
        (*_psAudit)->UserType = L_PWDUMP;
      }

      (*_psAudit)->PassType = L_PWDUMP;

      return ret;
    }
  }

  if (iColonCount != 2)
  {
    writeError(ERR_DEBUG, "[processComboFile] Number of colons detected in first entry: %d", iColonCount);
    writeError(ERR_FATAL, "Invalid combo file format.");
  }

  pComboTmp = (*_psAudit)->pGlobalCombo;

  if (*pComboTmp == ':')
  {               /* no host specified */
    writeError(ERR_DEBUG, "[processComboFile] No host combo field specified.");
    if (((*_psAudit)->HostType != L_SINGLE) && ((*_psAudit)->HostType != L_FILE))
    {
      writeError(ERR_FATAL, "Combo format used requires host information via (-h/-H).");
    }
  }
  else
  {
    writeError(ERR_DEBUG, "[processComboFile] Host combo field specified.");
    (*_psAudit)->HostType = L_COMBO;

    while (*pComboTmp != ':')
    {
      if (pComboTmp == NULL)
      {
        writeError(ERR_FATAL, "Failed to process combo file. Incorrect format.");
      }

      pComboTmp++;
    }
  }
  pComboTmp++;

  if (*pComboTmp == ':')
  {              /* no user specified */
    writeError(ERR_DEBUG, "[processComboFile] No user combo field specified.");
    if (((*_psAudit)->UserType != L_SINGLE) && ((*_psAudit)->UserType != L_FILE))
    {
      writeError(ERR_FATAL, "Combo format used requires user information via (-u/-U).");
    }
  }
  else
  {
    writeError(ERR_DEBUG, "[processComboFile] User combo field specified.");
    (*_psAudit)->UserType = L_COMBO;

    while (*pComboTmp != ':')
    {
      if (pComboTmp == NULL)
      {
        writeError(ERR_FATAL, "Failed to process combo file. Incorrect format.");
      }

      pComboTmp++;
    }
  }
  pComboTmp++;

  if (*pComboTmp == '\0')
  {             /* no password specified */
    writeError(ERR_DEBUG, "[processComboFile] No password combo field specified.");
    if (((*_psAudit)->PassType != L_SINGLE) && ((*_psAudit)->PassType != L_FILE))
    {
      writeError(ERR_FATAL, "Combo format used requires password information via (-p/-P).");
    }
  }
  else
  {
    writeError(ERR_DEBUG, "[processComboFile] Password combo field specified.");
    (*_psAudit)->PassType = L_COMBO;
  }

  return ret;
}


/*
  Return next user-specified host during audit data table building process.
  This host information may be a single global entry, from a file containing
  a list of hosts, or from a combo file.
*/
char* findNextHost(sAudit *_psAudit, char *_pHost)
{

  if (_psAudit->pGlobalCombo)
  {
    writeError(ERR_DEBUG, "[findNextHost] Process global combo file.");
    /* advance to next entry in combo list */
    if ((_psAudit->iUserListFlag == LIST_COMPLETE) && (_psAudit->iHostListFlag == LIST_COMPLETE))
    {
      writeError(ERR_DEBUG, "[findNextHost] Advance to next entry in combo list.");
      /* skip host */
      while (*_psAudit->pGlobalCombo != '\0')
        _psAudit->pGlobalCombo++;
      _psAudit->pGlobalCombo++;

      /* skip user */
      while (*_psAudit->pGlobalCombo != '\0')
        _psAudit->pGlobalCombo++;
      _psAudit->pGlobalCombo++;

      /* skip pass */
      while (*_psAudit->pGlobalCombo != '\0')
        _psAudit->pGlobalCombo++;
      _psAudit->pGlobalCombo++;

      if (*_psAudit->pGlobalCombo == '\0')
      {
        _psAudit->iAuditFlag = AUDIT_COMPLETE;
      }
      else
      {
        _psAudit->iAuditFlag = AUDIT_IN_PROGRESS;
      }
    }

    /* convert ':' to '\0' in combo entries */
    if ((_psAudit->pComboEntryTmp == NULL) || ((_psAudit->iUserListFlag == LIST_COMPLETE) && (_psAudit->iHostListFlag == LIST_COMPLETE)))
    {
      writeError(ERR_DEBUG, "[findNextHost] Convert ':' to '\\0' in combo entries.");
      _psAudit->pComboEntryTmp = _psAudit->pGlobalCombo;

      if (*_psAudit->pComboEntryTmp != '\0')
      {
        /* host:user ==> host\0user */
        while (*_psAudit->pComboEntryTmp != ':')
          _psAudit->pComboEntryTmp++;
        memset(_psAudit->pComboEntryTmp, 0, 1);

        /* user:pass ==> user\0pass */
        while (*_psAudit->pComboEntryTmp != ':')
          _psAudit->pComboEntryTmp++;
        memset(_psAudit->pComboEntryTmp, 0, 1);
      }
    }

    _psAudit->pComboEntryTmp = _psAudit->pGlobalCombo;
  }
  else
  {
    if ((_psAudit->iUserListFlag == LIST_COMPLETE) && (_psAudit->iHostListFlag == LIST_COMPLETE))
    {
      _psAudit->iAuditFlag = AUDIT_COMPLETE;
    }
  }

  _psAudit->iHostListFlag = LIST_COMPLETE;

  if (_psAudit->iAuditFlag == AUDIT_COMPLETE)
  {
    _pHost = NULL;
  }
  else if (_psAudit->HostType == L_COMBO)
  {
    if (*_psAudit->pGlobalCombo == '\0')
    {
      _pHost = NULL;
    }
    else
    {
      _pHost = _psAudit->pGlobalCombo;
    }
  }
  else if (_psAudit->HostType == L_FILE)
  {
    _pHost = _psAudit->pGlobalHost;

    if (*_psAudit->pGlobalHost != '\0')
    {
      /* advancing host list */
      while (*_psAudit->pGlobalHost != '\0')
        _psAudit->pGlobalHost++;
      _psAudit->pGlobalHost++;

      if (*_psAudit->pGlobalHost != '\0')
      {
        _psAudit->iHostListFlag = LIST_IN_PROGRESS;
      }
      else
      {
        /* resetting host list */
        _psAudit->pGlobalHost = _psAudit->pHostFile;
      }
    }
  }
  else if (_psAudit->HostType == L_SINGLE)
  {
    _pHost = _psAudit->pGlobalHost;
    _psAudit->iAuditFlag = AUDIT_COMPLETE;
  }
  else
  {
    writeError(ERR_FATAL, "[findNextHost] HostType not properly defined.");
  }

  return _pHost;
}


/*
  Return next user-specified user during audit data table building process.
  This host information may be a single global entry, from a file containing
  a list of users, or from a combo file.
*/
char* findNextUser(sAudit *_psAudit, char *_pUser)
{
  char* pComboTmp;

  _psAudit->iUserListFlag = LIST_COMPLETE;

  if (_psAudit->UserType == L_COMBO)
  {
    /* advance to username */
    if (_psAudit->pGlobalCombo)
    {
      pComboTmp = _psAudit->pComboEntryTmp;
      while (*pComboTmp != '\0')
        pComboTmp++;
      pComboTmp++;
    }

    if (_pUser != NULL)
      _pUser = NULL;
    else
      _pUser = pComboTmp;

    writeError(ERR_DEBUG, "[findNextUser] Combo User: %s", _pUser);
  }
  else if (_psAudit->UserType == L_PWDUMP)
  {
    if (_pUser != NULL)
      _pUser = NULL;
    else
      _pUser = _psAudit->pComboEntryTmp;

    writeError(ERR_DEBUG, "[findNextUser] PwDump User: %s", _pUser);
  }
  else if (_psAudit->UserType == L_FILE)
  {
    _pUser = _psAudit->pGlobalUser;

    if (*_psAudit->pGlobalUser != '\0')
    {
      /* advance user list pointer */
      while (*_psAudit->pGlobalUser != '\0')
        _psAudit->pGlobalUser++;
      _psAudit->pGlobalUser++;

      _psAudit->iUserListFlag = LIST_IN_PROGRESS;
    }
    else
    {
      /* reset list */
      _psAudit->pGlobalUser = _psAudit->pUserFile;
      _pUser = NULL;
    }

    writeError(ERR_DEBUG, "[findNextUser] L_FILE User: %s", _pUser);
  }
  else if (_psAudit->UserType == L_SINGLE)
  {
    if (_pUser != NULL)
      _pUser = NULL;
    else
      _pUser = _psAudit->pGlobalUser;
  }
  else
  {
    writeError(ERR_FATAL, "[findNextUser] UserType (%d) not properly defined.", _psAudit->UserType);
  }

  return _pUser;
}

/*
  Return next user-specified password during audit data table building process.
  This password information is only from the combo file.
*/
char* findLocalPass(sAudit *_psAudit)
{
  char *pPass;
  char *pComboTmp;

  if ((_psAudit->PassType == L_COMBO) || (_psAudit->PassType == L_PWDUMP))
  {
    /* advance to password */
    if (_psAudit->pGlobalCombo)
    {
      pComboTmp = _psAudit->pComboEntryTmp;

      while (*pComboTmp != '\0')
        pComboTmp++;
      pComboTmp++;

      while (*pComboTmp != '\0')
        pComboTmp++;
      pComboTmp++;
    }

    pPass = pComboTmp;
    writeError(ERR_DEBUG, "[findLocalPass] pPass: %s", pPass);
  }
  else
  {
    pPass = NULL;
  }

  return pPass;
}


/*
  Grab the next password for a particular user
*/
char* getNextPass(sAudit *_psAudit, sUser *_psUser)
{
  char *pPass = NULL;

  pthread_mutex_lock(&_psUser->ptmMutex);

  if (_psUser->iPassStatus != PL_DONE)
  {
    if ((_psUser->iPassStatus == PL_NULL) || (_psUser->iPassStatus == PL_USERNAME))
    {
      if ((_psUser->iPassStatus == PL_NULL) && (_psAudit->iPasswordBlankFlag))
      {
        pPass = "";
        _psUser->iPassStatus = PL_USERNAME;
      }
      else if (_psAudit->iPasswordUsernameFlag)
      {
        pPass = _psUser->pUser;
        _psUser->iPassStatus = PL_LOCAL;
      }
      else
      {
        _psUser->iPassStatus = PL_LOCAL;
      }
    }

    if (pPass == NULL )
    {
      if ((_psUser->iPassStatus == PL_LOCAL) && (_psUser->psPassCurrent))
      {
        pPass = _psUser->psPassCurrent->pPass;
        _psUser->psPassCurrent = _psUser->psPassCurrent->psPassNext;
      }
      else if (_psAudit->pGlobalPass)
      {
        _psUser->iPassStatus = PL_GLOBAL;

        if (_psUser->pPass)
        {
          while (*_psUser->pPass != '\0')
            _psUser->pPass++;
          _psUser->pPass++;

          if (*_psUser->pPass != '\0')
          {
            pPass = _psUser->pPass;
          }
          else
          {
            _psUser->iPassStatus = PL_DONE;
          }
        }
        else
        {
          _psUser->pPass = _psAudit->pGlobalPass;
          pPass = _psUser->pPass;
        }
      }
      else
      {
        _psUser->iPassStatus = PL_DONE;
      }
    }
  }
  else
  {
    writeError(ERR_DEBUG_AUDIT, "PL_DONE");
  }

  pthread_mutex_unlock(&_psUser->ptmMutex);

  return pPass;
}

/*
  Handle module password result call
*/
int setPassResult(sLogin *_psLogin, char *_pPass)
{
  pthread_mutex_lock(&_psLogin->psUser->ptmMutex);

  writeVerbose(VB_CHECK,
               "[%s] Host: %s (%d/%d) User: %s (%d/%d) Password: %s (%d/%d)",
               _psLogin->psServer->psAudit->pModuleName,
               _psLogin->psServer->pHostIP,
               _psLogin->psServer->psAudit->iHostsDone + 1,
               _psLogin->psServer->psAudit->iHostCnt,
               _psLogin->psUser->pUser,
               _psLogin->psUser->iId,
               _psLogin->psServer->psHost->iUserCnt,
               _pPass,
               _psLogin->psUser->iLoginsDone + 1,
               _psLogin->psUser->iPassCnt
              );

  _psLogin->iLoginsDone++;
  _psLogin->psUser->iLoginsDone++,
  _psLogin->psServer->iLoginsDone++;

  switch (_psLogin->iResult)
  {
  case LOGIN_RESULT_SUCCESS:
    if (_psLogin->pErrorMsg) {
      writeVerbose(VB_FOUND, "[%s] Host: %s User: %s Password: %s [SUCCESS (%s)]", _psLogin->psServer->psAudit->pModuleName, _psLogin->psServer->pHostIP, _psLogin->psUser->pUser, _pPass, _psLogin->pErrorMsg);
      free(_psLogin->pErrorMsg);
      _psLogin->pErrorMsg = NULL;
    }
    else
      writeVerbose(VB_FOUND, "[%s] Host: %s User: %s Password: %s [SUCCESS]", _psLogin->psServer->psAudit->pModuleName, _psLogin->psServer->pHostIP, _psLogin->psUser->pUser, _pPass);
    
    _psLogin->psServer->psAudit->iValidPairFound = TRUE;
    _psLogin->psServer->iValidPairFound = TRUE;
    _psLogin->psUser->iPassStatus = PL_DONE;
    break;
  case LOGIN_RESULT_FAIL:
    if (_psLogin->pErrorMsg) {
      writeError(ERR_INFO, "[%s] Host: %s User: %s [FAILED (%s)]", _psLogin->psServer->psAudit->pModuleName, _psLogin->psServer->pHostIP, _psLogin->psUser->pUser, _psLogin->pErrorMsg);
      free(_psLogin->pErrorMsg);
      _psLogin->pErrorMsg = NULL;
    }
    else
      writeError(ERR_INFO, "[%s] Host: %s User: %s [FAILED]", _psLogin->psServer->psAudit->pModuleName, _psLogin->psServer->pHostIP, _psLogin->psUser->pUser);
    
    break;
  case LOGIN_RESULT_ERROR:
    if (_psLogin->pErrorMsg) {
      writeVerbose(VB_FOUND, "[%s] Host: %s User: %s Password: %s [ERROR (%s)]", _psLogin->psServer->psAudit->pModuleName, _psLogin->psServer->pHostIP, _psLogin->psUser->pUser, _pPass, _psLogin->pErrorMsg);
      free(_psLogin->pErrorMsg);
      _psLogin->pErrorMsg = NULL;
    }
    else
      writeVerbose(VB_FOUND, "[%s] Host: %s User: %s Password: %s [ERROR]", _psLogin->psServer->psAudit->pModuleName, _psLogin->psServer->pHostIP, _psLogin->psUser->pUser, _pPass);
    
    _psLogin->psUser->iPassStatus = PL_DONE;
    break;
  default:
    writeError(ERR_INFO, "[%s] Host: %s User: %s [UNKNOWN %d]", _psLogin->psServer->psAudit->pModuleName, _psLogin->psServer->pHostIP, _psLogin->psUser->pUser, _psLogin->iResult);
    break;
  }

  pthread_mutex_unlock(&_psLogin->psUser->ptmMutex);
}

int loadLoginInfo(sAudit *_psAudit)
{
  int res;

  sHost *psHost = NULL;
  sHost *psHostPrevTmp = NULL;
  char *pHost = NULL;

  sUser *psUser = NULL;
  char *pUser = NULL;

  sPass *psPass = NULL;
  char *pPass = NULL;

  /* initialize / reset */
  _psAudit->iHostCnt = 0;
  _psAudit->iHostsDone = 0;

  while ((pHost = findNextHost(_psAudit, pHost)))
  {
    /* combo file: search list to see if host has already been added */
    psHost = _psAudit->psHostRoot;
    while (psHost)
    {
      if ( strcmp(pHost,psHost->pHost) )
        psHost = psHost->psHostNext;
      else
        break;
    }

    /* create new host table in list */
    if (psHost == NULL)
    {
      _psAudit->iHostCnt++;
      psHost = malloc(sizeof(sHost));
      memset(psHost, 0, sizeof(sHost));

      /* set root pointer if this is the first host */
      if (_psAudit->psHostRoot == NULL)
      {
        _psAudit->psHostRoot = psHost;
        psHostPrevTmp = _psAudit->psHostRoot;
      }
      else
      {
        psHostPrevTmp->psHostNext = psHost;
        psHostPrevTmp = psHost;
      }

      psHost->pHost = malloc( strlen(pHost) + 1 );
      memset(psHost->pHost, 0, strlen(pHost) + 1);
      strncpy(psHost->pHost, pHost, strlen(pHost) + 1);
      psHost->iPortOverride = _psAudit->iPortOverride;
      psHost->iUseSSL = _psAudit->iUseSSL;
      psHost->iTimeout = _psAudit->iTimeout;
      psHost->iRetryWait = _psAudit->iRetryWait;
      psHost->iRetries = _psAudit->iRetries;
      psHost->iUserCnt = 0;
      psHost->iHostStatus = HL_NULL; 
    }

    while ((pUser = findNextUser(_psAudit, pUser)))
    {
      /* combo file: search list to see if user has already been added */
      psUser = psHost->psUser;
      while (psUser)
      {
        if ( strcmp(pUser,psUser->pUser) )
          psUser = psUser->psUserNext;
        else
          break;
      }

      /* create new user table in list */
      if (psUser == NULL)
      {
        psHost->iUserCnt++;
        psUser = malloc(sizeof(sUser));
        memset(psUser, 0, sizeof(sUser));

        if (psHost->psUserPrevTmp)
        {
          /* setting host next user pointer */
          psHost->psUserPrevTmp->psUserNext = psUser;
        }
        else
        {
          /* setting host root user pointer */
          psHost->psUser = psUser;
        }

        psHost->psUserPrevTmp = psUser;

        psUser->pUser = malloc( strlen(pUser) + 1 );
        memset(psUser->pUser, 0, strlen(pUser) + 1);
        strncpy(psUser->pUser, pUser, strlen(pUser));
        psUser->iPassCnt = _psAudit->iPassCnt;
        psUser->iPassStatus = PL_NULL;
        psUser->iId = psHost->iUserCnt;
        psHost->iUserStatus = UL_NULL;
        psHost->iUserPassCnt += _psAudit->iPassCnt;

        if (_psAudit->iPasswordUsernameFlag) {
          psHost->iUserPassCnt++;
          psUser->iPassCnt++;
        }

        if (_psAudit->iPasswordBlankFlag) {
          psHost->iUserPassCnt++;
          psUser->iPassCnt++;
        }

        res = pthread_mutex_init(&psUser->ptmMutex, NULL);
        if (res != 0)
        {
          writeError(ERR_FATAL, "Mutex initialization failed - %s", strerror( errno ) );
        }
      }

      pPass = findLocalPass(_psAudit);
      if (pPass)
      {
        psPass = malloc(sizeof(sPass));
        memset(psPass, 0, sizeof(sPass));
        psPass->pPass = malloc( strlen(pPass) + 1 );
        memset(psPass->pPass, 0, strlen(pPass) + 1);
        strncpy(psPass->pPass, pPass, strlen(pPass));
        psUser->iPassCnt++;
        psHost->iUserPassCnt++;

        if (psUser->psPassPrevTmp)
        {
          /* setting user next pass pointer */
          psUser->psPassPrevTmp->psPassNext = psPass;
        }
        else
        {
          /* setting user root pass pointer */
          psUser->psPass = psPass;
          psUser->psPassCurrent = psPass;
        }

        psUser->psPassPrevTmp = psPass;
      }
    }
  }

  return SUCCESS;
}


/*  This function manages the per login threads
 *  1 host, 1+ logins
 *  each module called with 1 username and 1 password pointer
*/
void *startServer(void *arg)
{
  sServer *_psServer = (sServer *)arg;
  sLogin psLogin[_psServer->psAudit->iLoginCnt];
  pthread_t *pptLogin;
  void *thread_result;
  int iLoginId = 0;
  int res;
  sHost *psHost = NULL;
  sUser *psUser = NULL;
  char *pPass = NULL;
  struct hostent *sHostent = NULL;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  int iLoginCnt = _psServer->psAudit->iLoginCnt;
  int iIdleLoginCnt;
  int iLoginDoneCnt = 0;

  writeError(ERR_DEBUG_SERVER, "[startServer] server: %d iUserPassCnt: %d iLoginCnt: %d", _psServer->iId, _psServer->psHost->iUserPassCnt, iLoginCnt);

  if (iLoginCnt > _psServer->psHost->iUserPassCnt)
    iLoginCnt = _psServer->psHost->iUserPassCnt;

  iIdleLoginCnt = iLoginCnt;

  pptLogin = malloc(iLoginCnt * sizeof(pthread_t));
  memset(pptLogin, 0, iLoginCnt * sizeof(pthread_t));

  /* initialize login modules */
  for (iLoginId = 0; iLoginId < iLoginCnt; iLoginId++)
  {
    psLogin[iLoginId].iId = iLoginId;
    psLogin[iLoginId].psServer = _psServer;
    psLogin[iLoginId].iStatus = LOGIN_IDLE;
    psLogin[iLoginId].iResult = LOGIN_RESULT_UNKNOWN;
    psLogin[iLoginId].pErrorMsg = NULL;
    psLogin[iLoginId].iLoginsDone = 0;
  }

  writeError(ERR_DEBUG_SERVER, "[startServer] server: %d host: %s user_count: %d parallel logins: %d", _psServer->iId, _psServer->psHost->pHost, _psServer->psHost->iUserCnt, iLoginCnt);

  sHostent = gethostbyname(_psServer->psHost->pHost);
  if (!sHostent) {
    writeError(ERR_CRITICAL, "Failed to resolve hostname: %s", _psServer->psHost->pHost);
    _psServer->iStatus = SRV_DONE;
  }
  else
  {
    _psServer->pHostIP = inet_ntoa(* (struct in_addr *) sHostent->h_addr);
    writeError(ERR_DEBUG_SERVER, "[server] set host: %s", _psServer->pHostIP);
    
    if (inet_addr(_psServer->pHostIP) == INADDR_NONE)
    {
      writeError(ERR_CRITICAL, "'%s' is not a valid IP address (inet_addr failed)", _psServer->pHostIP);
      _psServer->iStatus = SRV_DONE;
    }
  }

  psUser = _psServer->psHost->psUser;
  while (_psServer->iStatus != SRV_DONE)
  {
    for (iLoginId = 0; iLoginId < iLoginCnt; iLoginId++)
    {
      if ((psUser) && (psLogin[iLoginId].iStatus == LOGIN_IDLE))
      {
        writeError(ERR_DEBUG_SERVER, "[server] Starting login module: %d", iLoginId);

        psLogin[iLoginId].iStatus = LOGIN_ACTIVE;
        psLogin[iLoginId].psUser = psUser;
        iIdleLoginCnt--;

        sModuleStart* modParams;
        modParams = malloc(sizeof(sModuleStart));
        memset(modParams, 0, sizeof(sModuleStart));
        modParams->szModuleName = szModuleName;
        modParams->pLogin = &(psLogin[iLoginId]); //psLogin + (iLoginId * sizeof(sLogin));
        modParams->argc = nModuleParamCount;
        modParams->argv = (char**)arrModuleParams;

        writeError(ERR_DEBUG_SERVER, "iLoginId: %d modParams.pLogin.iId: %d pLogin %X modParams: %X", iLoginId, modParams->pLogin->iId, modParams->pLogin, modParams);

        res = pthread_create(&(pptLogin[iLoginId]), NULL, startModule, (void *)modParams);
        if (res != 0)
        {
          writeError(ERR_WARNING, "Login thread creation failed. Sometimes less is better.");
          psLogin[iLoginId].iStatus = LOGIN_IDLE;
        }
        else {
          // multiple login threads of same user -or-
          // multiple login threads of 1 unique user per thread
          if (_psServer->psAudit->iParallelLoginFlag == PARALLEL_LOGINS_PASSWORD)
          {
            if (psUser->iPassStatus != PL_DONE)
            {
              writeError(ERR_DEBUG_SERVER, "[server] (PARALLEL_LOGINS_PASSWORD) setting SAME user: %s", psUser->pUser);
            }
            else
            {
              pthread_mutex_lock(&_psServer->ptmMutex);
              psUser = psUser->psUserNext;
              _psServer->psHost->iUsersDone++;
              if (psUser)
                writeError(ERR_DEBUG_SERVER, "[server] (PARALLEL_LOGINS_PASSWORD) setting NEW user: %s", psUser->pUser);
              pthread_mutex_unlock(&_psServer->ptmMutex);
            }
          }
          else
          {
            pthread_mutex_lock(&_psServer->ptmMutex);

            if (psUser->iPassStatus == PL_DONE)
              _psServer->psHost->iUsersDone++;

            psUser = psUser->psUserNext;
            if (psUser)
              writeError(ERR_DEBUG_SERVER, "[server] (PARALLEL_LOGINS_USER) setting NEW user: %s", psUser->pUser);

            pthread_mutex_unlock(&_psServer->ptmMutex);
          }
        }
      }
      else if (psLogin[iLoginId].iStatus == LOGIN_ACTIVE)
      {
        // currently scanning... check if hung???
      }
      else if (psLogin[iLoginId].iStatus == LOGIN_DONE)
      {
          writeError(ERR_DEBUG_SERVER, "[%d] Login is done, thread is %X", iLoginId, pptLogin[iLoginId]);
          res = pthread_join(pptLogin[iLoginId], &thread_result);
          writeError(ERR_DEBUG_SERVER, "Join complete");

          writeError(ERR_DEBUG_SERVER, "[server] login thread %d completed %d of %d logins.", iLoginId, psLogin[iLoginId].iLoginsDone, _psServer->psHost->iUserPassCnt);
          writeError(ERR_DEBUG_SERVER, "[server] info for all threads currently completed %d of %d logins.", _psServer->iLoginsDone, _psServer->psHost->iUserPassCnt);

          if ((_psServer->iValidPairFound) && (_psServer->psAudit->iFoundPairExitFlag == FOUND_PAIR_EXIT_HOST))
          {
            writeError(ERR_INFO, "Exiting Login Module: %d [Stop Host Scan After Valid Pair Found Enabled]", iLoginId);
            iLoginDoneCnt++;
            psLogin[iLoginId].iStatus = LOGIN_DISABLED;
          }
          else if ((_psServer->psAudit->iValidPairFound) && (_psServer->psAudit->iFoundPairExitFlag == FOUND_PAIR_EXIT_AUDIT))
          {
            writeError(ERR_INFO, "Exiting Login Module: %d [Stop Audit Scans After Valid Pair Found Enabled]", iLoginId);
            iLoginDoneCnt++;
            psLogin[iLoginId].iStatus = LOGIN_DISABLED;
          }
          else if (psUser == NULL)
          {
            writeError(ERR_INFO, "Exiting Login Module: %d [User List Complete]", iLoginId);
            iLoginDoneCnt++;
            psLogin[iLoginId].iStatus = LOGIN_DISABLED;
            writeError(ERR_DEBUG_SERVER, "Module: %d iLoginCnt: %d iLoginDoneCnt: %d", iLoginId, iLoginCnt, iLoginDoneCnt);
          }
          else if (_psServer->iLoginsDone >= _psServer->psHost->iUserPassCnt)
          {
            writeError(ERR_INFO, "Exiting Login Module: %d [All Scheduled Logins for Server Complete]", iLoginId);
            iLoginDoneCnt++;
            psLogin[iLoginId].iStatus = LOGIN_DISABLED;
          }
          else
          {
            writeError(ERR_INFO, "Resetting Login Module: %d [Idle]", iLoginId);
            psLogin[iLoginId].iStatus = LOGIN_IDLE;
            iIdleLoginCnt++;
          }
      }
      else if (psLogin[iLoginId].iStatus == LOGIN_FAILED)
      {
        writeError(ERR_DEBUG_SERVER, "[%d] Login failed, thread is %X", iLoginId, pptLogin[iLoginId]);
        res = pthread_join(pptLogin[iLoginId], &thread_result);
        writeError(ERR_DEBUG_SERVER, "Join complete");
        iLoginDoneCnt++;

        psLogin[iLoginId].iStatus = LOGIN_DISABLED;
        writeError(ERR_DEBUG_SERVER, "[server] FAILED: iLoginId: %d iLoginDoneCnt: %d iIdleLoginCnt: %d iLoginCnt: %d", iLoginId, iLoginDoneCnt, iIdleLoginCnt, iLoginCnt);
      }
      else if (iLoginDoneCnt == (iLoginCnt - iIdleLoginCnt)) /* all login threads done */
      {
        writeError(ERR_INFO, "Exiting Server Module: %d [All Scheduled Logins for Server Complete]", _psServer->iId);
        _psServer->iStatus = SRV_DONE;
      }
    }
    usleep(1);
  }

  free(pptLogin);
  writeError(ERR_DEBUG_SERVER, "[server] server thread: %d exiting", _psServer->iId);

  pthread_exit(NULL);
}


/*  This function manages the per host threads
 *  1 audit, 1+ hosts
 *  each server called with 1 host and 1 username pointer
*/
int startAudit(sAudit *_psAudit)
{
  sServer psServer[_psAudit->iServerCnt];
  pthread_t *pptServer;
  void *thread_result;
  int iServerId = 0;
  int iHostDone = 0;
  int res;
  int ret = SUCCESS;
  sHost *psHost;
  sUser *psUser;
  char *pPass;

  writeVerbose(VB_GENERAL, "Parallel Hosts: %d Parallel Logins: %d", _psAudit->iServerCnt, _psAudit->iLoginCnt);

  writeVerbose(VB_GENERAL, "Total Hosts: %d ", _psAudit->iHostCnt);
  if (_psAudit->iUserCnt == 0) writeVerbose(VB_GENERAL, "Total Users: [combo]");
  else writeVerbose(VB_GENERAL, "Total Users: %d", _psAudit->iUserCnt);
  if (_psAudit->iPassCnt == 0) writeVerbose(VB_GENERAL, "Total Passwords: [combo]");
  else writeVerbose(VB_GENERAL, "Passwords: %d", _psAudit->iPassCnt);

  if (_psAudit->iServerCnt > _psAudit->iHostCnt)
    _psAudit->iServerCnt = _psAudit->iHostCnt;

  /* initialize servers */
  memset(psServer, 0, sizeof(sServer) * _psAudit->iServerCnt);
  for (iServerId = 0; iServerId < _psAudit->iServerCnt; iServerId++)
  {
    psServer[iServerId].psAudit = _psAudit;
    psServer[iServerId].iStatus = SRV_IDLE;
  }

  pptServer = malloc(_psAudit->iServerCnt * sizeof(pptServer));
  memset(pptServer, 0, _psAudit->iServerCnt * sizeof(pptServer));

  psHost = _psAudit->psHostRoot;
  while (iHostDone < _psAudit->iHostCnt)
  {
    for (iServerId = 0; iServerId < _psAudit->iServerCnt; iServerId++)
    {

      if ((psHost) && (psServer[iServerId].iStatus == SRV_IDLE))
      {
        writeError(ERR_DEBUG_AUDIT, "[audit] starting new server: %d", iServerId);

        psServer[iServerId].iStatus = SRV_ACTIVE;
        psServer[iServerId].iId = iServerId;
        psServer[iServerId].psHost = psHost;
        psServer[iServerId].iLoginsDone = 0;

        res = pthread_mutex_init(&psServer[iServerId].ptmMutex, NULL);
        if (res != 0)
        {
          writeError(ERR_FATAL, "Mutex initialization failed - %s", strerror( errno ) );
        }

        res = pthread_create(&(pptServer[iServerId]), NULL, startServer, (void *)psServer + iServerId * sizeof(sServer));
        if (res != 0)
        {
          writeError(ERR_WARNING, "Server thread creation failed. Sometimes less is better.");
          psServer[iServerId].iStatus = SRV_IDLE;
        }
        else {
          writeError(ERR_DEBUG_AUDIT, "[audit] created thread: %d", psServer[iServerId].iId);
          psHost = psHost->psHostNext;
        }
      }
      else if (psServer[iServerId].iStatus == SRV_ACTIVE)
      {
        //writeError(ERR_DEBUG_AUDIT, "[audit] thread: %d is currently active.", iServerId);
      }
      else if (psServer[iServerId].iStatus == SRV_DONE)
      {
        writeError(ERR_DEBUG_AUDIT, "[%d] Server is done, thread is %X", iServerId, pptServer[iServerId]);
        res = pthread_join(pptServer[iServerId], &thread_result);
        writeError(ERR_DEBUG_AUDIT, "Join complete");

        psServer[iServerId].iStatus = SRV_IDLE;
        iHostDone++;
        _psAudit->iHostsDone++;
        writeError(ERR_DEBUG_AUDIT, "[audit] server thread %d completed.", iServerId);
        writeVerbose(VB_GENERAL, "[audit] %d addresses completed.", iHostDone);
      }
    }
    usleep(1);
  }
  free(pptServer);
  return ret;
}

int main(int argc, char **argv, char *envp[])
{
  int iRes, iExitStatus;
  sAudit *psAudit = NULL;
  int i;
  
  struct tm *tm_ptr;
  time_t the_time;
  char time_buf[256];

  szModuleName = NULL;

  // Don't worry if there are NULL or blank values here - they will be checked when loading the module
  szModulePaths[0] = getenv("MEDUSA_MODULE_PATH");
  szModulePaths[1] = ".";
#ifdef DEFAULT_MOD_PATH
  szModulePaths[2] = DEFAULT_MOD_PATH;
#else
  szModulePaths[2] = "/usr/lib/medusa/modules";
#endif

  // This handles managing the parameters to be passed to the module
  szTempModuleParam = NULL;
  arrModuleParams = malloc(sizeof(char*));
  memset(arrModuleParams, 0, sizeof(char*));
  nModuleParamCount = 0;

  psAudit = malloc(sizeof(sAudit));
  memset(psAudit, 0, sizeof(sAudit));

  if (checkOptions(argc, argv, psAudit))
  {
    usage();
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < nModuleParamCount; i++)
  {
    writeVerbose(VB_GENERAL, "Module parameter: %s", arrModuleParams[i]);
  }

  if (szModuleName == NULL)
  {
    writeVerbose(VB_EXIT, "You must specify a module to execute using -M MODULE_NAME");
    freeModuleParams();
    exit(EXIT_FAILURE);
  }

  if (psAudit->HostType == L_FILE)
  {
    loadFile(psAudit->pOptHost, &psAudit->pHostFile, &psAudit->iHostCnt);
    psAudit->pGlobalHost = psAudit->pHostFile;
  }

  if (psAudit->UserType == L_FILE)
  {
    loadFile(psAudit->pOptUser, &psAudit->pUserFile, &psAudit->iUserCnt);
    psAudit->pGlobalUser = psAudit->pUserFile;
  }

  if (psAudit->PassType == L_FILE)
  {
    loadFile(psAudit->pOptPass, &psAudit->pPassFile, &psAudit->iPassCnt);
    psAudit->pGlobalPass = psAudit->pPassFile;
  }

  if (psAudit->pOptCombo != NULL)
  {
    loadFile(psAudit->pOptCombo, &psAudit->pComboFile, &psAudit->iComboCnt);
    psAudit->pGlobalCombo = psAudit->pComboFile;
    if (processComboFile(&psAudit))
    {
      exit(iExitStatus);
    }
  }

  iExitStatus = loadLoginInfo(psAudit);

  if (psAudit->pOptCombo != NULL) free(psAudit->pComboFile);
  if (psAudit->pHostFile != NULL) free(psAudit->pHostFile);
  if (psAudit->pUserFile != NULL) free(psAudit->pUserFile);

  if (psAudit->pOptOutput != NULL)
  {
    if ((pOutputFile = fopen(psAudit->pOptOutput, "a+")) == NULL)
    {
      writeError(ERR_FATAL, "Failed to open output file %s - %s", psAudit->pOptOutput, strerror( errno ) );
    }
    else
    {
      iRes = pthread_mutex_init(&ptmMutex, NULL);
      if (iRes != 0)
      {
        writeError(ERR_FATAL, "File mutex initialization failed - %s\n", strerror( errno ) );
      }

      /* write start time and user options to log */ 
      (void) time(&the_time);
      tm_ptr = localtime(&the_time);
      strftime(time_buf, 256, "%Y-%m-%d %H:%M:%S", tm_ptr); 
      writeVerbose(VB_NONE_FILE, "# Medusa v.%s (%s)\n", VERSION, time_buf);
      writeVerbose(VB_NONE_FILE, "# ");

      for (i =0; i < argc; i++)
      {
        writeVerbose(VB_NONE_FILE, "%s ", argv[i]);
      }
      writeVerbose(VB_NONE_FILE, "\n");
    }
  }

  iRes = startAudit(psAudit);
  
  /* stop time */ 
  (void) time(&the_time);
  tm_ptr = localtime(&the_time);
  strftime(time_buf, 256, "%Y-%m-%d %H:%M:%S", tm_ptr); 

  if (iRes)
  {
    writeVerbose(VB_NONE_FILE, "# Medusa failed (%s).\n", time_buf);
    writeError(ERR_CRITICAL, "Medusa failed.");
  }
  else
  {
    writeVerbose(VB_NONE_FILE, "# Medusa has finished (%s).\n", time_buf);
    writeVerbose(VB_GENERAL, "Medusa has finished.");
    iExitStatus = EXIT_SUCCESS;
  }

  // destroy all mutex (global file mutex + internal struct)
  // pthread_mutex_destroy(&mutex_name);

  free(psAudit->pPassFile);
  free(psAudit);

  if (szModuleName != NULL)
    free(szModuleName);

  freeModuleParams();

  exit(iExitStatus);
}

/*********************************************/
/* NLS Functions - RHNLS                     */
/* Autor: Robert Henschel                    */
/* created : 03.11.2003                      */
/* modified: 28.11.2003                      */

#ifndef MyNLS_def

#define MyNLS_def

#include <OS2.h>

/* this structure is returned on the "getSupportedLanguages" function call */
typedef struct _SupportedLanguages {

  PVOID *next;
  SHORT id;
  CHAR  name[100];
  CHAR  englishName[100];
  SHORT majorVersion;
  SHORT minorVersion;
  SHORT revision;
  CHAR  fileName[CCHMAXPATHCOMP];

} SupportedLanguages, *pSupportedLanguages;

/* sets the current language */
/* "languageName" should be the "englishName" field of the "SupportedLanguages" structure */
BOOL setLanguage(PSZ languageName);

/* returns the id of the current language */
int  getLanguage(void);

/* returns the name of the language in the language itself*/
PSZ  getLanguageName(int language);

/* returns the name of the current language in the langugae itself */
PSZ  getCurrentLanguageName(void);

/* returns the english name of the language */
PSZ  getENLanguageName(int language);

/* returns the english name of the current language */
PSZ  getCurrentENLanguageName(void);

/* returns a linked list of supported languages */
pSupportedLanguages getSupportedLanguages(void);

/* returns the NLS string for a given id-string */
PSZ  getNLSString(PSZ id);

#endif

#ifndef __KVCONF_H
#define __KVCONF_H

#define KEYVALLEN 256  

/*   É¾³ý×ó±ßµÄ¿Õ¸ñ   */
char * l_trim(char * szOutput, const char *szInput);

/*   É¾³ýÓÒ±ßµÄ¿Õ¸ñ   */
char *r_trim(char *szOutput, const char *szInput);

/*   É¾³ýÁ½±ßµÄ¿Õ¸ñ   */
char * a_trim(char * szOutput, const char * szInput);


int GetProfileString(char *profile, char *AppName, char *KeyName, char *KeyVal); 

#endif
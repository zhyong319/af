#ifndef __KVCONF_H
#define __KVCONF_H

#define KEYVALLEN 256  

/*   ɾ����ߵĿո�   */
char * l_trim(char * szOutput, const char *szInput);

/*   ɾ���ұߵĿո�   */
char *r_trim(char *szOutput, const char *szInput);

/*   ɾ�����ߵĿո�   */
char * a_trim(char * szOutput, const char * szInput);


int GetProfileString(char *profile, char *AppName, char *KeyName, char *KeyVal); 

#endif
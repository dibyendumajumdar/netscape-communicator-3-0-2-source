#include <stdio.h>
extern void SetMapValue(short u,short c);
void getinput()
{
  char buf[256];
  short c,d,u,idx;
  for(;gets(buf)!=NULL;)
  {
     if(buf[0]=='0' && buf[1] == 'x')
        {
          sscanf(buf,"%hx %hx %hx",&d,&c,&u);
          SetMapValue(u, c);
        }
  }
}

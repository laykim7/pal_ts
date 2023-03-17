#ifndef   _verMan_H_
#define   _verMan_H_

//version management class
//2020.10.15 create by laykim

#include <cstdio>
#include <string>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

// #include <stdlib.h>
// #include <termios.h>


#include "commonDefine.h"

using namespace std;

//================================================

class verMan
{
  private:
  protected:
  public:
    verMan(string _verName, string fullPathFileName);
    ~verMan();
    
    string verName;
    string strVer;
    char chVer[512];
};

#endif

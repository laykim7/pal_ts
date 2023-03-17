
#include "verMan.h"

#include <iostream>
#include <fstream>

//==============================================================================
//----===@ define
//==============================================================================

//==============================================================================
//----===@ global variable
//==============================================================================

//==============================================================================
//----===@ class
//==============================================================================

//=====================================
//----===@ verMan
// Parameters  :
// Description :
verMan::verMan(string _verName, string fullPathFileName)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  ifstream file(fullPathFileName.c_str());
  if(file.fail())
  {
    prnM3("[ERR] file open : %s\r\n", fullPathFileName.c_str());
    file.close();
    return;
  } 

  string strToken;
  char str[512];
  char *pch;

  verName = _verName;

  while(!file.eof())
  {
    getline(file, strToken);
    if(strToken[0] == '#' || strToken.empty()==true) continue;

    strcpy (str, strToken.c_str());

    if(!(pch = strtok (str,"\""))) continue;
    if(!(pch = strtok (NULL,"\""))) continue;
    strcpy(chVer, pch);
    strVer = chVer;
    break;
  };  
  // prnM3("%s : %s\r\n", verName.c_str(), chVer);

  file.close();
  // prnM3("verMan init ok : %s\r\n", chVer);
}

//=====================================
//----===@ ~verMan
// Parameters  :
// Description :
verMan::~verMan()
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  prnM3("verMan finalize ok;\r\n");
}



    // if(!(pch = strtok (str,","))) continue;
    // strcpy(regmap.drvname, pch);

    // if(!(pch = strtok (NULL,","))) continue;
    // regmap.pType = getAsynParamType(pch);

    // if(!(pch = strtok (NULL,","))) continue;
    // regmap.ipId = strtol(pch,NULL,10);

    // if(!(pch = strtok (NULL,","))) continue;
    // regmap.ipCnt = strtol(pch,NULL,10);

    // if(!(pch = strtok (NULL,","))) continue;
    // regmap.accType = strtol(pch,NULL,10);

    // if(!(pch = strtok (NULL,","))) continue;
    // regmap.address = (unsigned int)strtoll(pch,NULL,10);

    // if(!(pch = strtok (NULL,","))) continue;
    // regmap.addrType = strtol(pch,NULL,10);

    // if(!(pch = strtok (NULL,","))) continue;
    // regmap.dataType = strtol(pch,NULL,10);

    // if(!(pch = strtok (NULL,","))) continue;
    // regmap.dataWidth = strtol(pch,NULL,10);

    // if(!(pch = strtok (NULL,","))) continue;
    // regmap.dataOffset = strtol(pch,NULL,10);

//=====================================
//=====================================





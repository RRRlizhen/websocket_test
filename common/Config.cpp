// Config.hh
#include "sys/types.h"
#include <fstream>
#include <iostream>
#include "Config.h"
#include <string.h>
using namespace std;
#define MAX_LINE_LEN  10240

Magic_Config::Magic_Config(
   const std::string &configfile
  ):m_config(configfile)
{
}


Magic_Config&
    Magic_Config::operator= (const Magic_Config& file)
{
  this->m_config = file.m_config;
  this->m_section = file.m_section;
  return *this;


}




int Magic_Config::Init(void)
{
   return LoadFile();
}

 int Magic_Config::getSection(
     const std::string& name,
     std::map<std::string,std::string>& section
    )
{
	if(m_section.count(name)!=0) {
	    section = m_section[name];
	    return 0;
        }    
        return -1;
}



int Magic_Config::TrimString(
    string & str
 )
{
    if(str.empty())  return 0;
    ssize_t pos = str.find_first_not_of(" ");
 
    ssize_t endpos = str.find_last_not_of(" ");
    str = str.substr(pos,endpos-pos+1);

    return 0;
}


 
Magic_Config::~Magic_Config(void)
{
}

int Magic_Config::LoadFile(void)
{
   fstream instr;
   instr.open(m_config.c_str(),fstream::in);
   if(instr.fail()) return -1;

   char buf[MAX_LINE_LEN];
  
   string section="";
   
   int getsection = 0;
   ConfigType keyvalue;   
    memset(buf,0,sizeof(buf));
   while( instr.getline(buf,MAX_LINE_LEN)) {
    
      if(strlen(buf)==0) continue ;

      if(buf[strlen(buf)-1]=='\r')
                  buf[strlen(buf)-1] = '\0';
 
   string tmp(buf);
   if(tmp.empty()) continue;
   TrimString(tmp);
   size_t pos = tmp.find_first_of("[");
   if(pos != string::npos) {  // section define .
      size_t endpos = tmp.find_last_of("]");
      if(endpos  != string::npos) {  
	  if(getsection == 1) {
		m_section[section] = keyvalue;
           }
          // get new section.
	  section = tmp.substr(pos+1,endpos-pos-1);
	  getsection = 1;
	  keyvalue.clear();
          continue;
     } else {
	
       return -1;
     }
   }
   ssize_t compos1 = tmp.find_first_of(";");
   ssize_t compos2 = tmp.find_first_of("#");
   if((compos1 != 0)&& (compos2!=0)) {
        size_t keypos = tmp.find_first_of("=");
	if(keypos!=string::npos) {
 		string key = tmp.substr(0,keypos);
		string value = tmp.substr(keypos+1);
		TrimString(key);
		TrimString(value);
		keyvalue[key] = value;		 
	}
   }   
     memset(buf,0,sizeof(buf));
  }
  if(!section.empty())
  m_section[section] = keyvalue;
  instr.close();
  return 0;   
}

int Magic_Config::getSectionList(
      std::vector<std::string>& sectionlist
  )
{
   map<string,ConfigType>::iterator beg = m_section.begin();
   map<string,ConfigType>::iterator end = m_section.end();
   for(; beg!=end; ++beg)
   {
      sectionlist.push_back((*beg).first);
   }
   return 0;       
}

void Magic_Config::dumpinfo(void)
{

   string section;
   ConfigType config;
   map<string,ConfigType>::iterator beg = m_section.begin();
   map<string,ConfigType>::iterator end = m_section.end();
   for(; beg!=end; ++beg) {
	section = (*beg).first;
	string secitem = string("[")+ section +string("]\n");
	cout<<secitem.c_str();
	config = (*beg).second;
	ConfigType::iterator  itembeg = config.begin();
	ConfigType::iterator  itemend = config.end();
	for( ; itembeg!=itemend; ++itembeg) {
		string key = (*itembeg).first;
		string value =(*itembeg).second;
		string item = key + "=" + value +string("\n");
		cout<<item.c_str();
	}
       section="";
       config.clear();
   }

}


int Magic_Config::WriteFile(void)
{

   fstream outstr;
   outstr.open(m_config.c_str(),fstream::out);
   if(outstr.fail()) return -1;
   
   string section;
   ConfigType config;
   map<string,ConfigType>::iterator beg = m_section.begin();
   map<string,ConfigType>::iterator end = m_section.end();
   for(; beg!=end; ++beg) {
	section = (*beg).first;
	string secitem = string("[")+ section +string("]\n");
	outstr.write(secitem.c_str(),secitem.size());
	if(outstr.fail()) return -1;
	config = (*beg).second;
	ConfigType::iterator  itembeg = config.begin();
	ConfigType::iterator  itemend = config.end();
	for( ; itembeg!=itemend; ++itembeg) {
		string key = (*itembeg).first;
		string value =(*itembeg).second;
		string item = key + "=" + value +string("\n");
		outstr.write(item.c_str(),item.size());
		if(outstr.fail()) return -1;
	}
       section="";
       config.clear();
   }
  outstr.close();

  return 0;
}


int Magic_Config::WriteItem(
       const std::string& section,
       const std::string& key,
       const std::string& value
   )
{
    
   if(m_section.count(section) == 0) {
    // if section is not existed. 
      ConfigType config;
      config.clear();
      config[key] = value;
      m_section[section]= config; 

      return WriteFile();
   }
   ConfigType config = m_section[section];
   ConfigType::iterator  itr = config.begin();
   ConfigType::iterator  enditr = config.end();
   for(; itr!=enditr; ++itr) {
	 if((*itr).first == key) {
		(*itr).second = value;
	  }  
   } 
   if(itr == enditr) {
	
	config[key] = value;
   }
   m_section[section] = config;
   return WriteFile();
}


 int Magic_Config::ReadItem(
       const std::string& section,
       const std::string& key,
       const std::string& defaultvalue,
       std::string& itemvalue
   )
{
    if(m_section.count(section) == 0)  return -1;

    ConfigType config = m_section[section];
    if(config.count(key) == 0)
    {
       itemvalue = defaultvalue;
       return 0;
    }
    itemvalue = config[key];
    return 0;
}



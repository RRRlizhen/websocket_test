//Config.hh define common config 
//read and write interface.
#ifndef __CONFIG__HH_
#define __CONFIG__HH_

#include <string>
#include <map>
#include <vector>

/**
 * Magic_Config is used to 
 * read and write ini file 
 * format config file.
 */
class Magic_Config{

public:

   inline
    Magic_Config()
    {}
   

    Magic_Config&
    operator= (const Magic_Config& file);


explicit  inline
    Magic_Config(const Magic_Config& file)
    { operator=(file); }

   /**
    * Constructor
    */       
explicit  Magic_Config(const std::string& configfile);

  /**
   * Desctructor
   */
  virtual  ~Magic_Config(void);

  /**
   * Used to get all section title.
   * @param sectionlist indicates section title.
   * @return 0 on success.
   *         -1 on failure.
   */
  int getSectionList(
      std::vector<std::string>& sectionlist
  );

  /**
   * Used to get a section .
   * @param name indicates section name.
   * @param section indicates section value.
   * @return 0 on success.
   *         -1 on failure.
   */
  int getSection(
     const std::string& name,
     std::map<std::string,std::string>& section
    );

  /**
   *Used to Read config Item.
   *@param section indicates config section.
   *@param key indicates config item key.
   *@param defaultvalue indicates value which are
   *      are supplied by user as default value, if
   *      fail to get value.
   *@param itemvalue  indicates value which get from 
   *       config file.
   *@return 0 on success. 
   *        -1 on failure.
   */
  int ReadItem(
       const std::string& section,
       const std::string& key,
       const std::string& defaultvalue,
       std::string& itemvalue
   );


  /** 
   * Used to write config item.
   *@param section indicates  config section.
   *@param key config item key.
   *@param value config item value.
   *@return 0 on success.
   *        -1 on failure.
   */
  int WriteItem(
       const std::string& section,
       const std::string& key,
       const std::string& value
   );
 	  

 /**
  * This function is used to load
  * config file to buf.
  * @return 0 on success.
  *         -1 on failure.
  */
 int Init(void);


 /**
  * This function is used to dump
  * iternal config information.
  */

 void dumpinfo(void);

  
private:

 /**
  * This function is used to 
  * write back config information 
  * to physical file.
  * @return 0  on success.
  *         -1 on failure.
  */
  int WriteFile(void);
 

  /**
   * This function is used to 
   * load config information 
   * @return 0 on success.
   *         -1 on failure.
   */
  int LoadFile(void);
  

  /**
   * This function is used to 
   * trim \t empty string at string 
   * start or end. 
   * @return 0 on success.
   *        -1 on failure.
   */
  int TrimString (std::string& str);
  
  //config file name.
  std::string m_config; 
 
  typedef std::map<std::string,std::string> ConfigType;

  // config section defination.
  std::map<std::string,ConfigType> m_section;  


};

#endif //__CONFIG__HH_


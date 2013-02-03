 /*
 *
 * Argument Helper
 *
 * Daniel Russel drussel@alumni.princeton.edu
 * Stanford University
 *
 *
 * This software is not subject to copyright protection and is in the
 * public domain. Neither Stanford nor the author assume any
 * responsibility whatsoever for its use by other parties, and makes no
 * guarantees, expressed or implied, about its quality, reliability, or
 * any other characteristic.
 *
 */
#ifndef _DSR_ARGS_H_
#define _DSR_ARGS_H_
#include <vector>
#include <map>
#include <list>
#include <string>
 
#include "omicron/osystem.h"
#include "libconfig.h++"

 namespace libconfig {
   extern bool verbose, VERBOSE;
 
 
   class OMICRON_API ArgumentHelper
   {
   private:
     class Argument_target;
 
  
     class FlagTarget;
     class DoubleTarget;
     class IntTarget;
     class UIntTarget;
     class StringTarget;
     class CharTarget;
     class StringVectorTarget;
 
   public:
     ArgumentHelper();
     void newFlag(char key, const char *long_name, const char *description,bool &dest);
 
     void newString( const char *arg_description, const char *description, std::string &dest);
     void newNamedString(char key, const char *long_name,
                           const char *arg_description,
                           const char *description,  std::string &dest);
     void newOptionalString( const char *arg_description, const char *description, std::string &dest);
 
     void newInt( const char *arg_description, const char *description, int &dest);
     void newNamedInt(char key, const char *long_name,const char *value_name,
                        const char *description,
                        int &dest);
     void newOptionalInt(const char *value_name,
                           const char *description,
                           int &dest);
 
     void NewDouble(const char *value_name,
                     const char *description,
                     double &dest);
 
     void newNamedDouble(char key, const char *long_name,const char *value_name,
                           const char *description,
                           double &dest);
     void newOptionalDouble(const char *value_name,
                              const char *description,
                              double &dest);
 
     void newChar(const char *value_name,
                                 const char *description,
                                 char &dest);
     void newNamedChar(char key, const char *long_name,const char *value_name,
                            const char *description,
                            char &dest);
     void newOptionalChar(const char *value_name,
                            const char *description,
                            char &dest);
 
     void newUnsignedInt(const char *value_name, const char *description,
                           unsigned int &dest);
     void newOptionalUnsignedInt(const char *value_name, const char *description,
                                 unsigned int &dest);
     void newNamedUnsignedInt(char key, const char *long_name,
                                    const char *value_name, const char *description,
                                    unsigned int &dest);
 
 
 
     void newNamedStringVector(char key, const char *long_name,
                                  const char *value_name, const char *description,
                                  std::vector<std::string> &dest);
 
 
     void setStringVector(const char *arg_description, const char *description, std::vector<std::string> &dest);
 
     void setAuthor(const char *author);
 
     void setDescription(const char *descr);
 
     void setVersion(float v);
     void setVersion(const char *str);
 
     void setName(const char *name);
 
     void setBuildDate(const char *date);
 
 
     bool process(const char* args);
     bool process(int argc, const char **argv);
     bool process(int argc, char **argv){
       return process(argc, const_cast<const char **>(argv));
     }
     void writeUsage(std::ostream &out) const;
     void writeValues(std::ostream &out) const;
     
     ~ArgumentHelper();
   protected:
     typedef std::map<char, Argument_target*> SMap;
     typedef std::map<std::string, Argument_target*> LMap;
     typedef std::vector<Argument_target*> UVect;
     // A map from short names to arguments.
     SMap short_names_;
     // A map from long names to arguments.
     LMap long_names_;
     std::string author_;
     std::string name_;
     std::string description_;
     std::string date_;
     std::string version_;
     bool seen_end_named_;
     // List of unnamed arguments
     std::vector<Argument_target*> unnamed_arguments_;
     std::vector<Argument_target*> optional_unnamed_arguments_;
     std::vector<Argument_target*> all_arguments_;
     std::string extra_arguments_descr_;
     std::string extra_arguments_arg_descr_;
     std::vector<std::string> *extra_arguments_;
     std::vector<Argument_target*>::iterator current_unnamed_;
     std::vector<Argument_target*>::iterator current_optional_unnamed_;
     void new_argument_target(Argument_target*);
     void handle_error() const;
   private:
     ArgumentHelper(const ArgumentHelper &){};
     const ArgumentHelper& operator=(const ArgumentHelper &){return *this;}
   };
   
 }
 
 #define ARGUMENT_HELPER_BASICS(ah) ah.setAuthor("Daniel Russel, drussel@stanford.edu");\
 ah.setVersion(VERSION);\
 ah.setBuildDate(__DATE__);
 
 #endif
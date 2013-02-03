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
 
 #include "omicron/libconfig/ArgumentHelper.h"
 
 
 #include <iostream>
 #include <cstdlib>
 #include <cstdio>
 //#include <limits>
 #include <cassert>
 #include <string.h>
 
 
 
 namespace libconfig {
 
   bool verbose=false, VERBOSE=false;
 
 
 
   // This is a base class for representing one argument value.
   /* 
      This is inherited by many classes and which represent the different types. 
   */
   class ArgumentHelper::Argument_target {
   public:
     char key;
     std::string long_name;
     std::string description;
     std::string arg_description;
    
     Argument_target(char k, const std::string lname,
                     const std::string descr,
                     const std::string arg_descr) {
       key=k;
       long_name=lname;
       description=descr;
       arg_description=arg_descr;
     }
     Argument_target(const std::string descr,
                     const std::string arg_descr) {
       key=0;
       long_name="";
       description=descr;
       arg_description=arg_descr;
     }
     virtual bool process(int &, const char **&)=0;
     virtual void write_name(std::ostream &out) const;
     virtual void write_value(std::ostream &out) const=0;
     virtual void writeUsage(std::ostream &out) const;
     virtual ~Argument_target(){}
   };
 
   void ArgumentHelper::Argument_target::write_name(std::ostream &out) const {
     if (key != 0) out << '-' << key;
     else if (!long_name.empty()) out << "--" << long_name;
     else out << arg_description;
   }
 
   
   void ArgumentHelper::Argument_target::writeUsage(std::ostream &out) const {
     if (key != 0) {
       out << '-' << key;
       out << "/--" << long_name;
     }
     out << ' ' << arg_description;
     out << "\t" << description;
     //out << " Value: ";
     //write_value(out);
     out << std::endl;
   }
 
   class ArgumentHelper::FlagTarget: public ArgumentHelper::Argument_target{
   public:
     bool &val;
     FlagTarget(char k, const char *lname, 
                const char *descr,
                bool &b): Argument_target(k, std::string(lname), std::string(descr), 
                                          std::string()),  val(b){}
     virtual bool process(int &, const char **&){
       val= !val;
       return true;
     }
     virtual void write_value(std::ostream &out) const {
       out << val;
     }
 
     virtual void writeUsage(std::ostream &out) const {
       if (key != 0) {
         out << '-' << key;
         out << "/--" << long_name;
       }
       out << "\t" << description;
       //out << " Value: ";
       //write_value(out);
       out << std::endl;
     }
     virtual ~FlagTarget(){}
   };
 
   class ArgumentHelper::DoubleTarget: public Argument_target{
   public:
     double &val;
     DoubleTarget(char k, const char *lname, 
                  const char *arg_descr, 
                  const char *descr, double &b): Argument_target(k, std::string(lname),
                                                                     std::string(descr),
                                                                     std::string(arg_descr)),  val(b){}
     DoubleTarget(const char *arg_descr, 
                  const char *descr, double &b): Argument_target(std::string(descr),
                                                                     std::string(arg_descr)),  val(b){}
     virtual bool process(int &argc, const char **&argv){
       if (argc==0){
         std::cerr << "Missing value for argument." << std::endl;
         return false;
       }
       if (sscanf(argv[0], "%le", &val) ==1){
         --argc;
         ++argv;
         return true;
       }  else {
         std::cerr << "Double not found at " << argv[0] << std::endl;
         return false;
       }
     }
     virtual void write_value(std::ostream &out) const {
       out << val;
     }
     virtual ~DoubleTarget(){}
   };
 
   class ArgumentHelper::IntTarget: public Argument_target{
   public:
     int &val;
     IntTarget(const char *arg_descr, 
               const char *descr, int &b): Argument_target(0, std::string(),
                                                               std::string(descr),
                                                               std::string(arg_descr)),  
                                               val(b){}
     IntTarget(char k, const char *lname, 
               const char *arg_descr, 
               const char *descr, int &b): Argument_target(k, std::string(lname),
                                                               std::string(descr),
                                                               std::string(arg_descr)), 
                                               val(b){}
     virtual bool process(int &argc, const char **&argv){
       if (argc==0){
         std::cerr << "Missing value for argument." << std::endl;
         return false;
       }
       if (sscanf(argv[0], "%d", &val) ==1){
         --argc;
         ++argv;
         return true;
       }  else {
         std::cerr << "Integer not found at " << argv[0] << std::endl;
         return false;
       }
     }
     virtual void write_value(std::ostream &out) const {
       out << val;
     }
     virtual ~IntTarget(){}
   };
 
   class ArgumentHelper::UIntTarget: public Argument_target{
   public:
     unsigned int &val;
     UIntTarget(const char *arg_descr, 
                const char *descr, unsigned int &b): Argument_target(0, std::string(),
                                                                std::string(descr),
                                                                std::string(arg_descr)),  
                                                val(b){}
     UIntTarget(char k, const char *lname, 
                const char *arg_descr, 
                const char *descr, unsigned int &b): Argument_target(k, std::string(lname),
                                                                std::string(descr),
                                                                std::string(arg_descr)), 
                                                val(b){}
     virtual bool process(int &argc, const char **&argv){
       if (argc==0){
         std::cerr << "Missing value for argument." << std::endl;
         return false;
       }
       if (sscanf(argv[0], "%ud", &val) ==1){
         --argc;
         ++argv;
         return true;
       } else {
         std::cerr << "Unsigned integer not found at " << argv[0] << std::endl;
         return false;
       }
     }
     virtual void write_value(std::ostream &out) const {
       out << val;
     }
     virtual ~UIntTarget(){}
   };
  
 
   class ArgumentHelper::CharTarget: public Argument_target{
   public:
     char &val;
     CharTarget(char k, const char *lname, 
                const char *arg_descr, 
                const char *descr, char &b): Argument_target(k, std::string(lname),
                                                                 std::string(descr),
                                                                 std::string(arg_descr)),  val(b){}
     CharTarget(const char *arg_descr, 
                const char *descr, char &b): Argument_target(std::string(descr),
                                                                 std::string(arg_descr)),  val(b){}
     virtual bool process(int &argc, const char **&argv){
       if (argc==0){
         std::cerr << "Missing value for argument." << std::endl;
         return false;
       }
       if (sscanf(argv[0], "%c", &val) ==1){
         --argc;
         ++argv;
         return true;
       }  else {
         std::cerr << "Character not found at " << argv[0] << std::endl;
         return false;
       }
     }
     virtual void write_value(std::ostream &out) const {
       out << val;
     }
     virtual ~CharTarget(){}
   };
 
 
   class ArgumentHelper::StringTarget: public Argument_target{
   public:
     std::string &val;
     StringTarget(const char *arg_descr, 
                  const char *descr, std::string &b): Argument_target(0, std::string(),
                                                                          descr, 
                                                                          arg_descr),
                                                          val(b){}
 
     StringTarget(char k, const char *lname, const char *arg_descr, 
                  const char *descr, std::string &b): Argument_target(k, lname, descr, 
                                                                          arg_descr), 
                                                          val(b){}
 
     virtual bool process(int &argc, const char **&argv){
       if (argc==0){
         std::cerr << "Missing string argument." << std::endl;
         return false;
       }
       val= argv[0];
       --argc;
       ++argv;
       return true;
     }
     virtual void write_value(std::ostream &out) const {
       out << val;
     }
     virtual ~StringTarget(){}
   };
 
 
   class ArgumentHelper::StringVectorTarget: public Argument_target{
   public:
     std::vector<std::string> &val;
 
     StringVectorTarget(char k, const char *lname, const char *arg_descr, 
                  const char *descr, std::vector<std::string> &b): Argument_target(k, lname, descr, 
                                                                      arg_descr), 
                                                      val(b){}
 
     virtual bool process(int &argc, const char **&argv){
       while (argc >0 && argv[0][0] != '-'){
         val.push_back(argv[0]);
         --argc;
         ++argv;
       }
       return true;
     }
     virtual void write_value(std::ostream &out) const {
       for (unsigned int i=0; i< val.size(); ++i){
         out << val[i] << " ";
       }
     }
     virtual ~StringVectorTarget(){}
   };
 
 
 
 
   ArgumentHelper::ArgumentHelper(){
     author_="Someone";
     description_= "This program does something.";
     date_= "A long long time ago.";
     version_=-1;
     extra_arguments_=NULL;
     seen_end_named_=false;
     //newFlag('v', "verbose", "Whether to print extra information", verbose);
     //newFlag('V', "VERBOSE", "Whether to print lots of extra information", VERBOSE);
   }
 
 
 
   void ArgumentHelper::setStringVector(const char *arg_description, 
                                           const char *description, 
                                           std::vector<std::string> &dest){
     assert(extra_arguments_==NULL);
     extra_arguments_descr_= description;
     extra_arguments_arg_descr_= arg_description;
     extra_arguments_= &dest;
   }
 
   void ArgumentHelper::setAuthor(const char *author){
     author_=author;
   }
 
   void ArgumentHelper::setDescription(const char *descr){
     description_= descr;
   }
 
   void ArgumentHelper::setName(const char *descr){
     name_= descr;
   }
 
   //void ArgumentHelper::setVersion(float v){
   //  version_=v;
   //}
 
   void ArgumentHelper::setVersion(const char *s){
     version_=s;
   }
 
   void  ArgumentHelper::setBuildDate(const char *date){
     date_=date;
   }
   
   void ArgumentHelper::new_argument_target(Argument_target *t) {
     assert(t!= NULL);
     if (t->key != 0){
       if (short_names_.find(t->key) != short_names_.end()){
         std::cerr << "Two arguments are defined with the same character key, namely" << std::endl;
         short_names_[t->key]->writeUsage(std::cerr);
         std::cerr << "\n and \n";
         t->writeUsage(std::cerr);
         std::cerr << std::endl;
       }
       short_names_[t->key]= t;
     } 
     if (!t->long_name.empty()){
       if (long_names_.find(t->long_name) != long_names_.end()){
         std::cerr << "Two arguments are defined with the same long key, namely" << std::endl;
         long_names_[t->long_name]->writeUsage(std::cerr);
         std::cerr << "\n and \n";
         t->writeUsage(std::cerr);
         std::cerr << std::endl;
       }
       long_names_[t->long_name]= t;
     }
     all_arguments_.push_back(t);
   }
   
   void ArgumentHelper::newFlag(char key, const char *long_name, const char *description,bool &dest){
     Argument_target *t= new FlagTarget(key, long_name, description, dest);
     new_argument_target(t);
   };
 
 
 
   void ArgumentHelper::newString(const char *arg_description, const char *description,
                                    std::string &dest){
     Argument_target *t= new StringTarget(arg_description, description, dest);
     unnamed_arguments_.push_back(t);
     all_arguments_.push_back(t);
   };
   void ArgumentHelper::newOptionalString(const char *arg_description, const char *description,
                                             std::string &dest){
     Argument_target *t= new StringTarget(arg_description, description, dest);
     optional_unnamed_arguments_.push_back(t);
   };
   void ArgumentHelper::newNamedString(char key, const char *long_name,
                                          const char *arg_description, const char *description,
                                          std::string &dest){
     Argument_target *t= new StringTarget(key, long_name, arg_description, description, dest);
     new_argument_target(t);
   };
 
 
   void ArgumentHelper::newNamedStringVector(char key, const char *long_name,
                                          const char *arg_description, const char *description,
                                          std::vector<std::string> &dest){
     Argument_target *t= new StringVectorTarget(key, long_name, arg_description, description, dest);
     new_argument_target(t);
   };
 
 
 
   void ArgumentHelper::newInt(const char *arg_description, const char *description,
                                    int &dest){
     Argument_target *t= new IntTarget(arg_description, description, dest);
     unnamed_arguments_.push_back(t);
     all_arguments_.push_back(t);
   };
   void ArgumentHelper::newOptionalInt(const char *arg_description, const char *description,
                                             int &dest){
     Argument_target *t= new IntTarget(arg_description, description, dest);
     optional_unnamed_arguments_.push_back(t);
   };
   void ArgumentHelper::newNamedInt(char key, const char *long_name,
                                          const char *arg_description, const char *description,
                                          int &dest){
     Argument_target *t= new IntTarget(key, long_name, arg_description, description, dest);
     new_argument_target(t);
   };
 
   void ArgumentHelper::newUnsignedInt(const char *arg_description, const char *description,
                                         unsigned int &dest){
     Argument_target *t= new UIntTarget(arg_description, description, dest);
     unnamed_arguments_.push_back(t);
     all_arguments_.push_back(t);
   };
   void ArgumentHelper::newOptionalUnsignedInt(const char *arg_description, const char *description,
                                             unsigned int &dest){
     Argument_target *t= new UIntTarget(arg_description, description, dest);
     optional_unnamed_arguments_.push_back(t);
   };
   void ArgumentHelper::newNamedUnsignedInt(char key, const char *long_name,
                                                const char *arg_description, const char *description,
                                                unsigned int &dest){
     Argument_target *t= new UIntTarget(key, long_name, arg_description, description, dest);
     new_argument_target(t);
   };
 
 
   void ArgumentHelper::NewDouble(const char *arg_description, const char *description,
                                    double &dest){
     Argument_target *t= new DoubleTarget(arg_description, description, dest);
     unnamed_arguments_.push_back(t);
     all_arguments_.push_back(t);
   };
   void ArgumentHelper::newOptionalDouble(const char *arg_description, const char *description,
                                             double &dest){
     Argument_target *t= new DoubleTarget(arg_description, description, dest);
     optional_unnamed_arguments_.push_back(t);
   };
   void ArgumentHelper::newNamedDouble(char key, const char *long_name,
                                          const char *arg_description, const char *description,
                                          double &dest){
     Argument_target *t= new DoubleTarget(key, long_name, arg_description, description, dest);
     new_argument_target(t);
   };
 
   void ArgumentHelper::newChar(const char *arg_description, const char *description,
                                  char &dest){
     Argument_target *t= new CharTarget(arg_description, description, dest);
     unnamed_arguments_.push_back(t);
     all_arguments_.push_back(t);
   };
   void ArgumentHelper::newOptionalChar(const char *arg_description, const char *description,
                                             char &dest){
     Argument_target *t= new CharTarget(arg_description, description, dest);
     optional_unnamed_arguments_.push_back(t);
   };
   void ArgumentHelper::newNamedChar(char key, const char *long_name,
                                          const char *arg_description, const char *description,
                                          char &dest){
     Argument_target *t= new CharTarget(key, long_name, arg_description, description, dest);
     new_argument_target(t);
   };
 
 
 
   void ArgumentHelper::writeUsage(std::ostream &out) const {
     out << name_ << " version " << version_ << ", by " << author_ << std::endl;
     out << description_ << std::endl;
     //out << "Compiled on " << date_ << std::endl << std::endl;
     out << "Usage: " << name_  << " ";
     for (UVect::const_iterator it= unnamed_arguments_.begin(); it != unnamed_arguments_.end(); ++it){
       (*it)->write_name(out);
       out << " ";
     }
     for (UVect::const_iterator it= optional_unnamed_arguments_.begin(); 
          it != optional_unnamed_arguments_.end(); ++it){
       out << "[";
       (*it)->write_name(out);
       out << "] ";
     }
     if (extra_arguments_ != NULL) {
       out << "[" << extra_arguments_arg_descr_ << "]";
     }    
 
     out << std::endl << std::endl;
     out << "All arguments:\n";
     for (UVect::const_iterator it= unnamed_arguments_.begin(); it != unnamed_arguments_.end(); ++it){
       (*it)->writeUsage(out);
     }
     for (UVect::const_iterator it= optional_unnamed_arguments_.begin(); 
          it != optional_unnamed_arguments_.end(); ++it){
       (*it)->writeUsage(out);
     }
 
     out << extra_arguments_arg_descr_ << ": " << extra_arguments_descr_ << std::endl;
     for (SMap::const_iterator it= short_names_.begin(); it != short_names_.end(); ++it){
       (it->second)->writeUsage(out);
     }
   }
 
 
 
   void ArgumentHelper::writeValues(std::ostream &out) const {
     for (UVect::const_iterator it= unnamed_arguments_.begin(); it != unnamed_arguments_.end(); ++it){
       out << (*it)->description;
       out << ": ";
       (*it)->write_value(out);
       out << std::endl;
     }
     for (UVect::const_iterator it= optional_unnamed_arguments_.begin(); 
          it != optional_unnamed_arguments_.end(); ++it){
       out << (*it)->description;
       out << ": ";
       (*it)->write_value(out);
       out << std::endl;
     }
     if (extra_arguments_!=NULL){
       for (std::vector<std::string>::const_iterator it= extra_arguments_->begin(); 
            it != extra_arguments_->end(); ++it){
         out << *it << " ";
       }
     }
 
     for (SMap::const_iterator it= short_names_.begin(); it != short_names_.end(); ++it){
       out << it->second->description;
       out << ": ";
       it->second->write_value(out);
       out << std::endl;
     }
   }
 
   ArgumentHelper::~ArgumentHelper(){
     for (std::vector<Argument_target*>::iterator it= all_arguments_.begin();
          it != all_arguments_.end(); ++it){
       delete *it;
     }
   }
 
 
   bool ArgumentHelper::process(int argc,  const char **argv){
     name_= argv[0];
     ++argv;
     --argc;
 
     current_unnamed_= unnamed_arguments_.begin();
     current_optional_unnamed_= optional_unnamed_arguments_.begin();
     
     for ( int i=0; i< argc; ++i){
       if (strcmp(argv[i], "--help") == 0){
         writeUsage(std::cout);
         exit(0);
       }
     }
 
     while (argc != 0){
 
       const char* cur_arg= argv[0];
       if (cur_arg[0]=='-' && !seen_end_named_){
         --argc; ++argv;
         if (cur_arg[1]=='-'){
           if (cur_arg[2] == '\0') {
             //std::cout << "Ending flags " << std::endl;
             seen_end_named_=true;
           } else {
             // long argument
             LMap::iterator f= long_names_.find(cur_arg+2);
             if ( f != long_names_.end()){
               if (!f->second->process(argc, argv)) {
                 handle_error();
				 return false;
               }
             } else {
               std::cerr<< "Invalid long argument "<< cur_arg << ".\n";
               handle_error();
				 return false;
             }
           }
         } else {
           if (cur_arg[1]=='\0') {
             std::cerr << "Invalid argument " << cur_arg << ".\n";
             handle_error();
				 return false;
           }
           SMap::iterator f= short_names_.find(cur_arg[1]);
           if ( f != short_names_.end()){
             if (!f->second->process(argc, argv)) {
               handle_error();
				 return false;
             }
           } else {
             std::cerr<< "Invalid short argument "<< cur_arg << ".\n";
             handle_error();
				 return false;
           }
         }
       } else {
         if (current_unnamed_ != unnamed_arguments_.end()){
           Argument_target *t= *current_unnamed_;
           t->process(argc, argv);
           ++current_unnamed_;
         } else if (current_optional_unnamed_ != optional_unnamed_arguments_.end()){
           Argument_target *t= *current_optional_unnamed_;
           t->process(argc, argv);
           ++current_optional_unnamed_;
         } else if (extra_arguments_!= NULL){
           extra_arguments_->push_back(cur_arg);
           --argc;
           ++argv;
         } else {
           std::cerr << "Invalid extra argument " << argv[0] << std::endl;
           handle_error();
		 return false;
         }
       }
     }
 
     if (current_unnamed_ != unnamed_arguments_.end()){
       std::cerr << "Missing required arguments:" << std::endl;
       for (; current_unnamed_ != unnamed_arguments_.end(); ++current_unnamed_){
         (*current_unnamed_)->write_name(std::cerr);
         std::cerr << std::endl;
       }
       std::cerr << std::endl;
       handle_error();
	   return false;
     }
 
     if (VERBOSE) verbose=true;
	 return true;
   }
 
   void ArgumentHelper::handle_error() const {
     writeUsage(std::cerr);
     //exit(1);
   }

	bool ArgumentHelper::process(const char* argstr)
	{
		char* argv[64];
		char args[65535];

		strcpy(args, argstr);
		char* c = args;
		int argc = 2;
		argv[0] = "";
		argv[1] = args;
		while(*c != '\0') { if(*c == ' ') { *c = '\0'; argv[argc++] = (c + 1); } c++; }
		return process(argc, argv);
	}
 }
 

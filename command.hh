#ifndef command_hh
#define command_hh

#include "simpleCommand.hh"

// Command Data Structure

struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool _background;
  bool _inFile_two;
  bool _outFile_two;
  bool _errFile_two;
  bool _errOW;
  bool _outOW;
    void tilde(std::string* arg);
  void call_functions();

  void env_var_expand(std::string* arg);
  void func_1();

  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );


  void print();
  void execute();
  void clear();
  static SimpleCommand *_currentSimpleCommand;
    };

#endif


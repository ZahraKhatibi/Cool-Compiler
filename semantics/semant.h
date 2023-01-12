#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"
#include <map> 
#include <vector> 
using namespace std;

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;


typedef method_class* Method;
typedef attr_class* Attr;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
  ostream& semant_error_(tree_node* t, Class_ c);
  ostream& error_stream;
  void add_class(Class_ c);
  map<Symbol, Class_> class_map;
  map <Symbol, vector<Symbol> > parents_map;
};

#endif


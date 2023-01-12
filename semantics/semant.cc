#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include <algorithm>
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;

void handle_error_and_add_method(ClassTable*& classTable,Class_ curr_class,
Method new_method,map<Symbol,vector<Symbol> > &method_envr_list);

void handle_error_and_add_attr(ClassTable*& classTable, Class_ curr_class,
Attr new_attr,map <Symbol, Symbol>& attr_envr_list);


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Symbols                                                             //
//                                                                     //
// For convenience, a large number of symbols are predefined here.     //
// These symbols include the primitive type and method names, as well  // 
// as fixed names used by the runtime system.                          // 
//                                                                     //
/////////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copyy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copyy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}

ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) 
{
    /* Fill this in */

	//**** ebteda tabe install_basic_classes ra seda mizanim ta tavabe basic ra dar class_map save konad ****//
    install_basic_classes();

	//**** bar roye class ha for mizanim ****//
	//**** dar for aval ebteda class ha ra add mikonim, agar tekrari bood error modahim ****//
	//**** dar for dovom parent va inherit ha ra barresi mikonim **** //
    for(int i = classes->first(); classes->more(i); i = classes->next(i)) 
    {
		//**** agar tekrari nabod on ra dar class_map add mikonim****//
        if (class_map.find((classes->nth(i))->get_name()) == class_map.end())
		{
            add_class(classes->nth(i));
        }
        //**** agar tekrari bood sharayet ra barresi karde va motabegh sharayet error midahim****//
        else 
		{
            Class_ cur_class = classes->nth(i);
            Symbol cur_class_parent = cur_class->get_parent();
            Symbol cur_class_name = cur_class->get_name();

			//****agar current class basic class nabod, error 'Class " x " is repetitive.' midahim ****//
            if( cur_class_name != Object && cur_class_name != Int && cur_class_name != Str &&
                cur_class_name != IO && cur_class_name != Bool && cur_class_name != SELF_TYPE)
			{
                    semant_error(cur_class);
                    error_stream << "Class " << cur_class_name->get_string() << " is repetitive." << endl;
                    cur_class->to_obj_parent();
                    cur_class_parent = Object;
                
            }

			//**** agar basic class bod, error 'Class "x" is basic class and forbbiden use this name' midahim ****//
            else 
			{
                semant_error(cur_class);
                error_stream << "Class "<<cur_class_name->get_string()<<" is basic class and forbbiden use this name" << endl;
                cur_class->to_obj_parent();
                cur_class_parent = Object;

            }
        } 
    }

	//**** dar in for, parent va inherit ha ra barresi mikonim **** //
    for(int i = classes->first(); classes->more(i); i = classes->next(i))
    {
        Class_ cur_class = classes->nth(i);
        Symbol cur_class_parent = cur_class->get_parent();
        Symbol cur_class_name = cur_class->get_name();   
        
		//**** baresi mikonim az class haye Int, Str, Bool va SELF_TYPE ha inherit nakarde bashand ****// 
        if (cur_class_parent == Int || cur_class_parent == Str || cur_class_parent == Bool || cur_class_parent == SELF_TYPE)
		{
            semant_error(cur_class);
            error_stream << "Class " << cur_class_name->get_string()<<" cannot inherit class "<< cur_class_parent->get_string()<<endl;
            cur_class->to_obj_parent();
            cur_class_parent = Object;
        } 
		//**** baresi mikonim az class haye tarif shode inherit karde bashand ****//
        if (class_map.find(cur_class_parent) == class_map.end()) {
            semant_error(cur_class);
            error_stream << "Class " << cur_class_name->get_string()<< " inherits from an undefined class"
			<<cur_class_parent->get_string()<<endl;
            cur_class->to_obj_parent();
            cur_class_parent = Object;
        }
        
        vector<Symbol> grandparents;  //****dar in vector, ajdad har class ra save mikonim ****//
        grandparents.push_back(cur_class_parent);

        while (cur_class_parent != Object) 
		{
            cur_class_parent = (class_map.find(cur_class_parent)->second)->get_parent();
            grandparents.push_back(cur_class_parent);

			//****agra dar halghe oftade bodim error midahim ****//
            if (cur_class_parent == cur_class_name)
			{
                semant_error(cur_class);
                error_stream << "Class " << cur_class_name->get_string() << " or an ancestor of "
				<< cur_class_name->get_string() <<" is involved in an inheritance cycle"  << endl;
                cur_class->to_obj_parent();
                cur_class_parent = Object;
            }
        }
		// **** esm class ra be ajdadash map mikonim ****//
        grandparents.push_back(Object);	
        parents_map[cur_class->get_name()]=grandparents;
    }

	//**** agar barname shamel Main nabood error midahim ****//
    if(class_map.find(Main) == class_map.end())
    {
        semant_error();
        cout << "the program have not Main class"<<endl;
    }

	//**** dar sorat rokhdad semaitic errorha, barneme ra halt mikonim ****//
    if (semant_errors) 
    {
	    cerr << "semantic errors, Compilation halted" << endl;
	    exit(1);
    }
}

void ClassTable::install_basic_classes() 
{
    // **** add 'add_class' function to install_basic_classes becuse classes dosen't save any place ****//
    
    // The tree package uses these globals to annotate the classes built below.
    // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() :  Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copyy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);
    add_class(Object_class);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename); 
    add_class(IO_class);

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);
    add_class(Int_class);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    add_class(Bool_class);
    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);

    add_class(Str_class);
}

void ClassTable :: add_class(Class_ c) 
{ 
	//**** esm har class ra be haman class map mikonim ****//
    class_map[c->get_name()] = c;
}

void program_class::my_semantic_type_checking(ClassTable *&classTable)
{
	map <Symbol, Class_> class_map = classTable->class_map; 
	map <Symbol, vector<Symbol> > parents_map = classTable->parents_map;
    map <Symbol, ClassFeatures> temp_map_enviroment;
    map < Class_, map <Symbol, vector<Symbol> > > method_environment_list;


	//**** bar roye class ha for mizanim ****//
    for (map<Symbol, Class_>::iterator i = class_map.begin(); i != class_map.end(); i++)
    {
        map <Symbol, vector<Symbol> > method_enviroment;
		map <Symbol, Symbol> attr_enviroment;
        Symbol cur_name;
        Class_ cur_class;
        Features cur_feautures;

        cur_name = i->first;
		cur_class = i->second;
        cur_feautures = cur_class->get_features();

		//**** bar roye feacher ha for mizanim ****//
		//**** har feacher ya attribute ast ya method, on ha ra daste bandi mikonim ****//
		//**** az dynamic cast baraye in kar estefade mikonim ****//
        for (int j = cur_feautures->first(); cur_feautures->more(j); j = cur_feautures->next(j)) 
        {
			Feature cur_feautre;
            cur_feautre = cur_feautures->nth(j);
			Method cur_method = dynamic_cast<Method>(cur_feautre);
            if(cur_method == NULL)
            { //atrr
                Attr cur_attr = dynamic_cast<Attr>(cur_feautre);
				handle_error_and_add_attr(classTable, cur_class, cur_attr, attr_enviroment);
				// **** cut_attr ra be list attr ha ezafe mikonim ****//
            }
			else 
            { //method
				handle_error_and_add_method( classTable, cur_class, cur_method, method_enviroment);
				// **** cut_method ra be list method ha ezafe mikonim ****//
			}
		}
		//**** class feacher shamel method environment and attr enviroment mishavad****//
        ClassFeatures temp_features;
		temp_features.method_enviroment = method_enviroment;
		temp_features.attr_enviroment = attr_enviroment;
		temp_map_enviroment[cur_name] = temp_features; //****mapi az esm class be feacherha ****//
    }

	//****agar Mian shamel tabe main nabashad error midahim ****//
	if((temp_map_enviroment[Main].method_enviroment).find(main_meth)==(temp_map_enviroment[Main].method_enviroment).end()) 
    {
		classTable->semant_error(class_map[Main]);
		classTable->error_stream << "the Main Class have not main function" << endl;
	}

	for (map<Symbol, Class_>::iterator i = class_map.begin(); i != class_map.end(); i++) {
		Symbol cur_name = i->first;
		Class_ cur_class = i->second;
		vector<Symbol> parents = parents_map[cur_name];
		parents.insert(parents.begin(), cur_name);
        int number_of_parents = parents.size();
		for (int j = 0; j < number_of_parents; j++) 
        {
			Symbol cur_parent = parents[j];
			map <Symbol, vector<Symbol> > current_parent_methods;
            current_parent_methods = temp_map_enviroment[cur_parent].method_enviroment;
			for (map<Symbol, vector<Symbol> >::iterator k=current_parent_methods.begin();k!= current_parent_methods.end();k++)
            {
				method_environment_list[cur_class][k->first] = k->second;
			}
		}
	}

	SymbolTable<Symbol, Symbol> *object_environment = new SymbolTable<Symbol, Symbol>();

	for (map<Symbol, Class_>::iterator i = class_map.begin(); i != class_map.end(); i++)
    {
		Symbol cur_name = i->first;
		Class_ cur_class = i->second;
		vector<Symbol> parents = parents_map[cur_name];
		map <Symbol, vector<Symbol> > current_class_methods = temp_map_enviroment[cur_name].method_enviroment;
		map <Symbol, Symbol> current_class_attrs = temp_map_enviroment[cur_name].attr_enviroment;

		object_environment->enterscope();
		parents.insert(parents.begin(), cur_name);
        int number_of_parents = parents.size();
		for (int j=0 ; j < number_of_parents ;j++)
        {
			map <Symbol, Symbol>& cur_parent_attrs = temp_map_enviroment[parents[j]].attr_enviroment;
			for (map<Symbol, Symbol>::iterator k=current_class_attrs.begin(); k != current_class_attrs.end(); k++) 
            {
				object_environment->addid(k->first, &(k->second));
			}
		}
		ClassEnvironment  final_environment_list;
        final_environment_list.current_class = cur_class;
		final_environment_list.ObjEnv = object_environment;
		final_environment_list.method_enviroment_map = method_environment_list;

		cur_class->semantic_type_check(final_environment_list, classTable);
		final_environment_list.ObjEnv->exitscope();		
	}
}

void handle_error_and_add_method(ClassTable*& classTable,Class_ curr_class,Method new_method,map<Symbol,vector<Symbol> > &method_envr_list)
{
	//**** dar in tabe valid boodan method barresi mishavad ****//
	//**** dar sorat valid bodan be method_list add mishavad *****//
	vector<Symbol> formals_type_list;
	Formals cur_formals = new_method->get_formals();

	for (int i = cur_formals->first(); cur_formals->more(i); i = cur_formals->next(i)) 
    {
		Formal cur_formal = cur_formals->nth(i);
		Symbol cur_name = cur_formal-> get_name();
		Symbol cur_type = cur_formal-> get_type();

        if (cur_type == SELF_TYPE) 
        {
            classTable->semant_error(curr_class);
            classTable->error_stream << "Formal parameter " << cur_name->get_string() << " cannot SELF_TYPE" << endl;
            cur_type = Object;
        }
        else if (classTable->class_map.find(cur_type) == classTable->class_map.end()) 
		{
            classTable->semant_error(curr_class);
            classTable->error_stream << "Class " << cur_type->get_string() << " fقom formal parameter " << cur_name->get_string() 
			<< " is undefined" << endl;
            cur_type = Object;
        }
        if (find(formals_type_list.begin(), formals_type_list.end(), cur_name) != formals_type_list.end()) 
        {
            classTable->semant_error(curr_class);
            classTable->error_stream<<"Formal parameter "<<cur_name->get_string()<<"is multiply/redefined in method "
			<<new_method->get_name()->get_string()<< endl;
        }
        if (cur_name == self)
        {
            classTable->semant_error(curr_class);
            classTable->error_stream << "self cannot be name of a formal parameter" << endl;
        }
		formals_type_list.push_back(cur_type);
	}

	Symbol cur_name = new_method->get_name();
	Symbol retern_type = new_method->get_type();
	if (retern_type != SELF_TYPE) 
    {
		if (classTable->class_map.find(retern_type) == classTable->class_map.end()) {
			classTable->semant_error(curr_class);
			classTable->error_stream<<"retern type is Undefined "<<retern_type->get_string()<<" in method "<< cur_name->get_string() << endl;
			retern_type = Object;
		}

	}

	if (method_envr_list.find(cur_name) != method_envr_list.end()) {
		classTable->semant_error(curr_class);
		classTable->error_stream<< "Method "<<cur_name->get_string()<<" is multiply/redefined in class"<<curr_class->get_name()->get_string() << endl;
	}

	formals_type_list.push_back(retern_type);
	method_envr_list[cur_name] = formals_type_list;
}

void handle_error_and_add_attr(ClassTable*& classTable, Class_ curr_class,Attr new_attr,map <Symbol, Symbol>& attr_envr_list)
{
	//**** dar in tabe valid boodan attr barresi mishavad *****//
	//**** dar sorat valid bodan be attr_list add mishavad *****// 
	Symbol cur_name = new_attr->get_name();
	Symbol cur_type = new_attr->get_type();
    if (cur_type != SELF_TYPE) 
    {
        if (classTable->class_map.find(cur_type) == classTable->class_map.end()) {
            if (cur_type != prim_slot) 
			{
                classTable->semant_error(curr_class);
                classTable->error_stream<<"type "<<cur_type->get_string()<<" of attribute "<<cur_name->get_string()
				<<" is undefined"<< endl;
                cur_type = Object;
            }
        }
    }
    if (attr_envr_list.find(cur_name) != attr_envr_list.end())
    {
        classTable->semant_error(curr_class);
        classTable->error_stream<<"Attribute "<<cur_name->get_string()<<" is multiply/redefined in class "
		<<(curr_class->get_name())->get_string()<<endl;
    }

    if (cur_name == self) 
    {
        classTable->semant_error(curr_class);
        classTable->error_stream << "self cannot be name of attribute" << endl;
    }

	attr_envr_list[cur_name] = cur_type;
}

bool sub_type_checker(Symbol class1, Symbol class2, Class_ curr_clss, ClassTable*& classTable)
{
	//**** agar class1 sub type class2 bashad, return true else return False ****//
	vector<Symbol> class1_parents_map = classTable->parents_map[class1];
	if (class1 == class2) 
	{
		return true;
	}

	if (class1 == SELF_TYPE) 
	{
		return sub_type_checker(curr_clss->get_name(), class2, curr_clss, classTable);
	}

	if (class2 == SELF_TYPE) 
		return false; //****classi nemitavaand sub type self type basha ****//

	if (std::find(class1_parents_map.begin(), class1_parents_map.end(), class2) != class1_parents_map.end()) 
		return true;
	else 
		return false;
}

Symbol join(Symbol class1, Symbol class2, Class_ curr_clss, ClassTable * &classTable) {

	//**** parent_map shamel ajdad yek class ast ****//
	//**** be donbal yaftan avalin parent moshtarak hastim *****//
	vector<Symbol> class1_parents_map = classTable->parents_map[class1];
	vector<Symbol> class2_parents_map = classTable->parents_map[class2];
	if (class1 == class2) 
		return class1;

	if (class1 == SELF_TYPE) 
	{   //**** join(SELF_TYPE{D}, A) = join(D, A) ****//
		return join(curr_clss->get_name(), class2, curr_clss, classTable);
	}

	if (class2 == SELF_TYPE) 
	{
		return join(class1, curr_clss->get_name(), curr_clss, classTable);
	}

	for (size_t i = 0; i < class1_parents_map.size(); i++) {
		for (size_t j = 0; j < class2_parents_map.size(); j++) {
			if (class1_parents_map[i] == class2_parents_map[j])   {
				return class1_parents_map[i];
			}
		}
	}
	return Object;
}

void class__class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Features class_features = (environment.current_class)->get_features();
	for (int i = class_features->first(); class_features->more(i); i = class_features->next(i))
    {
		Feature cur_feature = features->nth(i);
		Method cur_method = dynamic_cast<Method>(cur_feature);
		if (cur_method == NULL)
        {
			Attr cur_attr = dynamic_cast<Attr>(cur_feature);
			cur_attr->semantic_type_check(environment, classTable);
		}
        else
        {
			cur_method->semantic_type_check(environment, classTable);
		}
	}
}

//**** Features type cheking ****//
// **** 'Method' type checing ****//
void method_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
	(environment.ObjEnv)->enterscope();
	(environment.ObjEnv)->addid(self, &SELF_TYPE);
	//**** kol formal paramer ha ra type check mikonim ****//
	for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
		Formal cur_formal = formals->nth(i);
		Symbol cur_type = cur_formal->get_type();
		cur_formal->semantic_type_check(environment, classTable);
	}
	//**** function body ra type check mikonim **** //
	Symbol body_type = expr->semantic_type_check(environment, classTable);

	if (!sub_type_checker(body_type, return_type, environment.current_class, classTable)) {
		if (body_type != No_type) 
		{
			classTable->semant_error_(this, environment.current_class);
			classTable->error_stream << "returned expression " <<
				body_type->get_string() << " of function " << name->get_string() <<
				" is not match the function return type " << return_type->get_string() << endl;
		}
	}
	(environment.ObjEnv)->exitscope();
}

// **** 'formal' type checing ****//
void formal_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
	(environment.ObjEnv)->addid(name, &type_decl);
}

//**** Attr-Init and Attr-No-Init ****//
void attr_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	(environment.ObjEnv)->enterscope();
	(environment.ObjEnv)->addid(self, &SELF_TYPE);

	if (dynamic_cast<no_expr_class*>(init) == NULL)
	{
		Symbol init_type = init->semantic_type_check(environment, classTable);
		if (sub_type_checker(init_type, type_decl, environment.current_class, classTable) == false)
		{
			classTable->semant_error_(this, environment.current_class);
			classTable->error_stream << "type of your initialaztion " <<
				init_type->get_string() << " of attribute " << name->get_string() <<
				" is not match the type " << type_decl->get_string() << endl;
		}
	}
	(environment.ObjEnv)->exitscope();
}

//**** ASSIGN type checking ****//
Symbol assign_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
	Symbol expr_type = expr->semantic_type_check(environment, classTable);
    //cout << expr_type->get_string() <<",,,,," << endl;

	Symbol* lookup_ptr = environment.ObjEnv->lookup(name);
	//cout << lookup_ptr->get_string()<<"----" <<endl;

	//**** baresi mikonim dar boject environment name vojod dashte bashad. //****
	if (lookup_ptr == NULL) 
    {
		//cout <<name->get_string() << endl;
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Undeclared identifier " <<name->get_string() << endl;
		type = Object;
		return type;
	}

	Symbol original_type = *lookup_ptr;

	//**** barresi mikonim be self type chizi assign nashavad ****//
	if (expr_type == SELF_TYPE)
	{
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Cannot assign to self object" << endl;
		expr_type = Object;
	}


	//**** baresi mikonim az type monasebi bashad ****//
	else if (sub_type_checker(expr_type, original_type, environment.current_class, classTable)==false)
	{
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Inferred type " <<
			expr_type->get_string() << " of assignment of attribute " << name->get_string() <<
			" does not conform to declared type " << original_type->get_string() << endl;
	}
	type = expr_type;
	return type;
}

//***** Dispatch ****//
//**** Dispatch ****//
Symbol dispatch_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
 
	Symbol expr_type = expr->semantic_type_check(environment, classTable); 
	Class_ method_class;
	if (expr_type == SELF_TYPE)
	{
		method_class = environment.current_class;
	}
	else
	{
		method_class = classTable->class_map[expr_type];
	}

	map <Symbol, vector<Symbol> > classMethods = environment.method_enviroment_map[method_class];

	vector<Symbol> methodSign;

	if (classMethods.find(name) == classMethods.end()) 
	{
		//**** halati ast ke dispatch be methodi tarif nashode dashte bashim ****//
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Dispatch to undefined method "<<name->get_string() << endl;
		type = Object;
		return type;
	}

	else 
    {
		methodSign = environment.method_enviroment_map[method_class][name];
	}

	Symbol method_ret_type = methodSign.back();
	methodSign.pop_back();
	vector<Symbol> argTypes = methodSign;

	Expressions actualArgs = actual; // e1,...,en
	size_t actualArgsSize = actualArgs->len();
	if (argTypes.size() != actualArgsSize)
	{
		//****haleti ast ke methodi ba tedad argument eshtebah farakhani shavad ****//
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream<<"Method "<<name->get_string() << " called with wrong number of arguments." <<endl;
		type = Object; // err recovery
		return type;
	}

	int j = 0;
	for (int i = actualArgs->first(); actualArgs->more(i); i = actualArgs->next(i)) {
		Expression cur_exp = actualArgs->nth(i);
		Symbol cur_type = cur_exp->semantic_type_check(environment, classTable); // e1:T1,...,en:Tn
		if (!sub_type_checker(cur_type, argTypes[j], environment.current_class, classTable))
		{
			//**** dar seda zadan methodi, type parameter j+1 ba type tarif shode barabar nist ****//
			classTable->semant_error_(this, environment.current_class);
			classTable->error_stream << "In call of method " <<
				name->get_string() << ", type " << cur_type->get_string() << " of the parameter " <<
				(j + 1) << "'th does not conform to declared type " << argTypes[j]->get_string() << "." << endl;
		}
		j++;
	}


	if (method_ret_type == SELF_TYPE)
	{
		type = expr_type; // T0
	}

	else 
	{
		type = method_ret_type;
	}
	return type;
}


//**** StaticDispatch *****//
Symbol static_dispatch_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	//**** dar static dispatch bayad type T0 ba atype T barabar bashad ****//

	Symbol expr_type = expr->semantic_type_check(environment, classTable); // e0:T0

	Class_ method_class = classTable->class_map[type_name];

	map <Symbol, vector<Symbol> > classMethods = environment.method_enviroment_map[method_class];
	vector<Symbol> methodSign;

	if (classMethods.find(name) == classMethods.end()) {
		//**** Dispatch be undefined method ****//
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Dispatch to undefined method " <<
			name->get_string() << endl;
		type = Object;
		return type;
	}
	else 
	{
		methodSign = environment.method_enviroment_map[method_class][name];
	}

	if (!sub_type_checker(expr_type, type_name, environment.current_class, classTable)) {
		//**** type ha moghayerat dashte bashad ****//
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Type " << expr_type->get_string() <<
			" of expression does not conform to static type " <<
			type_name->get_string() << " of dispatch " << name->get_string() << "." << endl;
		type = Object;
		return type;
	}

	Symbol method_ret_type = methodSign.back();
	methodSign.pop_back();
	vector<Symbol> argTypes = methodSign;

	size_t actualSize = actual->len(); // e1,...,en
	if (argTypes.size() != actualSize)
	{
		//**** tedad argument ha moghayerat dashte bashad ****//
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Incompatible number of arguments on dispatch of method " <<
			name->get_string() << "." << endl;
		type = Object;
		return type;
	}

	int j = 0;
	for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
		Expression cur_exp = actual->nth(i);
		Symbol cur_type = cur_exp->semantic_type_check(environment, classTable); // e1:T1,...,en:Tn
		if (!sub_type_checker(cur_type, argTypes[j], environment.current_class, classTable)) {
			//**** type ha moghayerat dashte bashad ****//
			classTable->semant_error_(this, environment.current_class);
			classTable->error_stream << "In call of method " <<
				name->get_string() << ", type " << cur_type->get_string() << " of the parameter no. " <<
				(j + 1) << " does not conform to declared type " << argTypes[j]->get_string() << "." << endl;
		}
		j++;
	}

	if (method_ret_type == SELF_TYPE) 
	{
		type = expr_type; // T0
	}
	else 
	{
		type = method_ret_type;
	}
	return type;
}

// **** Conditional, Loop, Case, Let ****//
//**** If ****//
Symbol cond_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
	Symbol typePred = pred->semantic_type_check(environment, classTable);
	Symbol typeThen = then_exp->semantic_type_check(environment, classTable);
	Symbol typeElse = else_exp->semantic_type_check(environment, classTable);
	if (typePred != Bool) 
	{
		//Loop condition daraye type Bool nist
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "If condition does not have type Bool." << endl;
	}
	type = join(typeThen, typeElse, environment.current_class, classTable);
	return type;
}

//****Loop ****/
Symbol loop_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Symbol typePred = pred->semantic_type_check(environment, classTable);
	Symbol typeBody = body->semantic_type_check(environment, classTable); // Is not used tho.
	if (typePred != Bool)
	{
		//****Loop condition daraye type Bool nist****//
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Loop condition does not have type Bool." << endl;
	}
	type = Object;
	return type;
}

// **** Case ****//
Symbol typcase_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
	expr->semantic_type_check(environment, classTable);

	vector<Symbol> var_type;

	Symbol ret_type;

	for (int i = cases->first(); cases->more(i); i = cases->next(i)) {

		Case cur_branch = cases->nth(i);
		Symbol varDecType = cur_branch->get_type();

		if ((i != cases->first()) && (std::find(var_type.begin(), var_type.end(), varDecType) != var_type.end())) 
		{
			classTable->semant_error_(this, environment.current_class);
			classTable->error_stream << "The variables declared on the branches of case do not have distinct types." << endl;
		}
		else 
        {
			var_type.push_back(varDecType);
		}

		Symbol branchType = cur_branch->semantic_type_check(environment, classTable);

		if (i == cases->first()) 
		{
			ret_type = join(branchType, branchType, environment.current_class, classTable);
		}

		else 
		{
			ret_type = join(branchType, ret_type, environment.current_class, classTable);
		}
	}

	type = ret_type;
	return type;
}

Symbol branch_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{ 
	(environment.ObjEnv)->enterscope();
	(environment.ObjEnv)->addid(name, &type_decl);

	Symbol branchType = expr->semantic_type_check(environment, classTable);

	(environment.ObjEnv)->exitscope();

	return branchType;
}

//**** Sequence *****//
Symbol block_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
	Expressions expr_list = body;
	for (int i = expr_list->first(); expr_list->more(i); i = expr_list->next(i)) 
	{
		Expression cur_expr = expr_list->nth(i);
		type = cur_expr->semantic_type_check(environment, classTable);
	}
	return type;
}

// **** Let-Init and Let-No-Init *****//
Symbol let_class :: semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	if (dynamic_cast<no_expr_class*>(init) == NULL)
	{
		Symbol init_type = init->semantic_type_check(environment, classTable);
		if (!sub_type_checker(init_type, type_decl, environment.current_class, classTable))
		{
			classTable->semant_error_(this, environment.current_class);
			classTable->error_stream << "Inferred type " <<
				init_type->get_string() << " of initialization of variable " << identifier->get_string() <<
				" in Let, does not conform to declared type " << type_decl->get_string() << endl;
		}
	}

	if (identifier == self) 
	{
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "'self' cannot be bound in a 'let' expression." << endl;
	}

	(environment.ObjEnv)->enterscope();
	(environment.ObjEnv)->addid(identifier, &type_decl);

	type = body->semantic_type_check(environment, classTable);

	(environment.ObjEnv)->exitscope();

	return type;
}

// ***** Arithmetic nodes ****//

// **** Arith **** //
Symbol plus_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Symbol type_exp1 = e1->semantic_type_check(environment, classTable);
	Symbol type_exp2 = e2->semantic_type_check(environment, classTable);

	if (type_exp1 != Int || type_exp2 != Int) {
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream<<"non-Int arguments: "<<type_exp1->get_string()<<" + "<<type_exp2->get_string() << endl;
	}
	type = Int;
	return type;
}

Symbol sub_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Symbol type_exp1 = e1->semantic_type_check(environment, classTable);
	Symbol type_exp2 = e2->semantic_type_check(environment, classTable);

	if (type_exp1 != Int || type_exp2 != Int) 
    {
		// non-Int arguments: Int - String
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "non-Int arguments: " <<
			type_exp1->get_string() << " - " <<
			type_exp2->get_string() << "." << endl;
	}
	type = Int;
	return type;
}

Symbol mul_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Symbol type_exp1 = e1->semantic_type_check(environment, classTable);
	Symbol type_exp2 = e2->semantic_type_check(environment, classTable);

	if (type_exp1 != Int || type_exp2 != Int) {
		// non-Int arguments: Int * String
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "non-Int arguments: " <<
			type_exp1->get_string() << " * " <<
			type_exp2->get_string() << "." << endl;
	}
	type = Int;
	return type;
}

Symbol divide_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Symbol type_exp1 = e1->semantic_type_check(environment, classTable);
	Symbol type_exp2 = e2->semantic_type_check(environment, classTable);

	if (type_exp1 != Int || type_exp2 != Int) {
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream <<"non-Int arguments: "<<type_exp1->get_string()<<" / "<<type_exp2->get_string()<<endl;
	}
	type = Int;
	return type;
}

Symbol neg_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Symbol type_exp = e1->semantic_type_check(environment, classTable);
	if (type_exp != Int)
	{
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Argument of 'neg' has type " <<type_exp->get_string() << "instead of Int" << endl;
	}
	type = Int;
	return type;
}

Symbol comp_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Symbol type_exp = e1->semantic_type_check(environment, classTable);
	if (type_exp != Bool) {
		
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream <<"Argument of 'not' has type "<<type_exp->get_string() <<" instead of Bool" << endl;
	}
	type = Bool;
	return type;
}

// **** Comparison ****//
// **** Equal ****//
Symbol eq_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{

	Symbol type_exp1 = e1->semantic_type_check(environment, classTable);
	Symbol type_exp2 = e2->semantic_type_check(environment, classTable);

	if (type_exp1 == Int || type_exp1 == Str || type_exp1 == Bool ||
		type_exp2 == Int || type_exp2 == Str || type_exp2 == Bool) 
	{
		if (type_exp1 != type_exp2)
		{
			classTable->semant_error_(this, environment.current_class);
			classTable->error_stream << "Illegal comparison with a basic type " <<type_exp1->get_string() <<" and " <<type_exp2->get_string()  << endl;
		}
	}
	type = Bool;
	return type;
}


// **** Compare ****//
Symbol lt_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Symbol type_exp1 = e1->semantic_type_check(environment, classTable);
	Symbol type_exp2 = e2->semantic_type_check(environment, classTable);
	if (type_exp1 != Int || type_exp2 != Int)
	{
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "non-Int arguments: " <<type_exp1->get_string() << " < " <<type_exp2->get_string() << endl;
	}
	type = Bool;
	return type;
}


Symbol leq_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
	Symbol type_exp1 = e1->semantic_type_check(environment, classTable);
	Symbol type_exp2 = e2->semantic_type_check(environment, classTable);

	if (type_exp1 != Int || type_exp2 != Int)
	{
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "non-Int arguments: " <<
			type_exp1->get_string() << " <= " <<
			type_exp2->get_string() << "." << endl;
	}
	type = Bool;
	return type;
}

// **** Leaf nodes****//
// **** Int ****
Symbol int_const_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	type = Int;
	return type;
}

//**** True and False *****//
Symbol bool_const_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	type = Bool;
	return type;
}

// **** String ****//
Symbol string_const_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
	type = Str;
	return type;
}

// **** New ****//
Symbol new__class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	type = type_name;
	return type;
}

// **** Isvoid **** //
Symbol isvoid_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable) 
{
	//****body of expression type checking ****//
	e1->semantic_type_check(environment, classTable);
	type = Bool;
	return type;
}


Symbol no_expr_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	type = No_type;
	return type;
}

//**** variable **** // 
Symbol object_class::semantic_type_check(ClassEnvironment environment, ClassTable * &classTable)
{
	Symbol* lookup_ptr = (environment.ObjEnv)->lookup(name);
	if (lookup_ptr == NULL) 
	{
		classTable->semant_error_(this, environment.current_class);
		classTable->error_stream << "Undeclared identifier " <<name->get_string() << endl;
		type = Object;
	}
	else {
		type = *lookup_ptr;
	}
	return type;
} 

////////////////////////////////////////////////////////////

void class__class::to_obj_parent() { parent = Object; }

Symbol class__class::get_name() { return name;}

Symbol class__class::get_parent() { return parent;}

Features class__class::get_features() { return features;}

Symbol method_class::get_name() { return name;}

Formals method_class::get_formals() { return formals;}

Symbol method_class::get_type() { return return_type;}

Symbol formal_class::get_type() {return type_decl;}

Symbol formal_class::get_name() { return name;}

Symbol attr_class::get_name() { return name;}

Symbol attr_class::get_type() { return type_decl;}

Symbol branch_class::get_name() { return name;}

Symbol branch_class::get_type() { return type_decl;}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 

ostream& ClassTable::semant_error_(tree_node* t, Class_ c) {
	semant_error(c->get_filename(), t);
	return error_stream;
}


/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    ClassTable *classtable = new ClassTable(classes);

    my_semantic_type_checking(classtable);
    /* some semantic analysis code may go here */

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    } 
}
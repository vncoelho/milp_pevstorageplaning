// OptFrame - Optimization Framework

// Copyright (C) 2009-2015
// http://optframe.sourceforge.net/
//
// This file is part of the OptFrame optimization framework. This framework
// is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License v3 as published by the
// Free Software Foundation.

// This framework is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License v3 for more details.

// You should have received a copy of the GNU Lesser General Public License v3
// along with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

#ifndef CREATEMODULE_HPP_
#define CREATEMODULE_HPP_

#include<string>

#include "../Command.hpp"

namespace optframe
{

template<class R, class ADS = OPTFRAME_DEFAULT_ADS, class DS = OPTFRAME_DEFAULT_DS>
class GeneralCommand: public Command<R, ADS, DS>
{
	string name;
	vector<string> parameters;
	vector<bool> references;
	vector<string> commands;

public:
	GeneralCommand(string _name, vector<string>& _parameters, vector<string>& _commands)
	{
		name = _name;
		parameters = _parameters;
		commands = _commands;

		references = vector<bool>(parameters.size(), false);
		for(unsigned i=0; i<parameters.size(); i++)
			if(parameters[i][0]=='&')
				references[i] = true;
		//cout << "GENERAL MODULE(" << _name << "," << _parameters << "," << _commands << ")" << endl;
	}

private:
	Command<R, ADS, DS>* getCommand(vector<Command<R, ADS, DS>*>& modules, string module)
	{
		for (unsigned int i = 0; i < modules.size(); i++)
			if (modules[i]->canHandle(module, "")) // TODO: fix
				return modules[i];
		return NULL;
	}


	bool exec_command(vector<Command<R, ADS, DS>*>& all_modules, vector<PreprocessFunction<R, ADS, DS>*>& allFunctions, HeuristicFactory<R, ADS, DS>& factory, map<string, string>& dictionary, map< string,vector<string> >& ldictionary, string command)
	{
		Scanner scanner(command);
		string module = scanner.next();
		Command<R, ADS, DS>* m = getCommand(all_modules, module);

		if (m == NULL)
			return false;

		string* rest = m->preprocess(allFunctions, factory, dictionary, ldictionary, scanner.rest());
		if(!rest)
			return false;

		bool b = m->run(all_modules, allFunctions, factory, dictionary, ldictionary, *rest);

		delete rest;

		return b;
	}

public:

	virtual ~GeneralCommand()
	{
	}

	string id()
	{
		return name;
	}

	// default: hashing by id()
	virtual unsigned long hash()
	{
		unsigned long h = aux_hash(name);

		for (unsigned p = 0; p < parameters.size(); p++)
			h += aux_hash(parameters[p]);

		for (unsigned k = 0; k < commands.size(); k++)
			h += aux_hash(commands[k]);

		return h;
	}

	// auxiliar hash function
    unsigned long aux_hash(string s)
	{
    	const char *str = s.c_str();

		unsigned long h = 5381;
		int c;

		while ((c = *str++))
			h = ((h << 5) + h) + c; /*option: hash * 33 + c */

		return h;
	}

	string usage()
	{
		string u = name;
		u.append(" ");
		for (unsigned int i = 0; i < parameters.size(); i++)
		{
			u.append(parameters[i]);
			u.append(" ");
		}

		return u;
	}

	string cleanReference(string ref)
	{
		Scanner scanner(ref);
		scanner.useSeparators("&");
		return scanner.next();
	}

	bool run(vector<Command<R, ADS, DS>*>& all_modules, vector<PreprocessFunction<R, ADS, DS>*>& allFunctions, HeuristicFactory<R, ADS, DS>& factory, map<string, string>& dictionary,  map< string,vector<string> >& ldictionary, string input)
	{
		// CHECK IF EXPLICIT LIST IS PASSED AS PARAMETER (CAN'T DO THIS!!!)
		for(unsigned i=0; i<input.size(); i++)
			if(input.at(i)=='[')
			{
				cout << "dynamic_module '" << id() << "' error: can't have explicit list as parameter! use 'silent_define_list' before calling this!" << endl;
				return false;
			}

		Scanner scanner(input);

		//cout << "general module '" << id() << "' (created) run: '" << input << "'" << endl;

		vector < string > values;

		for (unsigned int i = 0; i < parameters.size(); i++)
			if (!scanner.hasNext())
			{
				cout << "Usage: " << usage() << endl;
				return false;
			}
			else
				values.push_back(scanner.next());

		// -----------------------
		// FINISHED READING VALUES
		// -----------------------

		for (unsigned v = 0; v < values.size(); v++)
		{
			//cout << "CREATED MODULE " << id() << " DEFINING: '" << parameters[v] << "' as '" << values[v] << "'" << endl;
			string setvar = cleanReference(parameters[v]);
			setvar.append(" = ");
			setvar.append(values[v]);
			//cout << "ASSIGN: setvar='" << setvar << "'" << endl;

			if (!Command<R, ADS, DS>::run_module("operator.assign", all_modules, allFunctions, factory, dictionary, ldictionary, setvar))
			{
				cout << "module " << id() << " error: calling operator.assign " << setvar << endl;
				return false;
			}
			/*if (!Command<R, ADS, DS>::defineText(parameters[v], values[v], dictionary))
			{
				cout << "command.create error: failed to define parameter '" << parameters[v] << "' to value '" << values[v] << "'" << endl;
				return false;
			}*/
		}


		if (!Command<R, ADS, DS>::run_module("system.run", all_modules, allFunctions, factory, dictionary, ldictionary, OptFrameList::blockToString(commands)))
		{
			cout << "module " << id() << " error: problem running block of commands!" << endl;
			return false;
		}

		// point reference vars

		for (unsigned v = 0; v < values.size(); v++)
		{
			if (references[v] && Command<R, ADS, DS>::validVariableName(values[v]))
			{
				//cout << "module " << id() << " warning: pointing var '" << values[v] << "' to content of '" << parameters[v] << "'" << endl;
				if (!Command<R, ADS, DS>::pointVars(values[v], cleanReference(parameters[v]), dictionary, ldictionary))
					return false;
			}

			Command<R, ADS, DS>::undefine(cleanReference(parameters[v]), dictionary, ldictionary);
		}

		return true;
	}

	virtual string* preprocess(vector<PreprocessFunction<R, ADS, DS>*>& allFunctions, HeuristicFactory<R, ADS, DS>& hf, const map<string, string>& dictionary, const map<string, vector<string> >& ldictionary, string input)
	{
		return Command<R, ADS, DS>::defaultPreprocess(allFunctions, hf, dictionary, ldictionary, input);
	}
};

template<class R, class ADS = OPTFRAME_DEFAULT_ADS, class DS = OPTFRAME_DEFAULT_DS>
class CommandCreateCommand: public Command<R, ADS, DS>
{
private:

	bool moduleExists(string moduleName, vector<Command<R, ADS, DS>*>& allCommands)
	{
		for(unsigned i=0; i<allCommands.size(); i++)
			if(allCommands[i]->id() == moduleName)
				return true;
		return false;
	}

	Command<R, ADS, DS>* getCommand(vector<Command<R, ADS, DS>*>& modules, string module)
	{
		for (unsigned int i = 0; i < modules.size(); i++)
			if (module == modules[i]->id())
				return modules[i];
		return NULL;
	}

public:

	virtual ~CommandCreateCommand()
	{
	}

	string id()
	{
		return "command.create";
	}

	string usage()
	{
		return "command.create name list_of_$parameters block_of_commands";
	}

	bool run(vector<Command<R, ADS, DS>*>& modules, vector<PreprocessFunction<R, ADS, DS>*>& allFunctions, HeuristicFactory<R, ADS, DS>& factory, map<string, string>& dictionary, map< string,vector<string> >& ldictionary, string input)
	{
		Scanner scanner(input);
		//cout << "create_module run: '" << input << "'" << endl;

		if (!scanner.hasNext())
		{
			cout << "Usage: " << usage() << endl;
			return false;
		}

		string name = scanner.next();

		if(moduleExists(name, modules))
		{
			cout << "command.create command: couldn't create module '" << name << "' because it already exists!" << endl;
			return false;
		}

		if (!scanner.hasNext())
		{
			cout << "Usage: " << usage() << endl;
			return false;
		}

		vector<string>* plist1 = OptFrameList::readList(ldictionary, scanner);
		vector<string>  parameters;
		if(plist1)
		{
			parameters = vector<string>(*plist1);
			delete plist1;
		}
		else
		{
			cout << "command.create error: ill-formed parameter list for new module '" << name << "'!" << endl;
			return false;
		}

		for (unsigned int i = 0; i < parameters.size(); i++)
			if (parameters[i][0] == '$')
			{
				cout << "command.create error: operator $ in variable: '" << parameters[i] << "'" << endl;
				return false;
			}

		if (!scanner.hasNext())
		{
			cout << "Usage: " << usage() << endl;
			return false;
		}

		vector < string > commands;

		vector<string>* plist = OptFrameList::readBlock(scanner);
		if(plist)
		{
			commands = vector<string>(*plist);
			delete plist;
		}
		else
		{
			cout << "command.create error: ill-formed command block for new module '" << name << "'!" << endl;
			return false;
		}

		Command<R, ADS, DS>* m = getCommand(modules, name);

		if (m != NULL)
		{
			cout << "command.create error: module with name '" << name << "' already exists!" << endl;
			return false;
		}
		else
		{
			modules.push_back(new GeneralCommand<R, ADS, DS> (name, parameters, commands));
			//cout << "module '" << name << "' loaded." << endl;
			return true;
		}

	}


	virtual string* preprocess(vector<PreprocessFunction<R, ADS, DS>*>& fs, HeuristicFactory<R, ADS, DS>& hf, const map<string, string>& d, const map<string, vector<string> >& ld, string input)
	{
		int end = -1;
		string body = "";
		for (int i = 0; i < ((int) input.length()); i++)
			if (input[i] == '{')
			{
				end = i;
				break;
			}
			else
				body += input[i];

		if (end < 0)
			return NULL; // no brackets

		string* pbody = Command<R, ADS, DS>::defaultPreprocess(fs, hf, d, ld, body);

		if (!pbody)
			return NULL;

		string ninput = *pbody;

		delete pbody;

		for (int i = end; i < ((int) input.length()); i++)
			ninput += input[i];

		return new string(ninput);
	}
};

}

#endif /* CREATEMODULE_HPP_ */

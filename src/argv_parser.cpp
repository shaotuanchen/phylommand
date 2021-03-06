/********************************************************************
Copyright (C) 2016 Martin Ryberg

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

contact: martin.ryberg@ebc.uu.se
*********************************************************************/

#ifndef ARGV_PARSERPP
#define ARGV_PARSERPP

#include "argv_parser.h"

string argv_parser::pars_string ( const char input [] ) {
    string argv_string;
    bool file_pars(false);
    for (unsigned int i(0); input[i] != '\0'; ++i) {
	if ((input[i] == ':' || input[i] == '=') && !argv_string.compare("file")) {
	    file_pars = true;
	    argv_string.clear();
	}
	else argv_string += input[i];
    }
    if (file_pars) {
	ifstream fileinput;
	fileinput.open(argv_string.c_str());
	argv_string.clear();
	if (fileinput.is_open()) {
	    char in;
	    while (fileinput.get(in)) {
		//fileinput.get(in);// >> in;
		if (in != '\n' && in != '\r') argv_string += in;
	    }
	    if (fileinput.is_open()) fileinput.close();
	}
    }
    return argv_string;
}

void argv_parser::pars_sub_args (const char* argument, const char separator, vector<string>& container) {
    unsigned int j(0);
    bool escape(false);
    container.push_back(string());
    while (argument[j] != '\0') {
	char arg = argument[j];
	if (arg == '\\' && !escape) escape = true;
	else {
	    if (escape) {
		if (arg == 'n') arg = '\n';
		else if (arg == 'r') arg = '\r';
		else if (arg == 't') arg = '\t';
	    }
	    if (arg == separator && !escape) container.push_back(string());
	    else container.back() += arg;
	    escape = false;
	}
	++j;
    }
}

#endif //ARGV_PARSER
#define ARGV_PARSER


// install: libncurses-dev

#include <unistd.h>
#include <ncurses.h>
#include <string>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>

using namespace std;

const string WHITESPACES = " \t";
fstream debug;

// this function trims whitespaces from begin and end of the line
string trim(string str)
{
	size_t start = str.find_first_not_of(WHITESPACES);
	size_t stop = str.find_last_not_of(WHITESPACES);
	return str.substr(start, stop - start + 1);
}

// this function splits a string by any character from pattern
vector<string> split_by_any_of(string str, string pattern)
{
	size_t start = 0, stop = 0;
	vector<string> result;
	do
	{
		start = str.find_first_not_of(pattern, stop);
		if (start != string::npos)
		{
			stop = str.find_first_of(pattern, start);
			if (stop == string::npos)
			{
				result.push_back(trim(str.substr(start)));
			}
			else
			{
				result.push_back(trim(str.substr(start, stop - start)));
			}
		}
	}
	while (start != string::npos && stop != string::npos);
	return result;
}

// Case insensitive string comparison
bool string_equal(string left, string right, bool ignore_case)
{
	bool result = left.length() == right.length();
	if (ignore_case)
	{
		for (size_t idx = 0; result && idx < left.length(); idx++)
		{
			result = tolower(left[idx]) == tolower(right[idx]);
		}
	}
	else
	{
		result = left == right;
	}
	return result;
}

// Implementation of tail command
void command_tail(vector<string> arg, string &data)
{
	debug << "TAIL, data = [" << data << "]" << endl;
	bool lines = true;
	int verbosity = 1, count = 10;
	list<string> files;
	for (size_t idx = 1; idx < arg.size(); idx++)
	{
		if (arg[idx] == "-q" || arg[idx] == "--quiet" || arg[idx] == "--silent")
		{
			verbosity = 0;
		}
		else if (arg[idx] == "-v" || arg[idx] == "--verbose")
		{
			verbosity = 2;
		}
		else if (arg[idx] == "-c")
		{
			lines = false;
			idx++;
			count = atoi(arg[idx].c_str());
		}
		else if (arg[idx].find("--bytes=") == 0)
		{
			lines = false;
			count = atoi(arg[idx].substr(8).c_str());
		}
		else if (arg[idx] == "-n")
		{
			lines = true;
			idx++;
			count = atoi(arg[idx].c_str());
		}
		else if (arg[idx].find("--lines=") == 0)
		{
			lines = true;
			count = atoi(arg[idx].substr(8).c_str());
		}
		else if (arg[idx][0] != '-')
		{
			files.push_back(arg[idx]);
		}
	}
	if (lines)
	{
		if (files.size() == 0)
		{
			vector<string> tail = split_by_any_of(data, "\n");
			data.clear();
			int start = tail.size() - count;
			if (start < 0)
			{
				start = 0;
			}
			for (size_t row = start; row < tail.size(); row++)
			{
				data += tail[row] + "\n";
			}
		}
		else
		{
			string line;
			list<string> tail;
			fstream reader;
			data.clear();
			for (list<string>::iterator fit = files.begin(); fit != files.end(); fit++)
			{
				tail.clear();
				if (verbosity * files.size() > 1)
				{
					data += "==> " + (*fit) + " <==\n";
				}
				reader.open(fit->c_str(), fstream::in);
				while (!reader.eof())
				{
					getline(reader, line);
					tail.push_back(line);
					if (tail.size() > count + 1)
					{
						tail.pop_front();
					}
				}
				reader.close();
				for (list<string>::iterator tit = tail.begin(); tit != tail.end(); tit++)
				{
					data += (*tit) + "\n";
				}
			}
		}
	}
	else
	{
		if (files.size() == 0)
		{
			int start = data.length() - count - 1;
			if (start < 0)
			{
				start = 0;
			}
			data = data.substr(start, data.length() - start);
		}
		else
		{
			fstream reader;
			int start, end;
			char buffer[count + 1];
			for (list<string>::iterator fit = files.begin(); fit != files.end(); fit++)
			{
				if (verbosity * files.size() > 1)
				{
					data += "==> " + (*fit) + " <==\n";
				}
				reader.open(fit->c_str(), fstream::in);
				reader.seekg(0, fstream::end);
				end = reader.tellg();
				start = end - count;
				if (start < 0)
				{
					start = 0;
				}
				reader.seekg(start);
				reader.get(buffer, count, '\0');
				reader.close();
				data += buffer;
				data += "\n";
			}
		}
	}
}

// Implementation of uniq command
void command_uniq(vector<string> arg, string &data)
{
	debug << "UNIQ, data = [" << data << "]" << endl;
	bool ignore_case = false, duplicate = false;
	string input, output;
	for (size_t idx = 1; idx < arg.size(); idx++)
	{
		if (arg[idx] == "-i" || arg[idx] == "--ignore-case")
		{
			ignore_case = true;
		}
		else if (arg[idx] == "-d" || arg[idx] == "--repeated")
		{
			duplicate = true;
		}
		else if (arg[idx] == "-u" || arg[idx] == "--unique")
		{
			duplicate = false;
		}
		else if (arg[idx][0] != '-')
		{
			if (input.length() == 0)
			{
				input = arg[idx];
			}
			else if (output.length() == 0)
			{
				output = arg[idx];
			}
		}
	}
	string current, previous;
	iostream *reader = 0, *writer = 0;
	if (input.length() == 0)
	{
		reader = new stringstream(data);
		data.clear();
	}
	else
	{
		reader = new fstream(input.c_str(), fstream::in);
	}
	if (output.length() == 0)
	{
		writer = new stringstream();
	}
	else
	{
		writer = new fstream(output.c_str(), fstream::out);
	}
	bool first = true;
	while (!reader->eof())
	{
		if (!string_equal(previous, current, ignore_case))
		{
			previous = current;
		}
		getline(*reader, current);
		if (duplicate == (string_equal(previous, current, ignore_case)))
		{
			if (first)
			{
				first = false;
			}
			else
			{
				*writer << previous << endl;
			}
		}
	}
	if (!duplicate && !string_equal(previous, current, ignore_case))
	{
		*writer << current << endl;
	}
	if (output.length() > 0)
	{
		((fstream *) writer)->flush();
		((fstream *) writer)->close();
	}
	else
	{
		data = ((stringstream *) writer)->str();
	}
	delete writer;
	if (input.length() > 0)
	{
		((fstream *) reader)->close();
	}
	delete reader;
}

// Implementation of cd command
void command_cd(vector<string> arg, string &data)
{
	debug << "CD, data = [" << data << "]" << endl;
	if (arg.size() < 2)
	{
		chdir(getenv("HOME"));
	}
	else
	{
		chdir(arg[1].c_str());
	}
}

void system_exec(string cmd, string &data)
{
	debug << "System [" << cmd << "], data = [" << data << "]" << endl;
	int pipe_in[2], pipe_out[2];
	if (pipe(pipe_in) == 0 && pipe(pipe_out) == 0)
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			close(pipe_in[1]);
			dup2(pipe_in[0], 0);
			close(pipe_out[0]);
			dup2(pipe_out[1], 1);
			execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL);
		}
		else if (pid > 0)
		{
			write(pipe_in[1], data.c_str(), data.length());
			close(pipe_in[1]);
			data.clear();
			char buffer[4096];
			int count = read(pipe_out[0], buffer, 4096);
			buffer[count] = '\0';
			close(pipe_out[0]);
			data = buffer;
		}
	}
}

// This function dispatches the commands and produces output
bool execute_command(string cmd, string &data)
{
	debug << "Command [" << cmd << "], data = [" << data << "]" << endl;
	bool result = true;
	vector<string> token = split_by_any_of(cmd, WHITESPACES);
	if (token[0] == "exit")
	{
		result = false;
	}
	else if (token[0] == "tail")
	{
		command_tail(token, data);
	}
	else if (token[0] == "uniq")
	{
		command_uniq(token, data);
	}
	else if (token[0] == "cd")
	{
		command_cd(token, data);
	}
	else
	{
		system_exec(cmd, data);
	}
	return result;
}

// This function takes care of output, pipes and redirections
bool execute_line(string line)
{
	debug << "Line [" << line << "]" << endl;
	bool result = true;
	string data, file;
	vector<string> command = split_by_any_of(line, "|");
	if (command[command.size() - 1].find(">") != string::npos)
	{
		vector<string> tokens = split_by_any_of(command[command.size() - 1], ">");
		if (tokens.size() >= 2)
		{
			command[command.size() - 1] = tokens[0];
			file = tokens[1];
		}
	}
	for (size_t i = 0; result && i < command.size(); i++)
	{
		result = execute_command(command[i], data);
	}
	if (file.length() == 0)
	{
		printw("%s", data.c_str());
	}
	else
	{
		fstream writer(file, fstream::out);
		writer << data;
		writer.flush();
		writer.close();
	}
	return result;
}

// This function takes care of user input and commands history
void command_loop()
{
	bool loop = true;
	int ch, x, y, cmd_idx = 0, hist_idx = 0;
	size_t hist_size = 256;
	string history[hist_size];
	string data;
	debug.open("debug.log", fstream::out);
	printw("Welcome to Teodora Brotea's Shell\n");
	printw("Because this is just a study task, please be nice with it and don't\n");
	printw("enter commands with tens of thousands of lines output.\n\n");
	printw("%s> ", getcwd(0, 0));
	do
	{
		ch = getch();
		switch (ch)
		{
		case KEY_BACKSPACE:
			if (data.length() > 0)
			{
				data = data.substr(0, data.length() - 1);
			}
			break;
		case KEY_UP:
			if (hist_idx > 0)
			{
				hist_idx--;
				data = history[hist_idx % hist_size];
			}
			break;
		case KEY_DOWN:
			if (hist_idx < cmd_idx)
			{
				hist_idx++;
				data = history[hist_idx % hist_size];
			}
			break;
		case '\n':
			printw("\n");
			loop = execute_line(data);
			history[cmd_idx % hist_size] = data;
			cmd_idx++;
			hist_idx = cmd_idx;
			data.clear();
			break;
		default:
			data.push_back(ch);
		}
		getyx(stdscr, y, x);
		move(y, 0);
		clrtoeol();
		printw("%s> %s", getcwd(0, 0), data.c_str());
		refresh();
	}
	while (loop);
	debug.flush();
	debug.close();
}

int main(int argc, char *argv[])
{
	initscr();
	raw();
	keypad(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	noecho();
	command_loop();
	endwin();
	return 0;
}


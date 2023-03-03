/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cjulienn <cjulienn@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/17 08:10:37 by cjulienn          #+#    #+#             */
/*   Updated: 2023/03/03 13:38:29 by cjulienn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"

Parser::Parser(char *config_file) // to test
{
	this->_openFile(config_file);
	this->_processFile();
	this->_serv_num = this->_server_blocks.size();	
	for (int i = 0; i < this->_serv_num; i++)
	{
		this->_servers.push_back(ServConf()); // add a new Config to the linked list to be able to fulfill it
		this->_processBlock(this->_server_blocks[i], i);
	}
}

Parser::~Parser() {}

Parser::Parser(const Parser& original) : _conf_str(original._conf_str), _server_blocks(original._server_blocks),
_serv_num(original._serv_num), _servers(original._servers) {}

Parser&	Parser::operator=(const Parser &original)
{
	if (this != &original)
	{
		this->_conf_str = original._conf_str;
		this->_server_blocks = original._server_blocks;
		this->_serv_num = original._serv_num;
		this->_servers = original._servers;
	}
	return *this;
}

void	Parser::_openFile(char *config_file) // to test
{
	this->_conf_file.open(config_file, std::ios_base::in);
	if (!this->_conf_file.is_open())
	{
		std::cerr << "failure to open the conf file" << std::endl;
		exit(EXIT_FAILURE); // handle error there
	}
}

/* check whether the file is in valid format or not. Includes : 
=> file is not empty
=> there is at least a server part
=> */
void	Parser::_processFile(void) // to test
{
	// case file is empty
	if (this->_conf_file.peek() == std::ifstream::traits_type::eof())
	{
		std::cerr << "conf file is empty !!!" << std::endl;
		exit(EXIT_FAILURE); // case file is empty, handle error
	}
	this->_ifstreamToStr();
	this->_conf_file.close(); // handle possible errors there
	if (!this->_isBlockSyntaxValid()) // maybe not useful
	{
		std::cerr << "syntax error in the number of parenthesis" << std::endl;
		exit(EXIT_FAILURE);
	}
	this->_iterateThroughStr();
}

/* convert from ifstream format to std::string */
void	Parser::_ifstreamToStr(void) // to test
{
	std::stringstream		buffer;

	buffer << this->_conf_file.rdbuf();
	this->_conf_str = buffer.str();
}

/* check till EOF that there is a matching number of { and } and that the first is { */
bool	Parser::_isBlockSyntaxValid(void)
{
	std::size_t		i = 0;
	int				num_open_par = 0;
	int				num_close_par = 0;
	bool			first_bracket = false;

	while (i < this->_conf_str.size())
	{
		if (!first_bracket && this->_conf_str[i] == '{')
			first_bracket = true;
		if (!first_bracket && this->_conf_str[i] == '}')
			return (false);
		if (this->_conf_str[i] == '{')
			num_open_par++;
		if (this->_conf_str[i] == '}')
			num_close_par++;
		i++;
	}
	return (num_open_par > 0 && (num_open_par == num_close_par) ? true : false);
}

/* goal is to extract every server block within a vector and check if server conf is correct */
void	Parser::_iterateThroughStr(void) // to test
{
	std::string			block;
	std::string 		auth_char(" \ns");
	std::size_t			i = 0;
	
	while (i < this->_conf_str.size())
	{
		if (auth_char.find(this->_conf_str[i]) == std::string::npos) // check whether char is not a ' ', '\n' or 's'
		{
			std::cerr << "wrong format detected" << std::endl;
			exit(EXIT_FAILURE); // handle error there
		}
		if (this->_conf_str[i] == 's') // if s is the beginning of the keyword server
		{
			if (!this->_conf_str.compare(i, 6, "server")) // check whether there is only whitespace before '{'
			{
				i += 6;
				while (this->_conf_str[i] != '{')
				{
					if (!std::isspace(this->_conf_str[i]))
					{
						std::cerr << "wrong format detected" << std::endl;
						exit(EXIT_FAILURE); // handle error there
					}
					i++;
				}
				i += this->_splitServerBlock(i) - 1;
			}
			else
			{
				std::cerr << "wrong format detected" << std::endl;
				exit(EXIT_FAILURE); // handle error	
			} 
		}
		i++;
	}
}

int	Parser::_splitServerBlock(int i) // to test
{
	std::size_t		start;
	std::string 	substr;
	int				size;
		
	start = this->_conf_str.find("server", i);
	while ((start < this->_conf_str.size() && this->_conf_str[start + 6] == '_') ||
	(start > 0 && this->_conf_str[start - 1] == '_') ) // case it is a server_name directive	
		start = this->_conf_str.find("server", i + start + 6);
	if (start == std::string::npos) // case server block is the last
		substr = this->_conf_str.substr(i);
	else // case server block is not the last one
		substr = this->_conf_str.substr(i, start - i);
	if (!this->_isServerBlockValid(substr))
	{
		std::cerr << "wrong format detected" << std::endl;
		exit(EXIT_FAILURE); //handle error
	}
	size = substr.size();
	while (std::isspace(substr[0])) // trim whitespace before
		substr = substr.substr(1, substr.size() - 1);
	while (std::isspace(substr[substr.size() - 1])) // trim whitespaces after
		substr = substr.substr(0, substr.size() - 1);
	substr = substr.substr(1, substr.size() - 2); // remove {}
	this->_server_blocks.push_back(substr); /* should push back server block without 'server', whitespace,
	and opening and closing brackets */
	return (size);
}

/* check whether there is the same number of {} in the string */
bool	Parser::_isServerBlockValid(std::string substr) // to test
{
	int				num_open_par = 0;
	int				num_close_par = 0;
	std::size_t		i = 0;

	while (i < substr.size())
	{
		if (substr[i] == '{')
			num_open_par++;
		if (substr[i] == '}')
			num_close_par++;
		i++;
	}	
	return ((num_open_par == num_close_par) ? true : false && num_open_par > 0);
}

/* if directive refers to a location, triggers the specialized function. Otherwise, 
check if the directive format is valid (have at least two arguments, is bounded by ';').
Then, trigger relevant processing functions. Returns the length of chars used by the directive */
int	Parser::_dispatchInstructionProcessing(int type, std::string directive, int serv_idx, bool is_loc) // to test
{
	std::size_t		args_num;
	std::size_t		dir_len;
		
	if (type == LOCATION)
		return (this->_processLocationBlock(directive, serv_idx));
	if (!this->_isDirectiveValid(directive))
	{
		std::cerr << "invalid directive format" << std::endl;
		exit(EXIT_FAILURE);
	}
	args_num = this->_cutArgs(directive, ';').size();
	dir_len = directive.substr(0, directive.find(";")).size() + 1;
	
	switch (type)
	{
		case (LISTEN):
			this->_processListenDirective(directive, serv_idx, args_num, is_loc);
			break ;
		case (SERV_NAME):
			this->_processServerNameDirective(directive, serv_idx, args_num, is_loc);
			break ;
		case (ERR_PAGE):
			this->_processErrorPageDirective(directive, serv_idx, args_num, is_loc);
			break ;
		case (CLIENT_BODY_SIZE):
			this->_processBodySizeDirective(directive, serv_idx, args_num, is_loc);
			break ;
		case (ALLOW_HTTP_METHOD):
			this->_processAllowDirective(directive, serv_idx, args_num, is_loc);
			break ;
		case (REWRITE):
			this->_processRewriteDirective(directive, serv_idx, args_num, is_loc);
			break ;
		case (ROOT):
			this->_processRootDirective(directive, serv_idx, args_num, is_loc);
			break ;
		case (AUTOINDEX):
			this->_processAutoindexDirective(directive, serv_idx, args_num, is_loc);
			break ;
		case (INDEX):
			this->_processIndexDirective(directive, serv_idx, args_num, is_loc);
			break ;
		case (CGI):
			this->_processCgiDirective(directive, serv_idx, args_num, is_loc);
			break ;
		default:
			std::cerr << "instruction unknown" << std::endl;
			exit(EXIT_FAILURE);
			break ;
	}
	return (dir_len);
}

int	Parser::_rtnInstructionType(std::string directive) // to test
{	
	if (!directive.compare("location"))
		return (LOCATION);
	else if (!directive.compare("listen"))
		return (LISTEN);
	else if (!directive.compare("server_name"))
		return (SERV_NAME);
	else if (!directive.compare("error_page"))
		return (ERR_PAGE);
	else if (!directive.compare("client_max_body_size"))
		return (CLIENT_BODY_SIZE);
	else if (!directive.compare("allow"))
		return (ALLOW_HTTP_METHOD);
	else if (!directive.compare("rewrite"))
		return (REWRITE);
	else if (!directive.compare("root"))
		return (ROOT);
	else if (!directive.compare("autoindex"))
		return (AUTOINDEX);
	else if (!directive.compare("index"))
		return (INDEX);
	else if (!directive.compare("cgi"))
		return (CGI);
	else
		return (BAD_INSTR);
}

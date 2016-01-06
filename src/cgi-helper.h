#ifndef CGI_HELPER_H
#define CGI_HELPER_H

#include <map>
#include <string>


std::string cgi_get_to_text();

std::string cgi_post_to_text();

std::map< std::string, std::string > decode_cgi_to_plain(const std::string& raw);

#endif

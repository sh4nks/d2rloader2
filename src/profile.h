#ifndef PROFILE_H
#define PROFILE_H

#include <string>

struct Region {
    std::string name;
    std::string value;
};

enum AuthMethod {
    Token, Password
};

struct Account {
    std::string name;
    std::string email;
    AuthMethod auth_method;
    Region region;
    std::string token;
    std::string token_protected;
    std::string password;
    std::string game_params;
    std::string game_settings;
};

#endif

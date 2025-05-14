#ifndef UTILS_H_INC
#define UTILS_H_INC

#include <string>

std::string sha256(const std::string &input);
std::string hashCommitment(int value, const std::string& nonce);

#endif

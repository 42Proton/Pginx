#include <string>
#include <iostream>
#include <sstream>
#include <hash_map>

u_int32_t parseIpAddr(std::string& ip)
{
  u_int32_t res = 0;
  int i = 0;
  int octet;
  std::stringstream ss(ip);
  std::string tok;
  while (std::getline(ss, tok, '.'))
  {
    octet = atoi(tok.c_str());
    
  }
}

int compareIfIpMatch(std::string& ip, u_char ipRange, std::string& requestIp)
{
  if (ipRange > 32)
    return (-1);
  if (ipRange == 0)
    return (1);
  unsigned int octets = 0;
  
}

int main()
{
  std::string ip = "192.0.0.0";
  char ipRange = 8;
  std::string requestIp = "192.168.100.2";
}
#include <docopt/docopt.h>
#include <string.h>
#include <iostream>
#include "eeprom.h"

bool get_ihexfile(std::map<std::string, docopt::value> args, std::string &file)
{
  bool result = false; 
  auto p = args.find("--ihex")->second;

  if (p.isString())
  {
     file = p.asString();
     result = true;
  }
  
  return result;
}

int main(int argc, char *argv[])
{
        std::string ihex_filename = "";

	static const char USAGE[] =
	R"(eeprom
           Program Microchip i2c EEPROM

	    Usage:
	      eeprom  --ihex=<ihexfilename> 
	      eeprom (-h | --help)
	      eeprom --version
	    Options:
	      -h --help                    Show this screen.
	      --version                    Show version.
              --ihex=<ihexfilename>        iHex Filename
	)";

   std::map<std::string, docopt::value> args
        = docopt::docopt(USAGE,
                         { argv + 1, argv + argc },
                         true,                // show help if requested
                         "eeprom programmer 0.1");  // version string

  if (!get_ihexfile(args, ihex_filename))
  {
     std::cout << "ihex_filename: invalid file defined" << std::endl;
  }

}


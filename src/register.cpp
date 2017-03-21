/**
 * @brief Implementation of regiser class.
 */

#include "register.hpp"

std::ostream& operator<<(std::ostream& stream, const Register16& e)
{
	stream << std::dec << e.get() << " (0x" << std::hex << e.get() << ")" << std::dec;
	return stream;
}

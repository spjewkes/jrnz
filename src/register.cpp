/**
 * @brief Implementation of regiser class.
 */

#include "register.hpp"

std::ostream& operator<<(std::ostream& stream, const Register16& e)
{
	stream << std::dec << e.get() << " (0x" << std::hex << e.get() << ")" << std::dec;
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const RegisterAF& e)
{
	stream << "A: " << std::dec << static_cast<int>(e.hi()) << " (0x" << std::hex << static_cast<unsigned int>(e.hi()) << ")" << std::dec;
	stream << "    Flags:";
	stream << " C:" << e.flag(RegisterAF::Flags::Carry);
	stream << " N:" << e.flag(RegisterAF::Flags::AddSubtract);
	stream << " P:" << e.flag(RegisterAF::Flags::ParityOverflow);
	stream << " H:" << e.flag(RegisterAF::Flags::HalfCarry);
	stream << " Z:" << e.flag(RegisterAF::Flags::Zero);
	stream << " S:" << e.flag(RegisterAF::Flags::Sign);
	return stream;
}

/**
 * @brief Implementation of regiser class.
 */

#include "register.hpp"

std::ostream& operator<<(std::ostream& stream, const Register16& e)
{
	stream << "0x" << std::hex << std::setfill('0') << std::setw(4) << e.get() << std::dec << " (" << e.get() << ")";
	return stream;
}

std::stringstream RegisterAF::dump_flags()
{
	std::stringstream str;
	str << " S:" << flag(RegisterAF::Flags::Sign);
	str << " Z:" << flag(RegisterAF::Flags::Zero);
	str << " H:" << flag(RegisterAF::Flags::HalfCarry);
	str << " P:" << flag(RegisterAF::Flags::ParityOverflow);
	str << " N:" << flag(RegisterAF::Flags::AddSubtract);
	str << " C:" << flag(RegisterAF::Flags::Carry);
	return str;
}

std::ostream& operator<<(std::ostream& stream, const RegisterAF& e)
{
	stream << "0x" << std::hex << std::setfill('0') << std::setw(4) << e.get() << std::dec << " (" << e.get() << ")";
	return stream;
}

/**
 * @brief Implementation of regiser class.
 */

#include "register.hpp"

std::ostream& operator<<(std::ostream& stream, const Register16& e)
{
	stream << "0x" << std::hex << std::setfill('0') << std::setw(4) << e.get() << std::dec << " (" << e.get() << ")";
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const RegisterAF& e)
{
	stream << "A: 0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<uint32_t>(e.hi()) << std::dec << " (" << static_cast<uint32_t>(e.hi()) << ") ";
	stream << "F: 0x" << std::hex << std::setfill('0') << std::setw(4) << static_cast<uint32_t>(e.lo()) << std::dec << " (" << static_cast<uint32_t>(e.lo()) << ") ";
	stream << "    Flags:";
	stream << " C:" << e.flag(RegisterAF::Flags::Carry);
	stream << " N:" << e.flag(RegisterAF::Flags::AddSubtract);
	stream << " P:" << e.flag(RegisterAF::Flags::ParityOverflow);
	stream << " H:" << e.flag(RegisterAF::Flags::HalfCarry);
	stream << " Z:" << e.flag(RegisterAF::Flags::Zero);
	stream << " S:" << e.flag(RegisterAF::Flags::Sign);
	return stream;
}

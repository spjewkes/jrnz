/**
 * @brief Class managing ports of the system.
 */

#ifndef __PORTS_HPP__
#define __PORTS_HPP__

#include <vector>

/**
 * @brief Defines the ports.
 */
class Ports
{
public:
	Ports()
		{
			ports.resize(256);
		}

	unsigned char read(size_t pos) const { return ports[pos]; }
	void write(size_t pos, unsigned char v) { ports[pos] = v; }

	StorageElement element(size_t pos) { return StorageElement(&ports[pos], 1); }

private:
	std::vector<unsigned char> ports;
};

#endif // __PORTS_HPP__

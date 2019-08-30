/**
 * @brief Class managing ports of the system.
 */

#ifndef __PORTS_HPP__
#define __PORTS_HPP__

#include <vector>
#include <cstdint>

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

	uint8_t read(size_t pos) const { return ports[pos]; }
	void write(size_t pos, uint8_t v) { ports[pos] = v; }

	StorageElement element(size_t pos) { return StorageElement(&ports[pos], 1); }

private:
	std::vector<uint8_t> ports;
};

#endif // __PORTS_HPP__

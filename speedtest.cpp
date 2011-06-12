#include <vector>
#include <random>
#include <algorithm>

#include "search/Search.h"
#include "types.h"

class RandomMemBlock
{
	RandomMemBlock(const RandomMemBlock&);
public:
	RandomMemBlock(size_t size) {
		std::uniform_int_distribution<int> byteDist(0, 255);
		std::mt19937 mersenneTwister;
		
		mem.resize(size);
		
		//auto generator = std::bind(byteDist, mersenneTwister);
		std::generate(mem.begin(), mem.end(), []()->char{ return byteDist(mersenneTwister); } );
	}
	
	~RandomMemBlock() {
		delete [] mem;
	}
	
	POD::ConstBuffer getBuffer() {
		return POD::ConstBuffer(mem, size);
	}
private:
	std::vector<char> mem;
};

int main(int argc, char **argv)
{
	RandomMemBlock rnd(0x400 * 0x400 * 20);
	auto t = rnd.getBuffer();
	
	
	
	return 0;
}
